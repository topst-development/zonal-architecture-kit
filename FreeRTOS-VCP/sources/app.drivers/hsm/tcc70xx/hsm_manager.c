// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : hsm_manager.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM Mannger
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )

#if 0
#define NDEBUG
#endif
#define TLOG_LEVEL (TLOG_DEBUG)
#include "hsm_log.h"

// temp #include <app_cfg.h>
#include <sal_internal.h>
#include <string.h>
#include <stdlib.h>
//#include <fwug.h>
#include <debug.h>

#include "hsm_manager.h"
#include "mbox.h"
#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
#include "fwupdate.h"
#endif
#include "snor_mio.h"
#include "eflash.h"
#include "pmu_reg.h"

/*
***************************************************************************************************
*                                            DEFINITIONS
***************************************************************************************************
*/

#define HSM_REQUIRED_VER_X (0)
#define HSM_REQUIRED_VER_Y (1)
#define HSM_REQUIRED_VER_Z (1)

uint8 hsm_dev;

int32 HSM_Init(void)
{
    hsmVer ver = {0};

    if (hsm_dev == 0U) {
        hsm_dev = MBOX_INIT;
        if (hsm_dev == 0U) {
            ELOG("Error dev_open\n");
            return HSM_ERR;
        }

        HSM_GetVersion(&ver);
        if ((ver.x != HSM_REQUIRED_VER_X) || (ver.y != HSM_REQUIRED_VER_Y) || ver.z < HSM_REQUIRED_VER_Z) {
            ELOG(
                "HSM FW verison(%d.%d.%d) must be higher than equal to %d.%d.%d\n", ver.x, ver.y, ver.z,
                HSM_REQUIRED_VER_X, HSM_REQUIRED_VER_Y, HSM_REQUIRED_VER_Z);
            return HSM_ERR;
        }
    }

    return HSM_OK;
}

static int32 HSM_VerifyVCPCert(vcpCert *cert)
{
    hsmSha sha = {0};
    hsmCerti root = {0};
    hsmCerti target = {0};
    uint8 tempValue;
    uint32 i;
    int32 ret = HSM_ERR;
    uint8 hashFromOTP[HSM_SHA2_256_SIZE];

    /* Get L1's Hash from OTP */
    memset(hashFromOTP, 0xFF, HSM_SHA2_256_SIZE);
    FLASH_ReadLDT(L1_HASH_ADDR, (HSM_SHA2_256_SIZE / 4), (uint32 *)hashFromOTP);

    for (i = 1; i < HSM_SHA2_256_SIZE; i++) {
        tempValue = (hashFromOTP[i - 1] & hashFromOTP[i]);
        if (tempValue != 0xFF) {
            break;
        }
    }

    if (i < HSM_SHA2_256_SIZE) {
        /* calculate L1's hash */
        sha.objId = HSM_OID_SHA2_256; // Only support SHA256
        sha.opMode = OPMODE_SINGLECALL;
        sha.srcSize = CRT_SIZE;
        sha.src = cert->L1;
        sha.digSize = HSM_SHA2_256_SIZE;
        ret = HSM_GenHash(&sha);
        if (ret != HSM_OK) {
            ELOG("HSM_GenHash fail(%d)\n", ret);
            return HSM_ERR;
        }
        for (i = 0; i < HSM_SHA2_256_SIZE; i++) {
            if (hashFromOTP[i] != sha.dig[i]) {
                ELOG("L1 Hash check fail(%d)\n", ret);
                return HSM_ERR;
            }
        }
        DLOG("L1 Hash check success(%d)\n", ret);
    }

    /* Verify L2 */
    root.crt = cert->L1;
    root.crtSize = CRT_SIZE;
    target.crt = cert->L2;
    target.crtSize = CRT_SIZE;
    ret = HSM_CertiVerify(&root, &target);
    if (ret != HSM_OK) {
        ELOG("HSM_CertiVerify fail(%d)\n", ret);
        return ret;
    }
    DLOG("L2 certi Verify success(%d)\n", ret);

    /* Verify MCU_L3 */
    root.crt = cert->L2;
    root.crtSize = CRT_SIZE;
    target.crt = cert->MCU_L3;
    target.crtSize = CRT_SIZE;
    ret = HSM_CertiVerify(&root, &target);
    if (ret != HSM_OK) {
        ELOG("HSM_CertiVerify fail(%d)\n", ret);
        return ret;
    }
    DLOG("MCU_L3 certi Verify success(%d)\n", ret);

    /* Verify HSM_L3 */
    root.crt = cert->L2;
    root.crtSize = CRT_SIZE;
    target.crt = cert->HSM_L3;
    target.crtSize = CRT_SIZE;
    ret = HSM_CertiVerify(&root, &target);
    if (ret != HSM_OK) {
        ELOG("HSM_CertiVerify fail(%d)\n", ret);
        return ret;
    }
    DLOG("HSM_L3 certi Verify success(%d)\n", ret);

    return ret;
}

static int32 HSM_ParseVCPCert(vcpCert *cert, uint8 *hsmPubKey, uint8 *mcuPubKey)
{
    hsmCerti target = {0};
    int32 ret = HSM_ERR;

    /* Parse MCU_L3 */
    target.crt = cert->MCU_L3;
    target.crtSize = CRT_SIZE;
    target.pubSize = HSM_ECDH_P256_KEY_SIZE * 2;
    target.pubKey = mcuPubKey;
    ret = HSM_CertiParse(&target);
    if (ret != HSM_OK) {
        ELOG("MCU_CertiParse fail(%d)\n", ret);
        return ret;
    }

    /* Parse HSM_L3 */
    target.crt = cert->HSM_L3;
    target.crtSize = CRT_SIZE;
    target.pubSize = HSM_ECDH_P256_KEY_SIZE * 2;
    target.pubKey = hsmPubKey;
	ret = HSM_CertiParse(&target);
    if (ret != HSM_OK) {
        ELOG("MCU_CertiParse fail(%d)\n", ret);
        return ret;
    }

    return ret;
}

static int32 HSM_VerifyHSMFW(uint32 uihsmImgAddr, uint32 uihsmImgSize, uint8 *hsmPubKey)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = (HSM_MBOX_CID_R5 | HSM_MBOX_BSID_BL1); // multi channel number
    cmd[1] = HSM_REQ_VERIFY_HSM;                     // cmd
    cmd[2] = uihsmImgAddr;
    cmd[3] = uihsmImgSize;
    cmd[4] = (uint32)hsmPubKey;

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, 0);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)cmd[2];
    return ret;
}

static int32 HSM_VerifyMCUFW(
	uint32 uiHeaderAddr, uint32 uiHeaderSize, uint32 uiImgAddr, uint32 uiImgSize, uint8 *mcuPubKey)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = (HSM_MBOX_CID_R5 | HSM_MBOX_BSID_BL1); // multi channel number
    cmd[1] = HSM_REQ_VERIFY_MCU;                     // cmd
    cmd[2] = uiHeaderAddr;
    cmd[3] = uiHeaderSize;
    cmd[4] = uiImgAddr;
    cmd[5] = uiImgSize;
    cmd[6] = (uint32)mcuPubKey;

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, 0);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)cmd[2];
    return ret;
}

int32 HSM_UpdateVerify(uint32 storage)
{
#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
    uint32 mcuHeaderAddr;
    uint32 mcuBodySizeAddr;
    uint32 mcuBodyAddr;
    uint32 mcuBodySize;
    uint32 hsmFWAddr;
    uint32 certAddr;

    uint32 alignSize;
    int32 ret = HSM_ERR;
    vcpCert *cert;
    uint8 mcuPubKey[HSM_ECDH_P256_KEY_SIZE * 2] = {0};
    uint8 hsmPubKey[HSM_ECDH_P256_KEY_SIZE * 2] = {0};
	uint32 remap = (*(volatile uint32 *)(PMU_ADDR_GLB_CONFIG)) >> 16;

    /* HSM Init */
    if (HSM_Init() != HSM_OK) {
        mcu_printf("%s, HSM Init failed \n", __func__ );
        return HSM_ERR;
    }

    switch (storage)
    {
        case FWUD_STOR_TYPE_SNOR:
            mcuBodySizeAddr = SNOR_BANK_ADDR + FWUD_MCU_FW_BODY_SIZE_OFFSET;
            mcuHeaderAddr = SNOR_BANK_ADDR + FWUD_MCU_FW_HEADER_OFFSET + FWUD_MCU_FW_HEADER_DUMMY_SIZE;
            mcuBodyAddr = SNOR_BANK_ADDR + FWUD_MCU_FW_START_OFFSET;
            hsmFWAddr = SNOR_BANK_ADDR + FWUD_HSM_FW_START_ADDR;
            SNOR_MIO_AutoRun(SFMC0, 1u, 0u);
            break;
        case FWUD_STOR_TYPE_EFLASH:
            mcuBodySizeAddr = eFLASH_BANK0_ADDR + FWUD_MCU_FW_BODY_SIZE_OFFSET;
            mcuHeaderAddr = eFLASH_BANK0_ADDR + FWUD_MCU_FW_HEADER_OFFSET + FWUD_MCU_FW_HEADER_DUMMY_SIZE;
            mcuBodyAddr = eFLASH_BANK0_ADDR + FWUD_MCU_FW_START_OFFSET;
            hsmFWAddr = eFLASH_BANK0_ADDR + FWUD_HSM_FW_START_ADDR;
            break;
        case FWUD_STOR_TYPE_REMAP:
			if (remap == 0) {
				mcuBodySizeAddr = eFLASH_BANK0_ADDR + FWUD_MCU_FW_BODY_SIZE_OFFSET;
				mcuHeaderAddr = eFLASH_BANK0_ADDR + FWUD_MCU_FW_HEADER_OFFSET + FWUD_MCU_FW_HEADER_DUMMY_SIZE;
				mcuBodyAddr = eFLASH_BANK0_ADDR + FWUD_MCU_FW_START_OFFSET;
			}
			else {
				mcuBodySizeAddr = SNOR_BANK_ADDR + FWUD_MCU_FW_BODY_SIZE_OFFSET;
				mcuHeaderAddr = SNOR_BANK_ADDR + FWUD_MCU_FW_HEADER_OFFSET + FWUD_MCU_FW_HEADER_DUMMY_SIZE;
				mcuBodyAddr = SNOR_BANK_ADDR + FWUD_MCU_FW_START_OFFSET;
			}
            hsmFWAddr = REMAP_MEMORY_ADDR + FWUD_HSM_FW_START_ADDR;
            break;
        default:
            mcu_printf("%s, Unknown storage type \n", __func__ );
            return HSM_ERR;
            break;
    }
    SAL_MemCopy(&mcuBodySize, (uint32 *)mcuBodySizeAddr, sizeof(uint32));

    alignSize = mcuBodySize % FWUD_ADDR_ALIGN_SIZE;

    if(alignSize != 0)
    {
        alignSize = FWUD_ADDR_ALIGN_SIZE - alignSize;
    }

    certAddr = mcuBodyAddr + mcuBodySize + alignSize;

    cert = (vcpCert *)certAddr;

    ret = HSM_VerifyVCPCert(cert);
    if (ret == HSM_OK) {
        ret = HSM_ParseVCPCert(cert, hsmPubKey, mcuPubKey);
        if (ret != HSM_OK) {
            ELOG("HSM_ParseVCPCert fail(%d)\n", ret);
            return ret;
        }
    }
    else {
        ELOG("HSM_VerifyVCPCert fail(%d)\n", ret);
        return ret;
    }

    ret = HSM_VerifyHSMFW(hsmFWAddr, FWUD_HSM_FW_SIZE, hsmPubKey);
    if (ret != HSM_OK) {
        ELOG("HSM F/W verify fail(%d)\n", ret);
        return ret;
    }
    DLOG("HSM F/W Verify success(0x%x)\n", ret);

    ret = HSM_VerifyMCUFW(mcuHeaderAddr, FWUD_MCU_FW_HEADER_SIZE, mcuBodyAddr, mcuBodySize, mcuPubKey);
    if (ret != HSM_OK) {
        ELOG("MCU F/W verify fail(%d)\n", ret);
        return ret;
    }
    DLOG("MCU F/W Verify success(0x%x)\n", ret);

    return ret;
#else
    return 0;
#endif
}

int32 HSM_VerifyFw(
    uint32 uiHeaderAddr, uint32 uiHeaderSize, uint32 uiImgAddr, uint32 uiImgSize, uint32 uiImgId)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = (HSM_MBOX_CID_R5 | HSM_MBOX_BSID_BL1); // multi channel number
    cmd[1] = HSM_REQ_SB_VERIFY;                     // cmd
    cmd[2] = uiHeaderAddr;
    cmd[3] = uiHeaderSize;
    cmd[4] = uiImgAddr;
    cmd[5] = uiImgSize;
    cmd[6] = uiImgId;

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, 0);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)cmd[2];
    return ret;
}

int32 HSM_SetKey(hsmKey *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                                 // multi channel number
    cmd[1] = HSM_REQ_SET_KEY_FROM_NVM | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                                 // dma_addr
    cmd[3] = HSM_NONE_DMA;                                      // trans_type
    cmd[5] = 0;                                                 // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                          // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                           // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->dfAddr;
    idx++;
    data[idx] = param->keySize;
    idx++;
    data[idx] = param->keyIdx;
    idx++;

    cmd[4] = (idx * sizeof(uint32)); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    return ret;
}

int32 HSM_RunAes(hsmAes *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                        // multi channel number
    cmd[1] = HSM_REQ_RUN_AES | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                        // dma_addr
    cmd[3] = HSM_NONE_DMA;                             // trans_type
    cmd[5] = 0;                                        // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                 // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                  // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->opMode;
    idx++;
    data[idx] = param->keySize;
    idx++;
    if ((param->aesKey != NULL) && (param->keySize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->aesKey, param->keySize);
        idx += (param->keySize + 3U) / sizeof(uint32);
    }
    data[idx] = param->ivSize;
    idx++;
    if ((param->aesIv != NULL) && (param->ivSize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->aesIv, param->ivSize);
        idx += (param->ivSize + 3U) / sizeof(uint32);
    }

    data[idx] = param->aadSize;
    idx++;
    if (param->aadSize > 0U) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->aad, param->aadSize);
        idx += (param->aadSize + 3U) / sizeof(uint32);
    }

    data[idx] = param->srcSize;
    idx++;

    data[idx] = (uint32)param->src;
    idx++;

    data[idx] = param->dstSize;
    idx++;

    data[idx] = (uint32)param->dst;
    idx++;

    data[idx] = param->tagSize;
    idx++;
    if ((param->tagSize > 0U) && ((param->objId & HSM_AES_DECRYPT) == HSM_AES_DECRYPT)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->tag, param->tagSize);
        idx += (param->tagSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->tagSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }
    idx++;

    if ((data[idx] == param->tagSize) && ((param->objId & HSM_AES_ENCRYPT) == HSM_AES_ENCRYPT)
        && (param->tagSize != 0u)) {
        idx++;
        SAL_MemCopy((void *)param->tag, (const void *)&data[idx], param->tagSize);
    }

    return HSM_OK;
}

int32 HSM_RunAesByKt(hsmAes *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                              // multi channel number
    cmd[1] = HSM_REQ_RUN_AES_BY_KT | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                                   // trans_type
    cmd[5] = 0;                                              // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                       // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                        // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->opMode;
    idx++;
    data[idx] = param->keyIdx;
    idx++;

    data[idx] = param->ivSize;
    idx++;
    if ((param->aesIv != NULL) && (param->ivSize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->aesIv, param->ivSize);
        idx += (param->ivSize + 3U) / sizeof(uint32);
    }
    data[idx] = param->aadSize;
    idx++;
    if (param->aadSize > 0U) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->aad, param->aadSize);
        idx += (param->aadSize + 3U) / sizeof(uint32);
    }
    data[idx] = param->srcSize;
    idx++;

    data[idx] = (uint32)param->src;
    idx++;

    data[idx] = param->dstSize;
    idx++;

    data[idx] = (uint32)param->dst;
    idx++;

    data[idx] = param->tagSize;
    idx++;
    if ((param->tagSize > 0U) && ((param->objId & HSM_AES_DECRYPT) == HSM_AES_DECRYPT)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->tag, param->tagSize);
        idx += (param->tagSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->dstSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }
    idx++;

    if ((data[idx] == param->tagSize) && ((param->objId & HSM_AES_ENCRYPT) == HSM_AES_ENCRYPT)
        && (param->tagSize != 0u)) {
        idx++;
        SAL_MemCopy((void *)param->tag, (const void *)&data[idx], param->tagSize);
    }

    return HSM_OK;
}

int32 HSM_GenMac(hsmMac *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                         // multi channel number
    cmd[1] = param->req | HSM_MBOX_LOCATION_DATA;       // cmd
    cmd[2] = 0;                                         // dma_addr
    cmd[3] = HSM_NONE_DMA;                              // trans_type
    cmd[5] = 0;                                         // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                  // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                   // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->opMode;
    idx++;
    data[idx] = param->keySize;
    idx++;
    if ((param->macKey != NULL) && (param->keySize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->macKey, param->keySize);
        idx += (param->keySize + 3U) / sizeof(uint32);
    }

    if (param->req == REQ_HSM_GEN_CMAC) {
        data[idx] = param->ivSize;
        idx++;
        if ((param->macIv != NULL) && (param->ivSize > 0U)) {
            SAL_MemCopy((void *)&data[idx], (const void *)param->macIv, param->ivSize);
            idx += (param->ivSize + 3U) / sizeof(uint32);
        }
    }

    data[idx] = param->srcSize;
    idx++;

    if (param->srcSize != 0u) {
        data[idx] = (uint32)param->src;
        idx++;
    }

    data[idx] = param->macSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->macSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(%d)\n", data[0], idx);
        return HSM_ERR;
    }

    if (data[1] == param->macSize) {
        SAL_MemCopy((void *)param->mac, (const void *)&data[2], param->macSize);
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_GenMacByKt(hsmMac *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                               // multi channel number
    cmd[1] = param->req | HSM_MBOX_LOCATION_DATA;             // cmd
    cmd[2] = 0;                                               // dma_addr
    cmd[3] = HSM_NONE_DMA;                                    // trans_type
    cmd[5] = 0;                                               // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                        // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                         // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->opMode;
    idx++;
    data[idx] = param->keyIdx;
    idx++;

    if (param->req == REQ_HSM_GEN_CMAC_BY_KT) {
        data[idx] = param->ivSize;
        idx++;
        if ((param->macIv != NULL) && (param->ivSize > 0U)) {
            SAL_MemCopy((void *)&data[idx], (const void *)param->macIv, param->ivSize);
            idx += (param->ivSize + 3U) / sizeof(uint32);
        }
    }

    data[idx] = param->srcSize;
    idx++;

    if (param->srcSize != 0u) {
        data[idx] = (uint32)param->src;
        idx++;
    }

    data[idx] = param->macSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->macSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(%d)\n", data[0], idx);
        return HSM_ERR;
    }

    if (data[1] == param->macSize) {
        SAL_MemCopy((void *)param->mac, (const void *)&data[2], param->macSize);
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_Verifymac(hsmMac *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = param->req | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;       // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->opMode;
    idx++;
    data[idx] = param->keySize;
    idx++;
    if ((param->macKey != NULL) && (param->keySize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->macKey, param->keySize);
        idx += (param->keySize + 3U) / sizeof(uint32);
    }

    if (param->req == REQ_HSM_VERIFY_CMAC) {
        data[idx] = param->ivSize;
        idx++;
        if ((param->macIv != NULL) && (param->ivSize > 0U)) {
            SAL_MemCopy((void *)&data[idx], (const void *)param->macIv, param->ivSize);
            idx += (param->ivSize + 3U) / sizeof(uint32);
        }
    }
    data[idx] = param->srcSize;
    idx++;

    if (param->srcSize != 0u) {
        data[idx] = (uint32)param->src;
        idx++;
    }

    data[idx] = param->macSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)param->mac, param->macSize);
    idx += (param->macSize + 3U) / sizeof(uint32);

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->macSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(%d)\n", data[0], idx);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_GenHash(hsmSha *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                        // multi channel number
    cmd[1] = HSM_REQ_GEN_SHA | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                        // dma_addr
    cmd[3] = HSM_NONE_DMA;                             // trans_type
    cmd[5] = 0;                                        // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                 // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                  // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;

    data[idx] = param->opMode;
    idx++;

    data[idx] = param->srcSize;
    idx++;

    if (param->srcSize != 0u) {
        data[idx] = (uint32)param->src;
        idx++;
    }

    data[idx] = param->digSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->digSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(%d) mode(%d)\n", data[0], idx, (param->opMode));
        return HSM_ERR;
    }
    if ((param->opMode & OPMODE_FINISH) == OPMODE_FINISH) {
        if (data[1] == param->digSize) {
            SAL_MemCopy((void *)param->dig, (const void *)&data[2], param->digSize);
        } else {
            ELOG("wrong uiDstSize(%d)\n", data[1]);
            return HSM_ERR;
        }
    }

    return HSM_OK;
}

int32 HSM_RunRsassa(hsmAsym *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = param->req | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_RSASSA;        // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->opMode;
    idx++;
    data[idx] = param->digSize;
    idx++;
    if ((param->dig != NULL) && (param->digSize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->dig, param->digSize);
        idx += (param->digSize + 3U) / sizeof(uint32);
    }

    data[idx] = param->modSize;
    idx++;
    data[idx] = (uint32)param->modulus;
    idx++;

    if ((param->req == REQ_HSM_RUN_RSASSA_PKCS_SIGN) || (param->req == REQ_HSM_RUN_RSASSA_PSS_SIGN)) {
        data[idx] = param->priSize;
        idx++;
        data[idx] = (uint32)param->priKey;
        idx++;
    } else { // VERIFY
        data[idx] = param->pubSize;
        idx++;
        data[idx] = (uint32)param->pubKey;
        idx++;
    }

    data[idx] = param->sigSize;
    idx++;
    data[idx] = (uint32)param->sig;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_RSASSA, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x) idx(%x)\n", data[0], idx);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_RunEcdsa(hsmAsym *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                   // multi channel number
    cmd[1] = param->req | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                   // dma_addr
    cmd[3] = HSM_NONE_DMA;                        // trans_type
    cmd[5] = 0;                                   // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;            // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;             // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->opMode;
    idx++;

    if (param->req == HSM_REQ_RUN_ECDSA_SIGN) {
        data[idx] = param->priSize;
        idx++;
        if ((param->priKey != NULL) && (param->priSize > 0U)) {
            SAL_MemCopy((void *)&data[idx], (const void *)param->priKey, param->priSize);
            idx += (param->priSize + 3U) / sizeof(uint32);
        }
    } else { // HSM_REQ_RUN_ECDSA_VERIFY
        data[idx] = param->pubSize;
        idx++;
        if ((param->pubKey != NULL) && (param->pubSize > 0U)) {
            SAL_MemCopy((void *)&data[idx], (const void *)param->pubKey, param->pubSize);
            idx += (param->pubSize + 3U) / sizeof(uint32);
        }
    }

    data[idx] = param->digSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)param->dig, param->digSize);
    idx += (param->digSize + 3U) / sizeof(uint32);

    data[idx] = param->sigSize;
    idx++;
    if (param->req == HSM_REQ_RUN_ECDSA_VERIFY) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->sig, param->sigSize);
        idx += (param->sigSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (param->req == HSM_REQ_RUN_ECDSA_VERIFY) {
        dataSize = sizeof(uint32);
    } else {
        dataSize = param->sigSize + (sizeof(uint32) * 2U);
    }
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x) idx(%x)\n", data[0], idx);
        return HSM_ERR;
    }

    if (param->req == HSM_REQ_RUN_ECDSA_SIGN) {
        if (data[1] == param->sigSize) {
            SAL_MemCopy((void *)param->sig, (const void *)&data[2], param->sigSize);
        } else {
            ELOG("wrong uiDstSize(%d)\n", data[1]);
            return HSM_ERR;
        }
    }

    return HSM_OK;
}

int32 HSM_RunEcdhPhaseI(hsmAsymKey *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                                 // multi channel number
    cmd[1] = REQ_HSM_RUN_ECDH_PHASE_I | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                                 // dma_addr
    cmd[3] = HSM_NONE_DMA;                                      // trans_type
    cmd[5] = 0;                                                 // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                          // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                           // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->keyType;
    idx++;
    data[idx] = param->priSize;
    idx++;
    data[idx] = param->pubSize;
    idx++;
    data[idx] = param->seedSize;
    idx++;
    if ((param->seed != NULL) && (param->seedSize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->seed, param->seedSize);
        idx += (param->seedSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->priSize + param->pubSize + (sizeof(uint32) * 3U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }
    idx++;

    /* Copy priKey and pubKey */
    if ((data[idx] == param->priSize)) {
        idx++;
        SAL_MemCopy((void *)param->priKey, (const void *)&data[idx], param->priSize);
        idx += ((param->priSize + 3U) / sizeof(uint32));
    } else {
        ELOG("Error pucPriKey Size(0x%x)\n", data[idx]);
        return HSM_ERR;
    }
    if (data[idx] == param->pubSize) {
        idx++;
        SAL_MemCopy((void *)param->pubKey, (const void *)&data[idx], param->pubSize);
    } else {
        ELOG("Error PubKey Size(0x%x)\n", data[idx]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_RunEcdhPhaseII(hsmAsymKey *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                                  // multi channel number
    cmd[1] = REQ_HSM_RUN_ECDH_PHASE_II | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                                  // dma_addr
    cmd[3] = HSM_NONE_DMA;                                       // trans_type
    cmd[5] = 0;                                                  // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                           // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                            // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;
    data[idx] = param->keyType;
    idx++;
    data[idx] = param->priSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)param->priKey, param->priSize);
    idx += (param->priSize + 3U) / sizeof(uint32);

    data[idx] = param->pubSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)param->pubKey, param->pubSize);
    idx += (param->pubSize + 3U) / sizeof(uint32);

    data[idx] = param->secSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->secSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }
    idx++;

    /* Copy secret key */
    if ((data[idx] == param->secSize)) {
        idx++;
        SAL_MemCopy((void *)param->secKey, (const void *)&data[idx], param->secSize);
        idx += ((param->secSize + 3U) / sizeof(uint32));
    } else {
        ELOG("Error pucSecretKey Size(0x%x)\n", data[idx]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_ComputeEcdhPubKey(hsmAsymKey *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                                        // multi channel number
    cmd[1] = REQ_HSM_RUN_ECDH_PUBKEY_COMPUTE | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                                        // dma_addr
    cmd[3] = HSM_NONE_DMA;                                             // trans_type
    cmd[5] = 0;                                                        // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                                 // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                                  // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->objId;
    idx++;

    data[idx] = param->keyType;
    idx++;

    data[idx] = param->priSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)param->priKey, param->priSize);
    idx += (param->priSize + 3U) / sizeof(uint32);

    data[idx] = param->pubSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->pubSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }
    idx++;

    /* Copy pubKey */
    if (data[idx] == param->pubSize) {
        idx++;
        SAL_MemCopy((void *)param->pubKey, (const void *)&data[idx], param->pubSize);
    } else {
        ELOG("Error PubKey Size(0x%x)\n", data[idx]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_Pbkdf2(hsmKdf *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                           // multi channel number
    cmd[1] = REQ_HSM_RUN_PBKDF2 | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                           // dma_addr
    cmd[3] = HSM_NONE_DMA;                                // trans_type
    cmd[5] = 0;                                           // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                    // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                     // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    if ((param->pSize > HSM_KDF_MAX_PW_SIZE) || (param->saltSize > HSM_KDF_MAX_SALT_SIZE)
        || (param->keySize > HSM_KDF_MAX_KEY_SIZE)) {
        ELOG("Wrong len(pwLen:0x%x saltLen:0x%x keyLen:0x%x)\n", param->pSize, param->saltSize, param->keySize);
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->mdAlg;
    idx++;

    data[idx] = param->pSize;
    idx++;

    SAL_MemCopy((void *)&data[idx], (const void *)param->pw, param->pSize);
    idx += ((param->pSize + 3U) / sizeof(uint32));

    data[idx] = param->saltSize;
    idx++;

    SAL_MemCopy((void *)&data[idx], (const void *)param->salt, param->saltSize);
    idx += ((param->saltSize + 3U) / sizeof(uint32));

    data[idx] = param->iteration;
    idx++;

    data[idx] = param->keySize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->keySize + (sizeof(uint32) * 3U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }
    idx++;

    /* Copy priKey and pubKey */
    if ((data[idx] == param->keySize)) {
        idx++;
        SAL_MemCopy((void *)param->key, (const void *)&data[idx], param->keySize);
        idx += ((param->keySize + 3U) / sizeof(uint32));
    } else {
        ELOG("Error key Size(0x%x)\n", data[idx]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_CertiParse(hsmCerti *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[16] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                                  // multi channel number
    cmd[1] = REQ_HSM_CERTIFICATE_PARSE | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                                  // dma_addr
    cmd[3] = HSM_NONE_DMA;                                       // trans_type
    cmd[5] = 0;                                                  // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                           // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                            // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Directly send the address to hsm F/W
     * because the size of the certificate exceeds 512 bytes(maximum size of the Mbox) */

    /* Set data FIFO */
    data[idx] = (uint32)param->crt;
    idx++;
    data[idx] = param->crtSize;
    idx++;
    data[idx] = (uint32)param->pubKey;
    idx++;
    data[idx] = (uint32)&param->pubSize;
    idx++;
    data[idx] = (uint32)param->sig;
    idx++;
    data[idx] = (uint32)&param->sigSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_CertiVerify(hsmCerti *root, hsmCerti *target)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[16] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                                   // multi channel number
    cmd[1] = REQ_HSM_CERTIFICATE_VERIFY | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                                   // dma_addr
    cmd[3] = HSM_NONE_DMA;                                        // trans_type
    cmd[5] = 0;                                                   // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                            // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                             // mbox id, 0x4D5348 = "HSM"

    if ((root == NULL) || (target == NULL)) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Directly send the address to hsm F/W
     * because the size of the certificate exceeds 512 bytes(maximum size of the Mbox) */

    /* Set data FIFO */
    data[idx] = (uint32)root->crt;
    idx++;
    data[idx] = root->crtSize;
    idx++;
    data[idx] = (uint32)target->crt;
    idx++;
    data[idx] = target->crtSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    /* Check out data */
    idx = 0;
    if (data[idx] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x)\n", data[idx]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_DfWrite(hsmKey *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                          // multi channel number
    cmd[1] = HSM_REQ_WRITE_NVM | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                          // dma_addr
    cmd[3] = HSM_NONE_DMA;                               // trans_type
    cmd[5] = 0;                                          // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                   // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                    // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set data FIFO */
    data[idx] = param->dfAddr;
    idx++;
    data[idx] = param->keySize;
    idx++;
    data[idx] = 0; // offset
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)param->keydata, param->keySize);
    idx += (param->keySize + 3U) / sizeof(uint32);

    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    return ret;
}

int32 HSM_TRNG(hsmRng *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                        // multi channel number
    cmd[1] = HSM_REQ_GET_TRNG | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                        // dma_addr
    cmd[3] = HSM_NONE_DMA;                             // trans_type
    cmd[5] = 0;                                        // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                 // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                  // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    data[idx] = param->rngSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->rngSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    if (ret != HSM_OK) {
        ELOG("Error: 0x%x\n", ret);
    }

    if (data[1] == param->rngSize) {
        SAL_MemCopy((void *)param->rng, (const void *)&data[2], param->rngSize);
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return ret;
}

int32 HSM_PRNG(hsmRng *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                         // multi channel number
    cmd[1] = HSM_REQ_GET_PRNG | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                         // dma_addr
    cmd[3] = HSM_NONE_DMA;                              // trans_type
    cmd[5] = 0;                                         // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                  // Mbox timeout value
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                   // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    data[idx] = param->rngSize;
    idx++;
    data[idx] = param->seedSize;
    idx++;
    if ((param->seed != NULL) && (param->seedSize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)param->seed, param->seedSize);
        idx += (param->seedSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = param->rngSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    if (ret != HSM_OK) {
        ELOG("Error: 0x%x\n", ret);
    }

    if (data[1] == param->rngSize) {
        SAL_MemCopy((void *)param->rng, (const void *)&data[2], param->rngSize);
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return ret;
}

int32 HSM_SelectBank(uint32 uiBankIndex)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                          // multi channel number
    cmd[1] = HSM_REQ_SELECT_BANK | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                          // dma_addr
    cmd[3] = HSM_NONE_DMA;                               // trans_type
    cmd[5] = 0;                                          // Not used
    cmd[6] = 0;                                          // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;                    // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiBankIndex;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    return ret;
}

int32 HSM_GetVersion(hsmVer *param)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                        // multi channel number
    cmd[1] = HSM_REQ_GET_VER | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                        // dma_addr
    cmd[3] = HSM_NONE_DMA;                             // trans_type
    cmd[4] = 0;                                        // data length is 0,
    cmd[5] = 0;                                        // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                 // Mbox timeout value
    cmd[7] = 0x4D5348;                                 // mbox id, 0x4D5348 = "HSM"

    if (param == NULL) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    dataSize = cmd[4];
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, dataSize);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = (sizeof(uint32) * 5U);
    SAL_MemSet((void *)data, 0, dataSize);
    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);

    if (ret < 0) {
        ELOG("Error recv_sss (%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error: 0x%x\n", data[0]);
    }

    dataSize = data[1];
    if (dataSize == (sizeof(uint32) * 3U)) {
        param->x = data[2];
        param->y = data[3];
        param->z = data[4];
    } else {
        ELOG("wrong rdata size(%d)\n", dataSize);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_ProgramLDT(uint32 addr, uint32 data1, uint32 data2, uint32 data3, uint32 data4, uint32* dataCRC)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                        // multi channel number
    cmd[1] = HSM_REQ_PROGRAM_LDT | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                        // dma_addr
    cmd[3] = HSM_NONE_DMA;                             // trans_type
    cmd[5] = 0;                                        // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                 // Mbox timeout value
    cmd[7] = 0x4D5348;                                 // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = addr;
    idx++;
    data[idx] = data1;
    idx++;
    data[idx] = data2;
    idx++;
    data[idx] = data3;
    idx++;
    data[idx] = data4;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    SAL_MemSet((void *)data, 0, (sizeof(uint32) * 2));

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }
    SAL_MemCopy(dataCRC, &data[1], sizeof(uint32));

    ret = (int32)data[0];
    return ret;
}

int32 HSM_ReadLDT(uint32 addr, uint32* dataCRC)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[HSM_DATA_FIFO_CNT] = {0};
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;                        // multi channel number
    cmd[1] = HSM_REQ_READ_LDT | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                                        // dma_addr
    cmd[3] = HSM_NONE_DMA;                             // trans_type
    cmd[5] = 0;                                        // Not used
    cmd[6] = HSM_MBOX_TIMEOUT_GENERAL;                 // Mbox timeout value
    cmd[7] = 0x4D5348;                                 // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = addr;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    SAL_MemSet((void *)data, 0, (sizeof(uint32) * 2));

    ret = MBOX_DevRecvSss(hsm_dev, HSM_MBOX_TIMEOUT_GENERAL, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }
    SAL_MemCopy(dataCRC, &data[1], sizeof(uint32));

    ret = (int32)data[0];
    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )


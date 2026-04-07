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
#if 0
#define NDEBUG
#endif
#define TLOG_LEVEL (TLOG_DEBUG)
#include "hsm_log.h"

//temp #include <app_cfg.h>
//#include <sal_internal.h>
#include <bsp.h>
#include <string.h>
#include <stdlib.h>
//#include <fwug.h>
#include <debug.h>

#include "hsm_manager.h"
#include "mbox.h"

/*
***************************************************************************************************
*                                            DEFINITIONS
***************************************************************************************************
*/
#define TESTAPP_MAJOR_VER (2)
#define TESTAPP_MINOR_VER (5)

#define HSM_REQUIRED_VER_X (0)
#define HSM_REQUIRED_VER_Y (0)
#define HSM_REQUIRED_VER_Z (36)

static MBOXDvice_t *hsm_dev;

int32 HSM_Init(void)
{
    uint32 x = 0;
    uint32 y = 0;
    uint32 z = 0;
    int32 ret = HSM_ERR;

    int8 devName[7] = {'H', 'S', 'M', 0x00, 0x00, 0x00, 0x00};

    if (hsm_dev == NULL) {
        //DLOG("Test App Version:%d.%d\n", TESTAPP_MAJOR_VER, TESTAPP_MINOR_VER);
        hsm_dev = MBOX_DevOpen(MBOX_CH_HSM_NONSECURE, devName, 0);
        if (hsm_dev == NULL) {
            mcu_printf("Error dev_open\n");
            return HSM_ERR;
        }

        HSM_GetVersion(HSM_REQ_GET_VER, &x, &y, &z);
        if ((x > HSM_REQUIRED_VER_X) ||
            ((x == HSM_REQUIRED_VER_X) && (y > HSM_REQUIRED_VER_Y)) ||
            ((x == HSM_REQUIRED_VER_X) && (y == HSM_REQUIRED_VER_Y) && (z >= HSM_REQUIRED_VER_Z))) {
            ret = HSM_OK;
        }
        else {
            mcu_printf(
                "HSM FW verison(%d.%d.%d) must be higher than equal to %d.%d.%d\n", x, y, z,
                HSM_REQUIRED_VER_X, HSM_REQUIRED_VER_Y, HSM_REQUIRED_VER_Z);
            hsm_dev = NULL;
            ret = HSM_ERR;
        }
    }

    return ret;
}

int32 HSM_GetVersion(uint32 uiReq, uint32 *uiX, uint32 *uiY, uint32 *uiZ)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if ((uiX == NULL) || (uiY == NULL) || (uiZ == NULL)) {
        //ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[4] = 0;                              // data length is 0,
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = 0x4D5348;                       // mbox id, 0x4D5348 = "HSM"

    dataSize = cmd[4];
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, dataSize);
    if (ret != HSM_OK) {
        //ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = (sizeof(uint32) * 5U);
    SAL_MemSet((void *)data, 0, dataSize);

	do {
		ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    } while (ret < 0);

    if (data[0] != HSM_DATA_FIFO_OK) {
        //ELOG("Error: 0x%x\n", data[0]);
    }

    dataSize = data[1];
    if (dataSize == (sizeof(uint32) * 3U)) {
        *uiX = data[2];
        *uiY = data[3];
        *uiZ = data[4];
    } else {
        //ELOG("wrong rdata size(%d)\n", dataSize);
        return HSM_ERR;
    }

    //DLOG("HSM FW Version %d.%d.%d\n", *uiX, *uiY, *uiZ);

    return HSM_OK;
}

int32 HSM_GetMFCERT(void)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = HSM_REQ_GET_MFCERT | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[4] = 0;                              // data length is 0,
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = 0x4D5348;                       // mbox id, 0x4D5348 = "HSM"

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, 0);
    if (ret != HSM_OK) {
        //ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

	do {
		ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    } while (ret < 0);

    if (data[0] != HSM_OK) {
        //ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_GetSecureHSM(uint32 addr)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 idx = 0;

    int32 ret = -1;

	/* Set command FIFO */

	cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = HSM_REQ_GET_SECURE_HSM | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = 0x4D5348;                       // mbox id, 0x4D5348 = "HSM"

    data[idx] = addr;
	idx++;
    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

	do {
		ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    } while (ret < 0);

    if (data[0] != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_SBEnable(void)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = HSM_REQ_SB_ENABLE | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[4] = 0;                              // data length is 0,
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = 0x4D5348;                       // mbox id, 0x4D5348 = "HSM"

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, 0);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

	do {
		ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    } while (ret < 0);

    if (data[0] != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_SBVerifyStop(void)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = HSM_REQ_SB_VERIFY_STOP | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[4] = 0;                              // data length is 0,
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = 0x4D5348;                       // mbox id, 0x4D5348 = "HSM"

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, 0);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

    do {
        ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    } while (ret < 0);

    if (data[0] != HSM_OK) {
        ELOG("Error ret(%d)\n", data[0]);
        return HSM_ERR;
    }
    if (data[1] != 0x1000u) {
        ELOG("Error data(%d)\n", data[1]);
        return HSM_ERR;
    }


    return HSM_OK;
}


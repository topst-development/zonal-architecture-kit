// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : hsm_manager.h
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM MANAGER
*
*
***************************************************************************************************
*/

#ifndef HSM_MANAGER_HEADER
#define HSM_MANAGER_HEADER

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
// clang-format off
#define HSM_REQ_SB_VERIFY_STOP				(0x00060000U)
#define HSM_REQ_GET_VER                     (0x20010000U)
#define HSM_REQ_GET_MFCERT                  (0x20020000U)
#define HSM_REQ_GET_SECURE_HSM              (0x20030000U)
#define HSM_REQ_SB_ENABLE					(0x20040000U)
// clang-format on

#define HSM_CMD_FIFO_SIZE (0x0008U)
#define HSM_MBOX_LOCATION_DATA (0x0400U)
#define HSM_MBOX_LOCATION_CMD (0x0000U)
#define HSM_MBOX_ID_HSM (0x4D5348) /* 0x4D5348 = "HSM" */

#define HSM_MBOX_CID_A72 (0x7200U)
#define HSM_MBOX_CID_A53 (0x5300U)
#define HSM_MBOX_CID_SC (0xD300U)
#define HSM_MBOX_CID_HSM (0xA000U)
#define HSM_MBOX_CID_R5 (0xFF00U)

#define HSM_MBOX_BSID_BL0 (0x0042U)
#define HSM_MBOX_BSID_BL1 (0x0043U)
#define HSM_MBOX_BSID_BL3 (0x0045U)
#define HSM_MBOX_BSID_KERNEL (0x0046U)

#define HSM_MBOX_HSM_CMD0 (HSM_MBOX_CID_R5 | HSM_MBOX_BSID_KERNEL)
#define HSM_RSASSA_PSS_OID_HASH (HSM_OID_SHA2_256)

/* HSM Err code */
#define HSM_DATA_FIFO_OK (0U)
#define HSM_OK (0)
#define HSM_ERR (-1)
#define HSM_ERR_INVALID_PARAM (-2)
#define HSM_ERR_INVALID_STATE (-3)
#define HSM_ERR_INVALID_MEMORY (-4)
#define HSM_ERR_UNSUPPORT_FUNC (-5)
#define HSM_ERR_SOTB_CIPHER (-6)
#define HSM_ERR_OCCUPIED_RESOURCE (-7)

/* HSM DMA type */
#define HSM_NONE_DMA (0)
#define HSM_DMA (1)

#define TEMP_SRAM 0x00060000u

#define B_LUN0_HSM_FW_IMAGE_OFFSET 0x00002000
#define B_LUN1_HSM_FW_IMAGE_OFFSET 0x00022000
#define HSM_FW_SIZE 0x1F000 // 124K
#define MFCERT_SIZE 0x100

/* PMU for reset */
#define PMU_LOCK                 (*(volatile uint32*)(0xA0F283FC))
#define COLD_RESET_REQ           (*(volatile uint32*)(0xA0F28010))
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
int32 HSM_Init(void);
int32 HSM_GetVersion(uint32 uiReq, uint32 *uiX, uint32 *uiY, uint32 *uiZ);
int32 HSM_GetMFCERT(void);
int32 HSM_GetSecureHSM(uint32 addr);
int32 HSM_SBEnable(void);
int32 HSM_SBVerifyStop(void);

#endif /* HSM_MANAGER_HEADER */

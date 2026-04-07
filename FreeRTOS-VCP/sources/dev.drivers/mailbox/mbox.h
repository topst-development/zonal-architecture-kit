// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : mbox.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_MBOX_CFG_HEADER
#define MCU_BSP_MBOX_CFG_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_MBOX == 1 )


#define MBOX_CMD_FIFO_SIZE              (8UL)
#define MBOX_DATA_FIFO_SIZE             (128UL)

#define MBOX_MAX_QUEUE_SIZE             (4)
#define MBOX_MAX_MULTI_OPEN             (10UL)
#define MBOX_MAX_KEY_SIZE               (6)


/*=============================== INCLUSIONS =================================*/

/*============================ TYPE DEFINITIONS =============================*/

/* Mailbox Registers */
#define MBOX_BASE                       (0xA09F0000U)
#define MBOX_TXC(x)                     ((volatile uint32 *)(MBOX_BASE + (0x0000u + ((x)*4u))))
#define MBOX_RXC(x)                     ((volatile uint32 *)(MBOX_BASE + (0x0020u + ((x)*4u))))
#define MBOX_CTR                        ((volatile uint32 *)(MBOX_BASE + 0x0040u))
#define MBOX_CMD_STS                    ((volatile uint32 *)(MBOX_BASE + 0x0044u))
#define MBOX_TXD_STS                    ((volatile uint32 *)(MBOX_BASE + 0x0050u))
#define MBOX_RXD_STS                    ((volatile uint32 *)(MBOX_BASE + 0x0054u))
#define MBOX_TXD                        ((volatile uint32 *)(MBOX_BASE + 0x0060u))
#define MBOX_RXD                        ((volatile uint32 *)(MBOX_BASE + 0x0070u))
#define MBOX_TMN_STS                    ((volatile uint32 *)(MBOX_BASE + 0x007Cu))

#define MBOX_CTR_D_FLUSH                (0x00000080u)
#define MBOX_CTR_FLUSH                  (0x00000040u)
#define MBOX_CTR_OEN                    (0x00000020u)
#define MBOX_CTR_IEN                    (0x00000010u)

#define MBOX_CMD_STS_RXC_E              (0x00010000u)
#define MBOX_CMD_STS_TXC_E              (0x00000001u)

#define MBOX_TXD_STS_E                  (0x80000000u)
#define MBOX_TXD_STS_F                  (0x40000000u)

#define MBOX_RXD_STS_E                  (0x80000000u)
#define MBOX_RXD_STS_F                  (0x40000000u)

#define MBOX_SET_OWN_TMN                (0x00000001u)
#define MBOX_SET_OPP_TMN                (0x00010000u)

#define MBOX_IREQ_OUT_MASK              (0x00000180u)

#define MBOX_GET_RXC_COUNT(a)           (((a) >> 20u) & 0xFu)
#define MBOX_GET_TXC_COUNT(a)           (((a) >> 4u) & 0xFu)

#define MBOX_GET_RXD_COUNT(a)           ((a) & 0xFFFFu)
#define MBOX_GET_TXD_COUNT(a)           ((a) & 0xFFFFu)


/* Mailbox definitions */
#define MBOX_MSG_CMD_COUNT              (8u)
#define MBOX_MSG_CMD_MAX                (MBOX_MSG_CMD_COUNT * 4u)
#define MBOX_MSG_DATA_COUNT             (128u)
#define MBOX_MSG_DATA_MAX               (MBOX_MSG_DATA_COUNT * 4u)

#define MBOX_UNINIT                     (0x0U)
#define MBOX_INIT                       (0x1U)

#define MBOX_SUCCESS                    (0)
#define MBOX_ERR_INVALID_PARAM          (-1)
#define MBOX_ERR_DEV_BUSY               (-2)
#define MBOX_ERR_TIMEOUT                (-3)

#define MBOX_TIMEOUT_COUNT              (10000000u)
#define MBOX_TIMEOUT_ADJUSTMENET_VALUE  (7000u)

/*===================== EXTERNAL FUNCTION DECLARATIONS ======================*/

extern void MBOX_Init
(
    void
);

extern sint32 MBOX_DevSendSss
(
    uint8 ucDev,
    const uint32 *puiCmd,
    const uint32 *puiData,
    uint32 uiLen
);

extern sint32 MBOX_DevRecvSss
(
    uint8 ucDev,
    uint32 uiTimeout,
    uint32 *puiCmd,
    uint32 *puiData
);

extern void MBOX_Flush
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_MBOX == 1 )

#endif  // MCU_BSP_MBOX_CFG_HEADER


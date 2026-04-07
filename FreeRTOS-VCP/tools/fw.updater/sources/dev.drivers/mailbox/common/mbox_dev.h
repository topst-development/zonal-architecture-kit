// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : mbox_dev.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/
#ifndef MBOX_DEV_H
#define MBOX_DEV_H

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define MBOX_CTRL_IEN                   (0x00000010)
#define MBOX_CTRL_OEN                   (0x00000020)
#define MBOX_CTRL_CMD_FLUSH             (0x00000040)
#define MBOX_CTRL_DAT_FLUSH             (0x00000080)

#define MBOX_RD_BLOCK                   (0x0001)
#define MBOX_WR_BLOCK                   (0x0002)

#define MBOX_DEV_NULLCHK(D)             ((D) != (void *) 0)
#define MBOX_DEV_VALIDID(D)             ((D) >= 0)
#define MBOX_DEV_VALIDCH(D)             (((D) > (MBOXCh_t) MBOX_CH_INVAL) && ((D) < MBOX_CH_MAX))
#define MBOX_DEV_VALIDDL(D)             (((D) <= MBOX_DATA_FIFO_SIZE) && ((D) > 0UL))
#define MBOX_DEV_VALIDCL(D)             (((D) <= MBOX_CMD_FIFO_SIZE) && ((D) > 0UL))

#define MBOX_DCNT(c)                    (MBOX_PhyReadData(c))
#define MBOX_CCNT(c)                    (MBOX_PhyReadCmd(c))

#define MBOX_DevFunCall(R, F)           ((R) = (F))

#define MBOX_DevPtrCallVoid(F, P)       \
{                                       \
    if ((F) != (void *) 0)              \
    {                                   \
        (F)(P);                         \
    }                                   \
}

#define MBOX_DevPtrCall(R, F, P)        \
{                                       \
    if ((F) != (void *) 0)              \
    {                                   \
        (R) = (F)(P);                   \
    }                                   \
}

#define MBOX_DevCallJmpOnErr(R, F, P)	\
{                                       \
    if ((F) != (void *) 0)              \
    {                                   \
        (R) = (F)(P);                   \
        if ((R) != MBOX_SUCCESS)        \
	{				\
            goto err;                   \
	}				\
    }                                   \
}

#define MBOX_DevCallBreakOnErr(R, F, E) \
{                                       \
    (R) = (F);                          \
    if ((R) != (E))                     \
    {                                   \
        break;                          \
    }                                   \
}

#define MBOX_DevCallBreakOnNonBlock(R, F, P, E)         \
{                                                       \
    if (MBOX_DEV_NULLCHK((F)))                          \
    {                                                   \
        MBOX_DevCallBreakOnErr((R), (F)(P), (E))        \
        (R) = (int32) MBOX_DEV_GET_AGAIN;               \
    }                                                   \
    else                                                \
    {                                                   \
        break;                                          \
    }                                                   \
}

#define MBOX_DevBreakWithErr(R, E)      {R = E; break;}

typedef void (*MBOXIsrCallBack)
(
    void *                              arg
);

typedef struct MBOXIsrFunc
{
    MBOXIsrCallBack                     cbRecev;
    MBOXIsrCallBack                     cbWrite;
} MBOXIsrFunc_t;

typedef struct MBOXHashMap
{
    uint8                               hmSrkey[MBOX_MAX_KEY_SIZE];
    int32                               hmValue;
    int32                               hmInuse;
} MBOXHashMap_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void MBOX_DevMakeCtlPacket
(
    uint32 *                            puiCtl,
    uint32 *                            puiCmd,
    uint8 *                             ucpName
);

static void MBOX_DevMakeCtlPacketSss
(
    uint32 *                            puiCtl,
    uint32 *                            puiCmd,
    uint8 *                             ucpName
);

static void MBOX_DevIsrReceivedA53Secure
(
    void *                              pArg
);

static void MBOX_DevIsrWriteConfirmA53Secure
(
    void *                              pArg
);

static void MBOX_DevIsrReceivedA53NonSecure
(
    void *                              pArg
);

static void MBOX_DevIsrWriteConfirmA53NonSecure
(
    void *                              pArg
);

static void MBOX_DevIsrReceivedA72Secure
(
    void *                              pArg
);

static void MBOX_DevIsrWriteConfirmA72Secure
(
    void *                              pArg
);

static void MBOX_DevIsrReceivedA72NonSecure
(
    void *                              pArg
);

static void MBOX_DevIsrWriteConfirmA72NonSecure
(
    void *                              pArg
);

static void MBOX_DevIsrReceivedM04NonSecure
(
    void *                              pArg
);

static void MBOX_DevIsrWriteConfirmM04NonSecure
(
    void *                              pArg
);

static void MBOX_DevIsrReceivedHsmNonSecure
(
    void *                              pArg
);

static void MBOX_DevIsrWriteConfirmHsmNonSecure
(
    void *                              pArg
);

static int32 MBOX_DevDeploySwbuf
(
    MBOXCh_t                            siCh,
    uint32                              uiLen
);

static void MBOX_DevWordCpy
(
    uint32 *                            puiTarget,
    uint32 *                            puiSource,
    uint32                              uiLen
);

static void MBOX_DevByteCpy
(
    uint8 *                             puiTarget,
    uint8 *                             puiSource,
    uint32                              uiLen
);

static int32 MBOX_DevQput
(
    MBOXCh_t                            siCh,
    int32                               siDevId,
    MBOXQueue_t *                       pQueue,
    uint32                              uiLen
);

static uint32 MBOX_DevQget
(
    MBOXCh_t                            siCh,
    int32                               siDevId,
    MBOXQueue_t *                       pQueue,
    uint32 *                            puiCmd,
    uint32 *                            puiData
);

static uint32 MBOX_DevQgetSss
(
    MBOXCh_t                            siCh,
    int32                               siDevId,
    MBOXQueue_t *                       pQueue,
    uint32 *                            puiCmd,
    uint32 *                            puiData
);

static int32 MBOX_DevFind
(
    MBOXCh_t                            siCh,
    MBOXMsage_t *                       pMsg
);

static int32 MBOX_DevNew
(
    MBOXCh_t                            siCh,
    uint8 *                             pucDevName
);

static int32 MBOX_DevMapGet
(
    MBOXCh_t                            siCh,
    int8 *                              pucDevName
);

static int32 MBOX_DevMapPut
(
    MBOXCh_t                            siCh,
    uint8 *                             pucDevName,
    int32                               siIdx
);

static uint32 MBOX_DevCrc32
(
    const uint8 *                       puc,
    uint32                              uiLen
);

extern int32 MBOX_PhyReady
(
    MBOXCh_t                            siCh,
    uint32                              ucOpen
);

extern void MBOX_PhyBitSetCtrl
(
    MBOXCh_t                            siCh,
    uint32				                uiValue
);

extern void MBOX_PhyBitClearCtrl
(
    MBOXCh_t                            siCh,
    uint32                              uiValue
);

extern uint8 MBOX_PhyIsIlevelSet
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyIsIenSet
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyIsOenSet
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyIsFlushedCmd
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyIsFlushedData
(
    MBOXCh_t                            siCh
);

/* ================ Command FIFO Status Register ================ *
 * ============================================================== */
extern uint8 MBOX_PhyCountScmd
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyFullScmd
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyEmptyScmd
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyCountMcmd
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyFullMcmd
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyEmptyMcmd
(
    MBOXCh_t                            siCh
);

/* ============== Transmit Data FIFO Status Register ============ *
 * ============================================================== */
extern uint8 MBOX_PhyEmptyMdata
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyFullMdata
(
    MBOXCh_t                            siCh
);

extern uint16 MBOX_PhyCountMdata
(
    MBOXCh_t                            siCh
);

/* =============== Receive Data FIFO Status Register ============ *
 * ============================================================== */
extern uint8 MBOX_PhyEmptySdata
(
    MBOXCh_t                            siCh
);

extern uint8 MBOX_PhyFullSdata
(
    MBOXCh_t                            siCh
);

extern uint16 MBOX_PhyCountSdata
(
    MBOXCh_t                            siCh
);

/* =============== Transmit Register for Command FIFO =========== *
 * ============================================================== */
extern int32 MBOX_PhyWriteCmd
(
    MBOXCh_t                            siCh,
    uint32 *                            puiCmd
);

/* =============== Receive Register for Command FIFO ============ *
 * ============================================================== */
extern uint32 MBOX_PhyReadCmd
(
    MBOXCh_t                            siCh
);

/* =============== Transmit Register for Data FIFO ============== *
 * ============================================================== */
extern int32 MBOX_PhyWriteData
(
    MBOXCh_t                            siCh,
    uint32 *                            puiData,
    uint32                              uiLen
);

/* ================ Receive Register for Data FIFO ============== *
 * ============================================================== */
extern uint32 MBOX_PhyReadData
(
    MBOXCh_t                            siCh
);

/* ==================== Terminal Status Register ================ *
 * ============================================================== */
extern int32 MBOX_PhySetOwnTmnSts
(
    MBOXCh_t                            siCh,
    uint32                              uiValue
);

extern int32 MBOX_PhyGetOwnTmnSts
(
    MBOXCh_t                            siCh,
    uint32 *                            uiValue
);

extern int32 MBOX_PhyGetOppTmnSts
(
    MBOXCh_t                            siCh,
    uint32 *                            uiValue
);

#endif // MBOX_DEV_H


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : mbox_phy.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/
#ifndef MBOX_PHY_H
#define MBOX_PHY_H

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define MBOX_D(fmt, args...)        {LOGD(DBG_TAG_MBOX, fmt, ## args)}
    #define MBOX_E(fmt, args...)        {LOGE(DBG_TAG_MBOX, fmt, ## args)}
#else
    #define MBOX_D(fmt, args...)
    #define MBOX_E(fmt, args...)
#endif
/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define MBOX_BITSET(X, MASK)            ((X) |= (uint32)(MASK))
#define MBOX_BITSCLR(X, SMASK, CMASK)   ((X) = ((((uint32)(X)) | ((uint32)(SMASK))) & ~((uint32)(CMASK))))
#define MBOX_BITCSET(X, CMASK, SMASK)   ((X) = ((((uint32)(X)) & ~((uint32)(CMASK))) | ((uint32)(SMASK))))
#define MBOX_BITCLR(X, MASK)            ((X) &= ~((uint32)(MASK)))

#define MBOX_OFFSET_CTL_IEN             (4U)
#define MBOX_OFFSET_CTL_OEN             (5U)
#define MBOX_OFFSET_CTL_CFLUSH          (6U)
#define MBOX_OFFSET_CTL_DFLUSH          (7U)
#define MBOX_OFFSET_CMD_SCOUNT          (20U)
#define MBOX_OFFSET_CMD_SFUL            (17U)
#define MBOX_OFFSET_CMD_SEMP            (16U)
#define MBOX_OFFSET_CMD_MCOUNT          (4U)
#define MBOX_OFFSET_CMD_MFUL            (1U)
#define MBOX_OFFSET_DAT_SEMP            (31U)
#define MBOX_OFFSET_DAT_SFUL            (30U)
#define MBOX_OFFSET_DAT_MEMP            (31U)
#define MBOX_OFFSET_DAT_MFUL            (30U)

#define MBOX_BIT(N)                     (0x01U << (N))
#define MBOX_BITS02                     (0x03U)
#define MBOX_BITS04                     (0x0FU)
#define MBOX_BITS16                     (0xFFFFU)
#define MBOX_ENABLE                     (0x00000001)
#define MBOX_DISABLE                    (0x00000000)

#define MBOX_MAX_CMD_SIZE               (MBOX_CMD_FIFO_SIZE * 4)
#define MBOX_MAX_DATA_SIZE              (MBOX_DATA_FIFO_SIZE * 4)
#define MBOX_WORD_SIZE                  (4)
#define MBOX_HALF_SIZE                  (2)

#define MBOX_PHY_NULLCHK(D)             ((D) != (void *) 0)
#define MBOX_PHY_VALIDDL(D)             (((D) <= MBOX_DATA_FIFO_SIZE) && ((D) > 0UL))
#define MBOX_PHY_VALIDCL(D)             (((D) <= MBOX_CMD_FIFO_SIZE) && ((D) > 0UL))
#define MBOX_PhyFunCall(R, F)           ((R) = (F))

/*******************************************************************************
*    MailBox Register Base Address Define
********************************************************************************/
#define MBOX_HW3_HSMNS_BASE             (0x50000000U) /* HSM -> CR5 */
#define MBOX_HW3_CR5NS_BASE             (0xA09F0000U) /* CR5 -> HSM */

/*******************************************************************************
*    MailBox CTR Register Field Define
********************************************************************************/
#define MBOX_DAT_FLUSH_BIT              (0x00000080UL)
#define MBOX_CMD_FLUSH_BIT              (0x00000040UL)
#define MBOX_OEN_BIT                    (0x00000020UL)
#define MBOX_RD_IEN_BIT                 (0x00000010UL)
#define MBOX_WR_IEN_BIT                 (0x00100000UL)
#define MBOX_WR_ICLR_BIT                (0x00200000UL)
#define MBOX_TEST_BIT                   (0x80000000UL)

#define MBOX_INT_LEVEL_1                (0x0)
#define MBOX_INT_LEVEL_2                (0x1)
#define MBOX_INT_LEVEL_4                (0x2)
#define MBOX_INT_LEVEL_8                (0x3)

#define MBOX_SSS_STATUS_ADDR            (0x1E020000U)
#define MBOX_SSS_DATA_ADDR              (0x1E022000U)

/*******************************************************************************
*    MailBox Register Define (Base Addr = 0x1BA00000[main], 0x1B800000[micom])
********************************************************************************/
typedef struct MBOXCtlDataIdx
{
    uint32                              rDATA   :32;
} MBOXCtlDataIdx_t;

typedef union
{
    uint32                              nREG;
    MBOXCtlDataIdx_t                    bREG;
} MBOXCtlDmm_t;

typedef struct MBOXCtlStrIdx
{
    uint32                              rMEMP   :1;
    uint32                              rMFUL   :1;
    uint32                                      :2;
    uint32                              rMCOUNT :4;
    uint32                                      :8;
    uint32                              rSEMP   :1;
    uint32                              rSFUL   :1;
    uint32                                      :2;
    uint32                              rSCOUNT :4;
    uint32                                      :8;
} MBOXCtlStrIdx_t;

typedef union MBOXCtlStr
{
    uint32                              nREG;
    MBOXCtlStrIdx_t                     bREG;
} MBOXCtlStr_t;

typedef struct MBOXCtlCmdIdx
{
    uint32                              rLEVEL  :2;
    uint32                                      :2;
    uint32                              rIEN    :1;
    uint32                              rOEN    :1;
    uint32                              rFLUSH  :1;
    uint32                              rD_FLUSH:1;
    uint32                                      :12;
    uint32                              rWIEN   :1;
    uint32                              rWCLR   :1;
    uint32                                      :10;
} MBOXCtlCmdIdx_t;

typedef union MBOXCtlCmd
{
    uint32                              nREG;
    MBOXCtlCmdIdx_t                     bREG;
} MBOXCtlCmd_t;

typedef struct MBOXCtlDtsIdx
{
    uint32                              rMCOUNT :16;
    uint32                                      :14;
    uint32                              rMFUL   :1;
    uint32                              rMEMP   :1;
} MBOXCtlDtsIdx_t;

typedef union MBOXCtlDts
{
    uint32                              nREG;
    MBOXCtlDtsIdx_t                     bREG;
} MBOXCtlDts_t;

typedef struct MBOXCtlDrsIdx
{
    uint32                              rSCOUNT :16;
    uint32                                      :14;
    uint32                              rSFUL   :1;
    uint32                              rSEMP   :1;
} MBOXCtlDrsIdx_t;

typedef union MBOXCtlDrs {
    uint32                              nREG;
    MBOXCtlDrsIdx_t                     bREG;
} MBOXCtlDrs_t;

typedef struct MBOXCtlTxdIdx
{
    uint32                              rTXD    :32;
} MBOXCtlTxdIdx_t;


typedef union MBOXCtlTxd {
    uint32                              nREG;
    MBOXCtlTxdIdx_t                     bREG;
} MBOXCtlTxd_t;

typedef struct MBOXCtlRxdIdx
{
    uint32                              rRXD    :32;
} MBOXCtlRxdIdx_t;

typedef union MBOXCtlRxd
{
    uint32                              nREG;
    MBOXCtlRxdIdx_t                     bREG;
} MBOXCtlRxd_t;

typedef union MBOXCtlSetIdx
{
    uint32                              rLEVEL  :2;
    uint32                                      :2;
    uint32                              rIEN    :1;
    uint32                              rOEN    :1;
    uint32                              rFLUSH  :1;
    uint32                              rD_FLUSH:1;
    uint32                                      :12;
    uint32                              rWIEN   :1;
    uint32                              rWCLR   :1;
    uint32                                      :10;
} MBOXCtlSetIdx_t;

typedef union MBOXCtlSet
{
    uint32                              nREG;
    MBOXCtlSetIdx_t                     bREG;
} MBOXCtlSet_t;

typedef union MBOXCtlClrIdx
{
    uint32                              rLEVEL  :2;
    uint32                                      :2;
    uint32                              rIEN    :1;
    uint32                              rOEN    :1;
    uint32                              rFLUSH  :1;
    uint32                              rD_FLUSH:1;
    uint32                                      :12;
    uint32                              rWIEN   :1;
    uint32                              rWCLR   :1;
    uint32                                      :10;
} MBOXCtlClrIdx_t;

typedef union MBOXCtlClr
{
    uint32                              nREG;
    MBOXCtlClrIdx_t                     bREG;
} MBOXCtlClr_t;

typedef union MBOXCtlStsIdx
{
    uint32                              rOWNTSTS:1;
    uint32                                      :15;
    uint32                              rOPPTSTS:1;
    uint32                                      :15;
} MBOXCtlStsIdx_t;

typedef union MBOXCtlSts
{
    uint32                              nREG;
    MBOXCtlStsIdx_t                     bREG;
} MBOXCtlSts_t;

typedef struct
{
    MBOXCtlTxd_t                        mboxCmdTx0; // 0x0000
    MBOXCtlTxd_t                        mboxCmdTx1; // 0x0004
    MBOXCtlTxd_t                        mboxCmdTx2; // 0x0008
    MBOXCtlTxd_t                        mboxCmdTx3; // 0x000C
    MBOXCtlTxd_t                        mboxCmdTx4; // 0x0010
    MBOXCtlTxd_t                        mboxCmdTx5; // 0x0014
    MBOXCtlTxd_t                        mboxCmdTx6; // 0x0018
    MBOXCtlTxd_t                        mboxCmdTx7; // 0x001C
    MBOXCtlRxd_t                        mboxCmdRx0; // 0x0020
    MBOXCtlRxd_t                        mboxCmdRx1; // 0x0024
    MBOXCtlRxd_t                        mboxCmdRx2; // 0x0028
    MBOXCtlRxd_t                        mboxCmdRx3; // 0x002C
    MBOXCtlRxd_t                        mboxCmdRx4; // 0x0030
    MBOXCtlRxd_t                        mboxCmdRx5; // 0x0034
    MBOXCtlRxd_t                        mboxCmdRx6; // 0x0038
    MBOXCtlRxd_t                        mboxCmdRx7; // 0x003C
    MBOXCtlCmd_t                        mboxCtlCmd; // 0x0040
    MBOXCtlStr_t                        mboxCtlStr; // 0x0044
    MBOXCtlDmm_t                        mboxDummy0; // 0x0048
    MBOXCtlDmm_t                        mboxDummy1; // 0x004C
    MBOXCtlDts_t                        mboxTxStr;  // 0x0050
    MBOXCtlDrs_t                        mboxRxStr;  // 0x0054
    MBOXCtlDmm_t                        mboxDummy2; // 0x0058
    MBOXCtlDmm_t                        mboxDummy3; // 0x005C
    MBOXCtlTxd_t                        mboxTxFifo; // 0x0060
    MBOXCtlDmm_t                        mboxDummy4; // 0x0064
    MBOXCtlDmm_t                        mboxDummy5; // 0x0068
    MBOXCtlDmm_t                        mboxDummy6; // 0x006C
    MBOXCtlRxd_t                        mboxRxFifo; // 0x0070
    MBOXCtlSet_t                        mboxCtlSet; // 0x0074
    MBOXCtlClr_t                        mboxCtlClr; // 0x0078
    MBOXCtlSts_t                        mboxCtlSts; // 0x007C
} /* MBOXReg_t, */ *MBOXReg;

typedef struct MBOXPinfo
{
    volatile MBOXReg                    mbBase;
    uint32                              mbRirq;
    uint32                              mbWirq;
    uint32                              mbOcnt;
    uint32                              mbLock;
    uint32                              mbSiid;
    MBOXMsage_t                         mbMesg;
    MBOXDvice_t                        *mbDlst;
    MBOXCallback_t                      mbOsfn;
} MBOXPinfo_t;

extern MBOXPinfo_t                      mboxInfo[MBOX_CH_MAX];
#endif // MBOX_PHY_H


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : i2s_reg.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_I2S_REG_HEADER
#define MCU_BSP_I2S_REG_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_I2S == 1 )

/**************************************************************************************************
*                                             INCLUDE FILES
**************************************************************************************************/
#include <gpio.h>


/**************************************************************************************************
*                                             DEFINITIONS
**************************************************************************************************/

//#define AUDIO_RX_ENABLE
#ifdef AUDIO_RX_ENABLE
#define AUDIO_DEBUG_TEST
//#define I2S_CHIP_VERI
#endif

//#define WM8731

#define I2S_SW_RESET_REG_1              (SALReg32 *)0xA0F20010UL
#define I2S_BUS_CLK_MASK_REG_1          (SALReg32 *)0xA0F20004UL

#define I2S_CFG_REG_FIELD               (29UL)

#define I2S_CHMAX                       (3UL)

#define I2S_MCLK_CH0                    GPIO_GPB(20UL)
#define I2S_BCLK_CH0                    GPIO_GPB(21UL)
#define I2S_LRCK_CH0                    GPIO_GPB(22UL)
#define I2S_DAO_CH0                     GPIO_GPB(24UL)
#define I2S_DAI_CH0                     GPIO_GPB(23UL)

#define I2S_MCLK_CH1                    GPIO_GPA(25UL)
#define I2S_BCLK_CH1                    GPIO_GPA(27UL)
#define I2S_LRCK_CH1                    GPIO_GPA(28UL)
#define I2S_DAO_CH1                     GPIO_GPA(30UL)

#define I2S_MCLK_CH2                    GPIO_GPB(0UL)
#define I2S_BCLK_CH2                    GPIO_GPB(1UL)
#define I2S_LRCK_CH2                    GPIO_GPB(2UL)
#define I2S_DAO_CH2                     GPIO_GPB(3UL)
#define I2S_DAI_CH2                     GPIO_GPB(4UL)

#define TCC_GPNONE                      (0xFFFFUL)

/**************************************************************************************************
*                                             AUDIO_DMA Register
**************************************************************************************************/
typedef struct I2SRegFieldDaParam
{
    SALReg32                            SINC    : 8;
    SALReg32                            SMASK   : 24;
} I2SRegFieldDaParam_t;

typedef union I2SRegDaParam
{
    SALReg32                            nReg;
    I2SRegFieldDaParam_t                bReg;
} I2SRegDaParam_t;

typedef struct I2SRegFieldDaTCnt
{
    SALReg32                            ST_TCOUNT   : 16;
    SALReg32                            C_TCOUNT    : 16;
} I2SRegFieldDaTCnt_t;

typedef union I2SRegDaTCnt
{
    SALReg32                            nReg;
    I2SRegFieldDaTCnt_t                 bReg;
} I2SRegDaTCnt_t;

typedef struct I2SRegFieldTransCtrl
{
    SALReg32                            DTWSIZE : 2;
    SALReg32                                    : 2;
    SALReg32                            DRWSIZE : 2;
    SALReg32                                    : 2;
    SALReg32                            DTBSIZE : 2;
    SALReg32                                    : 2;
    SALReg32                            DRBSIZE : 2;
    SALReg32                                    : 2;
    SALReg32                            DTRPT   : 1;
    SALReg32                                    : 1;
    SALReg32                            DRRPT   : 1;
    SALReg32                                    : 1;
    SALReg32                            DTTRG   : 1;
    SALReg32                                    : 1;
    SALReg32                            DRTRG   : 1;
    SALReg32                                    : 1;
    SALReg32                            DTLCK   : 1;
    SALReg32                                    : 1;
    SALReg32                            DRLCK   : 1;
    SALReg32                                    : 1;
    SALReg32                            TCN     : 1;
    SALReg32                            RCN     : 1;
    SALReg32                            HCC     : 1;
    SALReg32                                    : 1;
} I2SRegFieldTransCtrl_t;

typedef union I2SRegTransCtrl
{
    SALReg32                            nReg;
    I2SRegFieldTransCtrl_t              bReg;
} I2SRegTransCtrl_t;


typedef struct I2SRegFieldRptCtrl
{
    SALReg32                            RPTCNT  : 24;   //Repeat Count
    SALReg32                            DBTH    : 4;
    SALReg32                                    : 3;
    SALReg32                            DRI     : 1;    //Disable Repeat Interrupt
} I2SRegFieldRptCtrl_t;

typedef union {
    SALReg32                            nReg;
    I2SRegFieldRptCtrl_t                bReg;
} I2SRegRptCtrl_t;


typedef struct I2SRegFieldChCtrl
{
    SALReg32                            DTIEN   : 1;
    SALReg32                                    : 1;
    SALReg32                            DRIEN   : 1;
    SALReg32                                    : 1;
    SALReg32                            DTWB    : 1;
    SALReg32                                    : 1;
    SALReg32                            DRWB    : 1;
    SALReg32                                    : 1;
    SALReg32                            DTSEN   : 1;
    SALReg32                                    : 1;
    SALReg32                            DRSEN   : 1;
    SALReg32                                    : 1;
    SALReg32                            DTDW    : 1;
    SALReg32                                    : 1;
    SALReg32                            DRDW    : 1;
    SALReg32                                    : 1;
    SALReg32                            DTLR    : 1;
    SALReg32                                    : 1;
    SALReg32                            DRLR    : 1;
    SALReg32                                    : 1;
    SALReg32                            DMTSEL  : 2;
    SALReg32                            DMRSEL  : 2;
    SALReg32                            DTMCM   : 1;
    SALReg32                            DRMCM   : 1;
    SALReg32                                    : 2;
    SALReg32                            DTEN    : 1;
    SALReg32                                    : 1;
    SALReg32                            DREN    : 1;
    SALReg32                                    : 1;
} I2SRegFieldChCtrl_t;

typedef union
{
    SALReg32                            nReg;
    I2SRegFieldChCtrl_t                 bReg;
} I2SRegChCtrl_t;

typedef struct I2SRegFieldIntStatus
{
    SALReg32                            DTMI : 1;
    SALReg32                                 : 1;
    SALReg32                            DRMI : 1;
    SALReg32                                 : 1;
    SALReg32                            DTI  : 1;
    SALReg32                                 : 1;
    SALReg32                            DRI  : 1;
    SALReg32                                 : 25;
} I2SRegFieldIntStatus_t;

typedef union I2SRegIntStatus
{
    SALReg32                            nReg;
    I2SRegFieldIntStatus_t              bReg;
} I2SRegIntStatus_t;


/**************************************************************************************************
*                                             AUDIO_DAIF Register
**************************************************************************************************/

typedef struct I2SRegFieldDAMR
{
    SALReg32                            LB      : 1;
    SALReg32                                    : 2;
    SALReg32                            BP      : 1;
    SALReg32                                    : 3;
    SALReg32                            DFE     : 1;
    SALReg32                                    : 1;
    SALReg32                            FM      : 1;
    SALReg32                            BM      : 1;
    SALReg32                            SM      : 1;
    SALReg32                            MD      : 1;
    SALReg32                            RE      : 1;
    SALReg32                            TE      : 1;
    SALReg32                            EN      : 1;
    SALReg32                            SP      : 1;
    SALReg32                            LBT     : 1;
    SALReg32                            TXS     : 2;
    SALReg32                            RXS     : 2;
    SALReg32                            RXE     : 1;
    SALReg32                            NMDT    : 1;
    SALReg32                            NMDR    : 1;
    SALReg32                                    : 2;
    SALReg32                            AFE     : 1;
    SALReg32                            MPE     : 1;
    SALReg32                            DBTEN   : 1;
    SALReg32                            LPS     : 1;
    SALReg32                            BPS     : 1;
} I2SRegFieldDAMR_t;

typedef union I2SRegDAMR
{
  SALReg32                              nReg;
  I2SRegFieldDAMR_t                     bReg;
} I2SRegDAMR_t;


typedef struct I2SRegFieldMCCR0
{
    SALReg32                                    : 31;
    SALReg32                            DAO0M   : 1;
} I2SRegFieldMCCR0_t;

typedef union I2SRegMCCR0
{
    SALReg32                            nReg;
    I2SRegFieldMCCR0_t                  bReg;
} I2SRegMCCR0_t;

//DAIF FIFO cleaer Register (DRMR)
typedef enum I2SDRMR {
    RFC = 0x00100000,   //20bit, Rx Fifo Clear
    TFC = 0x00200000,   //21bit, Tx Fifo Clear
}I2SDRMR_t;

typedef struct I2SRegFieldDRMR
{
    SALReg32                                : 20;
    SALReg32                            TFC : 1;
    SALReg32                            RFC : 1;
    SALReg32                                : 10;
} I2SRegFieldDRMR_t;

typedef union I2SRegDRMR{
    SALReg32                            nReg;
    I2SRegFieldDRMR_t                bReg;
} I2SRegDRMR_t;

typedef struct I2SRegFieldDCLKDIV
{
    SALReg32                            FD  : 3;
    SALReg32                                : 5;
    SALReg32                            BD  : 4;
    SALReg32                                : 20;
} I2SRegFieldDCLKDIV_t;

typedef union I2SRegDCLKDIV
{
    SALReg32                            nReg;
    I2SRegFieldDCLKDIV_t                bReg;
} I2SRegDCLKDIV_t;

typedef struct I2SRegFieldTASCFG
{
    SALReg32                                 : 20;
    SALReg32                            TCLR : 1;
    SALReg32                                 : 10;
    SALReg32                            TEN  : 1;
} I2SRegFieldTASCFG_t;

typedef union I2SRegTASCFG
{
    SALReg32                            nReg;
    I2SRegFieldTASCFG_t                 bReg;
} I2SRegTASCFG_t;

typedef struct I2SRegFieldFIFOCTRL
{
    SALReg32                            TTXTH : 8;
    SALReg32                            TRXTH : 8;
    SALReg32                                  : 16;
} I2SRegFieldFIFOCTRL_t;

typedef union I2SRegFIFOCTRL
{
    SALReg32                            nReg;
    I2SRegFieldFIFOCTRL_t               bReg;
} I2SRegFIFOCTRL_t;


typedef struct I2SRegFieldERRSTSCTRL
{
    SALReg32                            ERRCNT      : 16;
    SALReg32                            ERRCLR      : 1;
    SALReg32                            ERRSTS      : 1;
    SALReg32                            ERRCHECK_EN : 1;
    SALReg32                                        : 13;
} I2SRegFieldERRSTSCTRL_t;

typedef union I2SRegERRSTSCTRL
{
    SALReg32                            nReg;
    I2SRegFieldERRSTSCTRL_t             bReg;
} I2SRegERRSTSCTRL_t;

typedef enum I2SRegADmaReset
{
    AdTXRst = 0x01,
    AdRXRst = 0x02,
    DaTXRst = 0x04,
    DaRXRst = 0x08
}I2SRegADmaReset_t;


// Audio Register Offset

#define Base_ADMA                       (0xA0880000U)    //Audio_DMA
#define Base_ADAIF                      (0xA0881000U)    //Audio_DAIF

//(Base Address Symbol: Audio_DMA (0xA0880000))
#define RxDaDar0                        (0x000UL)
#define RxDaParam                       (0x004UL)
#define RxDaTCnt                        (0x008UL)
#define RxDaCdar0                       (0x00CUL)
//Reserve 0x010~0x024
#define RxDaDarL0                       (0x028UL)
#define RxDaCdarL0                      (0x02CUL)
//Reserve 0x030~0x034
#define TransCtrl                       (0x038UL)
#define RptCtrl                         (0x03CUL)
#define TxDaSar0                        (0x040UL)
#define TxDaParam                       (0x044UL)
#define TxDaTCnt                        (0x048UL)
#define TxDaCsar0                       (0x04CUL)
//Reserve 0x050~0x064
#define TxDaSarL0                       (0x068UL)
#define TxDaCsarL0                      (0x06CUL)
//Reserve 0x070~0x074
#define ChCtrl                          (0x078UL)
#define IntStatus                       (0x07CUL)
//Reserve 0x080~0x084
#define TxDaAdrCnt                      (0x088UL)
#define RxDaAdrCnt                      (0x08CUL)
//Reserve 0x090~0x17C
#define ADMARST                         (0x180UL)

#define DADIR0                          (0x00UL)
#define DADIR1                          (0x04UL)
#define DADIR2                          (0x08UL)
#define DADIR3                          (0x0CUL)
#define DADIR4                          (0x10UL)
#define DADIR5                          (0x14UL)
#define DADIR6                          (0x18UL)
#define DADIR7                          (0x1CUL)
#define DADOR0                          (0x20UL)
#define DADOR1                          (0x24UL)
#define DADOR2                          (0x28UL)
#define DADOR3                          (0x2CUL)
#define DADOR4                          (0x30UL)
#define DADOR5                          (0x34UL)
#define DADOR6                          (0x38UL)
#define DADOR7                          (0x3CUL)
#define DAMR                            (0x40UL)
//Reserve 0x44
#define MCCR0                           (0x48UL)
#define DRMR                            (0x50UL)
#define DCLKDIV                         (0x54UL)
#define TASCFG                          (0x58UL)
#define FIFOCTRL                        (0x80UL)
#define ERRSTS_CTRL                     (0x8CUL)

/* ADMA Register START */
#define Reg_RxDaDar0    ((SALReg32          *)(Base_ADMA + RxDaDar0))
#define Reg_RxDaParam   ((I2SRegDaParam_t   *)(Base_ADMA + RxDaParam))
#define Reg_RxDaTCnt    ((I2SRegDaTCnt_t    *)(Base_ADMA + RxDaTCnt))
#define Reg_RxDaCdar0   ((SALReg32          *)(Base_ADMA + RxDaCdar0))
#define Reg_RxDaDarL0   ((SALReg32          *)(Base_ADMA + RxDaDarL0))
#define Reg_RxDaCdarL0  ((SALReg32          *)(Base_ADMA + RxDaCdarL0))

#define Reg_TransCtrl   ((I2SRegTransCtrl_t *)(Base_ADMA + TransCtrl))
#define Reg_RptCtrl     ((I2SRegRptCtrl_t   *)(Base_ADMA + RptCtrl))

#define Reg_TxDaSar0    ((SALReg32          *)(Base_ADMA + TxDaSar0))
#define Reg_TxDaParam   ((I2SRegDaParam_t   *)(Base_ADMA + TxDaParam))
#define Reg_TxDaTCnt    ((I2SRegDaTCnt_t    *)(Base_ADMA + TxDaTCnt))
#define Reg_TxDaCsar0   ((SALReg32          *)(Base_ADMA + TxDaCsar0))
#define Reg_TxDaSarL0   ((SALReg32          *)(Base_ADMA + TxDaSarL0))
#define Reg_TxDaCsarL0  ((SALReg32          *)(Base_ADMA + TxDaCsarL0))

#define Reg_ChCtrl      ((I2SRegChCtrl_t    *)(Base_ADMA + ChCtrl))
#define Reg_IntStatus   ((I2SRegIntStatus_t *)(Base_ADMA + IntStatus))

#define Reg_TxDaAdrCnt  ((SALReg32          *)(Base_ADMA + TxDaAdrCnt))
#define Reg_RxDaAdrCnt  ((SALReg32          *)(Base_ADMA + RxDaAdrCnt))

#define Reg_ADMARST     ((SALReg32          *)(Base_ADMA + ADMARST))
/* ADMA Register END */


/* ADAIF Register START */
#define Reg_DAMR        ((I2SRegDAMR_t      *)(Base_ADAIF + DAMR))
#define Reg_MCCR0       ((I2SRegMCCR0_t     *)(Base_ADAIF + MCCR0))
#define Reg_DRMR        ((SALReg32          *)(Base_ADAIF + DRMR))
#define Reg_DCLKDIV     ((I2SRegDCLKDIV_t   *)(Base_ADAIF + DCLKDIV))
#define Reg_TASCFG      ((I2SRegTASCFG_t    *)(Base_ADAIF + TASCFG))
#define Reg_FIFOCTRL    ((I2SRegFIFOCTRL_t  *)(Base_ADAIF + FIFOCTRL))
#define Reg_ErrStsCtrl  ((I2SRegERRSTSCTRL_t  *)(Base_ADAIF + ERRSTS_CTRL))
#define Reg_TestMode    ((SALReg32          *)(Base_ADAIF + TESTMODE))


/* ADAIF Register END */

#endif  // ( MCU_BSP_SUPPORT_DRIVER_I2S == 1 )

#endif  // MCU_BSP_I2S_REG_HEADER


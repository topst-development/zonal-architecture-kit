// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : fmu.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_FMU_HEADER
#define MCU_BSP_FMU_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                            GLOBAL DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
    #include "debug.h"
    #define FMU_D(fmt,args...)          {LOGD(DBG_TAG_FMU, fmt, ## args)}
    #define FMU_E(fmt,args...)          {LOGE(DBG_TAG_FMU, fmt, ## args)}
#else
    #define FMU_D(fmt,args...)          {}
    #define FMU_E(fmt,args...)          {}
#endif

#define FMU_FAULT_SOURCE                (64UL)

#define FMU_BASE_ADDR                   (MCU_BSP_FMU_BASE)
#define FMU_REG                         ((volatile FMUMc_t *)(FMU_BASE_ADDR))

#define FMU_PW                          (0x5afeace5UL)
#define FMU_PW_ERROR                    (0x5afe8031UL)

#define FMU_FAULT_ACTIVE_LOW            (0)
#define FMU_FAULT_ACTIVE_HIGH           (1)

#define FMU_WriteReg(a,v)               (SAL_WriteReg(v,a))
#define FMU_ReadReg(a)                  (SAL_ReadReg(a))


// FMU fault ID enumeration
typedef enum FMUFaultid
{
    FMU_ID_ADC0                         =  0,
    FMU_ID_ADC1                         =  1,
    FMU_ID_GPSB0                        =  2,
    FMU_ID_GPSB1                        =  3,
    FMU_ID_GPSB2                        =  4,
    FMU_ID_GPSB3                        =  5,
    FMU_ID_GPSB4                        =  6,
    FMU_ID_PDM0                         =  7,
    FMU_ID_PDM1                         =  8,
    FMU_ID_PDM2                         =  9,
    FMU_ID_MC_CFG                       = 10,
    FMU_ID_CR5_CFG                      = 11,
    FMU_ID_CCU_CFG                      = 12,
    FMU_ID_GPIO_CFG                     = 13,
    FMU_ID_CMU_CFG                      = 14,
    FMU_ID_SYS_SM_CFG                   = 15,
    FMU_ID_TIMER0                       = 16,
    FMU_ID_TIMER1                       = 17,
    FMU_ID_TIMER2                       = 18,
    FMU_ID_TIMER3                       = 19,
    FMU_ID_TIMER4                       = 20,
    FMU_ID_TIMER5                       = 21,
    FMU_ID_TIMER6                       = 22,
    FMU_ID_TIMER7                       = 23,
    FMU_ID_TIMER8                       = 24,
    FMU_ID_TIMER9                       = 25,
    FMU_ID_GMAC_TXMEM_ECC               = 26,
    FMU_ID_GMAC_RXMEM_ECC               = 27,
    FMU_ID_GMAC_SOFT_FAULT              = 28,
    FMU_ID_HSM_ROM_CRC                  = 29,
    FMU_ID_HSM_SOFT_FAULT               = 30,
    FMU_ID_SRAM0_ECC                    = 31,

    FMU_ID_SFMC_ECC                     = 32,
    FMU_ID_MBOX0_S_ECC                  = 33,
    FMU_ID_HSM_ECC                      = 34,
    FMU_ID_PFLASH                       = 35,
    FMU_ID_DFLASH                       = 36,
    FMU_ID_WDT                          = 37,
    FMU_ID_PLL0_CLK                     = 38,
    FMU_ID_PLL1_CLK                     = 39,
    FMU_ID_PERI_CLK                     = 40,
    FMU_ID_CPU_CLK                      = 41,
    FMU_ID_BUS_CLK                      = 42,
    FMU_ID_HSM                          = 43,
    FMU_ID_FLASH                        = 44,
    FMU_ID_ROM_CRC                      = 45,
    FMU_ID_PMU_WDT_FMU                  = 46,
    FMU_ID_OSC                          = 47,
    FMU_ID_VLD_SM0                      = 48,
    FMU_ID_VLD_SM1                      = 49,
    FMU_ID_VLD_SM2                      = 50,
    FMU_ID_VLD_SM3                      = 51,
    FMU_ID_VLD_SM4                      = 52,
    FMU_ID_VLD_SM5                      = 53,
    FMU_ID_VLD_SM6                      = 54,
    FMU_ID_VLD_SM7                      = 55,
    FMU_ID_VLD_SM8                      = 56,
    FMU_ID_VLD_SM9                      = 57,
    FMU_ID_VLD_SM10                     = 58,
    FMU_ID_VLD_SM11                     = 59,
    FMU_ID_VLD_SM12                     = 60,
    FMU_ID_SFMC1_ECC                    = 61,
    FMU_ID_AUDIO                        = 62,
    FMU_ID_FMU_FAULT                    = 63
} FMUFaultid_t;

// FMU fault severity level enumeration
typedef enum FMUSeverityLevelType
{
    FMU_SVL_LOW                         = 0,
    FMU_SVL_MID                         = 1,
    FMU_SVL_HIGH                        = 2,
    FMU_SVL_REV                         = 3
}FMUSeverityLevelType_t;


typedef enum FMUErrTypes
{
    FMU_OK                              = 0,
    FMU_FAIL                            = 1,
    FMU_INVALID_ID_ERR                  = 2,
    FMU_RESERVE_ID_ERR                  = 3,
    FMU_INVALID_SEVERITY_ERR            = 4,
    FMU_ISR_HANDLER_ERR                 = 5,
    FMU_DS_COMPARE_FAIL_ERR             = 6,
    FMU_INTSET_ERR                      = 7,
    FMU_INTEN_ERR                       = 8,
    FMU_INTDIS_ERR                      = 9
}FMUErrTypes_t;


/* FMU_CTRL */
typedef struct FMUCtrl
{
    uint32                              ctForceFault        :  1;   //  [   00], RW
    uint32                              ctSwReset           :  1;   //  [   01], RW
    uint32                              ctSoftFaultTestEn   :  1;   //  [   02], RW
    uint32                              ctSoftFaultCheckEn  :  1;   //  [   03], RW
    uint32                              ctPeriodicFaultEn   :  1;   //  [   04], RW
    uint32                              ctReserved          : 27;   //  [31:05]
} FMUCtrl_t;

typedef union FMUCtrlUnion
{
    uint32                              cuNreg;
    FMUCtrl_t                           cuBreg;
} FMUCtrlUnion_t;

/* FMU_EN */
typedef struct FMUEn
{
    uint32                              enAdc0              :  1;   //  [0][   00], RW
    uint32                              enAdc1              :  1;   //  [0][   01], RW
    uint32                              enGpsb0             :  1;   //  [0][   02], RW
    uint32                              enGpsb1             :  1;   //  [0][   03], RW
    uint32                              enGpsb2             :  1;   //  [0][   04], RW
    uint32                              enGpsb3             :  1;   //  [0][   05], RW
    uint32                              enGpsb4             :  1;   //  [0][   06], RW
    uint32                              enPdm0              :  1;   //  [0][   07], RW
    uint32                              enPdm1              :  1;   //  [0][   08], RW
    uint32                              enPdm2              :  1;   //  [0][   09], RW
    uint32                              enMcCfg             :  1;   //  [0][   10], RW
    uint32                              enCr5Cfg            :  1;   //  [0][   11], RW
    uint32                              enCcuCfg            :  1;   //  [0][   12], RW
    uint32                              enGpioCfg           :  1;   //  [0][   13], RW
    uint32                              enCmuCfg            :  1;   //  [0][   14], RW
    uint32                              enSysSmCfg          :  1;   //  [0][   15], RW
    uint32                              enTimer0            :  1;   //  [0][   16], RW
    uint32                              enTimer1            :  1;   //  [0][   17], RW
    uint32                              enTimer2            :  1;   //  [0][   18], RW
    uint32                              enTimer3            :  1;   //  [0][   19], RW
    uint32                              enTimer4            :  1;   //  [0][   20], RW
    uint32                              enTimer5            :  1;   //  [0][   21], RW
    uint32                              enTimer6            :  1;   //  [0][   22], RW
    uint32                              enTimer7            :  1;   //  [0][   23], RW
    uint32                              enTimer8            :  1;   //  [0][   24], RW
    uint32                              enTimer9            :  1;   //  [0][   25], RW
    uint32                              enGmacTxmemEcc      :  1;   //  [0][   26], RW
    uint32                              enGmacRxmemEcc      :  1;   //  [0][   27], RW
    uint32                              enGmacSoftFault     :  1;   //  [0][   28], RW
    uint32                              enHsmRomCrc         :  1;   //  [0][   29], RW
    uint32                              enHsmSoftFault      :  1;   //  [0][   30], RW
    uint32                              enSramEcc           :  1;   //  [0][   31], RW
    uint32                              enSfmcEcc           :  1;   //  [1][   00], RW
    uint32                              enMboxSEcc          :  1;   //  [1][   01], RW
    uint32                              enHsmEcc            :  1;   //  [1][   02], RW
    uint32                              enProgEflash        :  1;   //  [1][   03], RW
    uint32                              enDataEflash        :  1;   //  [1][   04], RW
    uint32                              enWdt               :  1;   //  [1][   05], RW
    uint32                              enPll0Clk           :  1;   //  [1][   06], RW
    uint32                              enPll1Clk           :  1;   //  [1][   07], RW
    uint32                              enPeriClk           :  1;   //  [1][   08], RW
    uint32                              enCpuClk            :  1;   //  [1][   09], RW
    uint32                              enBusClk            :  1;   //  [1][   10], RW
    uint32                              enHsmClk            :  1;   //  [1][   11], RW
    uint32                              enEflashClk         :  1;   //  [1][   12], RW
    uint32                              enRomCrc            :  1;   //  [1][   13], RW
    uint32                              enPmuWdtFmu         :  1;   //  [1][   14], RW
    uint32                              enXinSmFmu          :  1;   //  [1][   15], RW
    uint32                              enVldSmFmu0         :  1;   //  [1][   16], RW
    uint32                              enVldSmFmu1         :  1;   //  [1][   17], RW
    uint32                              enVldSmFmu2         :  1;   //  [1][   18], RW
    uint32                              enVldSmFmu3         :  1;   //  [1][   19], RW
    uint32                              enVldSmFmu4         :  1;   //  [1][   20], RW
    uint32                              enVldSmFmu5         :  1;   //  [1][   21], RW
    uint32                              enVldSmFmu6         :  1;   //  [1][   22], RW
    uint32                              enVldSmFmu7         :  1;   //  [1][   23], RW
    uint32                              enVldSmFmu8         :  1;   //  [1][   24], RW
    uint32                              enVldSmFmu9         :  1;   //  [1][   25], RW
    uint32                              enVldSmFm10         :  1;   //  [1][   26], RW
    uint32                              enVldSmFmu11        :  1;   //  [1][   27], RW
    uint32                              enVldSmFmu12        :  1;   //  [1][   28], RW
    uint32                              enSfmc1Ecc          :  1;   //  [1][   29], RW
    uint32                              enAudio             :  1;   //  [1][   30], RW
    uint32                              enReserved          :  1;   //  [1][   31]
} FMUEn_t;

typedef union FMUEnUnion
{
    uint32                              euNreg[2];
    FMUEn_t                             euBreg;
} FMUEnUnion_t;

typedef struct FMUFaultTiming
{
    uint32                              ftPolarity          :  1;   //  [   00], RW
    uint32                              ftReserved0         :  3;   //  [03:01], RW
    uint32                              ftLow               :  4;   //  [07:04], RW
    uint32                              ftHigh              :  3;   //  [10:08], RW
    uint32                              ftReserved1         : 21;   //  [31:11]
} FMUFaultTiming_t;

typedef union FMUFaultTimingUnion
{
    uint32                              ftNreg;
    FMUFaultTiming_t                    ftBreg;
} FMUFaultTimingUnion_t;

// FMU_SEVERITY_LEVEL
typedef struct FMUSeverityLevel
{
    uint32                              slAdc0              :  1;   //  [0][   00], RW
    uint32                              slAdc1              :  1;   //  [0][   01], RW
    uint32                              slGpsb0             :  1;   //  [0][   02], RW
    uint32                              slGpsb1             :  1;   //  [0][   03], RW
    uint32                              slGpsb2             :  1;   //  [0][   04], RW
    uint32                              slGpsb3             :  1;   //  [0][   05], RW
    uint32                              slGpsb4             :  1;   //  [0][   06], RW
    uint32                              slPdm0              :  1;   //  [0][   07], RW
    uint32                              slPdm1              :  1;   //  [0][   08], RW
    uint32                              slPdm2              :  1;   //  [0][   09], RW
    uint32                              slMcCfg             :  1;   //  [0][   10], RW
    uint32                              slCr5Cfg            :  1;   //  [0][   11], RW
    uint32                              slCcuCfg            :  1;   //  [0][   12], RW
    uint32                              slGpioCfg           :  1;   //  [0][   13], RW
    uint32                              slCmuCfg            :  1;   //  [0][   14], RW
    uint32                              slSysSmCfg          :  1;   //  [0][   15], RW
    uint32                              slTimer0            :  1;   //  [0][   16], RW
    uint32                              slTimer1            :  1;   //  [0][   17], RW
    uint32                              slTimer2            :  1;   //  [0][   18], RW
    uint32                              slTimer3            :  1;   //  [0][   19], RW
    uint32                              slTimer4            :  1;   //  [0][   20], RW
    uint32                              slTimer5            :  1;   //  [0][   21], RW
    uint32                              slTimer6            :  1;   //  [0][   22], RW
    uint32                              slTimer7            :  1;   //  [0][   23], RW
    uint32                              slTimer8            :  1;   //  [0][   24], RW
    uint32                              slTimer9            :  1;   //  [0][   25], RW
    uint32                              slGmacTxmemEcc      :  1;   //  [0][   26], RW
    uint32                              slGmacRxmemEcc      :  1;   //  [0][   27], RW
    uint32                              slGmacSoftFault     :  1;   //  [0][   28], RW
    uint32                              slHsmRomCrc         :  1;   //  [0][   29], RW
    uint32                              slHsmSoftFault      :  1;   //  [0][   30], RW
    uint32                              slSramEcc           :  1;   //  [0][   31], RW
    uint32                              slSfmcEcc           :  1;   //  [1][   00], RW
    uint32                              slMboxSEcc          :  1;   //  [1][   01], RW
    uint32                              slHsmEcc            :  1;   //  [1][   02], RW
    uint32                              slProgEflash        :  1;   //  [1][   03], RW
    uint32                              slDataEflash        :  1;   //  [1][   04], RW
    uint32                              slWdt               :  1;   //  [1][   05], RW
    uint32                              slPll0Clk           :  1;   //  [1][   06], RW
    uint32                              slPll1Clk           :  1;   //  [1][   07], RW
    uint32                              slPeriClk           :  1;   //  [1][   08], RW
    uint32                              slCpuClk            :  1;   //  [1][   09], RW
    uint32                              slBusClk            :  1;   //  [1][   10], RW
    uint32                              slHsmClk            :  1;   //  [1][   11], RW
    uint32                              slEflashClk         :  1;   //  [1][   12], RW
    uint32                              slRomCrc            :  1;   //  [1][   13], RW
    uint32                              slPmuWdtFmu         :  1;   //  [1][   14], RW
    uint32                              slXinSmFmu          :  1;   //  [1][   15], RW
    uint32                              slVldSmFmu0         :  1;   //  [1][   16], RW
    uint32                              slVldSmFmu1         :  1;   //  [1][   17], RW
    uint32                              slVldSmFmu2         :  1;   //  [1][   18], RW
    uint32                              slVldSmFmu3         :  1;   //  [1][   19], RW
    uint32                              slVldSmFmu4         :  1;   //  [1][   20], RW
    uint32                              slVldSmFmu5         :  1;   //  [1][   21], RW
    uint32                              slVldSmFmu6         :  1;   //  [1][   22], RW
    uint32                              slVldSmFmu7         :  1;   //  [1][   23], RW
    uint32                              slVldSmFmu8         :  1;   //  [1][   24], RW
    uint32                              slVldSmFmu9         :  1;   //  [1][   25], RW
    uint32                              slVldSmFmu10        :  1;   //  [1][   26], RW
    uint32                              slVldSmFmu11        :  1;   //  [1][   27], RW
    uint32                              slVldSmFmu12        :  1;   //  [1][   28], RW
    uint32                              slSfmc1Ecc          :  1;   //  [1][   29], RW
    uint32                              slAudio             :  1;   //  [1][   30], RW
    uint32                              slReserved          :  1;   //  [1][   31]
} FMUSeverityLevel_t;

typedef union FMUSeverityLevelUnion
{
    uint32                              slNreg[4];
    FMUSeverityLevel_t                  slBreg;
} FMUSeverityLevelUnion_t;

// FMU_CLK_SEL
typedef struct FMUClkSel
{
    uint32                              csClkSel            :  1;   //  [   00], RW
    uint32                              csReserved          : 31;   //  [31:01]
} FMUClkSel_t;

typedef union FMUClkSelUnion
{
    uint32                              csNreg;
    FMUClkSel_t                         csBreg;
} FMUClkSelUnion_t;

// FMU_OUT_STATUS
typedef struct FMUOutStatus
{
    uint32                              osIrq               :  1;   //  [   00], RO
    uint32                              osFiq               :  1;   //  [   01], RO
    uint32                              osFault             :  1;   //  [   02], RO
    uint32                              osReserved0         : 13;   //  [15:03]
    uint32                              osFaultState        :  2;   //  [17:16], RO
    uint32                              osReserved1         : 14;   //  [31:18]
} FMUOutStatus_t;

typedef union FMUOutStatusUnion
{
    uint32                              osNreg;
    FMUOutStatus_t                      osBreg;
} FMUOutStatusUnion_t;

typedef struct FMUMc
{
    FMUCtrlUnion_t                      mcCtrl;             // 0x00
    FMUEnUnion_t                        mcEn;               // 0x04 ~ 0x08
    FMUFaultTimingUnion_t               mcFaultTiming;      // 0x0c
    FMUSeverityLevelUnion_t             mcSeverityLevel;    // 0x10 ~ 0x1c
    FMUEnUnion_t                        mcIrqMask;          // 0x20 ~ 0x24
    FMUEnUnion_t                        mcIrqClr;           // 0x28 ~ 0x2c
    FMUEnUnion_t                        mcFiqMask;          // 0x30 ~ 0x34
    FMUEnUnion_t                        mcFiqClr;           // 0x38 ~ 0x3c
    FMUEnUnion_t                        mcFaultMask;        // 0x40 ~ 0x44
    FMUEnUnion_t                        mcFaultClr;         // 0x48 ~ 0x4c
    FMUEnUnion_t                        mcIrqStatus;        // 0x50 ~ 0x54
    FMUEnUnion_t                        mcFiqStatus;        // 0x58 ~ 0x5c
    FMUEnUnion_t                        mcFaultStatus;      // 0x60 ~ 0x64
    FMUClkSelUnion_t                    mcClkSel;           // 0x68
    FMUOutStatusUnion_t                 mcOutStatus;        // 0x6c
    uint32                              mcCfgWrPw;          // 0x70
} FMUMc_t;

 // FMU_EN
typedef struct FMUSts
{
     FMUFaultid_t                       id;
     uint32                             cnt;
} FMUSts_t;


typedef void (*FMUIntFnctPtr)(void * pArg);

typedef struct FMUIntVectPtr
{
     FMUSeverityLevelType_t             severity_level;
     FMUIntFnctPtr                      func;
     void *                             arg;
} FMUIntVectPtr_t;


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          FMU_Init
*
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Init
(
    void
);

/*
***************************************************************************************************
*                                          FMU_IsrHandler
*
*
* @param    uiId [in]
* @param    uiSeverityLevel [in]
* @param    fnIntFnct [in]
* @param    pArg [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrHandler
(
    FMUFaultid_t                        uiId,
    FMUSeverityLevelType_t              uiSeverityLevel,
    FMUIntFnctPtr                       fnIntFnct,
    void *                              pArg
);

/*
***************************************************************************************************
*                                          FMU_Set
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Set
(
    FMUFaultid_t                        uiId
);

/*
***************************************************************************************************
*                                          FMU_IsrClr
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrClr
(
    FMUFaultid_t                        uiId
);


/*
***************************************************************************************************
*                                          FMU_Deinit
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Deinit
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

#endif  // MCU_BSP_FMU_HEADER


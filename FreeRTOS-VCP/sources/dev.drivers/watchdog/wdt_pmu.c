// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wdt_pmu.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#include "wdt_pmu.h"

#include <wdt_reg.h>
#include <reg_phys.h>
#include <bsp.h>
#include <gic.h>
#include <debug.h>
#include <pmu.h>

#include <fmu.h>

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/
#if (DEBUG_ENABLE)
// Deviation Record - MISRA C-2012 Rule 20.10, Preprocessing Directives
#   include <debug.h>
#   define WDTPMU_D(fmt, args...)       {LOGD(DBG_TAG_WDT, fmt, ## args)}
#   define WDTPMU_E(fmt, args...)       {LOGE(DBG_TAG_WDT, fmt, ## args)}
#else
#   define WDTPMU_D(fmt, args...)
#   define WDTPMU_E(fmt, args...)
#endif

#define WDTPMU_FMU_SEV_LV               (FMU_SVL_HIGH) //FMU_SVL_LOW,FMU_SVL_HIGH

// freq 12 MHz
#define WDTPMU_Time2Value(t)            ((t) * (12UL * 1000UL * 1000UL))

/* WTD time: second */
#define WDTPMU_IRQ_TIME                 (10UL)

#define WDTPMU_SM_FAULT_TEST            (SALDisabled)

#if (WDTPMU_SM_FAULT_TEST == SALEnabled)
#   define WDTPMU_RESET_TIME            (11UL)
#else
#   define WDTPMU_RESET_TIME            (24UL)
#endif

typedef enum WDTPMUStatusType
{
    WDTPMU_STS_NOT_INITIALIZED          = 0,
    WDTPMU_STS_INITIALIZED,
    WDTPMU_STS_STOPPED,
    WDTPMU_STS_STARTED,

} WDTPMUStatusType_t;

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/
static WDTPMUStatusType_t               gWdtPmuStatus = WDTPMU_STS_NOT_INITIALIZED;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void WDTPMU_Writel
(
    uint32                              uiVal,
    uint32                              uiAddr
);

static SALRetCode_t WDTPMU_SetTimeout
(
    uint32                              uiTimeout,
    uint32                              uiPretimeout
);

static void WDTPMU_EnableTimerIrq
(
    void
);

static void WDTPMU_DisableTimerIrq
(
    void
);

static void WDTPMU_IrqHandler
(
    void *                              pArgs
);

static void WDTPMU_Start
(
    void
);

/*
***************************************************************************************************
*                                          WDTPMU_Writel
*
* @param    uiVal [in]
* @param    uiAddr [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
// Deviation Record - HIS metric violation (HIS_CALLING)
static void WDTPMU_Writel
(
    uint32                              uiVal,
    uint32                              uiAddr
)
{
    /* Violation of MISRA C:2012 10.8 Inappropriate Cast Type: Expression*/
    uint32 reg = MCU_BSP_PMU_BASE + PMU_WR_PW;

    // Deviation Record - CERT INT36-C, CERT-C Integers
    SAL_WriteReg((uint32)PMU_PASSWORD, reg);
    SAL_WriteReg(uiVal, uiAddr);
}

/*
***************************************************************************************************
*                                          WDTPMU_SetTimeout
*
* @param    uiTimeout [in]
* @param    uiPretimeout [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t WDTPMU_SetTimeout
(
    uint32                              uiTimeout,
    uint32                              uiPretimeout
)
{
    SALRetCode_t ret;

    ret = SAL_RET_SUCCESS;

    if (uiPretimeout > uiTimeout)
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_WDT,
            WDTPMU_API_INIT,
            WDTPMU_ERR_INVALID_PARAM,
            __FUNCTION__
        );
    }
    else
    {
        WDTPMU_Writel(WDTPMU_Time2Value(uiPretimeout), MCU_BSP_PMU_BASE + WDTPMU_IRQ_CNT);
        WDTPMU_Writel(WDTPMU_Time2Value(uiTimeout),    MCU_BSP_PMU_BASE + WDTPMU_RSR_CNT);

        WDTPMU_D("Success to set time for PMU-WDT counter!\n");
    }

    return ret;
}

/*
***************************************************************************************************
*                                          WDTPMU_EnableTimerIrq
*
*
* Notes
*
***************************************************************************************************
*/
static void WDTPMU_EnableTimerIrq
(
    void
)
{
    /* enable watchdog1 timer interrupt */
    (void)GIC_IntSrcEn(GIC_PMU_WATCHDOG);
}

/*
***************************************************************************************************
*                                          WDTPMU_DisableTimerIrq
*
*
* Notes
*
***************************************************************************************************
*/
static void WDTPMU_DisableTimerIrq
(
    void
)
{
    /* disable watchdog timer interrupt */
    (void)GIC_IntSrcDis(GIC_PMU_WATCHDOG);
}

/*
***************************************************************************************************
*                                          WDTPMU_IrqHandler
*
* @param    pArgs [in]
*
* Notes
*
***************************************************************************************************
*/
static void WDTPMU_IrqHandler
(
    // Deviation Record - MISRA C-2012 Rule 8.13, Declarations and Definitions
    void *                              pArgs
)
{
    (void)pArgs;

#if (WDTPMU_SM_FAULT_TEST == SALEnabled)
    WDTPMU_D("\nPMU Windowed Watchdog Timer, Resetting System, This system will be reset in 11 seconds!!\n");
#else
    WDTPMU_KickPing();
#endif
}

/*
*********************************************************************************************************
*                                          WDTPMU_FmuHandler
*
* @param    pArgs [in]
*
* Notes
*
*********************************************************************************************************
*/
static void WDTPMU_FmuHandler
(
    // Deviation Record - MISRA C-2012 Rule 8.13, Declarations and Definitions
    void *                              pArg
)
{
    (void)pArg;

    WDTPMU_D("Handshake with FMU\n");
    // It could handle any external handler.
    (void)FMU_IsrClr(FMU_ID_PMU_WDT_FMU);
}

/*
***************************************************************************************************
*                                          WDTPMU_Start
*
*
* Notes
*
***************************************************************************************************
*/
static void WDTPMU_Start
(
    void
)
{
    WDTPMU_EnableTimerIrq();

    /* enable watchdog timer */
    WDTPMU_Writel(WDT_EN_ON, MCU_BSP_PMU_BASE + WDTPMU_EN);

    gWdtPmuStatus = WDTPMU_STS_STARTED;

    WDTPMU_D("Watchdog Started\n");
}

/*
***************************************************************************************************
*                                          WDTPMU_KickPing
*
* This function processes kick action to counter of PMU-WDT. It also clears and resets IRQ counter.
*
* Notes
*
***************************************************************************************************
*/
void WDTPMU_KickPing
(
    void
)
{
    // Ping, Kick Action
    WDTPMU_Writel(WDT_CLR_APPLY, MCU_BSP_PMU_BASE + WDTPMU_CLR);
}

/*
***************************************************************************************************
*                                          WDTPMU_ResetSystem
*
* This function reset or reboot system right away.
*
* Notes
*
***************************************************************************************************
*/
void WDTPMU_ResetSystem
(
    void
)
{
    WDTPMU_D("Reset MCU system right now!\n");

    WDTPMU_DisableTimerIrq();

    /* set the PMU watchdog timer reset request count to zero */
    WDTPMU_Writel(0x0U, MCU_BSP_PMU_BASE + WDTPMU_RSR_CNT);

    gWdtPmuStatus = WDTPMU_STS_STOPPED;
}

/*
***************************************************************************************************
*                                          WDTPMU_Stop
*
* This function stops PMU WDT counter and unregister interrupt handler.
*
* Notes
*
***************************************************************************************************
*/
void WDTPMU_Stop
(
    void
)
{
    WDTPMU_Writel(WDT_EN_OFF, MCU_BSP_PMU_BASE + WDTPMU_EN);

    WDTPMU_DisableTimerIrq();

    gWdtPmuStatus = WDTPMU_STS_STOPPED;

    WDTPMU_D("PMU Watchdog Stopped\n");
}

/*
***************************************************************************************************
*                                          WDTPMU_SmMode
*
* This function sets configuration of safety mechanism on PMU-WDT device.
*
* @param    ucUseFmu [in] The flag to use handshake with FMU. If true, handshake mode will be enabled.
* @param    uiUseHandler [in] When handshake with FMU, external FMU handler is available.
* @param    uiVote [in] 3 WDT reset request selection mode, selecting one of 3 reset request line
*                       or 2 out of 3 voted reset requests
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t WDTPMU_SmMode
(
    boolean             ucUseFmu,
    WDTPMUResetType_t   uiUseHandler,
    WDTPMU2oo3Type_t    uiVote
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;
    uint32          safety_mode = 0x000000000UL;

    safety_mode = (((ucUseFmu == TRUE) ? WDT_SM_MODE_CONT_EN : WDT_SM_MODE_CONT_DIS) | (uint32)uiUseHandler | (uint32)uiVote);

    PMU_EnableResetMonitor(PMU_MON_SRC_PMU_WDT);

    if ((ucUseFmu == TRUE) && (uiUseHandler == WDTPMU_RST_FMU_HANDLER))
    {
        /* WWDT sends timeout fault signal (RST_REQ) to fault management unit (FMU)
           after selected timeout signal have been activated. */
        (void)FMU_IsrHandler(FMU_ID_PMU_WDT_FMU, WDTPMU_FMU_SEV_LV, (FMUIntFnctPtr)&WDTPMU_FmuHandler, NULL_PTR);      // always FMU_OK

        ret = (FMU_Set(FMU_ID_PMU_WDT_FMU) == FMU_OK) ? SAL_RET_SUCCESS : SAL_RET_FAILED;

#if (WDTPMU_SM_FAULT_TEST == SALEnabled)
        /* Under normal circumstances, even if the FMU is set, the system reboots when the PMU is set.
           In other words, it is a safety mechanism to catch the fault situation where RST-REQ cannot
           be delivered to the PMU and handle it in the FMU handler.
           To test that the FMU signal has occurred, we can check it by disabling the PMU setting. */
        PMU_DisableResetMonitor(PMU_MON_SRC_PMU_WDT);
#endif
    }

    WDTPMU_Writel(safety_mode, MCU_BSP_PMU_BASE + WDTPMU_SM_MODE);

    //wdt_pr_dbg("safety_mode : 0x%08X", safety_mode);

    if (ret == SAL_RET_SUCCESS)
    {
        WDTPMU_D("Success to set safety mechanism!\n");
    }
    else
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_WDT,
            WDTPMU_API_SET_SM_MODE,
            WDTPMU_ERR_NOT_SUPPORT_MODE,
            __FUNCTION__
        );
    }

    return ret;
}

/*
***************************************************************************************************
*                                          WDTPMU_Init
*
* This function initializes configuration registers, registers interrupt handler for kick action,
* and starts PMU-WDT counter.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t WDTPMU_Init
(
    void
)
{
    SALRetCode_t ret;

    ret = SAL_RET_SUCCESS;

    if (gWdtPmuStatus == WDTPMU_STS_NOT_INITIALIZED)
    {
        /* WDT clock rate is fixed to 24 MHz. Unnecessary to set clock */
       (void)WDTPMU_SmMode(TRUE, WDTPMU_RST_DIRECT, WDTPMU_2OO3_VOTE);

        ret = WDTPMU_SetTimeout(WDTPMU_RESET_TIME, WDTPMU_IRQ_TIME);

        if (ret == SAL_RET_SUCCESS)
        {
            (void)GIC_IntVectSet
            (
                GIC_PMU_WATCHDOG,
                GIC_PRIORITY_NO_MEAN,
                GIC_INT_TYPE_EDGE_RISING,
                (GICIsrFunc)&WDTPMU_IrqHandler,
                NULL_PTR
            );
            gWdtPmuStatus = WDTPMU_STS_INITIALIZED;

            WDTPMU_Start();
        }
    }
    else if (gWdtPmuStatus == WDTPMU_STS_STOPPED)
    {
        WDTPMU_Start();
    }
    else
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_WDT,
            WDTPMU_API_INIT,
            WDTPMU_ERR_ALREADY_DONE,
            __FUNCTION__
        );
    }

    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )


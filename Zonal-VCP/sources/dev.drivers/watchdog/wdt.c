// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wdt.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#include "wdt.h"

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
#   define WDT_D(fmt, args...)          {LOGD(DBG_TAG_WDT, fmt, ## args)}
#   define WDT_E(fmt, args...)          {LOGE(DBG_TAG_WDT, fmt, ## args)}
#else
#   define WDT_D(fmt, args...)
#   define WDT_E(fmt, args...)
#endif

#define WDT_FMU_SEV_LV                  (FMU_SVL_HIGH) //FMU_SVL_LOW,FMU_SVL_HIGH

// freq 12 MHz
#define WDT_Time2Value(t)               ((t) * (12UL * 1000UL * 1000UL))

/* WTD time: second */
#define WDT_IRQ_TIME                    (10UL)

#define WDT_SM_FAULT_TEST               (SALDisabled)

#if (WDT_SM_FAULT_TEST == SALEnabled)
#   define WDT_RESET_TIME               (11UL)
#else
#   define WDT_RESET_TIME               (24UL)
#endif

typedef enum WDTStatusType
{
    WDT_STS_NOT_INITIALIZED             = 0,
    WDT_STS_INITIALIZED,
    WDT_STS_STOPPED,
    WDT_STS_STARTED,

} WDTStatusType_t;

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/
static WDTStatusType_t                  gWdtStatus = WDT_STS_NOT_INITIALIZED;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void WDT_Writel
(
    uint32                              uiVal,
    uint32                              uiAddr
);

static SALRetCode_t WDT_SetTimeout
(
    uint32                              uiTimeout,
    uint32                              uiPretimeout
);

static void WDT_EnableTimerIrq
(
    void
);

static void WDT_DisableTimerIrq
(
    void
);

static void WDT_IrqHandler
(
    void *                              pArgs
);

static void WDT_Start
(
    void
);

/*
***************************************************************************************************
*                                          WDT_Writel
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
static void WDT_Writel
(
    uint32                              uiVal,
    uint32                              uiAddr
)
{
    /* Violation of MISRA C:2012 10.8 Inappropriate Cast Type: Expression*/
    uint32 reg = MCU_BSP_WDT_BASE + WDT_WR_PW;

    // Deviation Record - CERT INT36-C, CERT-C Integers
    SAL_WriteReg((uint32)WDT_PASSWORD, reg);
    SAL_WriteReg(uiVal, uiAddr);
}

/*
***************************************************************************************************
*                                          WDT_SetTimeout
*
* @param    uiTimeout [in]
* @param    uiPretimeout [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t WDT_SetTimeout
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
            WDT_API_INIT,
            WDT_ERR_INVALID_PARAM,
            __FUNCTION__
        );
    }
    else
    {
        WDT_Writel(WDT_Time2Value(uiPretimeout), MCU_BSP_WDT_BASE + WDT_IRQ_CNT);
        WDT_Writel(WDT_Time2Value(uiTimeout),    MCU_BSP_WDT_BASE + WDT_RSR_CNT);

        WDT_D("Success to set time for watchdog counter!\n");
    }

    return ret;
}

/*
***************************************************************************************************
*                                          WDT_EnableTimerIrq
*
*
* Notes
*
***************************************************************************************************
*/
static void WDT_EnableTimerIrq
(
    void
)
{
    /* enable watchdog timer interrupt */
    (void)GIC_IntSrcEn(GIC_WATCHDOG);
}

/*
***************************************************************************************************
*                                          WDT_DisableTimerIrq
*
*
* Notes
*
***************************************************************************************************
*/
static void WDT_DisableTimerIrq
(
    void
)
{
    /* disable watchdog timer interrupt */
    (void)GIC_IntSrcDis(GIC_WATCHDOG);
}

/*
***************************************************************************************************
*                                          WDT_IrqHandler
*
* @param    pArgs [in]
*
* Notes
*
***************************************************************************************************
*/
static void WDT_IrqHandler
(
    // Deviation Record - MISRA C-2012 Rule 8.13, Declarations and Definitions
    void *                              pArgs
)
{
    (void)pArgs;

#if (WDT_SM_FAULT_TEST == SALEnabled)
    WDT_D("\n[IP-PV] MC-18, Windowed Watchdog Timer, Resetting System, This system will be reset in 11 seconds!!\n");
#else
    WDT_KickPing();
#endif
}

/*
*********************************************************************************************************
*                                          WDT_FmuHandler
*
* @param    pArgs [in]
*
* Notes
*
*********************************************************************************************************
*/
static void WDT_FmuHandler
(
    // Deviation Record - MISRA C-2012 Rule 8.13, Declarations and Definitions
    void *                              pArg
)
{
    (void)pArg;

    WDT_D("Handshake with FMU\n");
    // It could handle any external handler.
    (void)FMU_IsrClr(FMU_ID_WDT);
}

/*
***************************************************************************************************
*                                          WDT_Start
*
*
* Notes
*
***************************************************************************************************
*/
static void WDT_Start
(
    void
)
{
    WDT_EnableTimerIrq();

    /* enable watchdog timer */
    WDT_Writel(WDT_EN_ON, MCU_BSP_WDT_BASE + WDT_EN);

    gWdtStatus = WDT_STS_STARTED;

    WDT_D("Watchdog Started\n");
}

/*
***************************************************************************************************
*                                          WDT_KickPing
*
* This function processes kick action to counter of WDT. It also clears and resets IRQ counter.
*
* Notes
*
***************************************************************************************************
*/
void WDT_KickPing
(
    void
)
{
    // Ping, Kick Action
    WDT_Writel(WDT_CLR_APPLY, MCU_BSP_WDT_BASE + WDT_CLR);
}

/*
***************************************************************************************************
*                                          WDT_ResetSystem
*
* This function reset or reboot system right away.
*
* Notes
*
***************************************************************************************************
*/
void WDT_ResetSystem
(
    void
)
{
    WDT_D("Reset MCU system right now!\n");

    WDT_DisableTimerIrq();

    /* set the watchdog timer reset request count to zero */
    WDT_Writel(0x0U, MCU_BSP_WDT_BASE + WDT_RSR_CNT);

    gWdtStatus = WDT_STS_STOPPED;
}

/*
***************************************************************************************************
*                                          WDT_Stop
*
* This function stops WDT counter and unregister interrupt handler.
*
* Notes
*
***************************************************************************************************
*/
void WDT_Stop
(
    void
)
{
    WDT_Writel(WDT_EN_OFF, MCU_BSP_WDT_BASE + WDT_EN);

    WDT_DisableTimerIrq();

    gWdtStatus = WDT_STS_STOPPED;

    WDT_D("Watchdog Stopped\n");
}

/*
***************************************************************************************************
*                                          WDT_SmMode
*
* This function sets configuration of safety mechanism on WDT device.
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
SALRetCode_t WDT_SmMode
(
    boolean         ucUseFmu,
    WDTResetType_t  uiUseHandler,
    WDT2oo3Type_t   uiVote
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;
    uint32          safety_mode = 0x000000000UL;

    safety_mode = (((ucUseFmu == TRUE) ? WDT_SM_MODE_CONT_EN : WDT_SM_MODE_CONT_DIS) | (uint32)uiUseHandler | (uint32)uiVote);

    PMU_EnableResetMonitor(PMU_MON_SRC_WDT);

    if ((ucUseFmu == TRUE) && (uiUseHandler == WDT_RST_FMU_HANDLER))
    {
        /* WWDT sends timeout fault signal (RST_REQ) to fault management unit (FMU)
           after selected timeout signal have been activated. */
        (void)FMU_IsrHandler(FMU_ID_WDT, WDT_FMU_SEV_LV, (FMUIntFnctPtr)&WDT_FmuHandler, NULL_PTR);      // always FMU_OK

        ret = (FMU_Set(FMU_ID_WDT) == FMU_OK) ? SAL_RET_SUCCESS : SAL_RET_FAILED;

#if (WDT_SM_FAULT_TEST == SALEnabled)
        /* Under normal circumstances, even if the FMU is set, the system reboots when the PMU is set.
           In other words, it is a safety mechanism to catch the fault situation where RST-REQ cannot
           be delivered to the PMU and handle it in the FMU handler.
           To test that the FMU signal has occurred, we can check it by disabling the PMU setting. */
        PMU_DisableResetMonitor(PMU_MON_SRC_WDT);
#endif
    }

    WDT_Writel(safety_mode, MCU_BSP_WDT_BASE + WDT_SM_MODE);

    //wdt_pr_dbg("safety_mode : 0x%08X", safety_mode);

    if (ret == SAL_RET_SUCCESS)
    {
        WDT_D("Success to set safety mechanism!\n");
    }
    else
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_WDT,
            WDT_API_SET_SM_MODE,
            WDT_ERR_NOT_SUPPORT_MODE,
            __FUNCTION__
        );
    }

    return ret;
}

/*
***************************************************************************************************
*                                          WDT_Init
*
* This function initializes configuration registers, registers interrupt handler for kick action,
* and starts WDT counter.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t WDT_Init
(
    void
)
{
    SALRetCode_t ret;

    ret = SAL_RET_SUCCESS;

    if (gWdtStatus == WDT_STS_NOT_INITIALIZED)
    {
        /* WDT clock rate is fixed to 24 MHz. Unnecessary to set clock */
       (void)WDT_SmMode(TRUE, WDT_RST_DIRECT, WDT_2OO3_VOTE);

        ret = WDT_SetTimeout(WDT_RESET_TIME, WDT_IRQ_TIME);

        if (ret == SAL_RET_SUCCESS)
        {
            (void)GIC_IntVectSet
            (
                GIC_WATCHDOG,
                GIC_PRIORITY_NO_MEAN,
                GIC_INT_TYPE_EDGE_RISING,
                (GICIsrFunc)&WDT_IrqHandler,
                NULL_PTR
            );
            gWdtStatus = WDT_STS_INITIALIZED;

            WDT_Start();
        }
    }
    else if (gWdtStatus == WDT_STS_STOPPED)
    {
        WDT_Start();
    }
    else
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_WDT,
            WDT_API_INIT,
            WDT_ERR_ALREADY_DONE,
            __FUNCTION__
        );
    }

    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )


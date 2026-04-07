// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : pmu_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )

#include <reg_phys.h>
#include <errno.h>
#include <stdlib.h>
#include <debug.h>
#include "pmu_test.h"


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/**************************************************************************************************/
/*                                             LOCAL VARIABLES                                    */
/**************************************************************************************************/
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void PMU_IrqHandler
(
     void *                              pArg
);

/*
***************************************************************************************************
*                                         IMPLEMENT
***************************************************************************************************
*/
static void PMU_IrqHandler
(
     void *                              pArg
)
{
    mcu_printf(" %d\n", *((uint32*)pArg));

    if(*((uint32*)pArg) == 47UL)
    {
        PMU_ClearPvtError(PMU_PVT_SRC_XIN_PVT);
    }
    else if(*((uint32*)pArg) >= 48UL)
    {
        PMU_ClearPvtError((*((uint32*)pArg) - 48UL) + PMU_PVT_SRC_PWR0_PVT);
    }

    (void)FMU_IsrClr((FMUFaultid_t) *((uint32*)pArg));
}

void PMU_TEST_FmuMonitoringTest
(
    uint32 uiCmd
)
{
    static uint32 uiOscId = (uint32)FMU_ID_OSC;
    static uint32 uiVldId[15] = {0UL,};
    uint32 uiIdx = 0UL;

    if(uiCmd == 1UL)
    {
        mcu_printf(" - PMU Reset XIN_PVT is diabled.\n");
        mcu_printf(" - PMU Fmu   XIN_PVT is enabled.\n");

        PMU_DisableResetMonitor(PMU_MON_SRC_XIN_PVT);

        (void)FMU_IsrHandler(FMU_ID_OSC, (FMUSeverityLevelType_t)FMU_SVL_MID,
                (FMUIntFnctPtr)&PMU_IrqHandler,
                &uiOscId);

        (void)FMU_Set(FMU_ID_OSC);
    }
    else if(uiCmd == 2UL)
    {
        mcu_printf(" - PMU Reset PWR_PVT is diabled.\n");
        mcu_printf(" - PMU Fmu   PWR_PVT is enabled.\n");

        for(uiIdx = 0UL; uiIdx < 15 ; uiIdx++)
        {
            PMU_DisableResetMonitor((PMUMonSrc_t)(uiIdx + (uint32)PMU_MON_SRC_PWR0_PVT));

            uiVldId[uiIdx] = uiIdx + (uint32)FMU_ID_VLD_SM0;

            (void)FMU_IsrHandler((FMUFaultid_t)(uiVldId[uiIdx]), (FMUSeverityLevelType_t)FMU_SVL_MID,
                    (FMUIntFnctPtr)&PMU_IrqHandler,
                    &(uiVldId[uiIdx]));

            (void)FMU_Set((FMUFaultid_t)(uiVldId[uiIdx]));
        }
    }
    else
    {
        mcu_printf("PMU Cmd2:  1     (FMU_ID_OSC        )\n");
        mcu_printf("PMU Cmd2:  2     (FMU_ID_VLD_SM     )\n");
    }
}

void PMU_TEST_GetRstStatus
(
    void
)
{
    mcu_printf(" - PMU_RESET_COLD      = %s.\n", PMU_GetResetOccur(PMU_SRC_COLD     ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_HSM_COLD  = %s.\n", PMU_GetResetOccur(PMU_SRC_HSM_COLD ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_FMU_FAULT = %s.\n", PMU_GetResetOccur(PMU_SRC_FMU_FAULT) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_WARM      = %s.\n", PMU_GetResetOccur(PMU_SRC_WARM     ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_HSM_WARM  = %s.\n", PMU_GetResetOccur(PMU_SRC_HSM_WARM ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_WDT       = %s.\n", PMU_GetResetOccur(PMU_SRC_WDT      ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PMU_WDT   = %s.\n", PMU_GetResetOccur(PMU_SRC_PMU_WDT  ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_HSM_WDT   = %s.\n", PMU_GetResetOccur(PMU_SRC_HSM_WDT  ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_XIN_PVT   = %s.\n", PMU_GetResetOccur(PMU_SRC_XIN_PVT  ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR0_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR0_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR1_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR1_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR2_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR2_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR3_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR3_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR4_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR4_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR5_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR5_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR6_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR6_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR7_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR7_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR8_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR8_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR9_PVT  = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR9_PVT ) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR10_PVT = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR10_PVT) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR11_PVT = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR11_PVT) == 1 ? "1 !" : "0");
    mcu_printf(" - PMU_RESET_PWR12_PVT = %s.\n", PMU_GetResetOccur(PMU_SRC_PWR12_PVT) == 1 ? "1 !" : "0");
}

void PMU_TEST_RstMonitoringTest
(
    uint32 uiCmd
)
{
    if(uiCmd >=2UL && uiCmd <= 25UL && uiCmd != 4UL && uiCmd != 9UL)
    {
        mcu_printf(" - PMU Reset %d is enabled.\n", uiCmd);
        PMU_EnableResetMonitor((PMUMonSrc_t)uiCmd);
    }
    else
    {
        mcu_printf("PMU Cmd2:  2      (PMU_SRC_HSM_COLD  )\n");
        mcu_printf("PMU Cmd2:  3      (PMU_SRC_FMU_FAULT )\n");
        mcu_printf("\n");
        mcu_printf("PMU Cmd2:  5      (PMU_SRC_HSM_WARM  )\n");
        mcu_printf("PMU Cmd2:  6      (PMU_SRC_WDT       )\n");
        mcu_printf("PMU Cmd2:  7      (PMU_SRC_PMU_WDT   )\n");
        mcu_printf("PMU Cmd2:  8      (PMU_SRC_HSM_WDT   )\n");
        mcu_printf("\n");
        mcu_printf("PMU Cmd2:  10     (PMU_SRC_XIN_PVT   )\n");
        mcu_printf("PMU Cmd2:  11     (PMU_SRC_PWR0_PVT  )\n");
        mcu_printf("PMU Cmd2:  12     (PMU_SRC_PWR1_PVT  )\n");
        mcu_printf("PMU Cmd2:  13     (PMU_SRC_PWR2_PVT  )\n");
        mcu_printf("PMU Cmd2:  14     (PMU_SRC_PWR3_PVT  )\n");
        mcu_printf("PMU Cmd2:  15     (PMU_SRC_PWR4_PVT  )\n");
        mcu_printf("PMU Cmd2:  16     (PMU_SRC_PWR5_PVT  )\n");
        mcu_printf("PMU Cmd2:  17     (PMU_SRC_PWR6_PVT  )\n");
        mcu_printf("PMU Cmd2:  18     (PMU_SRC_PWR7_PVT  )\n");
        mcu_printf("PMU Cmd2:  19     (PMU_SRC_PWR8_PVT  )\n");
        mcu_printf("PMU Cmd2:  20     (PMU_SRC_PWR9_PVT  )\n");
        mcu_printf("PMU Cmd2:  21     (PMU_SRC_PWR10_PVT )\n");
        mcu_printf("PMU Cmd2:  22     (PMU_SRC_PWR11_PVT )\n");
        mcu_printf("PMU Cmd2:  23     (PMU_SRC_PWR12_PVT )\n");
    }
}

void PMU_TEST_RstReqTest
(
    uint32 uiCmd
)
{
    if(uiCmd == 1)
    {
        mcu_printf(" - PMU SW ColdReset is generated.\n");
        PMU_ColdReset();
    }
    else if(uiCmd == 4)
    {
        mcu_printf(" - PMU SW WarmReset is generated.\n");
        PMU_WarmReset();
    }
    else
    {
        mcu_printf("PMU Cmd2:  1      (PMU_SRC_COLD      )\n");
        mcu_printf("PMU Cmd2:  4      (PMU_SRC_WARM      )\n");
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )


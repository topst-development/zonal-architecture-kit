// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : pmu_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef  MCU_BSP_PMU_TEST_HEAEDER
#define  MCU_BSP_PMU_TEST_HEAEDER

#if ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_PMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_PMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_PMU != 1 )

#include <pmu.h>
#include <fmu.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
void PMU_TEST_GetRstStatus
(
    void
);

void PMU_TEST_FmuMonitoringTest
(
    uint32 uiCmd
);

void PMU_TEST_RstMonitoringTest
(
    uint32 uiCmd
);

void PMU_TEST_RstReqTest
(
    uint32 uiCmd
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )

#endif  // MCU_BSP_PMU_TEST_HEAEDER


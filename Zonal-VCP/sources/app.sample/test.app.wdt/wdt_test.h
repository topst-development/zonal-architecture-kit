// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wdt_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_WDT_TEST_HEADER
#define MCU_BSP_WDT_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_WATCHDOG value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG != 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <sal_internal.h>

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          WDT_StartTest
*
* This function is to verify the operation of WDT
*
* @param    iMode [in] the verification of Watchdog driver development or Watchdog operation
* @param    uiDurationSec [in] Duration to check if this system is alive
*
* Notes
*
***************************************************************************************************
*/
void WDT_StartTest
(
    int32 iMode,
    uint32 uiDurationSec
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )

#endif  // MCU_BSP_WDT_TEST_HEADER


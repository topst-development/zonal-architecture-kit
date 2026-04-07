// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : timer_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_TIMER_TEST_HEADER
#define MCU_BSP_TIMER_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_WATCHDOG value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG != 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          TIMER_StartTimerTest
*
* This function is to verify the operation of Timer
*
* @param    iMode [in] The verification of Timer driver development or Timer operation
* @param    uiMinDurationSec [in] The minimum duration to be repeated
*
* Notes
*
***************************************************************************************************
*/
void TIMER_StartTimerTest
(
    int32 iMode,
    uint32 uiMinDurationSec
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )

#endif  // MCU_BSP_TIMER_TEST_HEADER


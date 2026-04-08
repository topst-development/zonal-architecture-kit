// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : lin_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_LIN_TEST_HEADER
#define MCU_BSP_LIN_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1)

#if ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_UART value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_LIN != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_LIN value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_LIN != 1 )

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/


void LIN_TestUsage
(
    void
);

void LIN_SampleTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

void LIN_CreateAppTask
(
    void*                               pArg
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )

#endif  // MCU_BSP_LIN_TEST_HEADER


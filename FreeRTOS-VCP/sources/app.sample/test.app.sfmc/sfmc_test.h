// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : sfmc_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_SFMC_TEST_HEADER
#define MCU_BSP_SFMC_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_SFMC != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_SFMC value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_SFMC != 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

void SFMC_Test
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )

#endif  // MCU_BSP_SFMC_TEST_HEADER


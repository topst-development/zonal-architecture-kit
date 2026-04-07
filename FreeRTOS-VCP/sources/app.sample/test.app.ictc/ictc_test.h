// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : ictc_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_ICTC_TEST_HEADER
#define MCU_BSP_ICTC_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_ICTC != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_ICTC value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_ICTC != 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PDM != 1 )
    #error MCU_BSP_SUPPORT_TEST_APP_PDM value must be 1.
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PDM != 1 )

enum
{
    ICTC_PRD_DUTY_USING_KEY             = 0UL,
    ICTC_DUTY_USING_PDM                 = 1UL,
    ICTC_EDGE_USING_PDM                 = 2UL
};

/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

/*
***************************************************************************************************
*                                           FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                       ICTC_SelectTestCase
*
* @param    test case number
* @return
*
* Notes
*
***************************************************************************************************
*/

void ICTC_SelectTestCase
(
    uint32                              uiTestCase
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )

#endif  // MCU_BSP_ICTC_TEST_HEADER


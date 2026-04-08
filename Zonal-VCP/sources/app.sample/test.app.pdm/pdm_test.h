// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : pdm_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_PDM_TEST_HEADER
#define MCU_BSP_PDM_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_PDM != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_PDM value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_PDM != 1 )

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

#define PDM_CH_0                        (0UL)
#define PDM_OUT_SEL_CH                  (GPIO_PERICH_CH0) /* 0:GPIO-A, 1:GPIO-B, 2:GPIO-C, 3:GPIO-K */
#define PDM_CH3_FS                      (PMIO_GPK(8)|PMIO_GPK(9)|PMIO_GPK(10)|PMIO_GPK(11)|PMIO_GPK(12)|PMIO_GPK(13)|PMIO_GPK(14)|PMIO_GPK(15)|PMIO_GPK(16)|PMIO_GPK(17))

#define LED_PWR_EN                      (GPIO_GPC(11UL))


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                           PDM_SelectTestCase
*
* @param test case number
* @return
*
* Notes
*
***************************************************************************************************
*/

void PDM_SelectTestCase
(
    uint32                              uiTestCase
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )

#endif  // MCU_BSP_PDM_TEST_HEADER


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key_demo_adc.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_KEY_ADC_DEMO_HEADER
#define MCU_BSP_KEY_ADC_DEMO_HEADER

#if ( MCU_BSP_SUPPORT_APP_KEY_DEMO == 1 )

#include <sal_internal.h>

#define MAX_ADC_BTN                     (8)

/*
***************************************************************************************************
*                                          KEY_AdcScan_demo
*
*
* @param    piKeyVal [in]
* @param    piKPressed [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_DemoAdcScan
(
    uint8                               ucModNum
);

#endif  // ( MCU_BSP_SUPPORT_APP_KEY_DEMO == 1 )

#endif  // MCU_BSP_KEY_ADC_DEMO_HEADER


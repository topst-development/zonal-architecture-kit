// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : rtc_dev.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_RTC_DEV_HEADER
#define MCU_BSP_RTC_DEV_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <rtc.h>
#include <debug.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*RTC EXTRA FEATURE=====================================================*/
#ifdef RTC_CONF_DEBUG_ALONE
    #define RTC_FEATURE_USE_DEBUG_ALONE
#endif

/*=======================================================PMIO EXTRA FEATURE*/


#if defined(RTC_FEATURE_USE_DEBUG_ALONE)  || defined(DEBUG_ENABLE)
    #define RTC_D(fmt, args...)        {mcu_printf("[RTC][%s:%d] " fmt, \
                                                __func__, __LINE__, ## args);}
#else
    #define RTC_D(fmt, args...)
#endif

#define RTC_E(fmt, args...)        {mcu_printf("[RTC][%s:%d] Error ! " fmt, \
                                                __func__, __LINE__, ## args);}


typedef enum
{
    RTC_REG_PROT_TIME_LOCK              = 0,
    RTC_REG_PROT_ALARM_LOCK             = 1,
    RTC_REG_PROT_TIME_RELEASE           = 10,
    RTC_REG_PROT_ALARM_RELEASE          = 11,
} RTCRegProt_t;

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

#endif  // ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )

#endif  // MCU_BSP_RTC_DEV_HEADER


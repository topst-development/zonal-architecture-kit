// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : rtc.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef  MCU_BSP_RTC_HEADER
#define  MCU_BSP_RTC_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <sal_com.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

typedef enum
{
    RTC_DAY_SUN = 0,
    RTC_DAY_MON = 1,
    RTC_DAY_TUE = 2,
    RTC_DAY_WED = 3,
    RTC_DAY_THU = 4,
    RTC_DAY_FRI = 5,
    RTC_DAY_SAT = 6
} RTCDay_t;

typedef enum
{
    RTC_IRQ_DISABLE = 0,
    RTC_IRQ_ALARM   = 1,
    RTC_IRQ_PMWKUP  = 2
} RTCIrq_t;


/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

typedef void                            (* RTCHandler)(void);

/**************************************************************************************************/
/*                                         FUNCTION PROTOTYPES                                    */
/**************************************************************************************************/

void RTC_Init
(
        void
);

void RTC_SetTime
(
        uint32     uiYear,
        uint32     uiMon,
        uint32     uiDay,
        uint32     uiDate,
        uint32     uiHour,
        uint32     uiMin,
        uint32     uiSec
);

void RTC_GetTime
(
        uint32     *uiYear,
        uint32     *uiMon,
        uint32     *uiDay,
        uint32     *uiDate,
        uint32     *uiHour,
        uint32     *uiMin,
        uint32     *uiSec
);

void RTC_ClearTime
(
        void
);


void RTC_SetAlarm
(
        uint32     uiYear,
        uint32     uiMon,
        uint32     uiDay,
        uint32     uiDate,
        uint32     uiHour,
        uint32     uiMin,
        uint32     uiSec,
        RTCHandler fHandler
);

void RTC_EnableAlarm
(
        RTCIrq_t    tMode
);

void RTC_ClearAlarm
(
        void
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )

#endif  // MCU_BSP_RTC_HEADER


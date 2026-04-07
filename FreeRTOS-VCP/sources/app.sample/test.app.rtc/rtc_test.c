// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : rtc_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )

#include <reg_phys.h>
#include <errno.h>
#include <stdlib.h>
#include <debug.h>
#include "rtc_test.h"


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/**************************************************************************************************/
/*                                             LOCAL VARIABLES                                    */
/**************************************************************************************************/

static uint32 gRtcInit = 0UL;
static uint32 gAlarmCount = 0UL;

#define RTC_TEST_VA_TIME_YEAR           (2021UL)
#define RTC_TEST_VA_TIME_MON            (1UL)
#define RTC_TEST_VA_TIME_DAY            ((uint32)RTC_DAY_MON)
#define RTC_TEST_VA_TIME_DATE           (19UL)
#define RTC_TEST_VA_TIME_HOUR           (12UL)
#define RTC_TEST_VA_TIME_MIN            (34UL)
#define RTC_TEST_VA_TIME_SEC            (56UL)

#define RTC_TEST_VA_ALARM_YEAR          (0UL)
#define RTC_TEST_VA_ALARM_MON           (0UL)
#define RTC_TEST_VA_ALARM_DAY           (0UL)
#define RTC_TEST_VA_ALARM_DATE          (0UL)
#define RTC_TEST_VA_ALARM_HOUR          (0UL)
#define RTC_TEST_VA_ALARM_MIN           (0UL)
#define RTC_TEST_VA_ALARM_SEC           (5UL)

#define RTC_TEST_D(fmt, args...)        {mcu_printf("[RTC TEST][%s:%d] " fmt, \
                                            __func__, __LINE__, ## args);}
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void RTC_TEST_AlarmHandler
(
    void
);

/*
***************************************************************************************************
*                                         IMPLEMENT
***************************************************************************************************
*/
static void RTC_TEST_AlarmHandler
(
    void
)
{
    uint32     uiYear;
    uint32     uiMon;
    uint32     uiDay;
    uint32     uiDate;
    uint32     uiHour;
    uint32     uiMin;
    uint32     uiSec;

    gAlarmCount--;

    RTC_ClearAlarm();

    RTC_TEST_D("    Call Alarm Handler\n");

    RTC_GetTime( &uiYear, &uiMon, &uiDay, &uiDate, &uiHour, &uiMin, &uiSec );
    RTC_TEST_D("        Handler Get Time: [y]%d, [m]%d, [w]0x%03x, [d]%d, [h]%d, [m]%d, [s]%d\n",
            uiYear, uiMon, uiDay, uiDate, uiHour, uiMin, uiSec);

    if(gAlarmCount == 0UL)
    {
        RTC_EnableAlarm(RTC_IRQ_DISABLE);
        RTC_TEST_D("    Disable Alarm.\n");
    }
}

void RTC_TEST_Time
(
    void
)
{

    uint32     uiYear;
    uint32     uiMon;
    uint32     uiDay;
    uint32     uiDate;
    uint32     uiHour;
    uint32     uiMin;
    uint32     uiSec;

    if(gRtcInit == 0UL)
    {
        RTC_Init();
        gRtcInit = 1UL;
    }

    RTC_GetTime( &uiYear, &uiMon, &uiDay, &uiDate, &uiHour, &uiMin, &uiSec );

    RTC_TEST_D("Get Time: [y]%d, [m]%d, [w]0x%03x, [d]%d, [h]%d, [m]%d, [s]%d\n",
            uiYear, uiMon, uiDay, uiDate, uiHour, uiMin, uiSec);


    RTC_SetTime(
            RTC_TEST_VA_TIME_YEAR,
            RTC_TEST_VA_TIME_MON,
            RTC_TEST_VA_TIME_DAY,
            RTC_TEST_VA_TIME_DATE,
            RTC_TEST_VA_TIME_HOUR,
            RTC_TEST_VA_TIME_MIN,
            RTC_TEST_VA_TIME_SEC
            );

    RTC_TEST_D("Set Time: [y]%d, [m]%d, [w]0x%03x, [d]%d, [h]%d, [m]%d, [s]%d\n",
            RTC_TEST_VA_TIME_YEAR,
            RTC_TEST_VA_TIME_MON,
            RTC_TEST_VA_TIME_DAY,
            RTC_TEST_VA_TIME_DATE,
            RTC_TEST_VA_TIME_HOUR,
            RTC_TEST_VA_TIME_MIN,
            RTC_TEST_VA_TIME_SEC);

    RTC_GetTime( &uiYear, &uiMon, &uiDay, &uiDate, &uiHour, &uiMin, &uiSec );

    RTC_TEST_D("Get Time: [y]%d, [m]%d, [w]0x%03x, [d]%d, [h]%d, [m]%d, [s]%d\n",
            uiYear, uiMon, uiDay, uiDate, uiHour, uiMin, uiSec);


    RTC_SetTime(0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL);

    RTC_TEST_D("Clear Time:\n");


    RTC_GetTime( &uiYear, &uiMon, &uiDay, &uiDate, &uiHour, &uiMin, &uiSec );

    RTC_TEST_D("Get Time: [y]%d, [m]%d, [w]0x%03x, [d]%d, [h]%d, [m]%d, [s]%d\n",
            uiYear, uiMon, uiDay, uiDate, uiHour, uiMin, uiSec);
}

void RTC_TEST_Alarm
(
    uint32      uiCount
)
{
    uint32     uiYear;
    uint32     uiMon;
    uint32     uiDay;
    uint32     uiDate;
    uint32     uiHour;
    uint32     uiMin;
    uint32     uiSec;

    if(uiCount == 0UL)
    {
        RTC_TEST_D("Disable Alarm.\n");
        RTC_EnableAlarm(RTC_IRQ_DISABLE);
        RTC_ClearAlarm();
    }
    else
    {
        gAlarmCount = uiCount;

        if(gRtcInit == 0UL)
        {
            RTC_Init();
            gRtcInit = 1UL;
        }

        RTC_SetAlarm(
                RTC_TEST_VA_ALARM_YEAR,
                RTC_TEST_VA_ALARM_MON,
                RTC_TEST_VA_ALARM_DAY,
                RTC_TEST_VA_ALARM_DATE,
                RTC_TEST_VA_ALARM_HOUR,
                RTC_TEST_VA_ALARM_MIN,
                RTC_TEST_VA_ALARM_SEC,
                (RTCHandler)RTC_TEST_AlarmHandler
                );

        RTC_SetTime(0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL);

        RTC_EnableAlarm(RTC_IRQ_ALARM);



        RTC_GetTime( &uiYear, &uiMon, &uiDay, &uiDate, &uiHour, &uiMin, &uiSec );

        RTC_TEST_D("Get Time: [y]%d, [m]%d, [w]0x%03x, [d]%d, [h]%d, [m]%d, [s]%d\n",
                uiYear, uiMon, uiDay, uiDate, uiHour, uiMin, uiSec);

        RTC_TEST_D("Set Alarm: [y]%d, [m]%d, [w]0x%03x, [d]%d, [h]%d, [m]%d, [s]%d\n",
                uiYear +RTC_TEST_VA_ALARM_YEAR,
                uiMon + RTC_TEST_VA_ALARM_MON,
                uiDay + RTC_TEST_VA_ALARM_DAY,
                uiDate +RTC_TEST_VA_ALARM_DATE,
                uiHour +RTC_TEST_VA_ALARM_HOUR,
                uiMin + RTC_TEST_VA_ALARM_MIN,
                uiSec + RTC_TEST_VA_ALARM_SEC);

        RTC_TEST_D("Enable Alarm. Wait for %d sec\n", RTC_TEST_VA_ALARM_SEC);
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )


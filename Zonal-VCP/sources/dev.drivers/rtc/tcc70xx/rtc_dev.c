// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : rtc_dev.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )

#include <rtc.h>
#include <rtc_reg.h>
#include <rtc_dev.h>
#include <gic.h>
/*************************************************************************************************
 *                                             DEFINITIONS
 *************************************************************************************************/

/************************************************************************************************/
/*                                             STATIC FUNCTION                                  */
/************************************************************************************************/


RTCHandler gfHandler;

static void RTC_ProtectReg
(
    RTCRegProt_t                        uiLockType
);

static uint32 RTC_ConvData
(
    uint32                              uiAddr,
    uint32                              uiDec,
    uint32                              uiBcd
);

static void RTC_AlarmHandler
(
    void *pArg
);
/*************************************************************************************************/
/*                                             Implementation                                    */
/* ***********************************************************************************************/

static void RTC_ProtectReg
(
    RTCRegProt_t                        uiLockType
)
{
    static uint32 uiReleaseCnt = 0UL;

    switch(uiLockType)
    {
        case RTC_REG_PROT_TIME_LOCK:
            {
                if(uiReleaseCnt > 0UL)
                {
                    uiReleaseCnt--;
                }

                if(uiReleaseCnt == 0UL)
                {
                    RTC_REG_APPEND(
                            1UL,
                            RTC_ADDR_INTCON_FIELD_PROT,
                            RTC_ADDR_INTCON_FIELD_PROT_MASK,
                            RTC_ADDR_INTCON
                            );

                    RTC_REG_APPEND(
                            0UL,
                            RTC_ADDR_RTCCON_FIELD_STARTB,
                            RTC_ADDR_RTCCON_FIELD_STARTB_MASK,
                            RTC_ADDR_RTCCON
                            );

                    RTC_REG_APPEND(
                            0UL,
                            RTC_ADDR_RTCCON_FIELD_RTCWEN,
                            RTC_ADDR_RTCCON_FIELD_RTCWEN_MASK,
                            RTC_ADDR_RTCCON
                            );
                }
                break;
            }
        case RTC_REG_PROT_ALARM_LOCK:
            {
                if(uiReleaseCnt > 0UL)
                {
                    uiReleaseCnt--;
                }

                if(uiReleaseCnt == 0UL)
                {
                    RTC_REG_APPEND(
                            0UL,
                            RTC_ADDR_RTCCON_FIELD_RTCWEN,
                            RTC_ADDR_RTCCON_FIELD_RTCWEN_MASK,
                            RTC_ADDR_RTCCON
                            );
                }
                break;
            }
        case RTC_REG_PROT_TIME_RELEASE:
            {
                uiReleaseCnt++;

                RTC_REG_APPEND(
                        1UL,
                        RTC_ADDR_RTCCON_FIELD_RTCWEN,
                        RTC_ADDR_RTCCON_FIELD_RTCWEN_MASK,
                        RTC_ADDR_RTCCON
                        );

                RTC_REG_APPEND(
                        1UL,
                        RTC_ADDR_INTCON_FIELD_INTWREN,
                        RTC_ADDR_INTCON_FIELD_INTWREN_MASK,
                        RTC_ADDR_INTCON
                        );

                RTC_REG_APPEND(
                        1UL,
                        RTC_ADDR_RTCCON_FIELD_STARTB,
                        RTC_ADDR_RTCCON_FIELD_STARTB_MASK,
                        RTC_ADDR_RTCCON
                        );

                RTC_REG_APPEND(
                        0UL,
                        RTC_ADDR_INTCON_FIELD_PROT,
                        RTC_ADDR_INTCON_FIELD_PROT_MASK,
                        RTC_ADDR_INTCON
                        );
                break;
            }
        case RTC_REG_PROT_ALARM_RELEASE:
            {
                uiReleaseCnt++;

                RTC_REG_APPEND(
                        1UL,
                        RTC_ADDR_RTCCON_FIELD_RTCWEN,
                        RTC_ADDR_RTCCON_FIELD_RTCWEN_MASK,
                        RTC_ADDR_RTCCON
                        );

                RTC_REG_APPEND(
                        1UL,
                        RTC_ADDR_INTCON_FIELD_INTWREN,
                        RTC_ADDR_INTCON_FIELD_INTWREN_MASK,
                        RTC_ADDR_INTCON
                        );
                break;
            }
        default:
            {
                RTC_E("Unknown Protection type %d\n", uiLockType);
                break;
            }
    }
}
static uint32 RTC_ConvData
(
    uint32                              uiAddr,
    uint32                              uiDec,
    uint32                              uiBcd
)
{
    uint32 uiCalVa1;
    uint32 uiCalVa2;
    uint32 uiCalVa3;
    uint32 uiCalVa4;
    uint32 uiConvVa;

    uiCalVa1        = 0UL;
    uiCalVa2        = 0UL;
    uiCalVa3        = 0UL;
    uiCalVa4        = 0UL;
    uiConvVa        = 0UL;

    if(uiDec != 0UL)
    {
        uiCalVa1 = (uiDec / 1000);
        uiCalVa2 = (uiDec % 1000) / 100;
        uiCalVa3 = (uiDec % 100) / 10;
        uiCalVa4 = (uiDec % 10);

        uiConvVa = ((uiCalVa1 << 12) | (uiCalVa2 << 8) | (uiCalVa3 << 4) | (uiCalVa4 << 0));
    }

    if(uiBcd != 0UL)
    {
        uiCalVa1 = (uiBcd >> 12) & 0x000F;
        uiCalVa2 = (uiBcd >> 8) & 0x000F;
        uiCalVa3 = (uiBcd >> 4) & 0x000F;
        uiCalVa4 = (uiBcd >> 0) & 0x000F;

        uiConvVa = ((uiCalVa1 * 1000) + (uiCalVa2 * 100) + (uiCalVa3 * 10) + (uiCalVa4 * 1));
    }
    return uiConvVa;
}

static void RTC_AlarmHandler
(
    void *pArg
)
{
    static uint32 uiErrPrint = 0UL;

    (void)pArg;

    if(gfHandler != NULL)
    {
        uiErrPrint = 0UL;
        gfHandler();
    }
    else
    {
        if(uiErrPrint == 0UL)
        {
            RTC_E("Alarm Handler is NULL\n");
            uiErrPrint = 1UL;
        }
    }

}

void RTC_Init
(
    void
)
{

    RTC_ProtectReg(RTC_REG_PROT_TIME_RELEASE);

    RTC_REG_APPEND(
                    0UL,
                    RTC_ADDR_INTCON_FIELD_XDRV,
                    RTC_ADDR_INTCON_FIELD_XDRV_MASK,
                    RTC_ADDR_INTCON
                );

    RTC_REG_APPEND(
                    0UL,
                    RTC_ADDR_INTCON_FIELD_FSEL,
                    RTC_ADDR_INTCON_FIELD_FSEL_MASK,
                    RTC_ADDR_INTCON
                );

    RTC_REG_APPEND(
                    0UL,
                    RTC_ADDR_RTCIM_FIELD_PWDN,
                    RTC_ADDR_RTCIM_FIELD_PWDN_MASK,
                    RTC_ADDR_RTCIM
                );

    RTC_ProtectReg(RTC_REG_PROT_TIME_LOCK);

    (void)GIC_IntVectSet(
            (uint32)GIC_RTC_ALARM,
            GIC_PRIORITY_NO_MEAN,
            GIC_INT_TYPE_LEVEL_HIGH,
            (GICIsrFunc)&RTC_AlarmHandler,
            NULL
            );

}

void RTC_SetTime
(
    uint32     uiYear,
    uint32     uiMon,
    uint32     uiDay,
    uint32     uiDate,
    uint32     uiHour,
    uint32     uiMin,
    uint32     uiSec
)
{
    RTC_ProtectReg(RTC_REG_PROT_TIME_RELEASE);

    RTC_REG_SET(
            RTC_ConvData(RTC_ADDR_BCDSEC, uiSec, 0UL),
            RTC_ADDR_BCDSEC_FIELD_SECDATA,
            RTC_ADDR_BCDSEC_FIELD_SECDATA_MASK,
            RTC_ADDR_BCDSEC
            );

    RTC_REG_SET(
            RTC_ConvData(RTC_ADDR_BCDMIN, uiMin, 0UL),
            RTC_ADDR_BCDMIN_FIELD_MINDATA,
            RTC_ADDR_BCDMIN_FIELD_MINDATA_MASK,
            RTC_ADDR_BCDMIN
            );

    RTC_REG_SET(
            RTC_ConvData(RTC_ADDR_BCDHOUR, uiHour, 0UL),
            RTC_ADDR_BCDHOUR_FIELD_HOURDATA,
            RTC_ADDR_BCDHOUR_FIELD_HOURDATA_MASK,
            RTC_ADDR_BCDHOUR
            );

    RTC_REG_SET(
            RTC_ConvData(RTC_ADDR_BCDDATE, uiDate, 0UL),
            RTC_ADDR_BCDDATE_FIELD_DATEDATA,
            RTC_ADDR_BCDDATE_FIELD_DATEDATA_MASK,
            RTC_ADDR_BCDDATE
            );

    RTC_REG_SET(
            uiDay,
            RTC_ADDR_BCDDAY_FIELD_DAYDATA,
            RTC_ADDR_BCDDAY_FIELD_DAYDATA_MASK,
            RTC_ADDR_BCDDAY
            );

    RTC_REG_SET(
            RTC_ConvData(RTC_ADDR_BCDMON, uiMon, 0UL),
            RTC_ADDR_BCDMON_FIELD_MONDATA,
            RTC_ADDR_BCDMON_FIELD_MONDATA_MASK,
            RTC_ADDR_BCDMON
            );

    RTC_REG_SET(
            RTC_ConvData(RTC_ADDR_BCDYEAR, uiYear, 0UL),
            RTC_ADDR_BCDYEAR_FIELD_YEARDATA,
            RTC_ADDR_BCDYEAR_FIELD_YEARDATA_MASK,
            RTC_ADDR_BCDYEAR
            );

    RTC_ProtectReg(RTC_REG_PROT_TIME_LOCK);
}

void RTC_GetTime
(
    uint32     *uiYear,
    uint32     *uiMon,
    uint32     *uiDay,
    uint32     *uiDate,
    uint32     *uiHour,
    uint32     *uiMin,
    uint32     *uiSec
)
{
    *uiYear     = RTC_ConvData(RTC_ADDR_BCDYEAR, 0UL, RTC_REG_BCDYEAR);
    *uiMon      = RTC_ConvData(RTC_ADDR_BCDMON , 0UL, RTC_REG_BCDMON);
    *uiDay      = RTC_REG_BCDDAY;
    *uiDate     = RTC_ConvData(RTC_ADDR_BCDDATE, 0UL, RTC_REG_BCDDATE);
    *uiHour     = RTC_ConvData(RTC_ADDR_BCDHOUR, 0UL, RTC_REG_BCDHOUR);
    *uiMin      = RTC_ConvData(RTC_ADDR_BCDMIN , 0UL, RTC_REG_BCDMIN);
    *uiSec      = RTC_ConvData(RTC_ADDR_BCDSEC , 0UL, RTC_REG_BCDSEC);
}

void RTC_SetAlarm
(
    uint32                              uiYear,
    uint32                              uiMon,
    uint32                              uiDay,
    uint32                              uiDate,
    uint32                              uiHour,
    uint32                              uiMin,
    uint32                              uiSec,
    RTCHandler                          fHandler
)
{
    RTC_ProtectReg(RTC_REG_PROT_ALARM_RELEASE);

    if(uiSec != 0UL)
    {
        RTC_REG_SET(
                RTC_ConvData(RTC_ADDR_BCDSEC, uiSec, 0UL),
                RTC_ADDR_ALMSEC_FIELD_SECDATA,
                RTC_ADDR_ALMSEC_FIELD_SECDATA_MASK,
                RTC_ADDR_ALMSEC
                );

        RTC_REG_APPEND(
                1,
                RTC_ADDR_RTCALM_FIELD_SECEN,
                RTC_ADDR_RTCALM_FIELD_SECEN_MASK,
                RTC_ADDR_RTCALM
                );
    }
    else
    {
        RTC_REG_APPEND(
                0,
                RTC_ADDR_RTCALM_FIELD_SECEN,
                RTC_ADDR_RTCALM_FIELD_SECEN_MASK,
                RTC_ADDR_RTCALM
                );
    }

    if(uiMin != 0UL)
    {
        RTC_REG_SET(
                RTC_ConvData(RTC_ADDR_BCDMIN, uiMin, 0UL),
                RTC_ADDR_ALMMIN_FIELD_MINDATA,
                RTC_ADDR_ALMMIN_FIELD_MINDATA_MASK,
                RTC_ADDR_ALMMIN
                );

        RTC_REG_APPEND(
                1,
                RTC_ADDR_RTCALM_FIELD_MINEN,
                RTC_ADDR_RTCALM_FIELD_MINEN_MASK,
                RTC_ADDR_RTCALM
                );
    }
    else
    {
        RTC_REG_APPEND(
                0,
                RTC_ADDR_RTCALM_FIELD_MINEN,
                RTC_ADDR_RTCALM_FIELD_MINEN_MASK,
                RTC_ADDR_RTCALM
                );
    }

    if(uiHour != 0UL)
    {
        RTC_REG_SET(
                RTC_ConvData(RTC_ADDR_BCDHOUR, uiHour, 0UL),
                RTC_ADDR_ALMHOUR_FIELD_HOURDATA,
                RTC_ADDR_ALMHOUR_FIELD_HOURDATA_MASK,
                RTC_ADDR_ALMHOUR
                );

        RTC_REG_APPEND(
                1,
                RTC_ADDR_RTCALM_FIELD_HOUREN,
                RTC_ADDR_RTCALM_FIELD_HOUREN_MASK,
                RTC_ADDR_RTCALM
                );
    }
    else
    {
        RTC_REG_APPEND(
                0,
                RTC_ADDR_RTCALM_FIELD_HOUREN,
                RTC_ADDR_RTCALM_FIELD_HOUREN_MASK,
                RTC_ADDR_RTCALM
                );
    }

    if(uiDate != 0UL)
    {
        RTC_REG_SET(
                RTC_ConvData(RTC_ADDR_BCDDATE, uiDate, 0UL),
                RTC_ADDR_ALMDATE_FIELD_DATEDATA,
                RTC_ADDR_ALMDATE_FIELD_DATEDATA_MASK,
                RTC_ADDR_ALMDATE
                );

        RTC_REG_APPEND(
                1,
                RTC_ADDR_RTCALM_FIELD_DATEEN,
                RTC_ADDR_RTCALM_FIELD_DATEEN_MASK,
                RTC_ADDR_RTCALM
                );
    }
    else
    {
        RTC_REG_APPEND(
                0,
                RTC_ADDR_RTCALM_FIELD_DATEEN,
                RTC_ADDR_RTCALM_FIELD_DATEEN_MASK,
                RTC_ADDR_RTCALM
                );
    }

    if(uiDay != 0UL)
    {
        RTC_REG_SET(
                uiDay,
                RTC_ADDR_ALMDAY_FIELD_DAYDATA,
                RTC_ADDR_ALMDAY_FIELD_DAYDATA_MASK,
                RTC_ADDR_ALMDAY
                );

        RTC_REG_APPEND(
                1,
                RTC_ADDR_RTCALM_FIELD_DAYEN,
                RTC_ADDR_RTCALM_FIELD_DAYEN_MASK,
                RTC_ADDR_RTCALM
                );
    }
    else
    {
        RTC_REG_APPEND(
                0,
                RTC_ADDR_RTCALM_FIELD_DAYEN,
                RTC_ADDR_RTCALM_FIELD_DAYEN_MASK,
                RTC_ADDR_RTCALM
                );
    }

    if(uiMon != 0UL)
    {
        RTC_REG_SET(
                RTC_ConvData(RTC_ADDR_BCDMON, uiMon, 0UL),
                RTC_ADDR_ALMMON_FIELD_MONDATA,
                RTC_ADDR_ALMMON_FIELD_MONDATA_MASK,
                RTC_ADDR_ALMMON
                );

        RTC_REG_APPEND(
                1,
                RTC_ADDR_RTCALM_FIELD_MONEN,
                RTC_ADDR_RTCALM_FIELD_MONEN_MASK,
                RTC_ADDR_RTCALM
                );
    }
    else
    {
        RTC_REG_APPEND(
                0,
                RTC_ADDR_RTCALM_FIELD_MONEN,
                RTC_ADDR_RTCALM_FIELD_MONEN_MASK,
                RTC_ADDR_RTCALM
                );
    }

    if(uiYear != 0UL)
    {
        RTC_REG_SET(
                RTC_ConvData(RTC_ADDR_BCDYEAR, uiYear, 0UL),
                RTC_ADDR_ALMYEAR_FIELD_YEARDATA,
                RTC_ADDR_ALMYEAR_FIELD_YEARDATA_MASK,
                RTC_ADDR_ALMYEAR
                );

        RTC_REG_APPEND(
                1,
                RTC_ADDR_RTCALM_FIELD_YEAREN,
                RTC_ADDR_RTCALM_FIELD_YEAREN_MASK,
                RTC_ADDR_RTCALM
                );
    }
    else
    {
        RTC_REG_APPEND(
                0,
                RTC_ADDR_RTCALM_FIELD_YEAREN,
                RTC_ADDR_RTCALM_FIELD_YEAREN_MASK,
                RTC_ADDR_RTCALM
                );
    }

    if(fHandler != NULL)
    {
        RTC_D("Set Alarm Handler.\n");
        gfHandler = fHandler;
    }

    RTC_ProtectReg(RTC_REG_PROT_ALARM_LOCK);
}

void RTC_EnableAlarm
(
    RTCIrq_t tMode
)
{
    uint32 uiIM;
    uint32 uiIE;

    if(tMode == RTC_IRQ_ALARM)
    {
        uiIM  = (0 << RTC_ADDR_RTCIM_FIELD_PWDN);
        uiIM |= (1 << RTC_ADDR_RTCIM_FIELD_WKUPMODE);
        uiIM |= (3 << RTC_ADDR_RTCIM_FIELD_INTMODE);

        uiIE = 1UL;

        (void)GIC_IntSrcEn(GIC_RTC_ALARM);
    }
    else if(tMode == RTC_IRQ_PMWKUP)
    {
        uiIM  = (1 << RTC_ADDR_RTCIM_FIELD_PWDN);
        uiIM |= (1 << RTC_ADDR_RTCIM_FIELD_WKUPMODE);
        uiIM |= (0 << RTC_ADDR_RTCIM_FIELD_INTMODE);

        uiIE = 1UL;
    }
    else
    {
        uiIM  = (0 << RTC_ADDR_RTCIM_FIELD_PWDN);
        uiIM |= (1 << RTC_ADDR_RTCIM_FIELD_WKUPMODE);
        uiIM |= (0 << RTC_ADDR_RTCIM_FIELD_INTMODE);

        uiIE = 0UL;

        (void)GIC_IntSrcDis(GIC_RTC_ALARM);
        RTC_SetAlarm(0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, NULL);
    }

    RTC_ProtectReg(RTC_REG_PROT_ALARM_RELEASE);

    RTC_REG_SET(
            0UL,
            RTC_ADDR_RTCPEND_FIELD_PEND,
            RTC_ADDR_RTCPEND_FIELD_PEND_MASK,
            RTC_ADDR_RTCPEND
            );

    RTC_REG_SET(
            uiIM,
            RTC_ADDR_RTCIM_FIELD_INTMODE,
            (RTC_ADDR_RTCIM_FIELD_PWDN_MASK |
             RTC_ADDR_RTCIM_FIELD_WKUPMODE_MASK |
             RTC_ADDR_RTCIM_FIELD_INTMODE_MASK),
            RTC_ADDR_RTCIM
            );

    RTC_REG_APPEND(
            uiIE,
            RTC_ADDR_RTCALM_FIELD_ALMEN,
            RTC_ADDR_RTCALM_FIELD_ALMEN_MASK,
            RTC_ADDR_RTCALM
            );

    RTC_REG_APPEND(
            1,
            RTC_ADDR_RTCCON_FIELD_AIOUTEN,
            RTC_ADDR_RTCCON_FIELD_AIOUTEN_MASK,
            RTC_ADDR_RTCCON
            );


    RTC_ProtectReg(RTC_REG_PROT_ALARM_LOCK);
}

void RTC_ClearAlarm
(
    void
)
{
    RTC_ProtectReg(RTC_REG_PROT_ALARM_RELEASE);

    RTC_REG_SET(
            0UL,
            RTC_ADDR_RTCPEND_FIELD_PEND,
            RTC_ADDR_RTCPEND_FIELD_PEND_MASK,
            RTC_ADDR_RTCPEND
            );

    RTC_ProtectReg(RTC_REG_PROT_ALARM_LOCK);
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )


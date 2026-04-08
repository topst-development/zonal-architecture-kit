// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key_adc.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_KEY_ADC_HEADER
#define MCU_BSP_KEY_ADC_HEADER

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/


#define TCC70XX_KEY
#if ( MCU_BSP_SUPPORT_APP_KEY_DEMO == 1 )
#define EXTERNAL_KEY
#else
//#define EXTERNAL_KEY
#define INTERNAL_KEY
#endif  // ( MCU_BSP_SUPPORT_APP_KEY_DEMO == 1 )

#if defined(TCC803X_KEY)
#define MAX_ADC_KEY                     (4) //TCC803x EVB (ADC0 ch0 ~ ch2)
    //#define MAX_ADC_KEY 3 //TCC803x EVB (ADC0 ch0 ~ ch2)
#elif defined(TCC70XX_KEY)
#if defined(EXTERNAL_KEY)
#define MAX_ADC_KEY                     (6) // TCC70XX EVB (ADC0 ch1, ch7, ch8, ch9, ch10, ch11)
#else
#define MAX_ADC_KEY                     (1) // TCC70XX EVB (ADC0 ch1)
#endif  // defined(EXTERNAL_KEY)
#endif  // defined(TCC803X_KEY)

#define ADCKEY_DEBUG
#ifdef ADCKEY_DEBUG
#   define ADCKEY_DBG(fmt, args...)     {mcu_printf(fmt, ## args);}
#else
#   define ADCKEY_DBG(fmt, args...)     {}
#endif


/* Key definition*/
#define KEY_NULL                        (0x00)
#define KEY_1                           (0x01)
#define KEY_2                           (0x02)
#define KEY_3                           (0x03)
#define KEY_4                           (0x04)
#define KEY_5                           (0x05)
#define KEY_6                           (0x06)

#define KEY_ENTER                       (0x11)
#define KEY_SCAN                        (0x12)
#define KEY_SETUP                       (0x13)
#define KEY_MENU                        (0x14)
#define KEY_DISP                        (0x15)
#define KEY_TRACKDOWN                   (0x16)
#define KEY_TRACKUP                     (0x17)
#define KEY_FOLDERDOWN                  (0x18)
#define KEY_FOLDERUP                    (0x19)
#define KEY_POWER                       (0x1a)
#define KEY_RADIO                       (0x1b)
#define KEY_MEDIA                       (0x1c)
#define KEY_PHONE                       (0x1d) //KEY_HANDSFREE
#define KEY_VOLUMEUP                    (0x1e)
#define KEY_VOLUMEDOWN                  (0x1f)

#define KEY_MODE                        (0x20)
#define KEY_SEEKUP                      (0x21)
#define KEY_SEEKDOWN                    (0x22)
#define KEY_MUTE                        (0x23)
#define KEY_PHONEON                     (0x24)
#define KEY_PHONEOFF                    (0x25)

#define KEY_HOME                        (0x26)
#define KEY_RETURN                      (0x27)
#define KEY_PLAYPAUSE                   (0x28)
#define KEY_CARPLAY                     (0x29)

//Keys for Battleship keypad
#define KEY_BS_PHONE                    (0xA9)
#define KEY_BS_POWER                    (0x74)
#define KEY_BS_DMB                      (0xF9)
#define KEY_BS_MEDIA                    (0xE2)
#define KEY_BS_SETTING                  (0xFA)
#define KEY_BS_NAVI                     (0xFB)
#define KEY_BS_RADIO                    (0x181)

#define KEY_SW22                        (0x22)
#define KEY_SW23                        (0x23)
#define KEY_SW24                        (0x24)
#define KEY_SW25                        (0x25)
#define KEY_SW26                        (0x26)
#define KEY_SW27                        (0x27)

enum
{
    KEY_ADCKEY_READ                     = 1,
};

enum
{
    KEY_ADC_RELEASED                    =0,
    KEY_ADC_CHECK,
    KEY_ADC_PRESSED,
    KEY_ADC_PRESSING,
};

typedef struct KEYAdcKeyType
{
    uint32                              akADCKeyStatus;
    uint32                              akADCOpenOptions;
} KEYAdcKeyType_t;

typedef struct KEYButton
{
    uint32                              buSscancode;
    uint32                              buEscancode;
    uint16                              buVkcode;
} KEYButton_t;

typedef struct KEYKeyInfo
{
    int32                               old_key;
    int32                               ch;
    int32                               key_pressed;
    int32                               key_status;
    int32                               key_chk_cnt;
} KEYKeyInfo_t;

/*
***************************************************************************************************
*                                          KEY_AdcScan
*
*
* @param    piKeyVal [in]
* @param    piKPressed [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_AdcScan
(
    int32 *                             piKeyVal,
    int32 *                             piKPressed,
    uint8                               ucModNum
);

/*
***************************************************************************************************
*                                          KEY_AdcOpen
*
*
* @param    uiOptions [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_AdcOpen
(
    uint32                              uiOptions
);

extern int32                            siADCDebug;

#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )
#endif  // MCU_BSP_KEY_ADC_HEADER


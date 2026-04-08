// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key_gpio.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#include "key_gpio.h"
#include "key_adc.h"
#include "key.h"
#include <bsp.h>
#include <gpio.h>

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/
#define ADCKEY_DEBUG

#ifdef ADCKEY_DEBUG
#   define GPIOKEY_DBG(fmt, args...)    {LOGD(DBG_TAG_GPIO, fmt, ## args)}
#else
#   define GPIOKEY_DBG(fmt, args...)    {}
#endif


#define KEY_ROTARY_KEY_UP               (0)
#define KEY_ROTARY_KEY_DN               (1)
#define KEY_ROTARY_KEY_PUSH             (2)
#define KEY_ROTARY_VOL_UP               (28)
#define KEY_ROTARY_VOL_DN               (7)
#define KEY_ROTARY_VOL_PUSH             (6)
// define gpio port
#define KEY_ROTARY_KEY_UP_GPIO          (GPIO_GPC((uint32)KEY_ROTARY_KEY_UP))//temp
#define KEY_ROTARY_KEY_DN_GPIO          (GPIO_GPC((uint32)KEY_ROTARY_KEY_DN))//temp
#define KEY_ROTARY_KEY_PUSH_GPIO        (GPIO_GPC((uint32)KEY_ROTARY_KEY_PUSH))//temp
#define KEY_ROTARY_VOL_UP_GPIO          (GPIO_GPA((uint32)KEY_ROTARY_VOL_UP))//temp
#define KEY_ROTARY_VOL_DN_GPIO          (GPIO_GPB((uint32)KEY_ROTARY_VOL_DN))//temp
#define KEY_ROTARY_VOL_PUSH_GPIO        (GPIO_GPB((uint32)KEY_ROTARY_VOL_PUSH))//temp

#if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
static int32 rotary_key_pad_type = KEY_MANY_PUSHKEY;
#endif

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/


static GPIOKeyType_t GPIOKey_Main;


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void KEY_GpioSetPortMode
(
    GPIORotaryType_t                    uiRotary,
    GPIOMode_t                          uiMode
);

static uint8 KEY_GpioGetUpPulse
(
    uint8                               uiChannel
);

static uint8 KEY_GpioGetDownPulse
(
    uint8                               uiChannel
);

static void KEY_GpioRotaryInitialize
(
    void
);

/*
***************************************************************************************************
*                                          KEY_GpioSetPortMode
*
* @param    rotary [in]
* @param    mode [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

static void KEY_GpioSetPortMode(GPIORotaryType_t uiRotary, GPIOMode_t uiMode)
{
    //uint32 gpio_set;
    switch (uiRotary) {
        case KEY_ROTARY_CTRL_UPDOWN:
            if (uiMode == KEY_INPUT_MODE)
            {
                (void)GPIO_Config(KEY_ROTARY_KEY_UP_GPIO, GPIO_FUNC((uint32)0)|GPIO_INPUT|GPIO_INPUTBUF_EN);
                (void)GPIO_Config(KEY_ROTARY_KEY_DN_GPIO, GPIO_FUNC((uint32)0)|GPIO_INPUT|GPIO_INPUTBUF_EN);
            } //CS : Redundant Condition - Remove redundant else statement.
            break;
        case KEY_ROTARY_VOL_UPDOWN:
            if (uiMode == KEY_INPUT_MODE)
            {
                (void)GPIO_Config(KEY_ROTARY_VOL_UP_GPIO, GPIO_FUNC((uint32)0)|GPIO_INPUT|GPIO_INPUTBUF_EN);
                (void)GPIO_Config(KEY_ROTARY_VOL_DN_GPIO, GPIO_FUNC((uint32)0)|GPIO_INPUT|GPIO_INPUTBUF_EN);
            } //CS : Redundant Condition - Remove redundant else statement.
            break;
        default:
            GPIOKEY_DBG("INVALID Rotary value \n");
            break;
    }
}


/*
***************************************************************************************************
*                                          KEY_GpioGetUpPulse
*
* @param    uiChannel [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

static uint8 KEY_GpioGetUpPulse(uint8 uiChannel) //QAC
{
    uint8 tmp = 0; //CS : uninitialized variable - Add initialized value

    if (uiChannel == (uint8)KEY_ROTARY_CH1) {
        tmp = GPIO_Get(KEY_ROTARY_KEY_UP_GPIO);
    } else if (uiChannel == (uint8)KEY_ROTARY_CH2) {
        tmp = GPIO_Get(KEY_ROTARY_VOL_UP_GPIO);
    } else { // QAC : else
        tmp = 0xFF;
    }

    return tmp;
}


/*
***************************************************************************************************
*                                          KEY_GpioGetDownPulse
*
* @param    uiChannel [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

static uint8 KEY_GpioGetDownPulse(uint8 uiChannel) //QAC
{
    uint8 tmp = 0; //CS : uninitialized variable - Add initialized value


    if (uiChannel == (uint8)KEY_ROTARY_CH1) {
        tmp = GPIO_Get(KEY_ROTARY_KEY_DN_GPIO);
    } else if (uiChannel == (uint8)KEY_ROTARY_CH2) {
        tmp = GPIO_Get(KEY_ROTARY_VOL_DN_GPIO);
    } else { // QAC : else
        tmp = 0xFF;
    }

    return tmp;
}


/*
***************************************************************************************************
*                                          KEY_GpioRotaryInitialize
*
*
* Notes
*
***************************************************************************************************
*/

static void KEY_GpioRotaryInitialize(void)
{
    // Initialize CTRL. Rotary
    KEY_GpioSetPortMode(KEY_ROTARY_CTRL_UPDOWN, KEY_INPUT_MODE);

    // Initialize VOL. Rotary
    KEY_GpioSetPortMode(KEY_ROTARY_VOL_UPDOWN, KEY_INPUT_MODE);

    #if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
    //check key type
    rotary_key_pad_type = (int32)GPIO_Get(GPIO_GPK((uint32)5));
    #endif
    // TODO :
}

/*
***************************************************************************************************
*                                          KEY_GpioRotaryInitialize
*
* @param    uiOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t KEY_GpioKeyOpen(uint32 uiOptions)
{
    SALRetCode_t lRet;
    lRet = SAL_RET_SUCCESS;
    GPIOKey_Main.uiGPIOKeyStatus = KEY_DEVICE_ST_NO_OPEN; //CS : Useless Assignment - Modify initial value

    if (uiOptions  != 0UL )
    {
        lRet = SAL_RET_FAILED;
    }
    else
    {
        GPIOKey_Main.uiGPIOOpenOptions  = uiOptions;
        GPIOKey_Main.uiGPIOKeyStatus = KEY_DEVICE_ST_OPENED;

        KEY_GpioRotaryInitialize();
        // open driver
    }

    return lRet;
}

/*
***************************************************************************************************
*                                          KEY_GpioGetRotaryValue
*
* @param    uiChannel [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

GPIORotaryKeyType_t KEY_GpioGetRotaryValue(uint8 uiChannel)
{
    GPIORotaryKeyType_t ret = KEY_ROTARY_KEY_MAX;
    uint8 up_pulse;
    uint8 down_pulse;
    uint32 old_up_pulse_shift;
    uint32 old_down_pulse_shift;
    static uint32 old_up_pulse[KEY_ROTARY_CH_MAX] = {0, 0}; //QAC
    static uint32 old_down_pulse[KEY_ROTARY_CH_MAX] = {0, 0}; //QAC
    up_pulse = KEY_GpioGetUpPulse(uiChannel);
    down_pulse =  KEY_GpioGetDownPulse(uiChannel);

    if( uiChannel < (uint8)KEY_ROTARY_CH_MAX) //QAC : 21.18 : Dereference of an invalid pointer value
    {
        if ((up_pulse != (old_up_pulse[uiChannel] & (uint32)0x01)) || (down_pulse != (old_down_pulse[uiChannel] & (uint8)0x01))) {
            old_up_pulse_shift = (old_up_pulse[uiChannel] << (uint32)0x01);
            old_up_pulse[uiChannel] = (old_up_pulse_shift | up_pulse) & ((uint32)0x7);
            old_down_pulse_shift = (old_down_pulse[uiChannel] << (uint32)0x01);
            old_down_pulse[uiChannel] = (old_down_pulse_shift | down_pulse) & ((uint32)0x7);
            if (((old_up_pulse[uiChannel] == (uint32)0x3) &&(old_down_pulse[uiChannel] == (uint32)0x1)) ||
                /*((old_up_pulse[channel] == 0x1) &&(old_down_pulse[channel] == 0x4)) ||
                ((old_up_pulse[channel] == 0x6) &&(old_down_pulse[channel] == 0x3)) ||*/ //a half rotation condition
                ((old_up_pulse[uiChannel] == (uint32)0x4)&& (old_down_pulse[uiChannel] == (uint32)0x6))) { // UP
                if (uiChannel == (uint8)KEY_ROTARY_CH1) {
                    #if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
                    if(rotary_key_pad_type == KEY_ONLY_ROTARY)
                    {
                        ret = KEY_ROTARY_KEY_VOL_UP;
                    }
                    else
                    {
                        ret = KEY_ROTARY_KEY_CTRL_UP;
                    }
                    #else
                    ret = KEY_ROTARY_KEY_CTRL_UP;
                    #endif
                } else if(uiChannel ==(uint8)KEY_ROTARY_CH2) {
                    #if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
                    if(rotary_key_pad_type == KEY_ONLY_ROTARY)
                    {
                        ret = KEY_ROTARY_KEY_CTRL_UP;
                    }
                    else
                    {
                        ret = KEY_ROTARY_KEY_VOL_UP;
                    }
                    #else
                    ret = KEY_ROTARY_KEY_VOL_UP;
                    #endif
                } else { // QAC : else
                    ;
                }
            }
            else if (((old_up_pulse[uiChannel] == (uint32)0x1) &&(old_down_pulse[uiChannel] == (uint32)0x3)) ||
                /*((old_up_pulse[channel] == 0x4) &&(old_down_pulse[channel] == 0x1)) ||
                ((old_up_pulse[channel] == 0x3) &&(old_down_pulse[channel] == 0x6)) ||*/ //a half rotation condition
                ((old_up_pulse[uiChannel] == (uint32)0x6)&& (old_down_pulse[uiChannel] == (uint32)0x4))) { // DOWN
                if (uiChannel == (uint8)KEY_ROTARY_CH1) {
                    #if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
                    if(rotary_key_pad_type == KEY_ONLY_ROTARY)
                    {
                        ret = KEY_ROTARY_KEY_VOL_DOWN;
                    }
                    else
                    {
                        ret = KEY_ROTARY_KEY_CTRL_DOWN;
                    }
                    #else
                    ret = KEY_ROTARY_KEY_CTRL_DOWN;
                    #endif
                } else if(uiChannel ==(uint8)KEY_ROTARY_CH2) {
                    #if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
                    if(rotary_key_pad_type == KEY_ONLY_ROTARY)
                    {
                        ret = KEY_ROTARY_KEY_CTRL_DOWN;
                    }
                    else
                    {
                        ret = KEY_ROTARY_KEY_VOL_DOWN;
                    }
                    #else
                    ret = KEY_ROTARY_KEY_VOL_DOWN;
                    #endif
                } else { // QAC : else
                    ;
                }
            }
            else { // QAC : else
                ;
            }
        }
    }
    else
    {
        ret = (GPIORotaryKeyType_t)0; //Error
    }
    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )


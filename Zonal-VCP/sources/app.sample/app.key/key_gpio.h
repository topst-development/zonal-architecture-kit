// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key_gpio.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_KEY_GPIO_HEADER
#define MCU_BSP_KEY_GPIO_HEADER

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define KEY_ROTARY_CH1                  (0)
#define KEY_ROTARY_CH2                  (1)
#define KEY_ROTARY_CH_MAX               (2)


typedef struct GPIOKeyType
{
    uint32                              uiGPIOKeyStatus;
    uint32                              uiGPIOOpenOptions;
} GPIOKeyType_t;



typedef enum GPIOMode
{
    KEY_INPUT_MODE                      = 0,
    KEY_OUTPUT_MODE
} GPIOMode_t;

typedef enum GPIORotaryKeyType
{
    KEY_ROTARY_KEY_CTRL_UP              = 1,
    KEY_ROTARY_KEY_CTRL_DOWN,
    KEY_ROTARY_KEY_VOL_UP,
    KEY_ROTARY_KEY_VOL_DOWN,
    KEY_ROTARY_KEY_MAX
} GPIORotaryKeyType_t;

typedef enum GPIORotaryType
{
    KEY_ROTARY_CTRL_UPDOWN              = 1,
    KEY_ROTARY_VOL_UPDOWN,
    KEY_ROTARY_MAX
} GPIORotaryType_t;


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

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

SALRetCode_t KEY_GpioKeyOpen
(
    uint32                              uiOptions
);


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
GPIORotaryKeyType_t KEY_GpioGetRotaryValue
(
    uint8                               uiChannel
);

#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#endif  // MCU_BSP_KEY_GPIO_HEADER


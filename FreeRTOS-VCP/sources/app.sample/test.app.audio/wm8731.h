// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wm8731.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_WM8731_HEADER
#define MCU_BSP_WM8731_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <i2s.h>        //include i2s_reg.h
#include <i2c_reg.h>    //include i2c.h

/* WM8731 register space */

#define WM8731_LINVOL                   0x00    //Left Line In
#define WM8731_RINVOL                   0x01    //Right Line In
#define WM8731_LOUT1V                   0x02    //Left Headphone Out
#define WM8731_ROUT1V                   0x03    //Right Headphone Out
#define WM8731_APANA                    0x04    //Analogue Audio Path Control
#define WM8731_APDIGI                   0x05    //Digital Audio Path Control
#define WM8731_PWR                      0x06    //Power Down Control
#define WM8731_IFACE                    0x07    //Digital Audio Interface Format
#define WM8731_SRATE                    0x08    //Sampling Control
#define WM8731_ACTIVE                   0x09    //Active Control
#define WM8731_RESET                    0x0f

#define WM8731_CACHEREGNUM              10

#define WM8731_SYSCLK_MCLK              0
#define WM8731_SYSCLK_XTAL              1

#define WM8731_DAI                      0


typedef enum WM8731DacPower
{
    WM8731_DACPOWER_ALLON               = 0,    /**< DAC/ADC all on                            */
    WM8731_DACPOWER_DACON               = 1,    /**< DAC on,  ADC off                          */
    WM8731_DACPOWER_ADCON               = 2,    /**< DAC off, ADC on                           */
    WM8731_DACPOWER_ALLOFF              = 3,    /**< DAC/ADC all off                           */
    WM8731_DACPOWER_MICON               = 4,    /**< DAC off, ADC on                           */
    WM8731_DACPOWER_MICOFF              = 5,    /**< DAC/ADC all off                           */

}WM8731DacPower_t;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

/*
***************************************************************************************************
*                                          WM8731_Initial
*
* The function for H/W Codec Initialize function
*
* @return
* Notes
*
**************************************************************************************************
*/

SALRetCode_t WM8731_Initial
(
    uint8                               ucI2cCh,
    uint32                              uiI2cPortSel,
    I2SConfig_t *                       psI2sConf
);

void WM8731_DACSoftMute
(
    uint8                               ucI2cCh,
    boolean                             bEnable
);

void WM8731_Power
(
    uint8                               ucI2cCh,
    WM8731DacPower_t                    sDacPower
);

void WM8731_SetAudioOutVolume
(
    uint8                               ucI2cCh,
    uint8                               ucHPV
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#endif  //  MCU_BSP_WM8731_HEADER


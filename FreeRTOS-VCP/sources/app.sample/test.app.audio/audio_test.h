// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : audio_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_AUDIO_TEST_HEADER
#define MCU_BSP_AUDIO_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_I2S != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_I2S value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_I2C != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_I2C value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

/**************************************************************************************************
*                                             INCLUDE FILES
**************************************************************************************************/
#include <sal_internal.h>

#include <app_cfg.h>
#include <i2s.h>
#include <gic_enum.h>
#include <gic.h>
#include <debug.h>

#ifdef WM8731
    #include <wm8731.h>
#else
    #include <wm8904.h>
#endif

typedef enum AUDIOStatus
{
    I2S_ERROR                           = 0,
    I2S_DONE,
    I2S_READY,
    I2S_PLAYBACK,
    I2S_PAUSE,
    I2S_RESUME
} AUDIOStatus_t;


/**************************************************************************************************
*                                             DEFINITIONS
**************************************************************************************************/
typedef struct AUDIOConfig
{
    I2SConfig_t *                       acI2sCfg;
    uint8                               acCodecCh;
} AUDIOConfig_t;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/
void AUDIO_TestUsage
(
    void
);

void AUDIO_SampleTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

void AUDIO_CreateAppTask
(
    AUDIOConfig_t *                     psAudioCfg
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#endif  // MCU_BSP_AUDIO_TEST_HEADER


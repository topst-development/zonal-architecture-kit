// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spu.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_SPU_HEADER
#define MCU_BSP_SPU_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_SPU == 1 )

/****************************************************************************************************
*                                             INCLUDE FILES
****************************************************************************************************/
#include "spu_dev.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#if (DEBUG_ENABLE)
    #include "debug.h"

    #define SPU_D(fmt, args...)         {LOGD(DBG_TAG_PDM, fmt, ## args)}
    #define SPU_E(fmt, args...)         {LOGE(DBG_TAG_PDM, fmt, ## args)}
#else
    #define SPU_D(fmt, args...)
    #define SPU_E(fmt, args...)
#endif

#define SPU_OK                          0UL
#define SPU_INVALID_ERR                 0xE0000000UL
#define SPU_INVALID_CH                  0xE0000001UL

#define SPU_DISABLE                     0UL
#define SPU_ENABLE                      1UL

#define SPU_IDLE                        0UL
#define SPU_ACTIVE                      1UL

#define MAX_NUM_OF_SSL                  5UL
#define MAX_NUM_OF_WG                   5UL
#define MAX_NUM_OF_SRC                  6UL
#define MAX_NUM_OF_AVC                  16UL

#define AVC_BYPASS                      0UL
#define AVC_VOL_CTL                     1UL

#define AVC_UNMUTE                      0UL
#define AVC_MUTE                        1UL

#define AVC_PARAM_CLEAR                 1UL
#define AVC_DB_CLEAR                    1UL

#define SRC_SRC_MODE                    0UL
#define SRC_BYPASS_MODE                 1UL

#define SSL_MONO                        0UL
#define SSL_STEREO                      1UL

#define SSL_FORCE_STOP                  1UL
#define SSL_STABLE_STOP                 0UL

#define SSL_IRQ_MASKED                  0UL
#define SSL_IRQ_PASSED                  1UL

#define SSL_IRQ_STATUS_CLEAR            1UL
#define SSL_IRQ_NO_ACTION               0UL

#define SPU_INIT_STS                    0UL
#define WG_CHAIN_STS(N)                 (uint32)(1UL + (N))
#define SSL_CHAIN_STS(N)                (uint32)(6UL + (N))
#define MAX_CH_STS                      10UL

typedef enum SPU_SSL_CH{
    SPU_SSL_0                           = 0UL,
    SPU_SSL_1                           = 1UL,
    SPU_SSL_2                           = 2UL,
    SPU_SSL_3                           = 3UL,
    SPU_SSL_4                           = 4UL
} SPU_SSL_CH_t;

typedef enum SPU_WG_CH{
    SPU_WG_0                            = 0UL,
    SPU_WG_1                            = 1UL,
    SPU_WG_2                            = 2UL,
    SPU_WG_3                            = 3UL,
    SPU_WG_4                            = 4UL
} SPU_WG_CH_t;

typedef enum SPU_AVC_CH{
    SPU_AVC_WG0                         = 0UL,
    SPU_AVC_WG1                         = 1UL,
    SPU_AVC_WG2                         = 2UL,
    SPU_AVC_WG3                         = 3UL,
    SPU_AVC_WG4                         = 4UL,
    SPU_AVC_SSL0                        = 5UL,
    SPU_AVC_SSL1                        = 7UL,
    SPU_AVC_SSL2                        = 9UL,
    SPU_AVC_SSL3                        = 11UL,
    SPU_AVC_SSL4                        = 13UL,
    SPU_AVC_MAX                         = 15UL
} SPU_AVC_CH_t;

typedef enum SSL_STS{
    SSL_STS_IDLE                        = 0UL,
    SSL_STS_DATA                        = 1UL,
    SSL_STS_DUMMY                       = 2UL,
    SSL_STS_FINAL                       = 3UL
} SSL_STS_t;

typedef enum WG_WAVE_TYPE{
    WG_SINE_WAVE                       = 0x0UL,
    WG_SAWTOOTH_WAVE                   = 0x1UL,
    WG_SQUARE_WAVE                     = 0x2UL
} WG_WAVE_TYPE_t;

typedef struct WGConfig
{
    double                              dSampleOverFreq;
    uint32                              uiWaveType;
    uint32                              uiIdleTimeMS;
    uint32                              uiAttackTimeMS;
    uint32                              uiStableTimeMS;
    uint32                              uiReleaseTimeMS;
    uint32                              uiFilterType;
    uint32                              uiTonegenRepeat;
} WGConfig_t;

typedef struct SSLConfig
{
    uint32                              uiMode;
    uint32                              uiRepeat;
    uint32                              uiFifoAddr;
    uint32                              uiFifoSize;
    uint32                              uiDummySize;
} SSLConfig_t;

typedef struct SRCConfig
{
    uint32                              uiByPass;
    uint32                              uiFifoSetPoint;
    uint32                              uiInitZeroSize;
    double                              dRatio;
} SRCConfig_t;

typedef struct AVCConfig
{
    uint32                              uiMode;
    uint32                              uiPeriod;
    uint32                              uiInterval;
    uint32                              uiWait;
    double                              dGain;
} AVCConfig_t;

/***************************************************************************************************
*                                       FUNCTION PROTOTYPES
***************************************************************************************************/
/*
***************************************************************************************************
*                                       SPU_Init
*
* SPU Initialize.
* @param    psSRCConfig : SRC config table
* @param    psAVCConfig : avc config table
* @return   void
*
***************************************************************************************************
*/
void SPU_Init
(
    const SRCConfig_t *                 psSRCConfig,
    const AVCConfig_t*                  psAVCConfig
);

/*
***************************************************************************************************
*                                       SPU_DeInit
*
* SPU DeInit
*
* @param    void
* @return   void
*
***************************************************************************************************
*/
void SPU_DeInit
(
    void
);

/*
***************************************************************************************************
*                                       SPU_Start_WG_Chain
*
* Start Wave-generation chain
*
* @param    uiNth : num of WG
* @param    psWgConfig : wg config table
* @return   void
*
***************************************************************************************************
*/
void SPU_Start_WG_Chain
(
    uint32                              uiNth,
    const WGConfig_t *                  psWgConfig
);

/*
***************************************************************************************************
*                                       SPU_Stop_WG_Chain
*
* Stop Wave-generation chain
*
* @param    uiNth : num of WG
* @return   void
*
***************************************************************************************************
*/
void SPU_Stop_WG_Chain
(
    uint32                              uiNth
);

/*
***************************************************************************************************
*                                       SPU_Start_SSL_Chain
*
* Start Sound Source Loader.Chain
*
* @param    uiNth : num of SSL
* @param    psSSLConfig : ssl config table
* @return   void
*
***************************************************************************************************
*/
void SPU_Start_SSL_Chain
(
    uint32                              uiNth,
    const SSLConfig_t *                 psSSLConfig
);

/*
***************************************************************************************************
*                                       SPU_Stop_SSL_Chain
*
* Stop SSL Chain
*
* @param    uiNth : num of SSL
* @param    uiForced : if 1 forced stop otherwise stable stop
* @return   void
*
***************************************************************************************************
*/
void SPU_Stop_SSL_Chain
(
    uint32                              uiNth,
    uint32                              uiForced
);

/*
***************************************************************************************************
*                                       SPU_Set_Volume_Controller
*
* Set Volume Controller
*
* @param    uiAvcCH : num of AVC Channel (WG0 ~ WG4 and SSL0 ~ SSL4)
* @param    uiForced : if 1 forced stop otherwise stable stop
* @return   void
*
***************************************************************************************************
*/
void SPU_Set_Volume_Controller
(
    uint32                              uiAvcCH,
    const AVCConfig_t *                 psAVCConfig
);


/*
***************************************************************************************************
*                                       SPU_Set_Channel_Mute
*
* Set Channel Mute or UnMute
*
* @param    uiAvcCH : num of AVC Channel (WG0 ~ WG4 and SSL0 ~ SSL4)
* @param    uiMute : Mute or Unmute
* @return   void
*
***************************************************************************************************
*/
void SPU_Set_Channel_Mute
(
    uint32                              uiAvcCH,
    uint32                              uiMute
);

/*
***************************************************************************************************
*                                       SPU_Get_Channel_Status
*
* Get SPU channel status.
* @param    void
* @return   channel status
*
***************************************************************************************************
*/
uint32 SPU_Get_STS
(
    uint32                              uiChannel
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SPU == 1 )

#endif  // MCU_BSP_SPU_HEADER


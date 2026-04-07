// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : i2s.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_I2S_HEADER
#define MCU_BSP_I2S_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_I2S == 1 )

/**************************************************************************************************
*                                             INCLUDE FILES
**************************************************************************************************/

#include <i2s_reg.h>
#include <debug.h>


/**************************************************************************************************
*                                             DEFINITIONS
**************************************************************************************************/

#if (DEBUG_ENABLE)
#define I2S_D(fmt, args...)             {LOGD(DBG_TAG_I2S, fmt, ## args)}
#define I2S_E(fmt, args...)             {LOGE(DBG_TAG_I2S, fmt, ## args)}
#else
#define I2S_D(fmt, args...)
#define I2S_E(fmt, args...)
#endif


#define I2S_MAX_PERI_CLOCK              100000000U       //100Mhz    //Temporary

//Relation with GPIO Port
#define PERICH_SEL                      (0xA0F222B8U)
#define I2SSEL_0                        (8) //9:8 bit


typedef enum I2SDataDirect
{
    I2S_DOUT                            = 0x01,
    I2S_DIN                             = 0x02

} I2SDataDirect_t;

typedef enum I2SDmaStatus
{
    I2S_PRE                             = 0x01,
    I2S_RUN,
    I2S_UNDERRUN_B,
    I2S_OVERRUN_B,
    I2S_STOP,
}I2SDmaStatus_t;

typedef enum I2SHwCh
{
    I2S_CH0                             = 0U,
    I2S_CH1,
    I2S_CH2

} I2SHwCh_t;

typedef enum I2SMode
{
    I2S_MASTER_MODE                     = 0,
    I2S_SLAVE_MODE                      = 1

} I2SMode_t;

typedef enum I2SFormat
{
    I2S_FORMAT_I2S                      = 1,        /* I2S mode */
    I2S_FORMAT_RIGHTJ,                              /* Right Justified mode = Least Significant Bit (LSB) */
    I2S_FORMAT_LEFTJ                                /* Left Justified mode = Most Significant Bit (MSB) */

} I2SFormat_t;

#define I2S_FORMAT_MSB                  I2S_FORMAT_LEFTJ
#define I2S_FORMAT_LSB                  I2S_FORMAT_RIGHTJ


//Support Only Mono
typedef enum I2SChannel
{
    I2S_MONO                            = 0,
    I2S_STEREO                          = 1

} I2SChannel_t;

typedef enum I2SLRmode
{
    I2S_LRMODE_OFF                      = 0,
    I2S_LRMODE_ON                       = 1

} I2SLRmode_t;


typedef enum I2SSampleRate
{
    I2S_SAMPLE_RATE_8000                = 0, //8 kHz, 16 kHz, 24 kHz, 32 kHz, 44.1 kHz, 48 kHz, 64 kHz, 96 kHz, 128 kHz, 192 kHz
    I2S_SAMPLE_RATE_16000,
    I2S_SAMPLE_RATE_24000,
    I2S_SAMPLE_RATE_32000,
    I2S_SAMPLE_RATE_44100,
    I2S_SAMPLE_RATE_48000,
    I2S_SAMPLE_RATE_64000,
    I2S_SAMPLE_RATE_96000,
    I2S_SAMPLE_RATE_128000,
    I2S_SAMPLE_RATE_192000

} I2SSampleRate_t;

typedef enum I2SBitDepth
{
    I2S_BIT_DEPTH_16                    = 16,         /* I2S mode */
    I2S_BIT_DEPTH_24                    = 24,         /* Right Justified mode */

} I2SBitDepth_t;

typedef enum I2SBclkDiv
{
    I2S_BCLK_DIV_32                     = 0,
    I2S_BCLK_DIV_48                     = 1,
    I2S_BCLK_DIV_64                     = 2,
    I2S_BCLK_RESERVED                   = 3
    //I2S_BCLK_DIV_512 = 4,  //only for DP transfer, not available in I2S Tx/Rx mode

} I2SBclkDiv_t;

typedef enum I2SMclkDiv
{
    I2S_MCLK_DIV_4                      = 0,
    I2S_MCLK_DIV_6                      = 1,
    I2S_MCLK_DIV_8                      = 2,
    I2S_MCLK_DIV_16                     = 3,
    I2S_MCLK_DIV_24                     = 4,
    I2S_MCLK_DIV_32                     = 5,
    I2S_MCLK_DIV_48                     = 6,
    I2S_MCLK_DIV_64                     = 7

} I2SMclkDiv_t;

typedef struct I2SBufferInfo
{
    uint32                              i2sBufferBytes;
    uint32                              i2sPeriodBytes;
    uint32                              i2sThresholdBytes;
    uint32                              i2sDataPos;  //Rx : Read Pos, Tx : Write Pos
    uint32                              i2sPreCurPos;
    uint32 *                            i2sDmaAddr;
    I2SDmaStatus_t                      i2sDmaStatus;

} I2SBufferInfo_t;

typedef struct I2SStream
{
    I2SBufferInfo_t                     i2sOut;
    I2SBufferInfo_t                     i2sIn;

} I2SStream_t;

typedef struct I2SConfig
{
    I2SHwCh_t                           i2sHwCh;             //I2S_CH_SELECT
    I2SMode_t                           i2sMode;           //master, slave
    I2SFormat_t                         i2sFormat;         //i2s, left_j, right_j
    I2SChannel_t                        i2sNumCh;         //mono, stereo
    I2SLRmode_t                         i2sLRmode;         //LR Mode Enable, Disable
    I2SSampleRate_t                     i2sSampleRate;     //44,1khz, 48khz etc..
    I2SBitDepth_t                       i2sBitPerSample;   //Bit Depth 16bit, 24bit
    I2SBclkDiv_t                        i2sBclkDiv;       //32, 48, 64,
    I2SMclkDiv_t                        i2sMclkDiv;       //4, 6, 8, 16, 24, 32, 48, 64
    uint32                              i2sDataSize;
    I2SStream_t                         i2sStreamInfo;
} I2SConfig_t;

typedef enum I2SRetCode
{
    I2S_RET_SUCCESS                     = 0,
    I2S_RET_FAILED                      = 1

} I2SRetCode_t;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

I2SRetCode_t I2S_SetGpiofunction
(
    const I2SConfig_t*                  psI2sConf
);

I2SRetCode_t I2S_SetClock
(
    const I2SConfig_t*                  psI2sConf
);

void I2S_DaifSetting
(
    const I2SConfig_t*                  psI2sConf
);

void I2S_SetTransferSize
(
    const I2SConfig_t*                  psI2sConf,
    I2SDataDirect_t                     i2sDirect
);

uint32 I2S_GetTransferSize
(
    I2SDataDirect_t                     i2sDirect
);


void I2S_SetTASMode
(
    void
);

void I2S_SetAICMode
(
    void
);

void I2S_TASClear
(
    void
);

void I2S_SetTASRxFifoThreshold
(
    uint8 ucThreshold
);

void I2S_SetTASTxFifoThreshold
(
    uint8 ucThreshold
);

void I2S_HopClear
(
    void
);

void I2S_FifoClear
(
    uint32                              uiClrBit
);

void I2S_DmaReset
(
    uint32                              uiRstBit
);

void I2S_SWReset
(
    boolean                             bReset
);

void I2S_DAMREnable
(
    void
);

void I2S_DAMRDisable
(
    void
);

void I2S_DAIEnable
(
    uint8                               ucflag
);

void I2S_DAIDisable
(
    uint8                               ucflag
);

void I2S_DMAEnable
(
    uint8                               ucflag
);

void I2S_DMADisable
(
    uint8                               ucflag
);

void I2S_Irq_Enable
(
    uint8                               ucflag
);

void I2S_Irq_Disable
(
    uint8                               ucflag
);

void I2S_ErrCheck_Enable
(
    void
);

void I2S_ErrCheck_Disable
(
    void
);

void I2S_ErrCheck_Clear
(
    void
);

uint8 I2S_ErrCheck_GetStatus
(
    void
);

void I2S_Enable
(
    uint8                               ucflag
);

void I2S_Disable
(
    uint8                               ucflag
);

void I2S_DMAInit
(
    I2SStream_t*                        i2sStreamInfo,
    uint8                               ucflag
);

#ifdef AUDIO_RX_ENABLE

void I2S_RxAdmaSetting
(
    const I2SConfig_t*                        psI2sConf
);

uint8 I2S_GetRxDMAStatus
(
    void
);

void I2S_SetRxDaDar
(
    uint32                              uiRxDataSourceAddr
);

void I2S_SetRxDaDarL
(
    uint32                              uiRxDataSourceAddr
);

uint32 I2S_GetRxDaCdar
(
    void
);

uint32 I2S_GetRxDaCdarL
(
    void
);

#endif

void I2S_TxAdmaSetting
(
    const I2SConfig_t*                        psI2sConf
);

uint8 I2S_GetTxDMAStatus
(
    void
);

void I2S_SetTxDaSar
(
    uint32                              uiTxDataSourceAddr
);

void I2S_SetTxDaSarL
(
    uint32                              uiTxDataSourceAddr
);

uint32 I2S_GetTxDaCsar
(
    void
);

uint32 I2S_GetTxDaCsarL
(
    void
);

void I2S_Irq
(
    void * pArg
);

void I2S_UpdateStatus
(
    I2SConfig_t*                        psI2sConf,
    I2SDataDirect_t                     i2sDirect
);

uint32 I2S_GetAvailable
(
    I2SConfig_t*                        psI2sConf,
    I2SDataDirect_t                     i2sDirect
);


uint32 I2S_PcmWrite
(
    I2SConfig_t*                        psI2sConf,
    void *                              pToWrite,
    uint32                              uiSize
);

uint32 I2S_PcmRead
(
    I2SConfig_t*                        psI2sConf,
    void*                               pToRead,
    uint32                              uiSize
);

void I2S_RepeatMode
(
    uint8                               ucflag,
    boolean                             bEnable
);

#ifdef AUDIO_DEBUG_TEST
void I2S_LoopBackTest
(
    boolean                             bEnable
);
#endif

#endif  // ( MCU_BSP_SUPPORT_DRIVER_I2S == 1 )

#endif  // MCU_BSP_I2S_HEADER


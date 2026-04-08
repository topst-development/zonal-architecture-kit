// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : audio_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <stdlib.h>
#include <audio_test.h>
#include <mpu.h>

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
    #include <audio_sample.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )

#ifdef I2S_CHIP_VERI
    #include <audio_chip_veri.h>
#endif

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define AUDIO_TEST_TASK_STK_SIZE        (256)
#define AUDIO_MUTE_PIN                  (GPIO_GPB(05UL))    /* MUTE_CONTROL */
#define AUDIO_CODEC_I2C_CH              (0UL)
#define AUDIO_CODEC_I2C_PORT            (1UL)

#define DMA_DONE                        0U
#define BUF1_DONE                       1U
#define BUF2_DONE                       2U

#define AUDIO_PERIOD_SIZE               0x400U
#define AUDIO_BUFFER_SIZE               0x2000U

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

uint8           gReplay                 = 0U;
AUDIOStatus_t   gAudioRunning           = I2S_DONE;

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
uint32  DATA_FLASH_ADDR                 = (uint32)g_Audio_Sample;
uint32  SIZE_OF_SAMPLE                  = sizeof(g_Audio_Sample);
#else
uint32  DATA_FLASH_ADDR                 = 0x30000000UL;
uint32  SIZE_OF_SAMPLE                  = 0xf9bcUL;     // Size of "Windows Ringout_stereo_32khz.pcm"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )

#ifdef AUDIO_RX_ENABLE
uint32  AUDIO_AudioFile[1024*16]        = { 0, };
#endif


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

void AUDIO_Read
(
    void *                              pAddr,
    uint32                              uiSize,
    void *                              pBuf
);

void AUDIO_Init
(
    I2SConfig_t *                       psI2sConfig
);

void AUDIO_DACHwMute
(
    boolean                             bEnable
);

static void AUDIO_AppTask
(
    void *                              pArg
);

void AUDIO_Read(void * pAddr, uint32 uiSize, void * pBuf)
{
    if ( pAddr )
    {
       SAL_MemCopy((void *)pBuf, (const void *)pAddr, uiSize);
    }
    else
    {
        mcu_printf("Destination Address is NULL\n");
    }
}

void AUDIO_DACHwMute(boolean bEnable)
{
    // MUTE_CONTROL
#if ( VCP_MODULE_BOARD == 1 ) 
    if(bEnable)
    {
        ( void ) GPIO_Set( AUDIO_MUTE_PIN, 0UL ); //Set to Low for Mute
    }
    else
    {
        ( void ) GPIO_Set( AUDIO_MUTE_PIN, 1UL ); //Set to High for NOT Mute
    }
#else
    if(bEnable)
    {
        ( void ) GPIO_Set( AUDIO_MUTE_PIN, 1UL ); //Set to High for Mute
    }
    else
    {
        ( void ) GPIO_Set( AUDIO_MUTE_PIN, 0UL ); //Set to Low for NOT Mute
    }
#endif
}

void AUDIO_CreateAppTask(AUDIOConfig_t* psAudioCfg)
{
    static uint32   AudioTaskID;
    static uint32   AudioTaskStk[AUDIO_TEST_TASK_STK_SIZE];

    AudioTaskID = 0UL;

    (void)SAL_TaskCreate
    (
        &AudioTaskID,
        (const uint8 *)"Audio_Task",
        (SALTaskFunc)&AUDIO_AppTask,
        (void * const)&AudioTaskStk[0],
        AUDIO_TEST_TASK_STK_SIZE,
        SAL_PRIO_AUDIO_TEST,
        psAudioCfg
    );
}

static void AUDIO_AppTask(void * pArg)
{
    AUDIOConfig_t *     s_Audio_cfg;
    I2SConfig_t *       s_i2s_config;

    uint32              ui_AuDataCurAddr;
    uint32              ui_AuDataSize;
    uint32              ui_TxUnitSize;
    uint32              ui_TxAvailableSize;

#ifdef AUDIO_RX_ENABLE  //AUDIO_DEBUG_TEST
    uint32              ui_SaveCurAddr;
    uint32              ui_SaveBufSize;
    uint32              ui_RxUnitSize;
    uint32              ui_RxAvailableSize;
#endif

#ifdef WM8731
    uint8               ui_CodecCh;
#endif

    if(pArg != NULL)
    {

        s_Audio_cfg         = ( AUDIOConfig_t * ) pArg;
        s_i2s_config        = s_Audio_cfg->acI2sCfg;

#ifdef WM8731
        ui_CodecCh          = s_Audio_cfg->acCodecCh;
#endif
        ui_AuDataCurAddr    = DATA_FLASH_ADDR;
        ui_AuDataSize       = SIZE_OF_SAMPLE;
        ui_TxUnitSize       = s_i2s_config->i2sStreamInfo.i2sOut.i2sPeriodBytes;

#ifdef AUDIO_RX_ENABLE  //AUDIO_DEBUG_TEST
        ui_SaveCurAddr      = (uint32)AUDIO_AudioFile;
        ui_SaveBufSize      = sizeof(AUDIO_AudioFile);
        ui_RxUnitSize       = s_i2s_config->i2sStreamInfo.i2sIn.i2sPeriodBytes;
#endif

        while( 1 )
        {
            if(gAudioRunning == I2S_PLAYBACK)
            {
                ui_TxAvailableSize = I2S_GetAvailable(s_i2s_config, I2S_DOUT);
                if(ui_TxAvailableSize >= ui_TxUnitSize)
                {
                    if(ui_AuDataCurAddr + ui_TxUnitSize >= DATA_FLASH_ADDR + ui_AuDataSize)
                    {
                        ui_TxUnitSize = DATA_FLASH_ADDR + ui_AuDataSize - ui_AuDataCurAddr;
                    }

                    ui_AuDataCurAddr += I2S_PcmWrite(s_i2s_config, (void *)ui_AuDataCurAddr, ui_TxUnitSize);
                }

#ifdef AUDIO_RX_ENABLE
                ui_RxAvailableSize = I2S_GetAvailable(s_i2s_config, I2S_DIN);
                if(ui_RxAvailableSize >= ui_RxUnitSize)
                {
                    if(ui_SaveCurAddr + ui_RxUnitSize >= (uint32)AUDIO_AudioFile + ui_SaveBufSize)
                    {
                        ui_RxUnitSize = (uint32)AUDIO_AudioFile + ui_SaveBufSize - ui_SaveCurAddr;
                    }

                    ui_SaveCurAddr += I2S_PcmRead(s_i2s_config, (void *)ui_SaveCurAddr, ui_RxUnitSize);
                }
#endif

#ifdef AUDIO_DEBUG_TEST
                /* Checking Audio Rx Buffer */
                if(ui_SaveCurAddr >= (uint32)AUDIO_AudioFile + ui_SaveBufSize)
                {
                    I2S_Disable(I2S_DOUT|I2S_DIN);
                    gAudioRunning = I2S_DONE;
                    ui_SaveCurAddr = (uint32)AUDIO_AudioFile;
                    ui_RxUnitSize = s_i2s_config->i2sStreamInfo.i2sIn.i2sPeriodBytes;

                    ui_AuDataCurAddr = DATA_FLASH_ADDR;
                    ui_TxUnitSize = s_i2s_config->i2sStreamInfo.i2sOut.i2sPeriodBytes;
                }

                /* Checking Audio Tx Buffer */
                if(ui_AuDataCurAddr >= DATA_FLASH_ADDR + ui_AuDataSize)
                {
                   ui_AuDataCurAddr = DATA_FLASH_ADDR;
                   ui_TxUnitSize = s_i2s_config->i2sStreamInfo.i2sOut.i2sPeriodBytes;
                }
#else
                if(ui_AuDataCurAddr >= DATA_FLASH_ADDR + ui_AuDataSize)
                {
                    if(gReplay != 1)
                    {
                        AUDIO_DACHwMute(SALEnabled);
                        gAudioRunning = I2S_DONE;
                    }
                    ui_AuDataCurAddr = DATA_FLASH_ADDR;
                    ui_TxUnitSize = s_i2s_config->i2sStreamInfo.i2sOut.i2sPeriodBytes;
                }
#endif
            }
            else if(gAudioRunning == I2S_DONE)
            {

#ifdef AUDIO_DEBUG_TEST
                if(I2S_GetRxDMAStatus() == 0)
#else
                if(I2S_GetTxDMAStatus() == 0)
#endif
                {
                    mcu_printf("gAudioRunning == I2S_DONE");
                    AUDIO_DACHwMute(SALEnabled);
                    //I2S_Disable(I2S_DOUT | I2S_DIN);
#ifdef WM8731
                    WM8731_DACSoftMute(ui_CodecCh, SALEnabled);
                    WM8731_Power(ui_CodecCh, WM8731_DACPOWER_ALLOFF);
#endif
                    I2S_FifoClear(I2S_DOUT | I2S_DIN);

                    gAudioRunning = I2S_READY;
                }
            }

            ( void ) SAL_TaskSleep( 5 );
        }

    }
    else
    {
        mcu_printf("[Error] please Set i2s_config before starting audio. \r\n");
    }
}

void AUDIO_Init(I2SConfig_t *psI2sConfig)
{

    //Save non cacheable Memory Address to AUDIO DMA (ADMA)
#ifdef AUDIO_RX_ENABLE
    uint32 *  AUDIO_RxBuffer            = (uint32 *)MPU_GetDMABaseAddress();
#endif
    uint32 *  AUDIO_TxBuffer            = (uint32 *)(MPU_GetDMABaseAddress() + AUDIO_BUFFER_SIZE);

    psI2sConfig->i2sHwCh                = I2S_CH0;
    psI2sConfig->i2sMode                = I2S_MASTER_MODE;
    psI2sConfig->i2sFormat              = I2S_FORMAT_I2S;
    psI2sConfig->i2sNumCh               = I2S_STEREO;
    psI2sConfig->i2sLRmode              = I2S_LRMODE_OFF;
    psI2sConfig->i2sBitPerSample        = I2S_BIT_DEPTH_16;

    psI2sConfig->i2sSampleRate          = I2S_SAMPLE_RATE_32000;
    psI2sConfig->i2sBclkDiv             = I2S_BCLK_DIV_64;
    psI2sConfig->i2sMclkDiv             = I2S_MCLK_DIV_6;

    psI2sConfig->i2sStreamInfo.i2sOut.i2sDmaAddr = AUDIO_TxBuffer;
    psI2sConfig->i2sStreamInfo.i2sOut.i2sPeriodBytes = AUDIO_PERIOD_SIZE;
    psI2sConfig->i2sStreamInfo.i2sOut.i2sBufferBytes = AUDIO_BUFFER_SIZE;
    psI2sConfig->i2sStreamInfo.i2sOut.i2sThresholdBytes = psI2sConfig->i2sStreamInfo.i2sOut.i2sBufferBytes - psI2sConfig->i2sStreamInfo.i2sOut.i2sPeriodBytes;
    psI2sConfig->i2sStreamInfo.i2sOut.i2sDmaStatus = I2S_PRE;

    SAL_MemSet((void *)AUDIO_TxBuffer, 0, 1024*4*2);

#ifdef AUDIO_RX_ENABLE
    psI2sConfig->i2sStreamInfo.i2sIn.i2sDmaAddr = AUDIO_RxBuffer;
    psI2sConfig->i2sStreamInfo.i2sIn.i2sPeriodBytes = AUDIO_PERIOD_SIZE;
    psI2sConfig->i2sStreamInfo.i2sIn.i2sBufferBytes = AUDIO_BUFFER_SIZE;
    psI2sConfig->i2sStreamInfo.i2sIn.i2sThresholdBytes = psI2sConfig->i2sStreamInfo.i2sIn.i2sBufferBytes - psI2sConfig->i2sStreamInfo.i2sIn.i2sPeriodBytes;
    psI2sConfig->i2sStreamInfo.i2sIn.i2sDmaStatus = I2S_PRE;

    SAL_MemSet((void *)AUDIO_RxBuffer, 0, 1024*4*2);
#endif

    I2S_SWReset(SALEnabled);

    I2S_SWReset(SALDisabled);

    I2S_SetGpiofunction(psI2sConfig);
    I2S_SetClock(psI2sConfig);
    I2S_DaifSetting(psI2sConfig);

    // Set interrupt handler for i2s irq
    (void)GIC_IntVectSet(GIC_I2S_DMA, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&I2S_Irq, (void *)psI2sConfig);
    (void)GIC_IntSrcEn(GIC_I2S_DMA);

    I2S_TxAdmaSetting(psI2sConfig);
    I2S_SetTransferSize(psI2sConfig, I2S_DOUT);

#ifdef AUDIO_RX_ENABLE
    I2S_RxAdmaSetting(psI2sConfig);
    I2S_SetTransferSize(psI2sConfig, I2S_DIN);
#endif

    I2S_DAMREnable();
    I2S_FifoClear(I2S_DOUT);
#ifdef AUDIO_RX_ENABLE
    I2S_FifoClear(I2S_DIN);
#endif

}

void AUDIO_SampleTest(uint8 ucArgc, void* pArgv[])
{
#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
    static boolean                      running_task;
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )

    sint32                              ret;
    const uint8 *                       pucStr;
    uint8                               codec_I2Cch;
    uint32                              codec_I2CPortSel;

    static AUDIOConfig_t                s_Audio_cfg;
    static I2SConfig_t                  s_i2s_config;

    ret                                 = 0;
    pucStr                              = NULL_PTR;
    codec_I2Cch                         = AUDIO_CODEC_I2C_CH;
    codec_I2CPortSel                    = AUDIO_CODEC_I2C_PORT;

    if(pArgv != NULL_PTR)
    {
        pucStr = (const uint8 *)pArgv[0];

        if(pucStr != NULL_PTR)
        {
            if ((SAL_StrNCmp(pucStr, (const uint8 *)"init", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                mcu_printf("Audio Test : init \r\n");

                AUDIO_Init(&s_i2s_config);

                s_Audio_cfg.acI2sCfg = &s_i2s_config;
                s_Audio_cfg.acCodecCh = codec_I2Cch;
                gAudioRunning = I2S_READY;

                AUDIO_CreateAppTask(&s_Audio_cfg);

            }
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"codec", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {

                /* Set AUDIO MUTE PIN on GPIO OUTPUT mode */
                ( void ) GPIO_Config( AUDIO_MUTE_PIN,    ( uint32 ) ( GPIO_FUNC( 0U ) | GPIO_OUTPUT ) ); //audio codec mute pin
#ifdef WM8731
                WM8731_Initial(codec_I2Cch, codec_I2CPortSel, &s_i2s_config);
                mcu_printf("Audio Test : WM8731_Initial \r\n");
#else
                WM8904_Initial(codec_I2Cch, codec_I2CPortSel, &s_i2s_config);
                WM8904_SetVolume(codec_I2Cch, 240);
                mcu_printf("Audio Test : WM8904_Initial \r\n");
#endif
            }
 #ifndef AUDIO_DEBUG_TEST
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"repeat", 6, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                if(gReplay != 0)
                {
                    gReplay = 0;
                }
                else
                {
                    gReplay = 1;
                }
                mcu_printf("Audio Test : repeat [%d] \r\n", gReplay);
            }
#endif
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"play", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                uint32  volum;

                volum = 39;

                if(ucArgc >= 2)
                {
                    if((const uint8 *)pArgv[1] != NULL)
                    {
                        pucStr = (const uint8 *)pArgv[1];
                        volum = atoi((void *)pucStr);
                    }
                }

                I2S_DMAInit(&(s_i2s_config.i2sStreamInfo), I2S_DOUT);
#ifdef AUDIO_RX_ENABLE
                I2S_DMAInit(&(s_i2s_config.i2sStreamInfo), I2S_DIN);
#endif

#ifdef WM8731
                WM8731_Power(codec_I2Cch, WM8731_DACPOWER_DACON);
                WM8731_DACSoftMute(codec_I2Cch, SALDisabled);
                WM8731_SetAudioOutVolume(codec_I2Cch, volum);
#endif

                ( void ) SAL_TaskSleep( 10 );

                AUDIO_DACHwMute(SALDisabled);

                gAudioRunning = I2S_PLAYBACK;

                mcu_printf("Audio Test : play , vol [%d] \r\n", volum);
            }
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"end", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                gReplay = 0;

                I2S_Disable(I2S_DOUT);
#ifdef AUDIO_RX_ENABLE
                I2S_Disable(I2S_DIN);
#endif
#ifdef WM8731
                WM8731_Power(codec_I2Cch, WM8731_DACPOWER_ALLOFF);
#endif
                I2S_DAMRDisable();
                mcu_printf("Audio Test : end \r\n");
            }
#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"test", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                if( running_task == FALSE )
                {
                    running_task = TRUE;
                    /* "init" */
                    AUDIO_Init(&s_i2s_config);

                    s_Audio_cfg.acI2sCfg = &s_i2s_config;
                    s_Audio_cfg.acCodecCh = codec_I2Cch;
                    gAudioRunning = I2S_READY;

                    AUDIO_CreateAppTask(&s_Audio_cfg);

                    /* "codec" */
                    ( void ) GPIO_Config( AUDIO_MUTE_PIN,    ( uint32 ) ( GPIO_FUNC( 0U ) | GPIO_OUTPUT ) ); //audio codec mute pin
#ifdef WM8731
                    WM8731_Initial(codec_I2Cch, codec_I2CPortSel, &s_i2s_config);
                    mcu_printf("Audio Test : WM8731 \r\n");
#else
                    WM8904_Initial(codec_I2Cch, codec_I2CPortSel, &s_i2s_config);
                    WM8904_SetVolume(codec_I2Cch, 240);
                    mcu_printf("Audio Test : WM8904 \r\n");
#endif
                }
                /* "play" */
                I2S_DMAInit(&(s_i2s_config.i2sStreamInfo), I2S_DOUT);

#ifdef WM8731
                WM8731_Power(codec_I2Cch, WM8731_DACPOWER_DACON);
                WM8731_DACSoftMute(codec_I2Cch, SALDisabled);
                WM8731_SetAudioOutVolume(codec_I2Cch, 40);
#endif
                ( void ) SAL_TaskSleep( 10 );

                AUDIO_DACHwMute(SALDisabled);

                gAudioRunning = I2S_PLAYBACK;
            }
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )

#ifdef I2S_CHIP_VERI /* Chip Verification Code */
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"lrmode", 6, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                AUDIO_LRMode_Test();
            }
#endif
            else
            {
                AUDIO_TestUsage();
            }
        }
    }

}

void AUDIO_TestUsage(void)
{
    mcu_printf("=== Audio Command Usage ===\n");
    mcu_printf("    : audio init  \n");
    mcu_printf("    : audio codec  \n");
    mcu_printf("    : audio play [vol]\n");

#ifndef AUDIO_DEBUG_TEST
    mcu_printf("    : audio repeat \n");
#endif
    mcu_printf("    : audio end   \n");

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
    mcu_printf("    : audio test  \n");
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
    mcu_printf("\r\n");

    mcu_printf("=== Audio CMD Info ===\n");
    mcu_printf( "init       : Setting I2S register \n" );
    mcu_printf( "codec      : Initialize codec\n" );
    mcu_printf( "play [vol] : Start Audio Play \n" );
    mcu_printf( "             vol range is 0 to 40 \n" );

#ifndef AUDIO_DEBUG_TEST
    mcu_printf( "repeat     : Enable/Disable Repeat \n" );
#endif

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
        mcu_printf( "test       : Run whole sequence of Audio Test \n" );
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO == 1 )
    mcu_printf( "end        : I2S, ADMA, Codec Disable \n" );
};

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )


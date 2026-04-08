// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spu_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : spu full chain test
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )

#include <i2s.h>
#include <i2s_reg.h>
#include <spu.h>
#include <spu_test.h>

extern uint32  DATA_FLASH_ADDR;
extern uint32  SIZE_OF_SAMPLE;
static uint32  g_initialized_audio_interface;

#ifdef WM8731
    #include <wm8731.h>
#else
    #include <wm8904.h>
#endif

#define SPU_AUDIO_MUTE_PIN (GPIO_GPB(05UL))    /* MUTE_CONTROL */

static void spu_audio_init_8khz(I2SConfig_t *psI2sConfig, uint32 source_size)
{
    psI2sConfig->i2sHwCh                = I2S_CH0;
    psI2sConfig->i2sMode                = I2S_MASTER_MODE;
    psI2sConfig->i2sFormat              = I2S_FORMAT_I2S;
    psI2sConfig->i2sNumCh               = I2S_STEREO;
    psI2sConfig->i2sLRmode              = I2S_LRMODE_OFF;
    psI2sConfig->i2sSampleRate          = I2S_SAMPLE_RATE_8000;
    psI2sConfig->i2sBitPerSample        = I2S_BIT_DEPTH_16;
    psI2sConfig->i2sBclkDiv             = I2S_BCLK_DIV_64;
    psI2sConfig->i2sMclkDiv             = I2S_MCLK_DIV_24;
    psI2sConfig->i2sDataSize            = source_size; // non used variable
}

static void spu_audio_init_32khz(I2SConfig_t *psI2sConfig, uint32 source_size)
{
    psI2sConfig->i2sHwCh                = I2S_CH0;
    psI2sConfig->i2sMode                = I2S_MASTER_MODE;
    psI2sConfig->i2sFormat              = I2S_FORMAT_I2S;
    psI2sConfig->i2sNumCh               = I2S_STEREO;
    psI2sConfig->i2sLRmode              = I2S_LRMODE_OFF;
    psI2sConfig->i2sSampleRate          = I2S_SAMPLE_RATE_32000;
    psI2sConfig->i2sBitPerSample        = I2S_BIT_DEPTH_16;
    psI2sConfig->i2sBclkDiv             = I2S_BCLK_DIV_64;
    psI2sConfig->i2sMclkDiv             = I2S_MCLK_DIV_6;
    psI2sConfig->i2sDataSize            = source_size; // non used variable
}

static void spu_audio_init_48khz(I2SConfig_t *psI2sConfig, uint32 source_size)
{
    psI2sConfig->i2sHwCh                = I2S_CH0;
    psI2sConfig->i2sMode                = I2S_MASTER_MODE;
    psI2sConfig->i2sFormat              = I2S_FORMAT_I2S;
    psI2sConfig->i2sNumCh               = I2S_STEREO;
    psI2sConfig->i2sLRmode              = I2S_LRMODE_OFF;
    psI2sConfig->i2sSampleRate          = I2S_SAMPLE_RATE_48000;
    psI2sConfig->i2sBitPerSample        = I2S_BIT_DEPTH_16;
    psI2sConfig->i2sBclkDiv             = I2S_BCLK_DIV_64;
    psI2sConfig->i2sMclkDiv             = I2S_MCLK_DIV_6;
    psI2sConfig->i2sDataSize            = source_size; // non used variable
}

SSLConfig_t SSLConigTable[ MAX_NUM_OF_SSL ] =
{
    /* Mono or Stereo,      Repeat,     Source Address,     Source Size,    Dummy Size */
    { SSL_STEREO,           0UL,          0x48000040UL,         0x2E630UL,       0xFFF0UL},
    { SSL_STEREO,           0UL,          0x48033040UL,         0x2E630UL,       0xFFF0UL},
    { SSL_STEREO,           0UL,          0x48066040UL,         0x2E630UL,       0xFFF0UL},
    { SSL_STEREO,           2UL,          0x48099040UL,         0x2E630UL,       0xFFF0UL},
    { SSL_STEREO,           3UL,          0x480CC040UL,         0x2E630UL,       0xFFF0UL},
};

WGConfig_t sWGConigTable[ MAX_NUM_OF_WG ] =
{
    /* Smaple over Freq, Wave Type,     Idle Time(ms) Attack Time(ms) Stable Time(ms)  Release Time(ms) Filter Type, Repeat*/
    { (48000.0/16000.0), WG_SINE_WAVE,         10UL,              0UL,          1000UL,             0UL,         0x0UL,      0UL},
    { (48000.0/500.0),   WG_SINE_WAVE,         0UL,               0UL,          10000UL,            0UL,         0x0UL,      0UL},
    { (48000.0/4000.0),  WG_SINE_WAVE,         100UL,             0UL,          5000UL,             0UL,         0x0UL,      0UL},
    { (48000.0/8000.0),  WG_SINE_WAVE,         100UL,             0UL,          5000UL,             0UL,         0x0UL,      0UL},
    { (48000.0/16000.0), WG_SINE_WAVE,         100UL,             0UL,          5000UL,             0UL,         0x0UL,      0UL},
};

SRCConfig_t sSRCConigTable[ MAX_NUM_OF_SRC ] =
{
    /*SRC or ByPass     Fifo Set Point,   Initital Zero Size,   Ratio*/
    { SRC_SRC_MODE,     16UL,               16UL,                   1.5/1.0},  /* SSL0 */
    { SRC_BYPASS_MODE,  16UL,               16UL,                   3.0/1.0},  /* SSL1 */
    { SRC_BYPASS_MODE,  16UL,               16UL,                   2.0/1.0},  /* SSL2 */
    { SRC_BYPASS_MODE,  16UL,               16UL,                   1.5/1.0},  /* SSL3 */
    { SRC_BYPASS_MODE,  16UL,               16UL,                   1.0/1.0},  /* SSL4 */
    { SRC_BYPASS_MODE,  16UL,               16UL,                   1.0/1.0},  /* TAS */
};

AVCConfig_t sAVCConigTable[ MAX_NUM_OF_AVC ] =
{
    /*1sec / 48000 = 20833.333 nano sec
      10.4msec = 20833.333 nano sec * 500frame
      check WG idle time of WG When setting wait time
    */
    /* Mode         Period,  Interval,   Wait,  Gain */
    { AVC_VOL_CTL,     1UL,     5000UL,     100UL, (-2)}, /* WG0 */
    { AVC_VOL_CTL,     1UL,     5000UL,     100UL, (-2)}, /* WG1 */
    { AVC_VOL_CTL,     1UL,     5500UL,     100UL, (-2)}, /* WG2 */
    { AVC_VOL_CTL,     1UL,     5000UL,     100UL, (-2)}, /* WG3 */
    { AVC_VOL_CTL,     1UL,     5000UL,     100UL, (-2)}, /* WG4 */
    { AVC_VOL_CTL,     1UL,     20000UL,     5000UL, (10)}, /* SSL0 - CH0 */
    { AVC_VOL_CTL,     1UL,     20000UL,     5000UL, (10)}, /* SSL0 - CH1 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL1 - CH0 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL1 - CH1 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL2 - CH0 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL2 - CH1 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL3 - CH0 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL3 - CH1 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL4 - CH0 */
    { AVC_BYPASS,      5UL,     1000UL,     5000UL, (-2)}, /* SSL5 - CH1 */
    { 0UL,             0UL,        0UL,        0UL,  0UL}, /* Zero */
};

/*
***************************************************************************************************
*                                           set_audio_interface
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void set_audio_interface
(
    void
)
{
    if(g_initialized_audio_interface == 0UL)
    {
        /* AIC, Codec Setting */
        uint32 fifo_size    = 0UL;
        uint8 codec_I2Cch = 0UL;
        uint32 codec_I2CPortSel = 0UL;

        I2SConfig_t        spu_i2s_config;

        /* I2S Clock */
        spu_audio_init_48khz((I2SConfig_t *)&spu_i2s_config, fifo_size);
        I2S_SetGpiofunction((I2SConfig_t *)&spu_i2s_config);
        I2S_SetClock((I2SConfig_t *)&spu_i2s_config);

        /* HW Codec */
        (void)GPIO_Config( SPU_AUDIO_MUTE_PIN, ( uint32 ) ( GPIO_FUNC( 0U ) | GPIO_OUTPUT ) );

    #ifdef WM8731
        WM8731_Initial(codec_I2Cch, codec_I2CPortSel, (I2SConfig_t *)&spu_i2s_config);
        WM8731_Power(codec_I2Cch, WM8731_DACPOWER_DACON);
        WM8731_DACSoftMute(codec_I2Cch, SALDisabled);
        WM8731_SetAudioOutVolume(codec_I2Cch, 30);
    #else
        WM8904_Initial(codec_I2Cch, codec_I2CPortSel, (I2SConfig_t *)&spu_i2s_config);
        WM8904_SetVolume(codec_I2Cch, 100);
    #endif

        (void)GPIO_Set( SPU_AUDIO_MUTE_PIN, 0UL ); //Set to Low for NOT Mute

        /* DAIF Setup */
        I2S_DaifSetting((I2SConfig_t *)&spu_i2s_config);
        /* DAIF TAS Mode for using SPU  */
        I2S_SetTASMode();
        /* DAIF Transmitter Enable */
        I2S_DAIEnable(I2S_DOUT);
        /* DAIF Enable */
        I2S_DAMREnable();
        g_initialized_audio_interface = 1UL;
    }
}

/*
***************************************************************************************************
*                                           SPU_Test_SSL
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void SPU_Test_SSL
(
    void
)
{
    set_audio_interface();

    /* Initialize SPU */
    SPU_Init((SRCConfig_t*)&sSRCConigTable, (AVCConfig_t*)&sAVCConigTable);

    /* SSL0 load & Force Stop */
    SPU_Start_SSL_Chain(SPU_SSL_0, (SSLConfig_t*)&SSLConigTable);
    (void) SAL_TaskSleep(3000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_0, SSL_FORCE_STOP);

    /* Sleep for next */
    (void) SAL_TaskSleep(1000UL);

    /* SSL1 load & Force Stop */
    SPU_Start_SSL_Chain(SPU_SSL_1, (SSLConfig_t*)&SSLConigTable);
    (void) SAL_TaskSleep(3000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_1, SSL_FORCE_STOP);

    /* Sleep for next */
    (void) SAL_TaskSleep(1000UL);

    /* SSL2 load & Force Stop */
    SPU_Start_SSL_Chain(SPU_SSL_2,(SSLConfig_t*)&SSLConigTable);
    (void) SAL_TaskSleep(3000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_2, SSL_FORCE_STOP);

    /* Sleep for next */
    (void) SAL_TaskSleep(1000UL);

    /* SSL3 load & Stable Stop */
    SPU_Start_SSL_Chain(SPU_SSL_3,(SSLConfig_t*)&SSLConigTable);
    (void) SAL_TaskSleep(3000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_3, SSL_STABLE_STOP);

    /* Sleep for next */
    (void) SAL_TaskSleep(1000UL);

    /* SSL4 load & Repeat Stop*/
    SPU_Start_SSL_Chain(SPU_SSL_4, (SSLConfig_t*)&SSLConigTable);

    /* Sleep for next */
    (void) SAL_TaskSleep(1000UL);
    SPU_DeInit();
}

/*
***************************************************************************************************
*                                           SPU_Test_SSL_Interrupt
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void SPU_Test_SSL_Interrupt
(
    void
)
{
    set_audio_interface();

    /* Initialize SPU */
    SPU_Init((SRCConfig_t*)&sSRCConigTable, (AVCConfig_t*)&sAVCConigTable);

    SPU_Start_SSL_Chain(SPU_SSL_0, (SSLConfig_t*)&SSLConigTable);
    SPU_Start_SSL_Chain(SPU_SSL_1, (SSLConfig_t*)&SSLConigTable);
    SPU_Start_SSL_Chain(SPU_SSL_2, (SSLConfig_t*)&SSLConigTable);
    SPU_Start_SSL_Chain(SPU_SSL_3, (SSLConfig_t*)&SSLConigTable);
    SPU_Start_SSL_Chain(SPU_SSL_4, (SSLConfig_t*)&SSLConigTable);

    (void) SAL_TaskSleep(1000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_0, SSL_FORCE_STOP);

    (void) SAL_TaskSleep(1000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_1, SSL_FORCE_STOP);

    (void) SAL_TaskSleep(1000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_2, SSL_FORCE_STOP);

    (void) SAL_TaskSleep(1000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_3, SSL_FORCE_STOP);

    (void) SAL_TaskSleep(1000UL);
    SPU_Stop_SSL_Chain(SPU_SSL_4, SSL_FORCE_STOP);

    (void) SAL_TaskSleep(1000UL);

    SPU_DeInit();
}

/*
***************************************************************************************************
*                                           SPU_Test_WG
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void SPU_Test_WG
(
    void
)
{
    set_audio_interface();

    /* Initialize SPU */
    SPU_Init((SRCConfig_t*)&sSRCConigTable, (AVCConfig_t*)&sAVCConigTable);

    SPU_Start_WG_Chain(SPU_WG_0, (WGConfig_t*)&sWGConigTable);
    SPU_Start_WG_Chain(SPU_WG_1, (WGConfig_t*)&sWGConigTable);
    SPU_Start_WG_Chain(SPU_WG_2, (WGConfig_t*)&sWGConigTable);
    SPU_Start_WG_Chain(SPU_WG_3, (WGConfig_t*)&sWGConigTable);
    SPU_Start_WG_Chain(SPU_WG_4, (WGConfig_t*)&sWGConigTable);

    (void) SAL_TaskSleep(5000UL);

    SPU_Stop_WG_Chain(SPU_WG_0);
    SPU_Stop_WG_Chain(SPU_WG_1);
    SPU_Stop_WG_Chain(SPU_WG_2);
    SPU_Stop_WG_Chain(SPU_WG_3);
    SPU_Stop_WG_Chain(SPU_WG_4);

    (void) SAL_TaskSleep(1000UL);

    SPU_DeInit();
}

/*
***************************************************************************************************
*                                           SPU_Test_MAVC
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void SPU_Test_MAVC
(
    void
)
{
    set_audio_interface();

    /* Initialize SPU */
    SPU_Init((SRCConfig_t*)&sSRCConigTable, (AVCConfig_t*)&sAVCConigTable);

    /* WG0 Sine Wave */
    SPU_Start_WG_Chain(SPU_WG_0, (WGConfig_t*)&sWGConigTable);
    (void) SAL_TaskSleep(2000UL);
    SPU_Stop_WG_Chain(SPU_WG_0);
    (void) SAL_TaskSleep(50UL);

    /* WG1 Saw Wave */
    SPU_Start_WG_Chain(SPU_WG_1, (WGConfig_t*)&sWGConigTable);
    (void) SAL_TaskSleep(2000UL);
    SPU_Stop_WG_Chain(SPU_WG_1);
    (void) SAL_TaskSleep(50UL);

    /* WG2 Square Wave */
    SPU_Start_WG_Chain(SPU_WG_2, (WGConfig_t*)&sWGConigTable);
    (void) SAL_TaskSleep(2000UL);
    SPU_Stop_WG_Chain(SPU_WG_2);
    (void) SAL_TaskSleep(50UL);

    /* SSL0 load Sine Wave */
    SPU_Start_SSL_Chain(SPU_SSL_0,(SSLConfig_t*)&SSLConigTable);
    (void) SAL_TaskSleep(1000UL);

    /* Forced stop */
    SPU_Stop_SSL_Chain(SPU_SSL_0, SSL_FORCE_STOP);

    (void) SAL_TaskSleep(1000UL);
    SPU_DeInit();
}

/*
***************************************************************************************************
*                                           SPU_Test_SRC
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void SPU_Test_SRC
(
    void
)
{
    set_audio_interface();

    /* Initialize SPU */
    SPU_Init((SRCConfig_t*)&sSRCConigTable, (AVCConfig_t*)&sAVCConigTable);

    /* SSL0 load Sine Wave */
    SPU_Start_SSL_Chain(SPU_SSL_0,(SSLConfig_t*)&SSLConigTable);

    (void) SAL_TaskSleep(2000UL);

    /* Forced stop */
    SPU_Stop_SSL_Chain(SPU_SSL_0, SSL_STABLE_STOP);

    (void) SAL_TaskSleep(1000UL);

    /* SSL1 load Sine Wave */
    SPU_Start_SSL_Chain(SPU_SSL_1,(SSLConfig_t*)&SSLConigTable);
    (void) SAL_TaskSleep(2000UL);

    /* Forced stop */
    SPU_Stop_SSL_Chain(SPU_SSL_1, SSL_STABLE_STOP);

    (void) SAL_TaskSleep(1000UL);

    /* SSL2 load Sine Wave */
    SPU_Start_SSL_Chain(SPU_SSL_2,(SSLConfig_t*)&SSLConigTable);

    (void) SAL_TaskSleep(2000UL);

    /* Forced stop */
    SPU_Stop_SSL_Chain(SPU_SSL_2, SSL_STABLE_STOP);

    (void) SAL_TaskSleep(1000UL);

    /* SSL3 load Sine Wave */
    SPU_Start_SSL_Chain(SPU_SSL_3,(SSLConfig_t*)&SSLConigTable);

    (void) SAL_TaskSleep(2000UL);

    /* Forced stop */
    SPU_Stop_SSL_Chain(SPU_SSL_3, SSL_STABLE_STOP);

    (void) SAL_TaskSleep(3000UL);

    /* SSL4 load Sine Wave */
    SPU_Start_SSL_Chain(SPU_SSL_4,(SSLConfig_t*)&SSLConigTable);

    (void) SAL_TaskSleep(2000UL);

    /* Forced stop */
    SPU_Stop_SSL_Chain(SPU_SSL_4, SSL_STABLE_STOP);

    (void) SAL_TaskSleep(1000UL);

    SPU_DeInit();
}

/*
***************************************************************************************************
*                                           SPU_Test_FullChain
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void SPU_Test_FullChain
(
    void
)
{
    SSLConfig_t SSLConig;

    SSLConig.uiMode = SSL_STEREO;
    SSLConig.uiRepeat = 0UL;
    SSLConig.uiFifoAddr = DATA_FLASH_ADDR;
    SSLConig.uiFifoSize = SIZE_OF_SAMPLE;
    SSLConig.uiDummySize = 0xFFF0UL;

    set_audio_interface();

    /* Initialize SPU */
    mcu_printf("\n== SPU Full Chain : Initialize  ==\n");
    SPU_Init((SRCConfig_t*)&sSRCConigTable, (AVCConfig_t*)&sAVCConigTable);

    mcu_printf("\n== SPU Full Chain : Start Waveform Generator for 10sec  ==\n");
    SPU_Start_WG_Chain(SPU_WG_0, (WGConfig_t*)&sWGConigTable);

    (void) SAL_TaskSleep(5000UL);

    /* SSL0 load Ring sound */
    mcu_printf("\n== SPU Full Chain : Start Sound Source Loader  for 10sec ==\n");
    SPU_Start_SSL_Chain(SPU_SSL_0, (SSLConfig_t*)&SSLConig);

    /* Sleep for next */
    (void) SAL_TaskSleep(5000UL);

    mcu_printf("\n== SPU Full Chain : Stop Waveform Generator ==\n");
    SPU_Stop_WG_Chain(SPU_WG_0);

    (void) SAL_TaskSleep(5000UL);

    mcu_printf("\n== SPU Full Chain : Stop Sound Source Loader  ==\n");
    SPU_Stop_SSL_Chain(SPU_SSL_0, SSL_FORCE_STOP);

   (void) SAL_TaskSleep(1000UL);

    SPU_DeInit();
}

/*
***************************************************************************************************
*                                           SPU_Test_Help
*
* @param test case
* @return
*
* Notes
*
***************************************************************************************************
*/
static void SPU_Test_Help
(
    void
)
{
    mcu_printf("\n== Test Case ==\n");
    mcu_printf("\n ssl     : spu sound source loader test \n");
    mcu_printf("\n ssl_int : spu sound source loader interrupt test \n");
    mcu_printf("\n wg      : spu wave generator test \n");
    mcu_printf("\n avc     : spu volume control test \n");
    mcu_printf("\n src     : spu sample rate convert test \n");
    mcu_printf("\n full    : spu full chain test \n");
}

/*
***************************************************************************************************
*                                           SPU_SelectTestCase
*
* @param test case
* @return
*
* Notes
*
***************************************************************************************************
*/
void SPU_SelectTestCase
(
    uint8 ucArgc,
    void* pArgv[]
)
{
    sint32                              ret;
    const uint8 *                       str;

    ret = 0;
    str = (const uint8 *)pArgv[0];

    if(str != NULL_PTR)
    {
        if ((SAL_StrNCmp(str, (const uint8 *)"ssl", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0L))
        {
            mcu_printf("\n== Test Case : SPU-SSL ==\n");
            SPU_Test_SSL();
        }
        else if ((SAL_StrNCmp(str, (const uint8 *)"ssl_int", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0L))
        {
            mcu_printf("\n== Test Case : SPU-SSL_Interrupt ==\n");
            SPU_Test_SSL_Interrupt();
        }
        else if ((SAL_StrNCmp(str, (const uint8 *)"wg", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0L))
        {
            mcu_printf("\n== Test Case : SPU-WG ==\n");
            SPU_Test_WG();
        }
        else if ((SAL_StrNCmp(str, (const uint8 *)"avc", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            mcu_printf("\n== Test Case : SPU-AVC ==\n");
            SPU_Test_MAVC();
        }
        else if ((SAL_StrNCmp(str, (const uint8 *)"src", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0L))
        {
            mcu_printf("\n== Test Case : SPU-SRC ==\n");
            SPU_Test_SRC();
        }
        else if ((SAL_StrNCmp(str, (const uint8 *)"full", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0L))
        {
            mcu_printf("\n== Test Case : SPU-Full-Chain ==\n");
            SPU_Test_FullChain();
        }
        else
        {
            mcu_printf("\n== Invaild Test Case ==\n");
            SPU_Test_Help();
        }
    }
    else
    {
        SPU_Test_Help();
    }
    return;
}

/*
***************************************************************************************************
*                                           SPU_Test_SSL_With_Key
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
void SPU_Test_SSL_With_Key
(
    unsigned int                        key_num
)
{
    SSLConfig_t SSLConig;

    SSLConig.uiMode = SSL_STEREO;
    SSLConig.uiRepeat = 0UL;
    SSLConig.uiFifoAddr = DATA_FLASH_ADDR;
    SSLConig.uiFifoSize = SIZE_OF_SAMPLE;
    SSLConig.uiDummySize = 0xFFF0UL;

    set_audio_interface();

    /* Initialize SPU */
    SPU_Init((SRCConfig_t*)&sSRCConigTable, (AVCConfig_t*)&sAVCConigTable);

    switch(key_num)
    {
        case 27:
            {
                if(SPU_Get_STS(SSL_CHAIN_STS(SPU_SSL_0)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start SSL0 ==\n");
                    SPU_Start_SSL_Chain(SPU_SSL_0, (SSLConfig_t*)&SSLConig);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop SSL0 ==\n");
                    SPU_Stop_SSL_Chain(SPU_SSL_0, SSL_FORCE_STOP);
                }
                break;
            }
        case 28:
            {
                if(SPU_Get_STS(SSL_CHAIN_STS(SPU_SSL_1)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start SSL1 ==\n");
                    SPU_Start_SSL_Chain(SPU_SSL_1, (SSLConfig_t*)&SSLConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop SSL1 ==\n");
                    SPU_Stop_SSL_Chain(SPU_SSL_1, SSL_FORCE_STOP);
                }
                break;
            }
        case 29:
            {
                if(SPU_Get_STS(SSL_CHAIN_STS(SPU_SSL_2)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start SSL2 ==\n");
                    SPU_Start_SSL_Chain(SPU_SSL_2, (SSLConfig_t*)&SSLConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop SSL2 ==\n");
                    SPU_Stop_SSL_Chain(SPU_SSL_2, SSL_FORCE_STOP);
                }
                break;
            }
        case 18:
            {
                if(SPU_Get_STS(SSL_CHAIN_STS(SPU_SSL_3)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start SSL3 ==\n");
                    SPU_Start_SSL_Chain(SPU_SSL_3, (SSLConfig_t*)&SSLConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop SSL3 ==\n");
                    SPU_Stop_SSL_Chain(SPU_SSL_3, SSL_FORCE_STOP);
                }
                break;
            }
        case 19:
            {
                if(SPU_Get_STS(SSL_CHAIN_STS(SPU_SSL_4)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start SSL4 ==\n");
                    SPU_Start_SSL_Chain(SPU_SSL_4, (SSLConfig_t*)&SSLConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop SSL4 ==\n");
                    SPU_Stop_SSL_Chain(SPU_SSL_4, SSL_FORCE_STOP);
                }
                break;
            }
        case 1:
            {
                if(SPU_Get_STS(WG_CHAIN_STS(SPU_WG_0)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start WG0 ==\n");
                    SPU_Start_WG_Chain(SPU_WG_0, (WGConfig_t*)&sWGConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop WG0 ==\n");
                    SPU_Stop_WG_Chain(SPU_WG_0);
                }
                break;
            }
        case 2:
            {
                if(SPU_Get_STS(WG_CHAIN_STS(SPU_WG_1)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start WG1 ==\n");
                    SPU_Start_WG_Chain(SPU_WG_1, (WGConfig_t*)&sWGConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop WG1 ==\n");
                    SPU_Stop_WG_Chain(SPU_WG_1);
                }
                break;
            }
        case 3:
            {
                if(SPU_Get_STS(WG_CHAIN_STS(SPU_WG_2)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start WG2 ==\n");
                    SPU_Start_WG_Chain(SPU_WG_2, (WGConfig_t*)&sWGConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop WG2 ==\n");
                    SPU_Stop_WG_Chain(SPU_WG_2);
                }
                break;
            }
        case 4:
            {
                if(SPU_Get_STS(WG_CHAIN_STS(SPU_WG_3)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start WG3 ==\n");
                    SPU_Start_WG_Chain(SPU_WG_3, (WGConfig_t*)&sWGConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop WG3 ==\n");
                    SPU_Stop_WG_Chain(SPU_WG_3);
                }
                break;
            }
        case 5:
            {
                if(SPU_Get_STS(WG_CHAIN_STS(SPU_WG_4)) == SPU_IDLE)
                {
                    mcu_printf("\n== SPU Demo : Start WG4 ==\n");
                    SPU_Start_WG_Chain(SPU_WG_4, (WGConfig_t*)&sWGConigTable);
                }
                else
                {
                    mcu_printf("\n== SPU Demo : Stop WG4 ==\n");
                    SPU_Stop_WG_Chain(SPU_WG_4);
                }
                break;
            }
        default:
            {
                mcu_printf("Invaild key button for spu demo. %d", key_num);
                break;
            }
    }


}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )


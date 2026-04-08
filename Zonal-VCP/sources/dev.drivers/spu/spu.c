// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spu.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_SPU == 1 )

#include "spu.h"
#include "gic.h"
#include <debug.h>
/***************************************************************************************************
*                                       LOCAL VARIABLES
***************************************************************************************************/
static uint32 spu_channel_status = 0x0UL;

/***************************************************************************************************
*                                       FUNCTION PROTOTYPES
***************************************************************************************************/
static void SPU_Set_STS
(
    uint32                              uiChannel,
    uint32                              uiStatus
);

/* Sound Source Loader */
static void SSL_Global_Enable
(
    void
);

static void SSL_Global_Disable
(
    void
);

static int SSL_Get_Status
(
    uint32                              uiNth
);

static int SSL_Get_IRQ_Status
(
    void
);

static void SSL_Set_IRQ
(
    uint32                              uiNth,
    uint32                              uiIRQ_Passed
);

static void SSL_Clear_IRQ
(
    uint32                              uiNth,
    uint32                              uiClear
);

static void SSL_START
(
    uint32                              uiNth,
    uint32                              uiMode,
    uint32                              uiRepeat,
    uint32                              uiFifo_Addr,
    uint32                              uiFifo_Size,
    uint32                              uiDummy_Size
) ;

static void SSL_STOP
(
    uint32                              uiNth,
    uint32                              uiForce
);

/* Wave Generator */
static void WG_Set_Configuration
(
    uint32                              uiNth,
    uint32                              uiSample_over_freq,
    uint32                              uiWave_type,
    uint32                              uiIdle_time_ms,
    uint32                              uiAttack_time_ms,
    uint32                              uiStable_time_ms,
    uint32                              uiRelease_time_ms,
    uint32                              uiFilter_type,
    uint32                              uiRepeat
);

static void WG_ENABLE
(
    void
);

static void WG_START
(
    uint32                              uiNth
);

static void WG_STOP
(
    uint32                              uiNth
);

static void WG_Disable
(
    void
);

static uint32 WG_Check_Running
(
    uint32                              uiNth
);

/* Sample Rate Converter */
static void SRC_Set_Config
(
    uint32                              uiNth,
    uint32                              uiFifo_Set_Point,
    uint32                              uiInit_Zero_Size,
    uint32                              uiRatio
);

static void SRC_Set_Bypass
(
    uint32                              uiNth,
    uint32                              uiBypass
);

static void SRC_Enable
(
    uint32                              uiNth,
    uint32                              uiEnable
);

/* Audio Volume Conteroller */
static void AVC_Set_Config
(
    uint32                              uiCH,
    uint32                              uiPeriod,
    uint32                              uiInterval,
    uint32                              uiWait,
    uint32                              uiGain
);

static void AVC_Clear
(
    uint32                              uiCH,
    uint32                              uiParam_Clear,
    uint32                              uidB_Clear
);

static void AVC_Set_Mode
(
    uint32                              uiCH,
    uint32                              uiMode
);

static void AVC_Set_Mute
(
    uint32                              uiCH,
    uint32                              uiMute
);

static void AVC_Start
(
    uint32                              uiCH
);

static int32 AVC_Get_Status
(
    uint32                              uiCH
);

/* Mixer */
static void MIXER_Set_MIXERSAT
(
    uint32                              uiMax_Value,
    uint32                              uiMin_Value
);

static void MIXER_Set_MixCombination
(
    uint32                              uiMixCombination
);

static uint32 MIXER_Get_MixCombination
(
    void
);

/******************************************************************************
 * Function name:  Convert_Gain
 * Description:    Convert gain valude, double -> unsigned int
 * Parameter:      double gain value (Resolution of gain is 0.0625)
 * Return value:   unsigned int gain value
 * Remarks:        Internal API.
 * Requirements:   None.
 ******************************************************************************/
static uint32 Convert_Gain
(
    double                              dGain
)
{
    double cal_val = 0.0f;
    unsigned int chk_res = 0UL;
    unsigned int gain_val = 0UL;

    /* check gain value expression boundary */
    if((-64.0f <= dGain) && (dGain <= 63.9375f))
    {
        /* check gain value resolution */
        chk_res = (unsigned int)(dGain * 10000.0f);
        if((chk_res % 625UL) == 0UL)
        {
            if(dGain < 0.0f)
            {
                /* Positive for MISRA C-2012 Rule 10.1 */
                cal_val = (dGain * -1.0f);
                if(0 < cal_val)
                {
                    /* 0.0625 * 16 = 1 */
                    gain_val = (unsigned int)(cal_val * (1UL<< SPU_AVC_GAIN_INTEGER_OFFSET));
                }
                else
                {
                    /* Handle Error */
                }
                /*2’s compliment for negative value */
                gain_val = ~gain_val;
                if(gain_val < 0xFFFFFFFEUL)
                {
                    gain_val++;
                }
            }
            else
            {
                /* 0.0625 * 16 = 1 */
                gain_val = (unsigned int)(dGain * (1UL<< SPU_AVC_GAIN_INTEGER_OFFSET));
            }
        }
    }
    return gain_val;
}

/******************************************************************************
 * Function name:  Convert_SOF
 * Description:    Convert sample over frequency value, double -> unsigned int
 * Parameter:      double SOF value (Resolution of SOF is 0.125, refer to 70xx full-spec)
 * Return value:   unsigned int SOF value
 * Remarks:        Internal API.
 * Requirements:   None.
 ******************************************************************************/
static uint32 Convert_SOF
(
    double                              dSample_over_freq
)
{
    unsigned int chk_res = 0UL;
    unsigned int sof_val = 0UL;

    /* check gain value resolution */
    chk_res = (unsigned int)(dSample_over_freq * 1000.0f);
    if((chk_res % 125UL) == 0UL)
    {
        /* 0.125 * 8 = 1 */
        sof_val = (unsigned int)(dSample_over_freq * (1UL<< SPU_WG_SAMPLE_FREQ_INTEGER_OFFSET));
    }
    return sof_val;
}

/******************************************************************************
 * Function name:  Convert_SRC_Ratio
 * Description:    Convert SRC ratio value, double -> unsigned int
 * Parameter:      double ratio value
 * Return value:   unsigned int ratio value
 * Remarks:        Internal API.
 * Requirements:   None.
 ******************************************************************************/
static uint32 Convert_SRC_Ratio
(
    double                              dRatio
)
{
    unsigned int ratio_val = 0UL;

    ratio_val = (unsigned int)(dRatio * (1UL<< SPU_SRC_RATIO_INTEGER_OFFSET));

    return ratio_val;
}

/*
***************************************************************************************************
*                                       SPU_Set_Channel_Status
*
* Update SPU channel status.
* @param    channel_status : channel status
* @return   void
*
***************************************************************************************************
*/
static void SPU_Set_STS
(
    uint32                              uiChannel,
    uint32                              uiStatus
)
{
    if(MAX_CH_STS < uiChannel)
    {
        SPU_E("\n SPU_Set_STS Invaild ch(%d) \n", uiChannel);
    }
    else
    {
        if(uiStatus == SPU_ACTIVE)
        {
            spu_channel_status |= ((uint32)(0x1UL) << (uiChannel));
        }
        else
        {
            spu_channel_status &= ~((uint32)(0x1UL) << (uiChannel));
        }
    }
}

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
)
{
    unsigned int ret_val = 0UL;

    if(MAX_CH_STS < uiChannel)
    {
        SPU_E("\n SPU_Set_STS Invaild ch(%d) \n", uiChannel);
        ret_val = SPU_INVALID_CH;
    }
    else
    {
        ret_val = (spu_channel_status & (0x1UL << (uiChannel))) >> uiChannel;
    }

    return ret_val;
}

/*
***************************************************************************************************
*                                       SPU_Init
*
* SPU Initialize.
* @param    psSRCConfig : SRC config table for 6th SRC(final TAS stereo)
* @return   void
*
***************************************************************************************************
*/
void SPU_Init
(
    const SRCConfig_t *                 psSRCConfig,
    const AVCConfig_t *                 psAVCConfig
)
{
    uint32 avc_param_clear = 1UL;
    uint32 avc_db_clear = 1UL;
    uint32 num_of_src = 0UL;
    uint32 num_of_avc = 0UL;

    if(SPU_Get_STS(SPU_INIT_STS) == SPU_IDLE)
    {
        /* clear mixer ctrl */
        MIXER_Set_MixCombination(0x00000000UL);

        /* enable wave generator */
        WG_ENABLE();

        /* enable global ssl */
        SSL_Global_Enable();

        /* Initialize src */
        for(num_of_src = 0; num_of_src < MAX_NUM_OF_SRC; num_of_src++)
        {
            if((psSRCConfig[num_of_src].uiByPass == SRC_SRC_MODE))
            {
                SRC_Set_Config(num_of_src,
                                psSRCConfig[num_of_src].uiFifoSetPoint,
                                psSRCConfig[num_of_src].uiInitZeroSize,
                                Convert_SRC_Ratio(psSRCConfig[num_of_src].dRatio));
                SRC_Set_Bypass(num_of_src, SRC_SRC_MODE);
                SRC_Enable(num_of_src, SPU_ENABLE);
            }
            else
            {
                SRC_Set_Bypass (num_of_src, SRC_BYPASS_MODE);
                SRC_Enable(num_of_src, SPU_DISABLE);
            }
        }

        /* Initialize avc */
        for(num_of_avc = 0UL; num_of_avc < MAX_NUM_OF_AVC; num_of_avc++)
        {
            AVC_Clear(num_of_avc, avc_param_clear, avc_db_clear);

            /* MAVC Setting */
            if(psAVCConfig[num_of_avc].uiMode == AVC_BYPASS)
            {
                AVC_Set_Mode(num_of_avc, AVC_BYPASS);
            }
            else
            {
                AVC_Set_Config(num_of_avc,
                                psAVCConfig[num_of_avc].uiPeriod,
                                psAVCConfig[num_of_avc].uiInterval,
                                psAVCConfig[num_of_avc].uiWait,
                                Convert_Gain(psAVCConfig[num_of_avc].dGain));
                AVC_Set_Mode(num_of_avc, AVC_VOL_CTL);
                AVC_Start(num_of_avc);
            }
        }
        SPU_Set_STS(SPU_INIT_STS, SPU_ACTIVE);
    }
}

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
)
{
    if(SPU_Get_STS(SPU_INIT_STS) == SPU_ACTIVE)
    {
        /* Disable All Wave Generator */
        WG_Disable();

        /* Disable SSL - Global */
        SSL_Global_Disable();
#if 0 //Temp.
        uint32 num_of_src = 0x0UL;
        /* Disable SRC */
        for(num_of_src = 0UL; num_of_src < MAX_NUM_OF_SRC; num_of_src++)
        {
            SRC_Enable(num_of_src, SPU_DISABLE);
        }
#endif
        SPU_Set_STS(SPU_INIT_STS, SPU_IDLE);
    }
}

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
)
{
    uint32 mixer_combination = 0x0UL;

    if((uiNth < MAX_NUM_OF_WG) && (SPU_Get_STS(WG_CHAIN_STS(uiNth)) == SPU_IDLE))
    {
        /* WG Setting */
        WG_Set_Configuration(uiNth,
                            Convert_SOF(psWgConfig[uiNth].dSampleOverFreq),
                            psWgConfig[uiNth].uiWaveType,
                            psWgConfig[uiNth].uiIdleTimeMS,
                            psWgConfig[uiNth].uiAttackTimeMS,
                            psWgConfig[uiNth].uiStableTimeMS,
                            psWgConfig[uiNth].uiReleaseTimeMS,
                            psWgConfig[uiNth].uiFilterType,
                            psWgConfig[uiNth].uiTonegenRepeat);
        WG_START(uiNth);

        mixer_combination = MIXER_Get_MixCombination();
        mixer_combination |= (1UL << uiNth);
        mixer_combination |= (1UL << (uiNth + 16UL));
        MIXER_Set_MixCombination(mixer_combination);

        SPU_Set_STS(WG_CHAIN_STS(uiNth), SPU_ACTIVE);
    }
}

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
)
{
    uint32 mixer_combination = 0x0UL;

    if((uiNth < MAX_NUM_OF_WG) && (SPU_Get_STS(WG_CHAIN_STS(uiNth)) == SPU_ACTIVE) )
    {
        WG_STOP(uiNth);

        mixer_combination = MIXER_Get_MixCombination();
        mixer_combination &= ~(1UL << uiNth);
        mixer_combination &= ~(1UL << (uiNth + 16UL));
        MIXER_Set_MixCombination(mixer_combination);

        SPU_Set_STS(WG_CHAIN_STS(uiNth), SPU_IDLE);
    }
}

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
)
{
    uint32 mixer_combination = 0x0UL;

    if((uiNth < MAX_NUM_OF_SSL) && (SPU_Get_STS(SSL_CHAIN_STS(uiNth)) == SPU_IDLE))
    {
        /* SSL Setting */
        SSL_START(uiNth,
                    psSSLConfig[uiNth].uiMode,
                    psSSLConfig[uiNth].uiRepeat,
                    psSSLConfig[uiNth].uiFifoAddr,
                    psSSLConfig[uiNth].uiFifoSize,
                    psSSLConfig[uiNth].uiDummySize);

        mixer_combination = MIXER_Get_MixCombination();
        mixer_combination |= (1UL << ((uiNth * 2UL) + MAX_NUM_OF_WG/*ch0 of SSL-n*/));
        mixer_combination |= (1UL << ((uiNth * 2UL) + MAX_NUM_OF_WG + 16UL + 1UL/*ch1 of SSL-n*/));
        MIXER_Set_MixCombination(mixer_combination);

        SPU_Set_STS(SSL_CHAIN_STS(uiNth), SPU_ACTIVE);
    }
}

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
)
{
    uint32 mixer_combination = 0x0UL;

    if((uiNth < MAX_NUM_OF_SSL) && (SPU_Get_STS(SSL_CHAIN_STS(uiNth)) == SPU_ACTIVE))
    {
        SSL_STOP(uiNth, uiForced);

        mixer_combination = MIXER_Get_MixCombination();
        mixer_combination &= ~(1UL << ((uiNth * 2UL) + MAX_NUM_OF_WG/*ch0*/));
        mixer_combination &= ~(1UL << ((uiNth * 2UL) + MAX_NUM_OF_WG + 16UL + 1UL /*ch1*/));
        MIXER_Set_MixCombination(mixer_combination);

        SPU_Set_STS(SSL_CHAIN_STS(uiNth), SPU_IDLE);
    }
}

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
)
{
    unsigned int cnt = 0x0UL;
    unsigned int avc_ch = 0x0UL;

    if(uiAvcCH < SPU_AVC_MAX)
    {
        for(cnt = 0; cnt < 2UL/* stereo */; cnt++ )
        {
            avc_ch = (uint32)uiAvcCH + cnt;
            if(psAVCConfig[avc_ch].uiMode == AVC_BYPASS)
            {
                AVC_Set_Mode(avc_ch, AVC_BYPASS);
            }
            else
            {
                AVC_Set_Config(avc_ch,
                                psAVCConfig[avc_ch].uiPeriod,
                                psAVCConfig[avc_ch].uiInterval,
                                psAVCConfig[avc_ch].uiWait,
                                Convert_Gain(psAVCConfig[avc_ch].dGain));
                AVC_Set_Mode(avc_ch, AVC_VOL_CTL);
                AVC_Start(avc_ch);
            }

            if(uiAvcCH < SPU_AVC_SSL0)/* mono */
            {
                break;
            }
        }
    }
    else
    {
        SPU_E("\n SPU Invalid AVC Num-%d \n", uiAvcCH);
    }
}

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
)
{
    unsigned int cnt = 0UL;
    unsigned int avc_ch = 0UL;

    if(uiAvcCH < SPU_AVC_MAX)
    {
        for(cnt = 0UL; cnt < 2UL/* stereo */; cnt++ )
        {
            avc_ch = (uint32)uiAvcCH + cnt;
            AVC_Set_Mute(avc_ch, uiMute);
            if(uiAvcCH < SPU_AVC_SSL0)/* mono */
            {
                break;
            }
        }
    }
    else
    {
        SPU_E("\n SPU Invalid AVC Num-%d \n", uiAvcCH);
    }
}

/*
***************************************************************************************************
*                                       SSL_Global_Enable
*
* Enable all SSL.
*
* @param    void
* @return   void
*
***************************************************************************************************
*/
static void SSL_Global_Enable
(
    void
)
{
    SPU_SSL_CFG_REG |= SPU_SSL_GEN_BIT;
}

/*
***************************************************************************************************
*                                       SSL_Global_Disable
*
* Disable all SSL
*
* @param    void
* @return   void
*
***************************************************************************************************
*/
static void SSL_Global_Disable
(
    void
)
{
    SPU_SSL_CFG_REG &= ~(SPU_SSL_GEN_BIT);
}

/*
***************************************************************************************************
*                                       SSL_Get_Status
*
* Get SSL Status.
*
* @param    uiNth : num of SSL
* @return   status value
*
* Notes
* 0: IDLE (Not working)
* 1: DATA (Fetch data from memory)
* 2: DUMMY (Zero data)
* 3: FINAL (Receive stable stop request)
***************************************************************************************************
*/
static int SSL_Get_Status
(
    uint32                              uiNth
)
{
    uint32  status = 0UL;
    uint32  ret = SPU_INVALID_CH;

    if(uiNth < MAX_NUM_OF_SSL)
    {
        status = SPU_SSL_STATUS_REG;
        status = (status >> SPU_SSL_STS_OFFSET(uiNth)) & SPU_SSL_STS_MASK;
        ret = status;
    }
    else
    {
        SPU_E("\n SPU Invalid SSL Num-%d \n", uiNth);
    }

    return ret;
}

/*
***************************************************************************************************
*                                       SSL_IrqHandler
*
* SSL Interrupt Service Routine.
*
* @param : argument
*
* Notes
* When SSL status is into IDLE, This ISR must be called
*
***************************************************************************************************
*/
static void SSL_IrqHandler
(
    /* Deviation Record - MISRA C-2012 Rule 8.13, Declarations and Definitions */
    void *                              pArgument
)
{
    uint32  uiNum_of_ssl = 0;
    uint32  uiIrq_status = 0;

    (void)pArgument;

    uiIrq_status = SSL_Get_IRQ_Status();
    for(uiNum_of_ssl = 0; uiNum_of_ssl < MAX_NUM_OF_SSL; uiNum_of_ssl++)
    {
        if(uiIrq_status == (0x1U << uiNum_of_ssl))
        {
            //STOP = 0, ENABLE = 0
            SPU_SSL_FIFO_CFG_REG(uiNum_of_ssl) &= ~(SPU_SSL_FIFO_CFG_EN_BIT);
            SPU_SSL_FIFO_CFG_REG(uiNum_of_ssl) &= ~(SPU_SSL_FIFO_CFG_STOP_BIT);
            SSL_Clear_IRQ(uiNum_of_ssl, SSL_IRQ_STATUS_CLEAR);
        }
    }
}

/*
***************************************************************************************************
*                                       SSL_Get_IRQ_Status
*
* Get SSL IRQ Status.
*
* @param    void
* @return   IRQ status value
*
* Notes
* 0: No interrupt occurred
* 1: Interrupt occurred (SSL is stopped)
***************************************************************************************************
*/
static int SSL_Get_IRQ_Status
(
    void
)
{
    return (0x1FUL & SPU_SSL_IRQ_STATUS_REG);
}

/*
***************************************************************************************************
*                                       SSL_Set_IRQ
*
* Set IRQ Mask.
*
* @param    uiNth          : num of SSL
*           uiIRQ_Passed 1 : IRQ is Passed or
*           uiIRQ_Passed 0 : IRQ is Masked
*
* @return   void
*
***************************************************************************************************
*/
static void SSL_Set_IRQ
(
    uint32                              uiNth,
    uint32                              uiIRQ_Passed
)
{
    if(uiNth < MAX_NUM_OF_SSL)
    {
        if(uiIRQ_Passed == SSL_IRQ_PASSED)
        {
            (void)GIC_IntVectSet((uint32)GIC_SPU,
                                 (uint32)GIC_PRIORITY_NO_MEAN,
                                 (uint8)GIC_INT_TYPE_LEVEL_HIGH,
                                 (GICIsrFunc)&SSL_IrqHandler,
                                 (void *) 0);

            (void)GIC_IntSrcEn((uint32)GIC_SPU);
            SPU_SSL_IRQ_CTRL_REG |= SPU_SSL_IRQ_MASK_BIT(uiNth);
        }
        else
        {
            SPU_SSL_IRQ_CTRL_REG &= ~SPU_SSL_IRQ_MASK_BIT(uiNth);
        }
    }
    else
    {
        SPU_E("\n SPU Invalid SSL Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       SSL_Clear_IRQ
*
* Clear IRQ.
*
* @param    uiNth     : num of SSL
*           uiClear 1 :  Clear IRQ status or
*           uiClear 0 :  No Action
*
* @return   void
*
* Notes
*   0: No action (Not Auto Cleared)
*   1: Clear status4
*
***************************************************************************************************
*/
static void SSL_Clear_IRQ
(
    uint32                              uiNth,
    uint32                              uiClear
)
{
    if(uiNth < MAX_NUM_OF_SSL)
    {
        if(uiClear == SSL_IRQ_STATUS_CLEAR)
        {
            SPU_SSL_IRQ_CTRL_REG |= SPU_SSL_IRQ_CLR_BIT(uiNth);
            SPU_SSL_IRQ_CTRL_REG &= ~SPU_SSL_IRQ_CLR_BIT(uiNth);
        }
    }
    else
    {
        SPU_E("\n SPU Invalid SSL Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       SSL_START
*
* Start SSL(sound source loader)
*
* @param    uiNth        : num of SSL
*           uiMode       : 1(Stereo) or 0(Mono)
*           uiRepeat 0   : 0(Infinite) or Finite
*           uiFifo_Addr  : Address of Data (must be a multiple of 16)
*           uiFifo_Size  : Size of data (must be a multiple of 16)
*           uiDummy_Size : Size of Dummy Data (must be a multiple of 4)
*
* @return   void
*
* Notes
*
***************************************************************************************************
*/
static void SSL_START
(
    uint32                              uiNth,
    uint32                              uiMode,
    uint32                              uiRepeat,
    uint32                              uiFifo_Addr,
    uint32                              uiFifo_Size,
    uint32                              uiDummy_Size
)
{
    uint32  fifo_cfg = 0UL;
    uint32  fifo_addr = 0UL;
    uint32  fifo_size = 0UL;
    uint32  fifo_dummy_size = 0UL;
    uint32  iteration = 0UL;

    if(uiNth < MAX_NUM_OF_SSL)
    {
        if(SSL_Get_Status(uiNth) == SSL_STS_IDLE)
        {
            SSL_Set_IRQ(uiNth, SSL_IRQ_PASSED);

            /* FIFO Address Multiple of 16 */
            fifo_addr = ((uiFifo_Addr + 15) >> 4) << 4;
            SPU_SSL_FIFO_ADDR_REG(uiNth) = fifo_addr;

            /* FIFO Address Multiple of 16 */
            fifo_size = ((uiFifo_Size + 15) >> 4) << 4;
            SPU_SSL_FIFO_SIZE_REG(uiNth) = fifo_size;

            /* FIFO Address Multiple of 4 */
            fifo_dummy_size = ((uiDummy_Size + 3) >> 2) << 2;
            SPU_SSL_DUMMY_SIZE_REG(uiNth) = fifo_dummy_size;

            /* FIFO Config Repeat */
            fifo_cfg = ((uiRepeat & SPU_SSL_FIFO_CFG_REPEAT_MASK) << SPU_SSL_FIFO_CFG_REPEAT_OFFSET);

            /* FIFO Config 1: Stereo Mode, 0 : Mono Mode */
            if(uiMode == SSL_STEREO)
            {
                fifo_cfg = fifo_cfg | SPU_SSL_FIFO_CFG_MODE_BIT;
            }
            else
            {
                fifo_cfg = fifo_cfg & ~(SPU_SSL_FIFO_CFG_MODE_BIT);
            }
            /* FIFO Config Enable */
            fifo_cfg = fifo_cfg | SPU_SSL_FIFO_CFG_EN_BIT;
            SPU_SSL_FIFO_CFG_REG(uiNth) = fifo_cfg;

            /* It is for Repeat Stop, After Repeatation SSL is into IDLE */
            if(uiRepeat != 0)
            {
                for (iteration = 0; iteration < 10000UL; iteration++)
                {
                    /* wait for STS_DATA */
                    if(SSL_Get_Status(uiNth) != (uint32)SSL_STS_IDLE)
                    {
                        break;
                    }
                }
                fifo_cfg = fifo_cfg & ~(SPU_SSL_FIFO_CFG_EN_BIT);
                SPU_SSL_FIFO_CFG_REG(uiNth) = fifo_cfg;
            }
        }
        else
        {
            SPU_E("\n SPU SSL is not IDLE status %d \n", SSL_Get_Status(uiNth));
        }
    }
    else
    {
        SPU_E("\n SPU Invalid SSL Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       SSL_STOP
*
* Stop Sound Source Loader
*
* @param    uiNth   : num of SSL
*           uiForce : Set Force Stop
*
* @return   void
*
***************************************************************************************************
*/
static void SSL_STOP
(
    uint32                              uiNth,
    uint32                              uiForce
)
{
    if(uiNth < MAX_NUM_OF_SSL)
    {
        if (uiForce == SSL_FORCE_STOP)
        {
            //Force STOP : STOP = 1, ENABLE = 0
            SPU_SSL_FIFO_CFG_REG(uiNth) &= ~(SPU_SSL_FIFO_CFG_EN_BIT);
            SPU_SSL_FIFO_CFG_REG(uiNth) |= SPU_SSL_FIFO_CFG_STOP_BIT;
        }
        else
        {
            //Stable STOP = 1, ENABLE = 1
            SPU_SSL_FIFO_CFG_REG(uiNth) |= SPU_SSL_FIFO_CFG_EN_BIT;
            SPU_SSL_FIFO_CFG_REG(uiNth) |= SPU_SSL_FIFO_CFG_STOP_BIT;
        }
    }
    else
    {
        SPU_E("\n SPU Invalid SSL Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       WG_Set_Configuration
*
* Set Wave Generator Configuration
*
* @param    uiNth              : num of WG
*           uiSample_over_freq : Set Force Stop
*           uiWave_type        : sin, saw, square wave
*           uiIdle_time_ms     : Amount of time to wait before gen start
*           uiAttack_time_ms   : Fade in time
*           uiStable_time_ms   : Constant db
*           uiRelease_time_ms  : Fade out time
*           uiFilter_type      : bypass, 8Khz, 5Khz, 2Khz low pass filter
*           uiRepeat           : Infinite(0x0), Finite mode (num of repeatation)
*
* @return   void
*
***************************************************************************************************
*/
static void WG_Set_Configuration
(
    uint32                              uiNth,
    uint32                              uiSample_over_freq,
    uint32                              uiWave_type,
    uint32                              uiIdle_time_ms,
    uint32                              uiAttack_time_ms,
    uint32                              uiStable_time_ms,
    uint32                              uiRelease_time_ms,
    uint32                              uiFilter_type,
    uint32                              uiRepeat
)
{
    uint32 cfg1_val = 0UL;
    uint32 cfg2_val = 0UL;
    uint32 cfg3_val = 0UL;
    uint32 cfg4_val = 0UL;

    if(uiNth < MAX_NUM_OF_WG)
    {
        cfg1_val = ((uiWave_type & SPU_WG_WAVE_TYPE_MASK) << SPU_WG_WAVE_TYPE_OFFSET) |
                    ((uiSample_over_freq & SPU_WG_SAMPLE_FREQ_MASK) << SPU_WG_SAMPLE_FREQ_OFFSET);

        cfg2_val = ((uiAttack_time_ms & SPU_WG_ATTACK_TIME_MASK) << SPU_WG_ATTACK_TIME_OFFSET) |
                    ((uiIdle_time_ms & SPU_WG_IDLE_TIME_MASK) << SPU_WG_IDLE_TIME_OFFSET);

        cfg3_val = ((uiRelease_time_ms & SPU_WG_RELEASE_TIME_MASK) << SPU_WG_RELEASE_TIME_OFFSET) |
                    ((uiStable_time_ms & SPU_WG_STABLE_TIME_MASK) << SPU_WG_STABLE_TIME_OFFSET);

        cfg4_val = ((uiRepeat & SPU_WG_REPEAT_MASK) << SPU_WG_REPEAT_OFFSET) |
                    ((uiFilter_type & SPU_WG_FILTER_TYPE_MASK) << SPU_WG_FILTER_TYPE_OFFSET);

        SPU_WG_CFG1_REG(uiNth) = cfg1_val;
        SPU_WG_CFG2_REG(uiNth) = cfg2_val;
        SPU_WG_CFG3_REG(uiNth) = cfg3_val;
        SPU_WG_CFG4_REG(uiNth) = cfg4_val;
    }
    else
    {
        SPU_E("\n SPU Invalid WG Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       WG_ENABLE
*
* Enable All Wave Generator.
*
* @param    void
* @return   void
*
* Notes
*   To start Nth WG, Must enable all wave generator.
*
***************************************************************************************************
*/
static void WG_ENABLE
(
    void
)
{
    uint32  uiNum_of_WG = 0UL;

    for(uiNum_of_WG = 0U; uiNum_of_WG < MAX_NUM_OF_WG; uiNum_of_WG++)
    {
        SPU_WG_CFG0_REG(uiNum_of_WG) |= SPU_WG_ENABLE_BIT;
    }
}

/*
***************************************************************************************************
*                                       WG_START
*
* START Nth Wave Generator.
*
* @param    uiNth              : num of WG
* @return   void
*
* Notes
*   To start Nth WG, Must enable all wave generator.
*
***************************************************************************************************
*/
static void WG_START
(
    uint32                              uiNth
)
{
    if(uiNth < MAX_NUM_OF_WG)
    {
        /* start Nth wave generation */
        SPU_WG_CFG0_REG(uiNth) |= SPU_WG_START_BIT;
    }
    else
    {
        SPU_E("\n SPU Invalid WG Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       WG_STOP
*
* STOP Nth Wave Generator.
*
* @param    uiNth              : num of WG
* @return   void
*
* Notes
*
***************************************************************************************************
*/
static void WG_STOP
(
    uint32                              uiNth
)
{
    if(uiNth < MAX_NUM_OF_WG)
    {
        /* force stop Nth, clear start, stop bit automatically */
        SPU_WG_CFG0_REG(uiNth) |= SPU_WG_STOP_BIT;
    }
    else
    {
        SPU_E("\n SPU Invalid WG Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       WG_Disable
*
* Disable All Wave Generator 0-4
*
* @param    void
* @return   void
*
* Notes
*
***************************************************************************************************
*/
static void WG_Disable
(
    void
)
{
    /* disable all WG */
    uint32  uiNum_of_WG = 0UL;

    for(uiNum_of_WG = 0U; uiNum_of_WG < MAX_NUM_OF_WG; uiNum_of_WG++)
    {
        SPU_WG_CFG0_REG(uiNum_of_WG) &= ~(SPU_WG_ENABLE_BIT);
    }
}

/*
***************************************************************************************************
*                                        WG_Check_Running
*
* Check START Bit of Wave Generator 0-4
*
* @param    uiNth              : num of WG
* @return   1 : Wave Generation,  0 : Zero Data Generation
*
* Notes
*
***************************************************************************************************
*/
static uint32 WG_Check_Running
(
    uint32                              uiNth
)
{
    uint32 uiCfg0_val = 0UL;
    uint32 ret = SPU_INVALID_CH;

    if(uiNth < MAX_NUM_OF_WG)
    {
        /* check start bit */
        uiCfg0_val = SPU_WG_CFG0_REG(uiNth);
        ret = uiCfg0_val & SPU_WG_START_BIT;
    }
    else
    {
        SPU_E("\n SPU Invalid WG Num-%d \n", uiNth);
    }

    return ret;
}

/*
***************************************************************************************************
*                                       SRC_Set_Config
*
* Set SRC Configuration
*
* @param    uiNth              : num of WG
*           uiFifo_Set_Point   : Buffer level for SRC
*           uiInit_Zero_Size   : Zero padding size
*           uiRatio            : Ratio for SRC
* @return   void
*
***************************************************************************************************
*/
static void SRC_Set_Config
(
    uint32                              uiNth,
    uint32                              uiFifo_Set_Point,
    uint32                              uiInit_Zero_Size,
    uint32                              uiRatio
)
{
    uint32 cfg_val = 0UL;
    uint32 ratio_val = 0UL;

    if(uiNth < MAX_NUM_OF_SRC)
    {
        /* INIT = 1, to initialize SRC */
        SPU_SRC_CFG_REG(uiNth) |= SPU_SRC_INIT_BIT;

        /* INIT = 0, to initialize SRC */
        SPU_SRC_CFG_REG(uiNth) &= ~SPU_SRC_INIT_BIT;

        /* set config */
        cfg_val = ((uiFifo_Set_Point & SPU_SRC_FIFO_POINT_MASK) << SPU_SRC_FIFO_POINT_OFFSET) |
                ((uiInit_Zero_Size & SPU_SRC_ZERO_SIZE_MASK) << SPU_SRC_ZERO_SIZE_OFFSET);

        /* set ratio */
        ratio_val = ((uiRatio & SPU_SRC_RATIO_MASK) << SPU_SRC_RATIO_OFFSET);

        SPU_SRC_CFG_REG(uiNth) = cfg_val;
        SPU_SRC_RATIO_REG(uiNth) = ratio_val;
    }
    else
    {
        SPU_E("\n SPU Invalid SRC Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       SRC_Set_Bypass
*
* Set SRC Bypass Mode
*
* @param    uiNth              : num of WG
*           uiBypass           : if 1 Bypass, 0 SRC
* @return   void
*
*
***************************************************************************************************
*/
static void SRC_Set_Bypass
(
    uint32                              uiNth,
    uint32                              uiBypass
)
{
    if(uiNth < MAX_NUM_OF_SRC)
    {
        if(uiBypass == SRC_BYPASS_MODE)
        {
            SPU_SRC_CFG_REG(uiNth) |= SPU_SRC_BYPASS_BIT;
        }
        else
        {
            SPU_SRC_CFG_REG(uiNth) &= ~(SPU_SRC_BYPASS_BIT);
        }
    }
    else
    {
        SPU_E("\n SPU Invalid SRC Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       SRC_Enable
*
* Enable SRC
*
* @param    uiNth              : num of WG
*           uiEnable           : if 1 Enable SRC, 0 Disable SRC
* @return   void
*
***************************************************************************************************
*/
static void SRC_Enable
(
    uint32                              uiNth,
    uint32                              uiEnable
)
{
    if(uiNth < MAX_NUM_OF_SRC)
    {
        if(uiEnable == SPU_ENABLE)
        {
            SPU_SRC_CFG_REG(uiNth) |= SPU_SRC_ENABLE_BIT;
        }
        else
        {
            SPU_SRC_CFG_REG(uiNth) &= ~(SPU_SRC_ENABLE_BIT);
        }
    }
    else
    {
        SPU_E("\n SPU Invalid SRC Num-%d \n", uiNth);
    }
}

/*
***************************************************************************************************
*                                       AVC_Set_Config
*
* Set AVC Config.
*
* @param    uiCH              : num of AVC Channel
*           uiPeriod          : num of volume changes
*           uiInterval        : num of frames
*           uiWait            : num of frames to wait before volume change
*           uiGain            : volume gain of ch.
*
* @return   void
*
* Notes
* Consider Idle time of the WG 0 ~ 4 when setting the wait time.
***************************************************************************************************
*/
static void AVC_Set_Config
(
    uint32                              uiCH,
    uint32                              uiPeriod,
    uint32                              uiInterval,
    uint32                              uiWait,
    uint32                              uiGain
)
{

    uint32 cfg1_val = 0UL;
    uint32 cfg2_val = 0UL;

    if(uiCH < MAX_NUM_OF_AVC)
    {
        /* avc config1 */
        cfg1_val =  ((uiInterval & SPU_AVC_INTERVAL_MASK) << SPU_AVC_INTERVAL_OFFSET) |
                    ((uiPeriod & SPU_AVC_PERIOD_MASK) << SPU_AVC_PERIOD_OFFSET);

        /* avc config2 */
        cfg2_val =  ((uiGain & SPU_AVC_GAIN_MASK) << SPU_AVC_GAIN_OFFSET) |
                    ((uiWait & SPU_AVC_WAIT_MASK) << SPU_AVC_WAIT_OFFSET);

        SPU_AVC_CFG1_REG(uiCH) = cfg1_val;
        SPU_AVC_CFG2_REG(uiCH) = cfg2_val;
    }
    else
    {
        SPU_E("\n SPU Invalid AVC CH-%d \n", uiCH);
    }
}

/*
***************************************************************************************************
*                                       AVC_Clear
*
* Clear AVC
*
* @param    uiCH              : num of AVC Channel
*           uiParam_Clear     : parameter clear signal for force stop
*           uidB_Clear        : db clear signal
* @return   void
*
* Notes
*   This bitfiled is automatically cleared after db clear.
*
***************************************************************************************************
*/
static void AVC_Clear
(
    uint32                              uiCH,
    uint32                              uiParam_Clear,
    uint32                              uidB_Clear
)
{
    if(uiCH < MAX_NUM_OF_AVC)
    {
        /* Set Clear Bit and Automatically cleared */
        if(uiParam_Clear == AVC_PARAM_CLEAR)
        {
            SPU_AVC_CFG3_REG(uiCH) |= SPU_AVC_PARAM_CLR_BIT;
        }

        if(uidB_Clear == AVC_DB_CLEAR)
        {
            SPU_AVC_CFG3_REG(uiCH) |= SPU_AVC_DB_CLR_BIT;
        }
    }
    else
    {
        SPU_E("\n SPU Invalid AVC CH-%d \n", uiCH);
    }
}

/*
***************************************************************************************************
*                                       AVC_Set_Mode
*
* Set AVC Mode
*
* @param    uiCH              : num of AVC Channel
*           uiMode            : if 0 bypass mode, 1 volume control mode
*
* @return   void
*
*
***************************************************************************************************
*/
static void AVC_Set_Mode
(
    uint32                              uiCH,
    uint32                              uiMode
)
{
    if(uiCH < MAX_NUM_OF_AVC)
    {
        if(uiMode == AVC_VOL_CTL)
        {
            /* volume control mode */
            SPU_AVC_CFG0_REG(uiCH) |= SPU_AVC_MODE_CTL_BIT;
        }
        else
        {
            /* bypass mode */
            SPU_AVC_CFG0_REG(uiCH) &= ~(SPU_AVC_MODE_CTL_BIT);
        }
    }
    else
    {
        SPU_E("\n SPU Invalid AVC CH-%d \n", uiCH);
    }
}

/*
***************************************************************************************************
*                                       AVC_Set_Mute
*
* Set AVC Mute
*
* @param    uiCH              : num of AVC Channel
*           uiMute            : if 0 ummute, 1 mute
* @return   void
*
*
***************************************************************************************************
*/
static void AVC_Set_Mute
(
    uint32                              uiCH,
    uint32                              uiMute
)
{
    if(uiCH < MAX_NUM_OF_AVC)
    {
        if(uiMute == AVC_MUTE)
        {
            /* mute */
            SPU_AVC_CFG0_REG(uiCH) |= SPU_AVC_MUTE_CTL_BIT;
        }
        else
        {
            /* unmute */
            SPU_AVC_CFG0_REG(uiCH) &= ~(SPU_AVC_MUTE_CTL_BIT);
        }
    }
    else
    {
        SPU_E("\n SPU Invalid AVC CH-%d \n", uiCH);
    }
}

/*
***************************************************************************************************
*                                       AVC_Start
*
* Start AVC Volume Control
*
* @param    uiCH              : num of AVC Channel
* @return   void
*
* Notes
*   This bitfiled is automatically cleared after volume control.
*
***************************************************************************************************
*/
static void AVC_Start
(
    uint32                              uiCH
)
{
    if(uiCH < MAX_NUM_OF_AVC)
    {
        /* automatically cleared */
        SPU_AVC_CFG0_REG(uiCH) |= SPU_AVC_VOL_CTL_START_BIT;
    }
    else
    {
        SPU_E("\n SPU Invalid AVC CH-%d \n", uiCH);
    }
}

/*
***************************************************************************************************
*                                       AVC_Get_Status
*
* Get AVC Status.
*
* @param    uiCH              : num of AVC Channel
* @return   void
*
*
***************************************************************************************************
*/
static int32 AVC_Get_Status
(
    uint32                              uiCH
)
{
    uint32 status = 0UL;
    uint32 ret = SPU_INVALID_CH;

    if(uiCH < MAX_NUM_OF_AVC)
    {
        status = SPU_AVC_STATUS_REG(uiCH);
        status = ((status >> SPU_AVC_DB_STATUS_OFFSET) & SPU_AVC_DB_STATUS_MASK);
        ret = status;
    }
    else
    {
        SPU_E("\n SPU Invalid AVC CH-%d \n", uiCH);
    }

    return ret;
}

/*
***************************************************************************************************
*                                       MIXER_Set_MIXERSAT
*
* Set Mixer Saturation MIN/MAX Value
*
* @param    uiMax_Value       : Signed Maximum limit of mixed each channel value
*           uiMin_Value       : Signed Minimum limit of mixed each channel value
* @return   void
*
* Notes
*   Recommend using default value.
*
***************************************************************************************************
*/
static void MIXER_Set_MIXERSAT
(
    uint32                              uiMax_Value,
    uint32                              uiMin_Value
)
{
    uint32 saturation_val = 0UL;

    saturation_val = ((uiMax_Value & SPU_MIXER_SAT_MAX_MASK) << SPU_MIXER_SAT_MAX_OFFSET) |
                        ((uiMin_Value & SPU_MIXER_SAT_MIN_MASK) << SPU_MIXER_SAT_MIN_OFFSET);

    SPU_MIXER_SAT_REG = saturation_val;
}

/*
***************************************************************************************************
*                                       MIXER_Set_MixCombination
*
* Set Mix Combination
*
* @param    uiMixCombination  : Enable MIX for each channel
* @return   void
*
***************************************************************************************************
*/
static void MIXER_Set_MixCombination
(
    uint32                              uiMixCombination
)
{
    SPU_MIXER_CTL_REG = uiMixCombination;
}

/*
***************************************************************************************************
*                                       MIXER_Get_MixCombination
*
* Get Mix Combination.
*
* @param    void
* @return   mix combination
*
***************************************************************************************************
*/
static uint32 MIXER_Get_MixCombination
(
    void
)
{
    return SPU_MIXER_CTL_REG;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SPU == 1 )


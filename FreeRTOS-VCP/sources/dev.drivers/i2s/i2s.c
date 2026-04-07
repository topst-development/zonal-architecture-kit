// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : i2s.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_I2S == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <i2s.h>
#include <clock_reg.h>
#include <clock_dev.h>
#include <clock.h>
#include <bsp.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*                                                                                               */
/*************************************************************************************************/
/*                                             LOCAL VARIABLES                                   */
/*************************************************************************************************/


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void I2S_delay1us
(
    uint32                              n
)
{
    uint32  i;
    uint32  val;

    val = n * 500UL;
    for (i = 0UL; i < val; i++)
    {
         BSP_NOP_DELAY();
    }
}

I2SRetCode_t I2S_SetGpiofunction(const I2SConfig_t* psI2sConf)
{

    SALRetCode_t    ret;
    uint32          uiRdata;
    uint32          uiPeriChSel;
    uint32          i;

    static const uint32 uiI2sGpio[I2S_CHMAX][6] =
    {
        /* MCLK,            BCLK,           LRCK,           DAO,            DAI,        FUNC_NUM */
        {I2S_MCLK_CH0,  I2S_BCLK_CH0,   I2S_LRCK_CH0,   I2S_DAO_CH0,    I2S_DAI_CH0,    1UL},
        {I2S_MCLK_CH1,  I2S_BCLK_CH1,   I2S_LRCK_CH1,   I2S_DAO_CH1,    TCC_GPNONE,     2UL},
        {I2S_MCLK_CH2,  I2S_BCLK_CH2,   I2S_LRCK_CH2,   I2S_DAO_CH2,    TCC_GPNONE,     2UL},
    };

    ret             = SAL_RET_SUCCESS;
    uiRdata         = 0;
    uiPeriChSel     = 0;

    if((uint32)psI2sConf->i2sHwCh < I2S_CHMAX)
    {
        uint32 uiI2sCh;
        uint32 uifuncNum;

        uiI2sCh = (uint32)psI2sConf->i2sHwCh;
        uifuncNum = uiI2sGpio[psI2sConf->i2sHwCh][5];

        for(i = 0U; i < 5U; i++)
        {
            if(uiI2sGpio[uiI2sCh][i] != TCC_GPNONE){

                if(i < 3U)       //MCLK, BCLK, LRCK
                {
                    if(psI2sConf->i2sMode == I2S_MASTER_MODE)
                    {
                        ret = GPIO_Config(uiI2sGpio[uiI2sCh][i], GPIO_FUNC(uifuncNum) | GPIO_OUTPUT);
                    }
                    else        //I2S_SLAVE_MODE
                    {
                        ret = GPIO_Config(uiI2sGpio[uiI2sCh][i], GPIO_FUNC(uifuncNum) | GPIO_INPUT | GPIO_INPUTBUF_EN);
                    }

                }
                else if(i == 3U) //DAO
                {
                    ret = GPIO_Config(uiI2sGpio[uiI2sCh][i], GPIO_FUNC(uifuncNum) | GPIO_OUTPUT);
                }
                else //i == 4U //DAI
                {
                    ret = GPIO_Config(uiI2sGpio[uiI2sCh][i], GPIO_FUNC(uifuncNum) | GPIO_INPUT | GPIO_INPUTBUF_EN);
                }

                if(ret == SAL_RET_FAILED)
                {
                    I2S_E("Seleted Audio Channel GPIO Config Fail");
                    break;
                }
            }
        }
    }
    else
    {
        ret = SAL_RET_FAILED;
        I2S_E("Selet Audio Channel Error");
    }

    if(ret != SAL_RET_FAILED)
    {
        /* Set PERICHSEL Register */
        uiRdata     = (SAL_ReadReg(PERICH_SEL) & ~( 0x3UL << I2SSEL_0 ));        //reset 9:8 bit
        uiPeriChSel = (uint32)psI2sConf->i2sHwCh;
        SAL_WriteReg(uiRdata | (uiPeriChSel << I2SSEL_0) , PERICH_SEL);
    }

    return (I2SRetCode_t)ret;
}

I2SRetCode_t I2S_SetClock(const I2SConfig_t* psI2sConf)
{
    I2SRetCode_t  ret;

    uint32  bclk_div = 0UL;
    uint32  mclk_div = 0UL;
    uint32  peri_clock = 0UL;
    uint32  filter_clock = 0UL;

    ret = I2S_RET_SUCCESS;

    switch (psI2sConf->i2sBclkDiv)
    {
        case I2S_BCLK_DIV_32    :
        {
            bclk_div = 32U;
            break;
        }
        case I2S_BCLK_DIV_48    :
        {
            bclk_div = 48U;
            break;
        }
        case I2S_BCLK_DIV_64    :
        {
            bclk_div = 64U;
            break;
        }
        case I2S_BCLK_RESERVED  :
        {
            //Nothing to do
            break;
        }
        default :
        {
            ret = I2S_RET_FAILED;
            I2S_E("I2S bclk div setting Error");
            break;
        }
    }

    switch (psI2sConf->i2sMclkDiv)
    {
        case I2S_MCLK_DIV_4 :
        {
            mclk_div = 4U;
            break;
        }
        case I2S_MCLK_DIV_6 :
        {
            mclk_div = 6U;
            break;
        }
        case I2S_MCLK_DIV_8 :
        {
            mclk_div = 8U;
            break;
        }
        case I2S_MCLK_DIV_16 :
        {
            mclk_div = 16U;
            break;
        }
        case I2S_MCLK_DIV_24 :
        {
            mclk_div = 24U;
            break;
        }
        case I2S_MCLK_DIV_32 :
        {
            mclk_div = 32U;
            break;
        }
        case I2S_MCLK_DIV_48 :
        {
            mclk_div = 48U;
            break;
        }
        case I2S_MCLK_DIV_64 :
        {
            mclk_div = 64U;
            break;
        }
        default :
        {
            ret = I2S_RET_FAILED;
            I2S_E("I2S mclk div setting Error");
            break;
        }
    }

    /* Set peri clock for MCLK */
    switch(psI2sConf->i2sSampleRate)
    {
        case I2S_SAMPLE_RATE_8000   :
            peri_clock = 8000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_16000  :
            peri_clock = 16000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_24000  :
            peri_clock = 24000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_32000  :
            peri_clock = 32000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_44100  :
            peri_clock = 44100U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_48000  :
            peri_clock = 48000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_64000  :
            peri_clock = 64000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_96000  :
            peri_clock = 96000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_128000 :
            peri_clock = 128000U * bclk_div * mclk_div;
            break;
        case I2S_SAMPLE_RATE_192000 :
            peri_clock = 192000U * bclk_div * mclk_div;
            break;
        default :
            ret = I2S_RET_FAILED;
            I2S_E("I2S i2sSampleRate setting Error");
            break;
    }

    if(peri_clock > I2S_MAX_PERI_CLOCK)
    {
        I2S_E("I2S Peri Clock value is over the Maximum Peri clock");
        ret = I2S_RET_FAILED;
    }

    if(ret == I2S_RET_SUCCESS)
    {
        I2S_D("peri_clock : %d", peri_clock);

        /* retrun  0 : CLOCK_SetPeriRate Success
           return -1 : CLOCK_SetPeriRate Fail    */
        if ((sint32)I2S_RET_SUCCESS == CLOCK_SetPeriRate((sint32)CLOCK_PERI_I2S0, (uint32)peri_clock)) //set clock
        {
            ret = (I2SRetCode_t)CLOCK_EnablePeri((sint32)CLOCK_PERI_I2S0);
        }
        else
        {
            I2S_E("Fail to I2S peri clock ");
            ret = I2S_RET_FAILED;
        }

        if(psI2sConf->i2sMode == I2S_SLAVE_MODE)
        {
            /* Set peri clock for Filter Clock */
            filter_clock = peri_clock*6U;
            I2S_D("filter_clock : %d", filter_clock);

            if ((sint32)I2S_RET_SUCCESS == CLOCK_SetPeriRate((sint32)CLOCK_PERI_I2S1, (uint32)filter_clock)) //set filter clock
            {
                (I2SRetCode_t)CLOCK_EnablePeri((sint32)CLOCK_PERI_I2S1);
            }
            else
            {
                I2S_E("Fail to I2S filter clock ");
                ret = I2S_RET_FAILED;
            }
        }
    }

    return ret;
}

uint32  I2S_GetTransferSize(I2SDataDirect_t      i2sDirect)
{
    uint32 ui_UnitSize;
    uint32 ui_DxBSIZE;
    uint32 ui_DxWSIZE;
    uint32 ui_ST_TCOUNT;
    uint32 ui_SINC;

#ifdef AUDIO_RX_ENABLE
    if(i2sDirect == I2S_DIN)
    {
        /* Size of 1 Burst transfer of DAIF Tx DMA */
        if(Reg_TransCtrl->bReg.DRBSIZE == 0U){
            ui_DxBSIZE = 1U;
        }else if(Reg_TransCtrl->bReg.DRBSIZE == 1U){
            ui_DxBSIZE = 2U;
        }else if(Reg_TransCtrl->bReg.DRBSIZE == 2U){
            ui_DxBSIZE = 4U;
        }else{
            //Reg_TransCtrl->bReg.DRBSIZE == 3U
            ui_DxBSIZE = 8U;
        }

        /* Word Size of DAIF Tx DMA */
        if(Reg_TransCtrl->bReg.DRWSIZE == 0U){
            ui_DxWSIZE = 1U; // 8 bit data
        }else if(Reg_TransCtrl->bReg.DRWSIZE == 1U){
            ui_DxWSIZE = 2U; // 16bit
        }else if(Reg_TransCtrl->bReg.DRWSIZE == 2U){
            ui_DxWSIZE = 4U; // 32bit
        }else{
            //Reg_TransCtrl->bReg.DRWSIZE == 3
            ui_DxWSIZE = 4U; // 32bit
        }

        /* DMA transfers data by amount of ST_TCOUNT */
        /* ST_TCOUNT should be set to n(M+1)-1, (n=1,2,3…). */
        ui_ST_TCOUNT = Reg_RxDaTCnt->bReg.ST_TCOUNT;
        ui_SINC = Reg_RxDaParam->bReg.SINC;

        ui_UnitSize = (ui_DxBSIZE * ui_DxWSIZE * (ui_ST_TCOUNT+1U) * ui_SINC)/4U; //?
    }
#endif //AUDIO_RX_ENABLE

    if(i2sDirect == I2S_DOUT)
    {
        /* Size of 1 Burst transfer of DAIF Tx DMA */
        if(Reg_TransCtrl->bReg.DTBSIZE == 0U){
            ui_DxBSIZE = 1U;
        }else if(Reg_TransCtrl->bReg.DTBSIZE == 1U){
            ui_DxBSIZE = 2U;
        }else if(Reg_TransCtrl->bReg.DTBSIZE == 2U){
            ui_DxBSIZE = 4U;
        }else{
            //Reg_TransCtrl->bReg.DTWSIZE == 3U
            ui_DxBSIZE = 8U;
        }

        /* Word Size of DAIF Tx DMA */
        if(Reg_TransCtrl->bReg.DTWSIZE == 0U){
            ui_DxWSIZE = 1U; // 8 bit data
        }else if(Reg_TransCtrl->bReg.DTWSIZE == 1U){
            ui_DxWSIZE = 2U; // 16bit
        }else{
            //Reg_TransCtrl->bReg.DTWSIZE == 2U
            //Reg_TransCtrl->bReg.DTWSIZE == 3U
            ui_DxWSIZE = 4U; // 32bit
        }

        /* DMA transfers data by amount of ST_TCOUNT */
        /* ST_TCOUNT should be set to n(M+1)-1, (n=1,2,3…). */
        ui_ST_TCOUNT = Reg_TxDaTCnt->bReg.ST_TCOUNT;
        ui_SINC = Reg_TxDaParam->bReg.SINC;

    }

    ui_UnitSize = (ui_DxBSIZE * ui_DxWSIZE * (ui_ST_TCOUNT+1U) * ui_SINC)/4U; //?

    return ui_UnitSize;
}

void I2S_SetTransferSize(const I2SConfig_t* psI2sConf, I2SDataDirect_t i2sDirect)
{

    const I2SBufferInfo_t *pI2SOut;
#ifdef AUDIO_RX_ENABLE
    const I2SBufferInfo_t *pI2SIn;
#endif
    uint32 ui_BSIZE;
    uint32 ui_WSIZE;
    uint32 ui_SMaskbit;

    if(psI2sConf != NULL_PTR)
    {

#ifdef AUDIO_RX_ENABLE
        if(i2sDirect == I2S_DIN)
        {
            pI2SIn = &(psI2sConf->i2sStreamInfo.i2sIn);

            /* Size of 1 Burst transfer of DAIF Tx DMA */
            if(Reg_TransCtrl->bReg.DRBSIZE == 0U){
                ui_BSIZE = 1U;
            }else if(Reg_TransCtrl->bReg.DRBSIZE == 1U){
                ui_BSIZE = 2U;
            }else if(Reg_TransCtrl->bReg.DRBSIZE == 2U){
                ui_BSIZE = 4U;
            }else{
                /* Reg_TransCtrl->bReg.DRBSIZE == 3U */
                ui_BSIZE = 8U;
            }

            /* Word Size of DAIF Tx DMA */
            if(Reg_TransCtrl->bReg.DRWSIZE == 0U){
                ui_WSIZE = 1U; // 8 bit data
            }else if(Reg_TransCtrl->bReg.DRWSIZE == 1U){
                ui_WSIZE = 2U; // 16 bit
            }else{
                /* Reg_TransCtrl->bReg.DRWSIZE == 2U or 3U */
                ui_WSIZE = 4U; // 32 bit
            }

            I2S_SetRxDaDar((uint32)pI2SIn->i2sDmaAddr);
            ui_SMaskbit = ~((pI2SIn->i2sBufferBytes-1U)>>4) & 0xFFFFFFU;

            Reg_RxDaParam->bReg.SINC    = 4U;                // Audio_DMA + 0x04
            Reg_RxDaParam->bReg.SMASK   = ui_SMaskbit; // 0xfffE00
            Reg_RxDaTCnt->bReg.ST_TCOUNT = (pI2SIn->i2sPeriodBytes*4U/(ui_BSIZE*ui_WSIZE*4U)) - 1U;  // Audio_DMA + 0x08 , 0x7f = 127
        }
#endif

        if(i2sDirect == I2S_DOUT)
        {
            pI2SOut = &(psI2sConf->i2sStreamInfo.i2sOut);

            /* Size of 1 Burst transfer of DAIF Tx DMA */
            if(Reg_TransCtrl->bReg.DTBSIZE == 0U){
                ui_BSIZE = 1U;
            }else if(Reg_TransCtrl->bReg.DTBSIZE == 1U){
                ui_BSIZE = 2U;
            }else if(Reg_TransCtrl->bReg.DTBSIZE == 2U){
                ui_BSIZE = 4U;
            }else{
                /* Reg_TransCtrl->bReg.DTBSIZE == 3U */
                ui_BSIZE = 8U;
            }

            /* Word Size of DAIF Tx DMA */
            if(Reg_TransCtrl->bReg.DTWSIZE == 0U){
                ui_WSIZE = 1U; // 8 bit data
            }else if(Reg_TransCtrl->bReg.DTWSIZE == 1U){
                ui_WSIZE = 2U; // 16 bit
            }else{
                /* Reg_TransCtrl->bReg.DTWSIZE == 2U or 3U */
                ui_WSIZE = 4U; // 32 bit
            }

            I2S_SetTxDaSar((uint32)pI2SOut->i2sDmaAddr); // Audio_DMA + 0x40
            ui_SMaskbit = ~((pI2SOut->i2sBufferBytes-1U)>>4) & 0xFFFFFFU;

            Reg_TxDaParam->bReg.SINC    = 4U;
            Reg_TxDaParam->bReg.SMASK   = ui_SMaskbit; // DMA Address 16bit is masked 0xffff0000 bit // 0xfffE00
            Reg_TxDaTCnt->bReg.ST_TCOUNT = (pI2SOut->i2sPeriodBytes*4U/(ui_BSIZE*ui_WSIZE*4U)) - 1U;  // Audio_DMA + 0x48 , 0x7f = 127
        }

        //Related to Repeat.
        Reg_RptCtrl->bReg.RPTCNT    = 0U;
        Reg_RptCtrl->bReg.DBTH      = 7U;        //set for BURST_4  or LRMOD (change because of Burst_8)
        Reg_RptCtrl->bReg.DRI       = 0U;

    }
}

void I2S_TxAdmaSetting(const I2SConfig_t* psI2sConf)
{

    if(psI2sConf->i2sNumCh == I2S_STEREO)
    {
        Reg_ChCtrl->bReg.DTMCM = 0U;
        Reg_ChCtrl->bReg.DMTSEL = 3U;

        if(psI2sConf->i2sLRmode == I2S_LRMODE_ON)
        {
            Reg_ChCtrl->bReg.DTLR = 1U;       /* Left/Right Data Mode of DAIF Tx */
        }
        else
        {
            Reg_ChCtrl->bReg.DTLR = 0U;
        }
    }
    else
    {
        /* psI2sConf->i2sNumCh == I2S_MONO) */
        I2S_E("TCC70xx does NOT support Mono type");
    }

    if(psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_16)
    {
        /* 1 : width of data is 16bit */
        Reg_ChCtrl->bReg.DTDW = 1U;
    }
    else if(psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_24)
    {
        /* 0 : width of data is 24bit */
        Reg_ChCtrl->bReg.DTDW = 0U;
    }
    else
    {
        I2S_E("Wrong value of i2sBitPerSample");
    }

    //[2,0] Interrupt Enable of DAI
    Reg_ChCtrl->bReg.DTIEN = 0U;
    Reg_TransCtrl->bReg.TCN = 1U;            //Issue Continuous Transfer of Tx DMA
    Reg_TransCtrl->bReg.DTRPT = 1U;          //After all of hop transfer has executed, DAIF Tx DMA channel remains enabled //Continuous

    if(psI2sConf->i2sLRmode != I2S_LRMODE_ON)
    {
        Reg_TransCtrl->bReg.DTBSIZE = 3U;    // set for BURST_8
    }
    else
    {
        //When LR Mode is enabled, 8 Read/Write is not available.
        Reg_TransCtrl->bReg.DTBSIZE = 2U;    // set for BURST_4
    }

    Reg_TransCtrl->bReg.DTWSIZE = 3U;        //Each cycle read or write 32 bit data of DAIF Tx DMA

}

#ifdef AUDIO_RX_ENABLE
void I2S_RxAdmaSetting(const I2SConfig_t* psI2sConf)
{

    if(psI2sConf->i2sNumCh == I2S_STEREO)
    {
        Reg_ChCtrl->bReg.DRMCM = 0U;
        Reg_ChCtrl->bReg.DMRSEL = 3U;

        if(psI2sConf->i2sLRmode == I2S_LRMODE_ON)
        {
            Reg_ChCtrl->bReg.DRLR = 1U;       /* Left/Right Data Mode of DAIF Rx */
        }
        else
        {
            Reg_ChCtrl->bReg.DRLR = 0U;
        }
    }
    else
    {
        /* psI2sConf->i2sNumCh == I2S_MONO) */
        I2S_E("TCC70xx does NOT support Mono type");
    }

    if(psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_16)
    {
        /* 1 : width of data is 16bit */
        Reg_ChCtrl->bReg.DRDW = 1U;
    }
    else if(psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_24)
    {
        /* 0 : width of data is 24bit */
        Reg_ChCtrl->bReg.DRDW = 0U;
    }
    else
    {
        I2S_E("Wrong value of i2sBitPerSample");
    }

    //[2,0] Interrupt Enable of DAI
    Reg_ChCtrl->bReg.DRIEN = 0U;
    Reg_TransCtrl->bReg.RCN = 1U;            //Issue Continuous Transfer of Rx DMA
    Reg_TransCtrl->bReg.DRRPT = 1U;          //After all of hop transfer has executed, DAIF Rx DMA channel remains enabled //Continuous

    if(psI2sConf->i2sLRmode != I2S_LRMODE_ON)
    {
        Reg_TransCtrl->bReg.DRBSIZE = 3U;    // set for BURST_8
    }
    else
    {
        //When LR Mode is enabled, 8 Read/Write is not available.
        Reg_TransCtrl->bReg.DRBSIZE = 2U;    // set for BURST_4
    }

    Reg_TransCtrl->bReg.DRWSIZE = 3U;        //Each cycle read or write 32 bit data of DAIF Rx DMA

}
#endif


void I2S_SetTASMode(void)
{
    Reg_TASCFG->bReg.TEN = 1;
}

void I2S_SetAICMode(void)
{
     Reg_TASCFG->bReg.TEN = 0;
}

void I2S_TASClear(void)
{
    Reg_TASCFG->bReg.TCLR = 1;

    I2S_delay1us(1);

    Reg_TASCFG->bReg.TCLR = 0;
}

void I2S_SetTASRxFifoThreshold(uint8 ucThreshold)
{
    Reg_FIFOCTRL->bReg.TRXTH = ucThreshold;
}

void I2S_SetTASTxFifoThreshold(uint8 ucThreshold)
{
    Reg_FIFOCTRL->bReg.TTXTH = ucThreshold;
}

void I2S_HopClear(void)
{
    /* Hop Count is cleared to 0 */
    Reg_TransCtrl->bReg.HCC = 1U; //HCC (30bit)
    I2S_delay1us(1U);
    Reg_TransCtrl->bReg.HCC = 0U;
}

void I2S_FifoClear(uint32 uiClrBit)
{
    uint32 uiValue = uiClrBit << 20UL;

    *Reg_DRMR |= uiValue;

    I2S_delay1us(1);

    *Reg_DRMR &= ~uiValue;
}

void I2S_DmaReset(uint32 uiRstBit)
{
    /* Refer to I2SRegADmaReset_t */

    //Reset
    *Reg_ADMARST |= uiRstBit;

    (void)I2S_delay1us(1);

    //Release
    *Reg_ADMARST &= ~uiRstBit;
}


void I2S_SWReset(boolean bReset)
{
    if(bReset == SALEnabled)
    {
        //Reset
        *I2S_SW_RESET_REG_1 &= (~(1UL << I2S_CFG_REG_FIELD)) & 0xFFFFFFFFUL;
    }
    else
    {
        //Release
        *I2S_SW_RESET_REG_1 |= (1U << I2S_CFG_REG_FIELD) & 0xFFFFFFFFUL;
    }
}

void I2S_DaifSetting(const I2SConfig_t* psI2sConf)
{

    //Code For Slave Mode
    /*
    uint32 bitSet;
    bitSet = Reg_DAMR->nReg;
    Reg_DAMR->nReg = bitSet & ~0xc0000e00;
    */

    if(psI2sConf->i2sMode == I2S_MASTER_MODE)
    {
       Reg_DAMR->bReg.BPS = 1U;    //LRCK, BLK direct
       Reg_DAMR->bReg.LPS = 1U;

       Reg_DAMR->bReg.SM = 1U;       //generate clock
       Reg_DAMR->bReg.BM = 1U;
       Reg_DAMR->bReg.FM = 1U;

       Reg_DAMR->bReg.AFE = 0U;    //Disable Audio Filter
       Reg_DAMR->bReg.DFE = 0U;    //Disable Audio Data Filter
    }
    else /* psI2sConf->i2sMode == I2S_SLAVE_MODE */
    {
        Reg_DAMR->bReg.BPS = 0U;  //LRCK, BLK source select Pad
        Reg_DAMR->bReg.LPS = 0U;

        Reg_DAMR->bReg.SM = 0U;  //clock from externel pin
        Reg_DAMR->bReg.BM = 0U;
        Reg_DAMR->bReg.FM = 0U;

        Reg_DAMR->bReg.AFE = 1U;  //Enable Audio Filter
        Reg_DAMR->bReg.DFE = 1U;  //Enable Audio Data Filter
    }

    Reg_DAMR->bReg.DBTEN = 1U;       //DAIF Buffer Threshold Enable, FIFO_Half_Trig
    //Reg_DAMR->bReg.DBTEN = 0;       //DAIF Buffer Threshold Enable, FIFO_Half_Trig

    Reg_DAMR->bReg.MPE = 0U;         //Stereo
    Reg_DAMR->bReg.SP  = 0U;       //DAIF System Clock Polarity


    if(psI2sConf->i2sFormat == I2S_FORMAT_I2S)
    {
        Reg_DAMR->bReg.MD = 0U; //Set DAIF Sync as I2S Mode

    }
    else if(psI2sConf->i2sFormat == I2S_FORMAT_RIGHTJ)
    {
        Reg_DAMR->bReg.NMDR = 1U;
        Reg_DAMR->bReg.NMDT = 1U;
        Reg_DAMR->bReg.MD = 1U; //Set DAIF Sync as Left/Rigt-J mode
    }
    else /* psI2sConf->i2sFormat == I2S_FORMAT_LEFTJ */
    {
        Reg_DAMR->bReg.NMDR = 0U;
        Reg_DAMR->bReg.NMDT = 0U;
        Reg_DAMR->bReg.MD = 1U; //Set DAIF Sync as Left/Rigt-J mode
    }

    if(psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_16)
    {
#ifdef AUDIO_RX_ENABLE
        Reg_DAMR->bReg.RXS = 3U;
#endif
        Reg_DAMR->bReg.TXS = 3U;     // if 16bit tx mode , this must set to 3
    }else{  /* psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_24 */
#ifdef AUDIO_RX_ENABLE
        Reg_DAMR->bReg.RXS = 2U;
#endif
        Reg_DAMR->bReg.TXS = 2U;
    }

    Reg_DAMR->bReg.BP  = 0U;       //DAIF Bit Clock Polarity

    //Reg_DAMR EN,TE,TR does not set this function.

    Reg_DCLKDIV->bReg.FD = (uint32)psI2sConf->i2sBclkDiv;
    Reg_DCLKDIV->bReg.BD = (uint32)psI2sConf->i2sMclkDiv;

}


void I2S_DAMREnable(void)
{
    Reg_DAMR->bReg.EN = 1U;  //EN (15bit)
}

void I2S_DAMRDisable(void)
{
    Reg_DAMR->bReg.EN = 0U;  //EN (15bit)
}

void I2S_DAIEnable(uint8 ucflag)
{
    if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
    {
        Reg_DAMR->bReg.RE = 1U;  //RE (13bit)
    }
    if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
    {
        Reg_DAMR->bReg.TE = 1U;  //TE (14bit)
    }
}

void I2S_DAIDisable(uint8 ucflag)
{
    if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
    {
        Reg_DAMR->bReg.RE = 0U;  //RE (13bit)
    }

    if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
    {
        Reg_DAMR->bReg.TE = 0U;  //TE (14bit)
    }
}

void I2S_DMAInit(I2SStream_t* i2sStreamInfo, uint8 ucflag)
{
    I2SBufferInfo_t *   pi2sOut;
    I2SBufferInfo_t *   pi2sIn;

    if( i2sStreamInfo != NULL_PTR )
    {
        I2S_Disable(ucflag);

        if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
        {
            pi2sOut = &(i2sStreamInfo->i2sOut);

            SAL_MemSet((void*)pi2sOut->i2sDmaAddr, 0, pi2sOut->i2sBufferBytes);
            pi2sOut->i2sDataPos = 0U;
            pi2sOut->i2sPreCurPos = 0U;
            pi2sOut->i2sDmaStatus = I2S_PRE;
            I2S_DmaReset(DaTXRst);
        }

        if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
        {
            pi2sIn = &(i2sStreamInfo->i2sIn);

            SAL_MemSet((void*)pi2sIn->i2sDmaAddr, 0, pi2sIn->i2sBufferBytes);
            pi2sIn->i2sDataPos = 0U;
            pi2sIn->i2sPreCurPos = 0U;
            pi2sIn->i2sDmaStatus = I2S_PRE;
            I2S_DmaReset(DaRXRst);
        }
    }
}

void I2S_DMAEnable(uint8 ucflag)
{

    if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
    {
        Reg_ChCtrl->bReg.DREN = 1U;  //DREN (30bit)
    }

    if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
    {
        Reg_ChCtrl->bReg.DTEN = 1U;  //DTEN (28bit)
    }
}

void I2S_DMADisable(uint8 ucflag)
{
    if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
    {
        Reg_ChCtrl->bReg.DREN = 0U;  //DREN (30bit)
    }

    if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
    {
        Reg_ChCtrl->bReg.DTEN = 0U;  //DTEN (28bit)
    }
}

void I2S_Irq_Enable(uint8 ucflag)
{
    if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
    {
        Reg_ChCtrl->bReg.DRIEN = 1U;  //DREN (30bit)
    }

    if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
    {
        Reg_ChCtrl->bReg.DTIEN = 1U;  //DTEN (28bit)
    }

}
void I2S_Irq_Disable(uint8 ucflag)
{
    if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
    {
        Reg_ChCtrl->bReg.DRIEN = 0U;  //DREN (30bit)
    }

    if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
    {
        Reg_ChCtrl->bReg.DTIEN = 0U;  //DTEN (28bit)
    }
}

void I2S_ErrCheck_Enable(void)
{
    Reg_DAMR->bReg.LBT = 1U;    // I2S_LoopBackTest(SALEnabled);
    Reg_ErrStsCtrl->bReg.ERRCHECK_EN = 1U;
}

void I2S_ErrCheck_Disable(void)
{
    Reg_DAMR->bReg.LBT = 0U;
    Reg_ErrStsCtrl->bReg.ERRCHECK_EN = 0U;
    I2S_ErrCheck_Clear();
}

void I2S_ErrCheck_Clear(void)
{
    Reg_ErrStsCtrl->bReg.ERRCLR = 1;
    I2S_delay1us(1);
    Reg_ErrStsCtrl->bReg.ERRCLR = 0;
}

uint8 I2S_ErrCheck_GetStatus(void)
{
    return (uint8)(Reg_ErrStsCtrl->bReg.ERRSTS);
}

void I2S_Enable(uint8 ucflag)
{
    I2S_DMAEnable(ucflag);
    I2S_DAIEnable(ucflag);
    I2S_Irq_Enable(ucflag);
#ifdef AUDIO_DEBUG_TEST
    I2S_ErrCheck_Enable();
#endif
}

void I2S_Disable(uint8 ucflag)
{
    I2S_Irq_Disable(ucflag);
    I2S_DMADisable(ucflag);
    I2S_HopClear();
#ifdef AUDIO_DEBUG_TEST
    mcu_printf("I2S Error Status : [%d] \n",I2S_ErrCheck_GetStatus());
    I2S_ErrCheck_Disable();
#endif
    I2S_DAIDisable(ucflag);
}

#ifdef AUDIO_RX_ENABLE

uint8 I2S_GetRxDMAStatus(void)
{
    return (uint8)(Reg_ChCtrl->bReg.DREN);
}

void I2S_SetRxDaDar(uint32 uiRxDataSourceAddr)
{
    /* DAIF Rx (Right) Data0 Destination Address */
    *Reg_RxDaDar0 = (uint32)uiRxDataSourceAddr;
}

void I2S_SetRxDaDarL(uint32 uiRxDataSourceAddr)
{
    /* DAIF Rx Left Data0 Destination Address */
    *Reg_RxDaDarL0 = (uint32)uiRxDataSourceAddr;
}

uint32 I2S_GetRxDaCdar(void)
{
    /* DAIF Rx (Right) Data0 Current Destination Address */

    return (uint32)(*Reg_RxDaCdar0);
}

uint32 I2S_GetRxDaCdarL(void)
{
    /* DAIF Rx Left Data0 Current Destination Address */

    return (uint32)(*Reg_RxDaCdarL0);
}

#endif

uint8 I2S_GetTxDMAStatus(void)
{
    return (uint8)(Reg_ChCtrl->bReg.DTEN);
}

void I2S_SetTxDaSar(uint32 uiTxDataSourceAddr)
{
    /* DAIF Tx (Right) Data0 Source Address */
    *Reg_TxDaSar0 = (uint32)uiTxDataSourceAddr;
}

void I2S_SetTxDaSarL(uint32 uiTxDataSourceAddr)
{
    /* DAIF Tx Left Data0 Source Address */
    *Reg_TxDaSarL0 = (uint32)uiTxDataSourceAddr;
}

uint32 I2S_GetTxDaCsar(void)
{
    /* DAIF Tx (Right) Data0 Current Source Address */

    return (uint32)(*Reg_TxDaCsar0);
}

uint32 I2S_GetTxDaCsarL(void)
{
    /* DAIF Tx Left Data0 Current Source Address */

    return (uint32)(*Reg_TxDaCsarL0);
}

void I2S_Irq(void * pArg)
{
    I2SConfig_t *psI2sConfig;
    uint32 dms_cur_pos;

    if( pArg != NULL_PTR )
    {
        psI2sConfig = (I2SConfig_t *)pArg;

        if(Reg_IntStatus->bReg.DTMI == 1U)
        {
            Reg_IntStatus->bReg.DTMI = 1U;
            dms_cur_pos = I2S_GetTxDaCsar();
            I2S_D("p:0x%x\n", dms_cur_pos);
            I2S_UpdateStatus(psI2sConfig, I2S_DOUT);
            psI2sConfig->i2sStreamInfo.i2sOut.i2sPreCurPos = dms_cur_pos - (uint32)psI2sConfig->i2sStreamInfo.i2sOut.i2sDmaAddr;
        }
#ifdef AUDIO_RX_ENABLE
        if(Reg_IntStatus->bReg.DRMI == 1U)
        {
            Reg_IntStatus->bReg.DRMI = 1U;
            dms_cur_pos = I2S_GetRxDaCdar();
            I2S_D("c:0x%x\n", dms_cur_pos);
            I2S_UpdateStatus(psI2sConfig, I2S_DIN);
            psI2sConfig->i2sStreamInfo.i2sIn.i2sPreCurPos = dms_cur_pos - (uint32)psI2sConfig->i2sStreamInfo.i2sIn.i2sDmaAddr;
        }
#endif
    }
}

void I2S_UpdateStatus(I2SConfig_t*     psI2sConf, I2SDataDirect_t i2sDirect)
{
    //uint32 available_bytes;
    uint32 remained_bytes;
    I2SBufferInfo_t *pI2SOut;
    I2SBufferInfo_t *pI2SIn;

    //available_bytes = 0;    //Unused size of Buffer
    remained_bytes = 0U;     //PCM Data Size
    pI2SOut = &(psI2sConf->i2sStreamInfo.i2sOut);
    pI2SIn = &(psI2sConf->i2sStreamInfo.i2sIn);

    if(i2sDirect == I2S_DOUT)
    {

        if(pI2SOut->i2sDataPos >= pI2SOut->i2sPreCurPos)
        {
            remained_bytes = pI2SOut->i2sDataPos - pI2SOut->i2sPreCurPos;
        }else{
            remained_bytes = (pI2SOut->i2sBufferBytes - pI2SOut->i2sPreCurPos) + pI2SOut->i2sDataPos;
        }

        //available_bytes = pI2SOut->i2sBufferBytes - remained_bytes;

        switch (pI2SOut->i2sDmaStatus)
        {
            case I2S_PRE        :
                if(remained_bytes > pI2SOut->i2sThresholdBytes)
                {
                    I2S_Enable((uint8)I2S_DOUT);
#ifdef AUDIO_DEBUG_TEST
                    I2S_Enable((uint8)I2S_DIN);
#endif
                    pI2SOut->i2sDmaStatus = I2S_RUN;
                }
                I2S_D("PRE : r=%d", remained_bytes);
                break;

            case I2S_RUN        :
                if(remained_bytes < pI2SOut->i2sPeriodBytes)
                {
                    pI2SOut->i2sDmaStatus = I2S_UNDERRUN_B;
                    I2S_E("UNDERRUN : r=%d", remained_bytes);
                }
                break;

            case I2S_UNDERRUN_B :
                pI2SOut->i2sDmaStatus = I2S_STOP;
                SAL_MemSet((void *)pI2SOut->i2sDmaAddr, 0, pI2SOut->i2sBufferBytes);
                I2S_Disable((uint8)I2S_DOUT);
                I2S_E("STOP");
                pI2SOut->i2sDataPos = pI2SOut->i2sPreCurPos;
                break;

            case I2S_STOP       :
                if(remained_bytes > pI2SOut->i2sThresholdBytes)
                {
                    I2S_Enable((uint8)I2S_DOUT);
                    pI2SOut->i2sDmaStatus = I2S_RUN;
                    I2S_D("RESTART : r=%d", remained_bytes);
                }
                break;
            default :
                I2S_E("Wrong Status : [%d]", pI2SOut->i2sDmaStatus);
                break;
        }

    }else{  //i2sDirect == I2S_DIN

        if(pI2SIn->i2sDataPos <= pI2SIn->i2sPreCurPos)
        {
            remained_bytes = pI2SIn->i2sPreCurPos - pI2SIn->i2sDataPos;
        }else{
            remained_bytes = (pI2SIn->i2sBufferBytes - pI2SIn->i2sDataPos) + pI2SIn->i2sPreCurPos;
        }

        switch (pI2SIn->i2sDmaStatus)
        {
            case I2S_PRE        :
                pI2SIn->i2sDmaStatus = I2S_RUN;
                I2S_D("[Cap] PRE : r=%d", remained_bytes);
                break;

            case I2S_RUN        :
                if((pI2SIn->i2sBufferBytes - remained_bytes) < pI2SIn->i2sPeriodBytes)
                {
                    pI2SIn->i2sDmaStatus = I2S_OVERRUN_B;
                    I2S_E("[Cap] Overrun : r=%d", (pI2SIn->i2sBufferBytes - remained_bytes));
                }
                break;

            case I2S_OVERRUN_B :
                I2S_E("[Cap] Overrun Error");
                pI2SIn->i2sDmaStatus = I2S_STOP;
                SAL_MemSet((void *)pI2SIn->i2sDmaAddr, 0, pI2SIn->i2sBufferBytes);
                pI2SIn->i2sDataPos = pI2SIn->i2sPreCurPos;
                break;

            case I2S_STOP       :
                I2S_Disable(I2S_DIN);
                I2S_D("[Cap] STOP");
                break;

            default :
                I2S_E("[Cap] Wrong Status : [%d]", pI2SOut->i2sDmaStatus);
                break;
        }

    }

    return;
}


uint32 I2S_GetAvailable(I2SConfig_t*     psI2sConf, I2SDataDirect_t i2sDirect)
{

    uint32 available_bytes;
    uint32 remained_bytes;
    I2SBufferInfo_t *pI2SOut;
    I2SBufferInfo_t *pI2SIn;

    available_bytes = 0U;    //Unused size of Buffer
    remained_bytes = 0U;     //PCM Data Size
    pI2SOut = &(psI2sConf->i2sStreamInfo.i2sOut);
    pI2SIn = &(psI2sConf->i2sStreamInfo.i2sIn);

    if(i2sDirect == I2S_DOUT)
    {
        if(pI2SOut->i2sDataPos >= pI2SOut->i2sPreCurPos)
        {
            remained_bytes = pI2SOut->i2sDataPos - pI2SOut->i2sPreCurPos;
        }
        else
        {
            remained_bytes = (pI2SOut->i2sBufferBytes - pI2SOut->i2sPreCurPos) + pI2SOut->i2sDataPos;
        }

        available_bytes = pI2SOut->i2sBufferBytes - remained_bytes;
    }
    else  //i2sDirect == I2S_DIN
    {

        if(pI2SIn->i2sPreCurPos >= pI2SIn->i2sDataPos)
        {
            available_bytes = pI2SIn->i2sPreCurPos - pI2SIn->i2sDataPos;
        }
        else
        {
            available_bytes = (pI2SIn->i2sBufferBytes - pI2SIn->i2sDataPos) + pI2SIn->i2sPreCurPos;
        }
    }

    return available_bytes;
}

uint32 I2S_PcmWrite(I2SConfig_t*     psI2sConf, void* pToWrite, uint32 uiSize)
{

    I2SBufferInfo_t *pI2SOut;
    void *pBuf;             // the address of buffer that will be wrote PCM data.
    uint32 uiAvailSize;     // the available size for writing PCM data.
    uint32 uiWriteSize;
    uint32 uiTailSize;      // the rest size of i2sBufferBytes for writing PCM data.

    pI2SOut = &(psI2sConf->i2sStreamInfo.i2sOut);

    if(pI2SOut->i2sDataPos == pI2SOut->i2sBufferBytes)
    {
        pI2SOut->i2sDataPos = 0U;
    }

    pBuf = (void *)pI2SOut->i2sDmaAddr + pI2SOut->i2sDataPos;
    uiAvailSize = I2S_GetAvailable(psI2sConf , I2S_DOUT);

    if(uiAvailSize <= uiSize)
    {
        uiWriteSize = uiAvailSize - 4U;
        //I2S_E("Write data size is bigger than available size of buffer");
    }
    else
    {
        uiWriteSize = uiSize;
    }

    uiTailSize = pI2SOut->i2sBufferBytes - pI2SOut->i2sDataPos;

    if(uiWriteSize <= uiTailSize)
    {
        SAL_MemCopy((void *)pBuf, (const void *)pToWrite, uiWriteSize);
        pI2SOut->i2sDataPos += uiWriteSize;
    }
    else
    {
        SAL_MemCopy((void *)pBuf, (const void *)pToWrite, uiTailSize);
        pBuf = (void *)pI2SOut->i2sDmaAddr;
        SAL_MemCopy((void *)pBuf, (const void *)(pToWrite + uiTailSize), (uiWriteSize - uiTailSize));

        pI2SOut->i2sDataPos = (uiWriteSize - uiTailSize);
    }

    I2S_UpdateStatus(psI2sConf, I2S_DOUT);

    return uiWriteSize;
}

uint32 I2S_PcmRead(I2SConfig_t*     psI2sConf, void* pToRead, uint32 uiSize)
{

    I2SBufferInfo_t *pI2SIn;

    void *pBuf;             // the addres of buffer. PCM data will save this address.
    uint32 uiAvailSize;     // the available size for reading PCM data.
    uint32 uiReadSize;
    uint32 uiTailSize;      // the rest size of i2sBufferBytes for reading PCM data.

    pI2SIn = &(psI2sConf->i2sStreamInfo.i2sIn);

    if(pI2SIn->i2sDataPos == pI2SIn->i2sBufferBytes)
    {
        pI2SIn->i2sDataPos = 0U;
    }

    pBuf = (void *)pI2SIn->i2sDmaAddr + pI2SIn->i2sDataPos;
    uiAvailSize = I2S_GetAvailable(psI2sConf , I2S_DIN);

    if(uiAvailSize <= uiSize)
    {
        uiReadSize = uiAvailSize - 4U;
        //I2S_E("Write data size is bigger than available size of buffer");
    }
    else
    {
        uiReadSize = uiSize;
    }

    //Precondition : Because of uiAvailSize, if i2sDataPos is smaller than i2sPreCurPos, uiReadSize should be smaller then uiTailSize.
    uiTailSize = pI2SIn->i2sBufferBytes - pI2SIn->i2sDataPos;

    if(uiReadSize <= uiTailSize)
    {
        SAL_MemCopy((void *)pToRead, (const void *)pBuf, uiReadSize);
        pI2SIn->i2sDataPos += uiReadSize;
    }
    else
    {
        SAL_MemCopy((void *)pToRead, (const void *)pBuf, uiTailSize);
        pBuf = (void *)pI2SIn->i2sDmaAddr;
        SAL_MemCopy((void *)(pToRead + uiTailSize), (const void *)pBuf, (uiReadSize - uiTailSize));

        pI2SIn->i2sDataPos = (uiReadSize - uiTailSize);
    }

    I2S_UpdateStatus(psI2sConf, I2S_DIN);

    return uiReadSize;
}

void I2S_RepeatMode(uint8 ucflag    , boolean bEnable)
{
    //After all of hop transfer has executed, DAIF Tx/Rx DMA channel remains enabled //Continuous

    if(bEnable != SALEnabled)
    {
        //SALDisabled
        if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
        {
            Reg_TransCtrl->bReg.DTRPT = 0U;
        }

        if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
        {
            Reg_TransCtrl->bReg.DRRPT = 0U;
        }

    }else{

        if((ucflag & (uint8)I2S_DOUT) == (uint8)I2S_DOUT)
        {
            Reg_TransCtrl->bReg.DTRPT = 1U;
        }

        if((ucflag & (uint8)I2S_DIN) == (uint8)I2S_DIN)
        {
            Reg_TransCtrl->bReg.DRRPT = 1U;
        }
    }
}




#ifdef AUDIO_DEBUG_TEST

void I2S_LoopBackTest(boolean bEnable)
{

#define OUTPUT_LB

#ifndef OUTPUT_LB
#define INPUT_LB
#endif


#ifdef OUTPUT_LB
    if( bEnable == TRUE )
    {
        Reg_DAMR->bReg.LBT = 1U;       //Loop Back Test Enable
    }
    else
    {
        Reg_DAMR->bReg.LBT = 0U;       //Loop Back Test Enable
    }
#endif

#ifdef INPUT_LB
    if( bEnable == TRUE )
    {
        Reg_DAMR->bReg.LB  = 1U;       //DAIF Loop Back Mode
    }
    else
    {
        Reg_DAMR->bReg.LB = 0U;       //Loop Back Test Enable
    }
#endif

}

#endif

#endif  // ( MCU_BSP_SUPPORT_DRIVER_I2S == 1 )


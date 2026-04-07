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
#include <audio_chip_veri.h>
#include <mpu.h>


void DR_Irq(void * pArg)
{
    //I2SConfig_t *psI2sConfig;
    uint32 dms_cur_pos;

    if( pArg != NULL_PTR )
    {
        //psI2sConfig = (I2SConfig_t *)pArg;

        if(Reg_IntStatus->bReg.DTMI == 1U)
        {
            Reg_IntStatus->bReg.DTMI = 1U;
            dms_cur_pos = I2S_GetTxDaCsar();
            mcu_printf("p:0x%x\n", dms_cur_pos);
        }
#ifdef AUDIO_RX_ENABLE
        if(Reg_IntStatus->bReg.DRMI == 1U)
        {
            Reg_IntStatus->bReg.DRMI = 1U;
            dms_cur_pos = I2S_GetRxDaCdar();
            mcu_printf("c:0x%x\n", dms_cur_pos);
        }
#endif
    }
}


void AUDIO_LRMode_Test(void)
{
    //None
    uint32 ui_dma_base_addr;
    static I2SConfig_t  s_i2s_lr_conf;

    s_i2s_lr_conf.i2sHwCh = I2S_CH0;
    s_i2s_lr_conf.i2sMode = I2S_MASTER_MODE;
    s_i2s_lr_conf.i2sFormat = I2S_FORMAT_I2S;
    s_i2s_lr_conf.i2sNumCh = I2S_STEREO;
    s_i2s_lr_conf.i2sLRmode = I2S_LRMODE_ON;
    s_i2s_lr_conf.i2sBitPerSample = I2S_BIT_DEPTH_16;
    s_i2s_lr_conf.i2sSampleRate = I2S_SAMPLE_RATE_32000;
    s_i2s_lr_conf.i2sBclkDiv = I2S_BCLK_DIV_64;
    s_i2s_lr_conf.i2sMclkDiv = I2S_MCLK_DIV_6;

    I2S_SWReset(SALEnabled);
    I2S_SWReset(SALDisabled);

    ui_dma_base_addr = MPU_GetDMABaseAddress();

    SAL_MemSet((void *)ui_dma_base_addr+0x20000, 0x55, 1024*4); //AUDIO_TxBuffer
    SAL_MemSet((void *)ui_dma_base_addr+0x30000, 0xCC, 1024*4);  //AUDIO_TxBuffer + 1024*4;
    SAL_MemSet((void *)ui_dma_base_addr, 0, 1024*4*2); //AUDIO_RxBuffer

#ifdef AUDIO_DEBUG_TEST
    I2S_SetRxDaDar(ui_dma_base_addr);
    I2S_SetRxDaDarL(ui_dma_base_addr+0x1000);
#endif
    I2S_SetTxDaSar(ui_dma_base_addr+0x2000);
    I2S_SetTxDaSarL(ui_dma_base_addr+0x3000);

    I2S_FifoClear(I2S_DOUT|I2S_DIN);

    I2S_SetGpiofunction(&s_i2s_lr_conf);
    I2S_SetClock(&s_i2s_lr_conf);
    I2S_DaifSetting(&s_i2s_lr_conf);

    // Set interrupt handler for i2s irq
    (void)GIC_IntVectSet(GIC_I2S_DMA, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&DR_Irq, (void *)&s_i2s_lr_conf);
    (void)GIC_IntSrcEn(GIC_I2S_DMA);

    I2S_TxAdmaSetting(&s_i2s_lr_conf);
#ifdef AUDIO_DEBUG_TEST
    I2S_RxAdmaSetting(&s_i2s_lr_conf);
#endif
    Reg_RxDaParam->bReg.SINC    = 4U;                // Audio_DMA + 0x04
    Reg_RxDaParam->bReg.SMASK   = 0xffff00;
    Reg_RxDaTCnt->bReg.ST_TCOUNT = 0x7f;  // Audio_DMA + 0x08 , 0x7f = 127

    Reg_TxDaParam->bReg.SINC    = 4U;
    Reg_TxDaParam->bReg.SMASK   = 0xffff00; // DMA Address 16bit is masked 0xffff8000 bit // 0xffff00
    Reg_TxDaTCnt->bReg.ST_TCOUNT = 0x7f;  // Audio_DMA + 0x48 , 0x7f = 127

    //Related to Repeat.
    Reg_RptCtrl->bReg.RPTCNT    = 1U;       //n: DMA transfer will be repeated (n + 1) * TCOUNT times.
    Reg_RptCtrl->bReg.DBTH      = 7U;       //set for BURST_4  or LRMOD (change because of Burst_8)
    Reg_RptCtrl->bReg.DRI       = 0U;

#ifdef AUDIO_DEBUG_TEST
    I2S_LoopBackTest(SALEnabled);
#endif

    I2S_DAMREnable();
    I2S_Enable((uint8)I2S_DOUT);
    I2S_Enable((uint8)I2S_DIN);

    while(1);
}

void AUDIO_ErrorOcuur_TEST(void)
{
    *Reg_TestMode = 0x9;
    //*Reg_TestMode = 0x5;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )


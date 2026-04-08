// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wm8731.c
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

#include <wm8731.h>


/*          Register Define
===========================================================================================================
          |     B8    ||     B7   |    B6    |    B5    |   B4    |   B3   |  B2   |    B1    |    B0     |
===========================================================================================================
R0  -0x00 | LRIN BOTH || LIN MUTE |    0     |    0     |                  LIN VOLUME                     |
-----------------------------------------------------------------------------------------------------------
R1  -0x01 | RRIN BOTH || RIN MUTE |    0     |    0     |                  RIN VOLUME                     |
-----------------------------------------------------------------------------------------------------------
R2  -0x02 | LRHP BOTH ||   LZCEN  |                           LHP VOLUME                                  |
-----------------------------------------------------------------------------------------------------------
R3  -0x03 | RRHP BOTH ||   RZCEN  |                           RHP VOLUME                                  |
-----------------------------------------------------------------------------------------------------------
R4  -0x04 |     0     ||        SIDEATT      | SIDETONE | DAC SEL | BYPASS | INSEL | MUTE MIC | MIC BOOST |
-----------------------------------------------------------------------------------------------------------
R5  -0x05 |     0     ||     0    |     0    |    0     |    0    |DAC MUTE|      DEEMPH      |  ADC HPD  |
-----------------------------------------------------------------------------------------------------------
R6  -0x06 |     0     || PWR OFF  | CLKOUTPD |  OSC PD  |  OUTPD  |  DACPD | ADCPD |  MICPD   |  LINEINPD |
-----------------------------------------------------------------------------------------------------------
R7  -0x07 |           || BCLKINV  |    MS    |  LRSWAP  |   LRP   |      IWL       |       FORMAT         |
-----------------------------------------------------------------------------------------------------------
R8  -0x08 |           || CLKODIV2 | CLKIDIV2 |                   SR                |   BOSR   | USB/NORMAL|
-----------------------------------------------------------------------------------------------------------
R9  -0x09 |     0     ||     0    |     0    |    0     |    0    |   0    |   0   |    0     |  ACTIVE   |
-----------------------------------------------------------------------------------------------------------
R15 -0x0F |                                          RESET                                                |
===========================================================================================================
*/

unsigned char   gIsAudioMute    = 0;

#define WM8731_ADDR(x)                  (x)<<1              /* Register Addr is MSB 7bit */
#define WM8731_DEV_ID                   (26U)               //0x1A , CSB = GND

/*Temp for Codec Control start*/
#define I2C_TEST_PORT                   (0UL)

#define I2C_TEST_CLK_RATE_100           (100)
#define I2C_TEST_CLK_RATE_400           (400)

#define I2C_TEST_MASTER_CH              (0)
#define I2C_TEST_SLAVE_CH               (0)
/*Temp for Codec Control end*/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

void WM8731_SendCmd
(
    uint8                               ucCh,
    uint8                               ucRegAddr,
    uint8                               ucData
);


/*****************************************************************************
* Function Name : WM8731_SendCmd()
******************************************************************************
* Desription        : The function for H/W Codec I2C Command function
* Parameter         : ucRegAddr - address, ucData - cmd
* Return            : None
******************************************************************************/
void WM8731_SendCmd(uint8 ucCh, uint8 ucRegAddr, uint8 ucData)
{
    uint8 ret;
    uint8 cnt;
    I2CXfer_t XferArg;
    // codec id, subaddr, data, size, subaddr use or nouse?

    ret = SAL_RET_SUCCESS;
    cnt = 0;

RETRY_I2C :

    XferArg.xCmdBuf = (uint8 *) &ucRegAddr;
    XferArg.xCmdLen = (uint8)1UL;
    XferArg.xOutBuf = (uint8 *) &ucData;
    XferArg.xOutLen = (uint8)1UL;
    XferArg.xInBuf  = NULL_PTR;
    XferArg.xInLen  = (uint8)0UL;
    XferArg.xOpt    = (uint8)NULL;

    ret = I2C_XferCmd(ucCh, (WM8731_DEV_ID << 1UL), XferArg, NULL);

    if(ret != SAL_RET_SUCCESS)
    {
        if(cnt > 20)
        {
            return;
        }

        (void)SAL_TaskSleep(5);
        cnt++;
        goto RETRY_I2C;
    }
}

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
)
{
    SALRetCode_t    ret;

    ret = I2C_Open(ucI2cCh, uiI2cPortSel, I2C_TEST_CLK_RATE_100, NULL , NULL);

    if(ret == SAL_RET_FAILED)
    {
        return ret;
    }

    //HwGDATA_A |= HP_CTRL; // set Headphone standby, mute
//  HwDCLKmode  = Hw14 | GET_DPHASE(44.1,256);          // PLL clock -> DCO -> 11.2851*2
    WM8731_SendCmd(ucI2cCh, WM8731_RESET, 0x00);    // reset

    ( void ) SAL_TaskSleep( 1 );

    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_LINVOL), 0x97);       // Llinein register control
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_RINVOL), 0x97);       // Rlinein register control
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_LOUT1V), 0x80);
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_ROUT1V), 0x80);
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APANA), 0x02);       // Lineout select
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APDIGI), 0x01);      // Highpass filter Enable
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_PWR), 0xFE);          // Powerdown control

    if(psI2sConf->i2sFormat == I2S_FORMAT_RIGHTJ )
    {
        WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_IFACE), 0x00);     // Right Justified mode setting
        WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_SRATE), 0x22);     // 384fs over samplingrate, Actually used BSR 16.9343!
    }
    else
    {
        /*
           **  Set Digital Audio Interface Format  **
         - I2S Format, MSB-First left-1 justified
         - Input Audio Data Bit Length Select - 16bit
         - Master Slave Mode Control - Slave Mode
         - Bit Clock Invert - Don't invert BCLK
        */

        //WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_IFACE),0x02); //origin value

        if(psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_16)
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_IFACE), 0x02); //origin value
        }
        else if(psI2sConf->i2sBitPerSample == I2S_BIT_DEPTH_24)
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_IFACE), 0x0A); //For Develop
        }

        /*
            ** Set Sampling Control **
          - Normal mode (256/384fs)
          - Base Over-Sampling Rate - 256fs (12.288Mhz)
          - SR[3:0] - 1000
          - 0 = Core Clock is MCLK
          - CLOCKOUT is Core Clock
        */

        /* Refer Table 18 of WM8731 Data Sheet */
        /* Bit0 : 0 - Normal Mode , Bit1 : 0 - BOSR , Bit2 : SR[3:0] */
        /* Support 8kHz, 32kHz, 44.1kHz, 48kHz, 88.2kHz and 96kHz*/
        if(psI2sConf->i2sSampleRate == I2S_SAMPLE_RATE_8000)
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_SRATE), 0x0C);  //SR:0011
        }
        else if(psI2sConf->i2sSampleRate == I2S_SAMPLE_RATE_32000)
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_SRATE), 0x18);   //SR:0110
        }
        else if(psI2sConf->i2sSampleRate == I2S_SAMPLE_RATE_44100)
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_SRATE), 0x20);   //SR:1000
        }
        else if(psI2sConf->i2sSampleRate == I2S_SAMPLE_RATE_48000)
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_SRATE), 0x00);   //SR:0000
        }
        else if(psI2sConf->i2sSampleRate == I2S_SAMPLE_RATE_96000)
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_SRATE), 0x1C);   //SR:0111
        }
    }

    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_ACTIVE), 0x01);  // Active

    WM8731_DACSoftMute(ucI2cCh, SALEnabled);
    WM8731_Power(ucI2cCh, WM8731_DACPOWER_ALLOFF);

    return ret;
}

/*****************************************************************************
* Function Name : WM8731DACSoftMute()
******************************************************************************
* Desription        : The function for H/W Codec Mute Control function
* Parameter         : bEnable - 1(Mute On), 0(Mute Off)
* Return            : None
******************************************************************************/
void WM8731_DACSoftMute(uint8 ucI2cCh, boolean bEnable)
{
    if(bEnable)
    {
        WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APDIGI), 0x08);
    }
    else
    {
        WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APDIGI), 0x00);
    }
    gIsAudioMute = bEnable;
}


/*****************************************************************************
* Function Name : WM8731Power()
******************************************************************************
* Desription        : The function for H/W Codec Dac Power Control function
* Parameter         : sDacPower - Dac Power variable
* Return            : None
******************************************************************************/
void WM8731_Power(uint8 ucI2cCh, WM8731DacPower_t sDacPower)
{
    switch(sDacPower){
        case WM8731_DACPOWER_ALLON:
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APANA), 0x12);   // Lineout select
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_PWR), 0x62); // Powerdown control
            break;
        }
        case WM8731_DACPOWER_ADCON:
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APANA), 0x0A);   // Lineout select
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_PWR), 0x6A); // Powerdown control
            break;
        }
        case WM8731_DACPOWER_DACON:
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APANA), 0x12);   // Lineout select
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_PWR), 0x66); // Powerdown control
            break;
        }
        case WM8731_DACPOWER_ALLOFF:
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_APANA), 0x02);   // Lineout select
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_PWR), 0xFE); // Powerdown control
            break;
        }
        case WM8731_DACPOWER_MICON:
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_PWR), 0x68); // Powerdown control Mic On
            break;
        }
        case WM8731_DACPOWER_MICOFF:
        {
            WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_PWR), 0x6A); // Powerdown control Mic On
            break;
        }
        default :
        {
            break;
        }
    }
}


/*****************************************************************************
* Function Name : WM8731SetAudioOutVolume()
******************************************************************************
* Desription        : The function for H/W Codec Output Volume Control function
* Parameter         : ucHPV - Output Volume Level
* Return            : None
******************************************************************************/
void WM8731_SetAudioOutVolume(uint8 ucI2cCh, uint8 ucHPV)
{
    // HPV value is limit to 40.
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_LOUT1V), 175 + (ucHPV * 2)); // LHPVol register control
    WM8731_SendCmd(ucI2cCh, WM8731_ADDR(WM8731_ROUT1V), 175 + (ucHPV * 2)); // RHPVol register control
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )


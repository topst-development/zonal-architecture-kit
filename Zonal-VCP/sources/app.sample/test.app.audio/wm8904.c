// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wm8904.c
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

#include <wm8904.h>

#define WM8904_DEV_ID                   (0x1A)               //0b 0011 0100 = 0x34 = 0x1A << 1


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

#define WM8904_NUMBER_VOLUME_COMMANDS   4U
#define WM8904_MAX_VOLUME               256U

const uint8 volumeSteps[WM8904_MAX_VOLUME]=
{  // 64 is -48 dB, 96 is -36 dB, 104 is -33 dB
  0,  64,  96, 104, 112, 113, 114, 115,     // 112 is -30 dB
116, 117, 118, 119, 120, 120, 121, 123,
123, 124, 125, 126, 127, 128, 128, 129,     // 128 is -24 dB
130, 130, 131, 132, 133, 134, 134, 135,
135, 136, 137, 138, 138, 139, 140, 140,
141, 142, 143, 144, 144, 144, 145, 145,     // 144 is -18 dB
145, 146, 146, 147, 147, 148, 148, 148,
149, 149, 149, 150, 150, 150, 151, 151,
152, 152, 153, 153, 153, 154, 154, 154,     // 152 is -15 dB
155, 155, 155, 156, 156, 157, 157, 157,
158, 158, 159, 159, 159, 160, 160, 161,     // 160 is -12 dB
161, 161, 162, 162, 162, 163, 163, 163,
164, 164, 165, 165, 165, 166, 166, 166,
167, 167, 167, 168, 168, 169, 169, 170,     // 168 is -9.0 dB
170, 170, 171, 171, 171, 172, 172, 172,
172, 172, 173, 173, 173, 173, 173, 173,     // 172 is -7.5 dB
174, 174, 174, 174, 174, 175, 175, 175,
175, 175, 175, 176, 176, 176, 176, 176,     // 176 is -6.0 dB
177, 177, 177, 177, 177, 178, 178, 178,
178, 178, 179, 179, 179, 179, 179, 179,
180, 180, 180, 180, 180, 180, 181, 181,     // 180 is -4.5 dB
181, 181, 181, 181, 181, 182, 182, 182,
182, 182, 182, 182, 182, 183, 183, 183,
183, 183, 183, 183, 183, 184, 184, 184,     // 184 is -3.0 dB
184, 184, 184, 184, 184, 185, 185, 185,
185, 185, 185, 185, 185, 186, 186, 186,
186, 186, 186, 187, 187, 187, 187, 187,
187, 187, 187, 188, 188, 188, 188, 188,     // 188 is -1.5 dB
188, 188, 188, 189, 189, 189, 189, 189,
189, 189, 189, 189, 190, 190, 190, 190,
190, 190, 190, 190, 191, 191, 191, 191,
191, 191, 191, 191, 192, 192, 192, 192,     // 192 is 0 dB
};


typedef struct
{
    uint8   ucRegAddr;
    uint16  usValue;
    uint16  msDelay;
}WM8904cmd_t;

static const WM8904cmd_t WM8904_InitCommand[] =
{
    { WM8904_BIAS_CONTROL_0, WM8904_ISEL_HP_BIAS, 2 },
    { WM8904_VMID_CONTROL_0, WM8904_VMID_BUF_ENA |WM8904_VMID_RES_FAST | WM8904_VMID_ENA, 5 },
    { WM8904_VMID_CONTROL_0, WM8904_VMID_BUF_ENA | WM8904_VMID_RES_NORMAL | WM8904_VMID_ENA, 2 },
    { WM8904_BIAS_CONTROL_0, WM8904_ISEL_HP_BIAS | WM8904_BIAS_ENA, 2 },
    { WM8904_POWER_MANAGEMENT_2, WM8904_HPL_PGA_ENA | WM8904_HPR_PGA_ENA, 2 },
    { WM8904_DAC_DIGITAL_1, WM8904_DEEMPH(0), 2 },
    { WM8904_ANALOGUE_OUT12_ZC, 0x0000, 2 },
    { WM8904_CHARGE_PUMP_0, WM8904_CP_ENA, 2 },
    { WM8904_CLASS_W_0, WM8904_CP_DYN_PWR, 2 },
    { WM8904_DIGITAL_PULLS, WM8904_LRCLK_PU | WM8904_BCLK_PU, 2},

    { WM8904_POWER_MANAGEMENT_6, WM8904_DACL_ENA | WM8904_DACR_ENA /*| WM8904_ADCL_ENA | WM8904_ADCR_ENA*/, 5 },
    { WM8904_ANALOGUE_HP_0, WM8904_HPL_ENA | WM8904_HPR_ENA, 2 },
    { WM8904_ANALOGUE_HP_0, WM8904_HPL_ENA_DLY | WM8904_HPL_ENA | WM8904_HPR_ENA_DLY | WM8904_HPR_ENA, 2 },
    { WM8904_DC_SERVO_0, WM8904_DCS_ENA_CHAN_3 | WM8904_DCS_ENA_CHAN_2 | WM8904_DCS_ENA_CHAN_1 | WM8904_DCS_ENA_CHAN_0, 2 },
    { WM8904_DC_SERVO_1, WM8904_DCS_TRIG_STARTUP_3 | WM8904_DCS_TRIG_STARTUP_2 | WM8904_DCS_TRIG_STARTUP_1 | WM8904_DCS_TRIG_STARTUP_0, 100 },
    { WM8904_ANALOGUE_HP_0, WM8904_HPL_ENA_OUTP | WM8904_HPL_ENA_DLY | WM8904_HPL_ENA | WM8904_HPR_ENA_OUTP | WM8904_HPR_ENA_DLY | WM8904_HPR_ENA, 2 },
    { WM8904_ANALOGUE_HP_0, WM8904_HPL_RMV_SHORT | WM8904_HPL_ENA_OUTP | WM8904_HPL_ENA_DLY | WM8904_HPL_ENA | WM8904_HPR_RMV_SHORT | WM8904_HPR_ENA_OUTP | WM8904_HPR_ENA_DLY | WM8904_HPR_ENA, 2 },

    //Volume set

    { WM8904_ANALOGUE_OUT1_LEFT, WM8904_HPOUTL_VOL(0x39), 2 },
    { WM8904_ANALOGUE_OUT1_RIGHT, WM8904_HPOUTR_VOL(0x39), 2 },
    { WM8904_ANALOGUE_OUT1_LEFT, WM8904_HPOUT_VU | WM8904_HPOUTL_VOL(0x39), 2 },
    { WM8904_ANALOGUE_OUT1_RIGHT, WM8904_HPOUT_VU | WM8904_HPOUTR_VOL(0x39), 100 },

    //Codec slave mode ( I2S Master Mode )
    { WM8904_CLOCK_RATES_2, WM8904_CLK_SYS_ENA | WM8904_CLK_DSP_ENA, 5 },
};

void WM8904_SendCmd
(
    uint8                               ucCh,
    uint8                               ucRegAddr,
    uint16                              usData,
    uint8                               ucLen
);


void WM8904_ReadCmd
(
    uint8                               ucCh,
    uint8                               ucRegAddr,
    uint16 *                            pusData,
    uint8                               ucLen
);

/*****************************************************************************
* Function Name : WM8904_SendCmd()
******************************************************************************
* Desription        : The function for H/W Codec I2C Command function
* Parameter         : ucRegAddr - address, ucData - cmd
* Return            : None
******************************************************************************/
void WM8904_SendCmd(uint8 ucCh, uint8 ucRegAddr, uint16 usData, uint8 ucLen)
{
    uint8 ret;
    uint8 cnt;
    uint16 byteSwap;
    I2CXfer_t XferArg;

    ret = SAL_RET_SUCCESS;
    cnt = 0;

    byteSwap = ((usData >> 8) | (usData << 8));

RETRY_I2C :

    XferArg.xCmdBuf = (uint8 *) &ucRegAddr;
    XferArg.xCmdLen = (uint8)1UL;
    XferArg.xOutBuf = (uint8 *) &byteSwap;
    XferArg.xOutLen = (uint8)ucLen;
    XferArg.xInBuf  = NULL_PTR;
    XferArg.xInLen  = (uint8)0UL;
    XferArg.xOpt    = (uint8)NULL;

    ret = I2C_XferCmd(ucCh, (WM8904_DEV_ID << 1UL), XferArg, NULL);

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


/*****************************************************************************
* Function Name : WM8904_ReadCmd()
******************************************************************************
* Desription        : The function for H/W Codec I2C Command function
* Parameter         : ucRegAddr - address, ucData - cmd
* Return            : None
******************************************************************************/


void WM8904_ReadCmd(uint8 ucCh, uint8 ucRegAddr, uint16 * pusData, uint8 ucLen)
{
    uint8 ret;
    uint8 cnt;
    uint16 byteSwap;
    I2CXfer_t XferArg;

    ret = SAL_RET_SUCCESS;
    cnt = 0;

RETRY_I2C :

    XferArg.xCmdBuf = (uint8 *) &ucRegAddr;
    XferArg.xCmdLen = (uint8)1UL;
    XferArg.xOutBuf = (uint8 *) NULL_PTR;
    XferArg.xOutLen = (uint8)NULL;
    XferArg.xInBuf  = (uint8 *) &byteSwap;
    XferArg.xInLen  = (uint8)ucLen;
    XferArg.xOpt    = (uint8)NULL;

    ret = I2C_XferCmd(ucCh, (WM8904_DEV_ID << 1UL), XferArg, NULL);

    *pusData = ((byteSwap >> 8) | (byteSwap << 8));

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
*                                          WM8904_Initial
*
* The function for H/W Codec Initialize function
*
* @return
* Notes
*
**************************************************************************************************
*/

SALRetCode_t WM8904_Initial
(
    uint8                               ucI2cCh,
    uint32                              uiI2cPortSel,
    I2SConfig_t *                       psI2sConf
)
{
    SALRetCode_t        ret;
    uint16              usWriteData = 0xffff;
    uint16              usReadData = 0x0000;
    uint8               ucSize;
    uint8               ucIdx;
    WM8904cmd_t         sCodecCmd;

    ret = I2C_Open(ucI2cCh, uiI2cPortSel, I2C_TEST_CLK_RATE_100, NULL , NULL);


    /* veify I2C communication status */

    WM8904_SendCmd(ucI2cCh, WM8904_SW_RESET_AND_ID, usWriteData, WM8904_DATA_LENGTH);
    ( void ) SAL_TaskSleep( 300 );

    WM8904_ReadCmd(ucI2cCh, WM8904_SW_RESET_AND_ID, &usReadData, WM8904_DATA_LENGTH);
    ( void ) SAL_TaskSleep( 10);

    mcu_printf("WM8904 R0 : 0x%x \n", usReadData);

    if(usReadData != 0x8904)
    {
        mcu_printf("WM8904 is NOT operating.\n");
    }

    /* Initialize WM8904 setting */
    ucSize = sizeof(WM8904_InitCommand)/sizeof(WM8904_InitCommand[0]);

    for(ucIdx = 0; ucIdx < ucSize; ucIdx++)
    {
        WM8904_SendCmd(ucI2cCh, WM8904_InitCommand[ucIdx].ucRegAddr, WM8904_InitCommand[ucIdx].usValue, WM8904_DATA_LENGTH);
        ( void ) SAL_TaskSleep(WM8904_InitCommand[ucIdx].msDelay);
    }

    //_setAudioCommunicationFormat -> set Format ( LeftJ, RightJ, I2S etc )
    sCodecCmd.ucRegAddr = WM8904_AUDIO_INTERFACE_1;
    sCodecCmd.msDelay = 2;
    if( psI2sConf->i2sFormat == I2S_FORMAT_I2S )
    {
        if( psI2sConf->i2sMode == I2S_MASTER_MODE) //CODEC SLAVE
        {
            sCodecCmd.usValue = WM8904_AIF_WL_16BIT | WM8904_AIF_FMT_I2S;
        }
        else  //I2S_SLAVE_MODE , CODEC MASTER
        {
            //BCLK_DIR = 1 , BCLK is output
            sCodecCmd.usValue = WM8904_BCLK_DIR | WM8904_AIF_WL_16BIT | WM8904_AIF_FMT_I2S;
        }
    }
    else if( psI2sConf->i2sFormat == I2S_FORMAT_RIGHTJ )
    {
        if( psI2sConf->i2sMode == I2S_MASTER_MODE) //CODEC SLAVE
        {
            sCodecCmd.usValue = WM8904_AIF_WL_16BIT | WM8904_AIF_FMT_RIGHT;
        }
        else   //I2S_SLAVE_MODE , CODEC MASTER
        {
            //BCLK_DIR = 1 , BCLK is output
            sCodecCmd.usValue = WM8904_BCLK_DIR | WM8904_AIF_WL_16BIT | WM8904_AIF_FMT_RIGHT;
        }
    }
    else if( psI2sConf->i2sFormat == I2S_FORMAT_LEFTJ )
    {
        if( psI2sConf->i2sMode == I2S_MASTER_MODE) //CODEC SLAVE
        {
            sCodecCmd.usValue = WM8904_AIF_WL_16BIT | WM8904_AIF_FMT_LEFT;
        }
        else   //I2S_SLAVE_MODE , CODEC MASTER
        {
            //BCLK_DIR = 1 , BCLK is output
            sCodecCmd.usValue = WM8904_BCLK_DIR | WM8904_AIF_WL_16BIT | WM8904_AIF_FMT_LEFT;
        }
    }
    else
    {
        mcu_printf("[WM8904] Wrong I2S Format set. \n");
    }

    WM8904_SendCmd(ucI2cCh, sCodecCmd.ucRegAddr, sCodecCmd.usValue, WM8904_DATA_LENGTH);
    ( void ) SAL_TaskSleep(sCodecCmd.msDelay);

    return ret;
}


void WM8904_SetVolume(uint8 ucI2cCh, uint32 volume)
{
    uint16      dacVol;
    uint8       indexedVolume;

    if(volume >= WM8904_MAX_VOLUME)
    {
        mcu_printf("[WM8904] volume value exceed maximum. \n");
        return;
    }

    indexedVolume = volumeSteps[volume];
    dacVol = WM8904_DACL_VOL(indexedVolume);

    WM8904_SendCmd(ucI2cCh, WM8904_DAC_DIGITAL_VOLUME_LEFT, dacVol , WM8904_DATA_LENGTH);
    ( void ) SAL_TaskSleep(2);
    /* Writing a 1 to "DAC_VU" causes left and right DAC valume to be updated simultaneously */
    WM8904_SendCmd(ucI2cCh, WM8904_DAC_DIGITAL_VOLUME_RIGHT, (uint16)(dacVol|WM8904_DAC_VU), WM8904_DATA_LENGTH);
    ( void ) SAL_TaskSleep(2);

    return;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key_i2c.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#include <bsp.h>

#include "key_i2c.h"
#include "key_adc.h"
#include "key.h"
#include "gpio.h"
#include "debug.h"
#include "i2c.h"

#include "i2c_test.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

//#define I2CKEY_DEBUG
#ifdef I2CKEY_DEBUG
#define I2CKEY_DBG(fmt, args...)        {LOGD(DBG_TAG_I2C, fmt, ## args)}
#else
#define I2CKEY_DBG(fmt, args...)        {}
#endif
#define I2CKEY_ERR(fmt, args...)        {LOGE(DBG_TAG_I2C, fmt, ## args)}

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/

static KEYI2cKeyinfo_t i2c_key          = {-1, 0, 0, 0, 0};
static KEYI2cKeyType_t I2CKey_Main;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void KEY_I2cResetKeystatus
(
    void
);

static int32 KEY_I2cGetkeycodebyscancode
(
    int32                               iCh,
    uint32                              uiI2cdata
);


/*
***************************************************************************************************
*                                          KEY_I2cResetKeystatus
*
*
*
* Notes
*
***************************************************************************************************
*/
static void KEY_I2cResetKeystatus (void)
{
    i2c_key.iOld_key = -1;
    i2c_key.iKey_pressed=(int32)KEY_I2C_RELEASED;
    i2c_key.iKey_status=(int32)KEY_I2C_RELEASED;
    i2c_key.iKey_chk_cnt=0;
    i2c_key.iCh = 0;
}


/*
***************************************************************************************************
*                                          KEY_I2cGetkeycodebyscancode
*
* @param    iCh [in]
* @param    uiI2cdata [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static int32 KEY_I2cGetkeycodebyscancode(int32 iCh, uint32 uiI2cdata)
{
static struct KEYI2cButton buttons0[8] = {
    {0xEE0, 0xF00, I2C_KEY_VOLUMEUP},
    {0xF10, 0xF20, I2C_KEY_VOLUMEDOWN},
    {0xF40, 0xF50, I2C_KEY_MODE},
    {0xFA0, 0xFB0, I2C_KEY_SEEKUP},
    {0x001, 0x010, I2C_KEY_SEEKDOWN},
    {0x080, 0x090, I2C_KEY_MUTE},
    {0x120, 0x130, I2C_KEY_PHONEON},
    {0x1F0, 0x200, I2C_KEY_PHONEOFF},
};

    uint32 i;
    uint32 i2cdata;
    int32 key = -1;
    int32 data=0;

    i2cdata = (uiI2cdata & 0x00000FFFUL);
    data = (int32)i2cdata;

    switch(iCh)
    {
        case 0: //ch 2 //MC_FRT_ADKEY1
            for (i = 0; i < (sizeof(buttons0)/sizeof(KEYI2cButton_t)); i++) {
                if ((data >= buttons0[i].ibSscancode) && (data <= buttons0[i].ibEscancode))
                {
                    key = (int32)buttons0[i].ibVkcode;
                    I2CKEY_DBG("(i2c key (%d) : 0x%x\n", iCh, key);
                }
            }
            break;
        default :
            I2CKEY_DBG("%s : Wrong parameter - %d\n", __func__, iCh);
            break;
    }

    return key;
}


/*
***************************************************************************************************
*                                          KEY_I2cOpen
*
* @param    uiOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_I2cOpen(void)
{
    SALRetCode_t i2c_key_lret;


    mcu_printf("\n\n-------TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1----------\n");
    i2c_key_lret = (SALRetCode_t)I2C_Open((uint8)I2C_KEY_CHNL, I2C_TEST_PORT,
                            (uint32)I2C_KEY_SCLK,//temp GPIO_GPMB ->GPB
                            I2C_KEY_NULL, I2C_KEY_NULL);


    if (i2c_key_lret == SAL_RET_SUCCESS)
    {
        I2CKey_Main.uiI2cKeyStatus = KEY_DEVICE_ST_OPENED;
    }
    else
    {
        I2CKEY_ERR("(E) %s open fail (%d)\n", __func__, i2c_key_lret);
        I2CKey_Main.uiI2cKeyStatus = KEY_DEVICE_ST_NO_OPEN;
    }

    return i2c_key_lret;
}


/*
***************************************************************************************************
*                                          KEY_I2cRead
*
* @param    buff [in]
* @param    size [in]
* @param    regaddr [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t KEY_I2cRead(uint8 * puiBuff, uint32 uiSize, uint8 uiRegaddr)
{
    uint8  i2c_key_cnvr[1] = {0x00};
    uint8  i2c_key_size  = 0;
    uint8  i2c_key_asyn  = 0;
    uint8 *i2c_key_buff  = NULL;
    uint32 i2c_key_optn  = 0;
    SALRetCode_t i2c_key_lret;
    I2CXfer_t XferArg;

    I2CKEY_DBG("%s buff(0x%02x%02x) size(%d)\n", __func__, puiBuff[0], puiBuff[1], uiSize);

    if (I2CKey_Main.uiI2cKeyStatus != (uint32)KEY_DEVICE_ST_OPENED )
    {
        i2c_key_lret = SAL_RET_FAILED;
        I2CKEY_ERR("(E) %s not opened\n", __func__);
    }
    else if ((puiBuff == ((void *)0)) || (uiSize <= (uint32)0))
    {
        i2c_key_lret = SAL_RET_FAILED;
        I2CKEY_ERR("(E) %s invalid parameter\n", __func__);
    }
    else
    {
        i2c_key_buff = puiBuff;
        i2c_key_size = (uint8)uiSize;
        i2c_key_cnvr[0] = uiRegaddr;

        XferArg.xCmdBuf = NULL_PTR;
        XferArg.xCmdLen = (uint8)NULL;
        XferArg.xOutBuf = i2c_key_cnvr;
        XferArg.xOutLen = (uint8)I2C_KEY_BLEN;
        XferArg.xInBuf  = i2c_key_buff;
        XferArg.xInLen  = (uint8)i2c_key_size;
        XferArg.xOpt    = (uint8)i2c_key_optn;

        i2c_key_lret = (SALRetCode_t)I2C_Xfer((uint8)I2C_KEY_CHNL, (uint8)I2C_KEY_SADR,
                                              XferArg, i2c_key_asyn);
    }

    return i2c_key_lret;
}

/*
***************************************************************************************************
*                                          KEY_I2cWrite
*
* @param    puiBuff [in]
* @param    uiSize [in]
* @param    uiRegaddr [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_I2cWrite(uint8 * puiBuff, uint32 uiSize, uint8 uiRegaddr)
{
    //uint8  i2c_key_cnfr[1] = {0x00};
    uint8  i2c_key_size  = 0;
    uint8  i2c_key_asyn  = 0;
    uint8 *i2c_key_buff  = NULL;
    uint32 i2c_key_optn  = 0;
    SALRetCode_t i2c_key_lret;
    I2CXfer_t XferArg;
    //I2CKEY_DBG("%s buff(0x%02x%02x) size(%d)\n", __func__, buff[0], buff[1], size);

    if (I2CKey_Main.uiI2cKeyStatus != (uint32)KEY_DEVICE_ST_OPENED )
    {
        i2c_key_lret = SAL_RET_FAILED;
        I2CKEY_ERR("(E) %s not opened [%d]\n", __func__,uiRegaddr);
    }
    else if ((puiBuff == ((void *)0)) || (uiSize <= 0UL))
    {
        i2c_key_lret = SAL_RET_FAILED;
        I2CKEY_ERR("(E) %s invalid parameter [%d]\n", __func__,uiRegaddr);
    }
    else
    {
        i2c_key_buff = puiBuff;
        i2c_key_size = (uint8)uiSize;

        XferArg.xCmdBuf = NULL_PTR;
        XferArg.xCmdLen = (uint8)NULL;
        XferArg.xOutBuf = i2c_key_buff;
        XferArg.xOutLen = (uint8)i2c_key_size;
        XferArg.xInBuf  = NULL_PTR;
        XferArg.xInLen  = (uint8)0UL;
        XferArg.xOpt    = (uint8)i2c_key_optn;

        i2c_key_lret = (SALRetCode_t)I2C_XferCmd(I2C_KEY_CHNL, I2C_KEY_SADR,
                                                XferArg, i2c_key_asyn);
    }

    return i2c_key_lret;
}

/*
***************************************************************************************************
*                                          KEY_I2cScan
*
* @param    piKeyVal [in]
* @param    piKeyPressed [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

int32 KEY_I2cScan(int32 *piKeyVal, int32 *piKeyPressed)
{

    SALRetCode_t lret;
    uint8 dev_read_buf[I2C_KEY_RLEN] = {0x00, 0x00};
    uint8 dev_write_1st_buf[I2C_KEY_WLEN1] = {0x01, 0x84,0x83};
    uint8 dev_write_2nd_buf[I2C_KEY_WLEN2] = {0x00};
    int32 ret_key = -1;
    int32 key = -1;
    int32 i = 0;
    int32 key_checked = 0;
    uint32 i2c_value =0;

    lret = SAL_RET_FAILED;

    lret = KEY_I2cWrite(dev_write_1st_buf,(uint32)I2C_KEY_WLEN1,(uint8)I2C_ADS_CNVT);

    lret = KEY_I2cWrite(dev_write_2nd_buf,(uint32)I2C_KEY_WLEN2,(uint8)I2C_ADS_CNVT);

    lret = KEY_I2cRead(dev_read_buf, (uint32)I2C_KEY_RLEN, (uint8)I2C_ADS_CNVT);

    if(lret == SAL_RET_SUCCESS)
    {
    i2c_value = ((uint32)dev_read_buf[0]<<(uint32)8) | ((uint32)dev_read_buf[1]);

    /*shift LSB 4bit, see the ADS specification 8.5.4*/
    i2c_value =  i2c_value>>(uint32)4;

    key = KEY_I2cGetkeycodebyscancode((int32)0,i2c_value);

    if (key > 0)
    {
        if((i2c_key.iKey_pressed == (int32)KEY_I2C_RELEASED)  && (i2c_key.iOld_key != key)) //first pressed
        {
            I2CKEY_DBG("Key info[%d] : [Pressed]old_key = %d / cur_key=%d\n", i, i2c_key.iOld_key, key);
            i2c_key.iOld_key = key;
            i2c_key.iCh = i;
            i2c_key.iKey_pressed = (int32)KEY_I2C_PRESSED;
            i2c_key.iKey_status = (int32)KEY_I2C_CHECK; // to verify
            i2c_key.iKey_chk_cnt = (int32)0;
            if(piKeyPressed != ((void *)0))
            {
                *piKeyPressed = (int32)0;
            }
            //*key_val = i2c_key.old_key[i];

            ret_key = -1;
        }
        else if ((i2c_key.iKey_pressed == (int32)KEY_I2C_PRESSED)  && (i2c_key.iOld_key == key)) //keep pressed
        {
            if (i2c_key.iKey_status == (int32)KEY_I2C_CHECK)
            {
                if(i2c_key.iKey_chk_cnt > 5)
                {
                    I2CKEY_DBG("Key info[%d] : [chk]ch = %d / old_key = %d / cur_key=%d\n", i, i2c_key.iCh, i2c_key.iOld_key, key);
                    i2c_key.iKey_status = (int32)KEY_I2C_PRESSED;
                    i2c_key.iKey_chk_cnt = 0;
                    if((piKeyPressed != ((void *)0)) && (piKeyVal != ((void *)0)))
                    {
                        *piKeyVal = i2c_key.iOld_key;
                        *piKeyPressed = 1;
                    }

                    ret_key = 0;
                    key_checked = 1;
                    //break;
                }
                else
                {
                    i2c_key.iKey_chk_cnt++;
                }
            }
            else {
                i2c_key.iKey_status = (int32)KEY_I2C_PRESSING;
                if((piKeyPressed != ((void *)0)) && (piKeyVal != ((void *)0)))
                {
                    *piKeyVal = i2c_key.iOld_key;
                    *piKeyPressed = 1;
                }
                ret_key = -1;
            }
            I2CKEY_DBG("Key info[%d] : [Pressing]old_key = %d / cur_key=%d\n", i, i2c_key.iOld_key, key);
        }
        else
        {
            if((i2c_key.iKey_status == (int32)KEY_I2C_PRESSED) || (i2c_key.iKey_status == (int32)KEY_I2C_PRESSING))
            {
                I2CKEY_DBG("Key info[%d] : [Released_1]ch = %d / old_key = %d / cur_key=%d\n", i, i2c_key.iCh, i2c_key.iOld_key, key);
                i2c_key.iKey_pressed = (int32)KEY_I2C_RELEASED;
                i2c_key.iKey_status = (int32)KEY_I2C_RELEASED;
                i2c_key.iKey_chk_cnt = 0;
                if((piKeyPressed != ((void *)0)) && (piKeyVal != ((void *)0)))
                {
                    *piKeyVal = i2c_key.iOld_key;
                    *piKeyPressed = 0;
                }
                ret_key = 0;
                key_checked = 1;
                //break;
            }
            else {
                I2CKEY_DBG("Key info[%d](No pressed key) : [Released_1]ch = %d / old_key = %d / cur_key=%d\n", i, i2c_key.iCh, i2c_key.old_key, key);
                i2c_key.iOld_key = -1;
                i2c_key.iKey_pressed = (int32)KEY_I2C_RELEASED;
                i2c_key.iKey_status = (int32)KEY_I2C_RELEASED;
                key_checked = 0;
            }
        }
        I2CKEY_DBG("Key info[%d] : ch = %d / old_key = %d / cur_key=%d / pressed=%d\n", i, i2c_key.iCh, i2c_key.iOld_key, key, i2c_key.key_pressed);
    }
    else
    {
        if(i2c_key.iKey_status == NULL)
        {
            KEY_I2cResetKeystatus();
            ret_key = -1;
        }
        else if((i2c_key.iKey_status > (int32)KEY_I2C_CHECK) && (i2c_key.iOld_key > 0) && (i2c_key.iCh == i)) //released
        {
            I2CKEY_DBG("Key info[%d] : [Released_2]ch = %d /old_key = %d / cur_key=%d\n", i, i2c_key.iCh, i2c_key.iOld_key, key);
            if((piKeyPressed != ((void *)0)) && (piKeyVal != ((void *)0)))
            {
                *piKeyVal = i2c_key.iOld_key;
                *piKeyPressed = 0;
            }
            KEY_I2cResetKeystatus();
            ret_key = 0;
            key_checked = 1;
            //break;
        }
        else
        {
            ret_key = -1; //none key
        }
    }

    if (key_checked == 1) //stop scaning key
    {
        key_checked = 0; //key detected
        //break;
    }
    else //keep scaning key
    {
        ; //none key
    }

    return ret_key;
    }
    else
    {
        I2CKEY_DBG("I2C Read/Write Fail!!\n");
        ret_key = -1;
    }
    return ret_key;
}

#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )


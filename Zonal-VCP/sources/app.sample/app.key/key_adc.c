// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key_adc.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#include "key_adc.h"
#include "adc.h"
#include "gpio.h"
#include "key.h"
#include <bsp.h>
#include "debug.h"


/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/

static KEYAdcKeyType_t                  ADCKey_Main;
static KEYKeyInfo_t                     tcc_key = {-1, 0, 0, 0, 0};

#ifdef INTERNAL_KEY
static struct KEYButton buttons0[6] =
{
    {
        0x59,
        0xE1,
        KEY_SW22
    },
    {
        0x238,
        0x347,
        KEY_SW23
    },
    {
        0x78B,
        0x897,
        KEY_SW24
    },
    {
        0xDFB,
        0xEFD,
        KEY_SW25
    },
    {
        0xA5D,
        0xBB6,
        KEY_SW26
    },
    {
        0x51C,
        0x62D,
        KEY_SW27
    },
};
#endif
#ifdef EXTERNAL_KEY
// CHANNEL7, STERRING WHEEL
static struct KEYButton buttons1[8] = {
    {
        0x59,
        0x97,
        KEY_VOLUMEUP
    },
    {
        0x10E,
        0x162,
        KEY_VOLUMEDOWN
    },
    {
        0x1E8,
        0x267,
        KEY_MODE
    },
    {
        0x35D,
        0x417,
        KEY_SEEKUP
    },
    {
        0x550,
        0x62C,
        KEY_SEEKDOWN
    },
    {
        0x78A,
        0x896,
        KEY_MUTE
    },
    {
        0xA5D,
        0xBB6,
        KEY_PHONEON
    },
    {
        0xDFA,
        0xEFD,
        KEY_PHONEOFF
    },
};
// CHANNEL8, MC_FRT_ADKEY0
static struct KEYButton buttons2[5] = {
    {
        0x00,
        0xE1,
        KEY_ENTER
    },
    {
        0x238,
        0x3BB,
        KEY_SCAN
    },
    {
        0x6EE,
        0x897,
        KEY_SETUP
    },
    {
        0xA5D,
        0XBB6,
        KEY_MENU
    },
    {
        0XDFB,
        0xEFD,
        KEY_DISP
    },
};
// CHANNEL9, MC_FRT_ADKEY1
static struct KEYButton buttons3[4] = {
    {
        0x59,
        0x1F2,
        KEY_TRACKDOWN
    },
    {
        0x67C,
        0x897,
        KEY_TRACKUP
    },
    {
        0xA5D,
        0xBB6,
        KEY_FOLDERDOWN
    },
    {
        0xDFB,
        0xEFD,
        KEY_FOLDERUP
    },
};
// CHANNEL10, MC_FRT_ADKEY2
static struct KEYButton buttons4[4] = {
    {
        0x00,
        0x1F2,
        KEY_POWER
    },
    {
        0x67C,
        0x897,
        KEY_RADIO
    },
    {
        0xA5D,
        0xBB6,
        KEY_MEDIA
    },
    {
        0xDFB,
        0xEFD,
        KEY_PHONE
    },
};
// CHANNEL11, MC_FRT_ADKEY3
static struct KEYButton buttons5[6] = {
    {
        0x59,
        0xE1,
        KEY_1
    },
    {
        0x238,
        0x347,
        KEY_2
    },
    {
        0x51C,
        0x62D,
        KEY_3
    },
    {
        0x78B,
        0x897,
        KEY_4
    },
    {
        0xA5D,
        0xBB6,
        KEY_5
    },
    {
        0xDFB,
        0xEFD,
        KEY_6
    },
};
#endif
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void KEY_AdcResetKeystatus
(
    void
);

static int32 KEY_AdcGetkeycodebyscancode
(
    int32 iCh,
    uint32 uiAdcdata
);


/*
***************************************************************************************************
*                                          KEY_AdcResetKeystatus
*
*
* Notes
*
***************************************************************************************************
*/
static void KEY_AdcResetKeystatus (void)
{
    tcc_key.old_key = -1;
    tcc_key.key_pressed=(int32)KEY_ADC_RELEASED;
    tcc_key.key_status=(int32)KEY_ADC_RELEASED;
    tcc_key.key_chk_cnt=0;
    tcc_key.ch = 0;
}

/*
***************************************************************************************************
*                                          KEY_AdcGetkeycodebyscancode
*
*
* @param    iCh [in]
* @param    uiAdcdata [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static int32 KEY_AdcGetkeycodebyscancode(int32 iCh, uint32 uiAdcdata)
{
    uint32 i;
    int32 key = -1;
    uint32 data=0;

    data = (uiAdcdata & 0x00000FFFUL);

    switch(iCh)
    {
#ifdef INTERNAL_KEY
        case 0: //ch 1 //ADKEY0
            for (i = 0; i < (sizeof(buttons0)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons0[i].buSscancode) && (data <= buttons0[i].buEscancode))
                {
                    key = (int32) buttons0[i].buVkcode;
                    ADCKEY_DBG("ADC KEY : SW%x \n", key);
                    break;
                }
            }
            break;
#endif // INTERNAL_KEY
#ifdef EXTERNAL_KEY
        case 1: //ch 2 //MC_FRT_ADKEY1
            for (i = 0; i < (sizeof(buttons1)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons1[i].buSscancode) && (data <= buttons1[i].buEscancode))
                {
                    key = (int32)buttons1[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    break;
                }
            }
            break;
        case 2: //ch 4 //MC_FRT_ADKEY3
            for (i = 0; i < (sizeof(buttons2)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons2[i].buSscancode) && (data <= buttons2[i].buEscancode))
                {
                    key = (int32)buttons2[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    break;
                }
            }
            break;
        case 3: //ch 5 //MC_FRT_ADKEY4
            for (i = 0; i < (sizeof(buttons3)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons3[i].buSscancode) && (data <= buttons3[i].buEscancode))
                {
                    key = (int32)buttons3[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    break;
                }
            }
            break;
        case 4: //ch 4 //MC_FRT_ADKEY3
            for (i = 0; i < (sizeof(buttons4)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons4[i].buSscancode) && (data <= buttons4[i].buEscancode))
                {
                    key = (int32)buttons4[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    break;
                }
            }
            break;
        case 5: //ch 5 //MC_FRT_ADKEY4
            for (i = 0; i < (sizeof(buttons5)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons5[i].buSscancode) && (data <= buttons5[i].buEscancode))
                {
                    key = (int32)buttons5[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    break;
                }
            }
            break;
#endif
         default :
            //ADCKEY_DBG("%s : Wrong parameter - %d\n", __func__, uiCh);
            break;
    }

    return key;
}


/*
***************************************************************************************************
*                                          KEY_AdcOpen
*
*
* @param    uiOptions [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_AdcOpen(uint32 uiOptions)
{
    SALRetCode_t lRet;

#if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
    static uint32 adc_key_pad_type = KEY_MANY_PUSHKEY;
#endif
    lRet = SAL_RET_SUCCESS;

    ADCKey_Main.akADCKeyStatus = KEY_DEVICE_ST_NO_OPEN; //CS : Useless Assignment - Modify initial value

    if (uiOptions  != 0UL)
    {
        lRet = SAL_RET_FAILED;
    }
    else
    {
        ADCKey_Main.akADCOpenOptions  = uiOptions;
        ADCKey_Main.akADCKeyStatus = KEY_DEVICE_ST_OPENED;
        // open driver
    }
#if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
    //check key type
    adc_key_pad_type = GPIO_Get(GPIO_GPK((uint32)5));

    ADCKEY_DBG("OPEN : adc_key_pad_type = %d\n",adc_key_pad_type);

    if(adc_key_pad_type == (uint32)KEY_ONLY_ROTARY)
    {
        //(void)SAL_MemSet(&buttons0[0], 0x00, sizeof(KEYButton_t));
        //buttons0[0].buSscancode = 0x9E5;
        //buttons0[0].buEscancode = 0xC18;
        //buttons0[0].buVkcode = KEY_POWER;

        (void)SAL_MemSet(&buttons1[0], 0x00, sizeof(KEYButton_t));
        buttons1[1].buSscancode = 0x732;
        buttons1[1].buEscancode = 0x8CC;
        buttons1[1].buVkcode = KEY_MODE;
        buttons1[2].buSscancode = 0x9E5;
        buttons1[2].buEscancode = 0xC18;
        buttons1[2].buVkcode = KEY_5;
        buttons1[3].buSscancode = 0xD4A;
        buttons1[3].buEscancode = 0xFA0;
        buttons1[3].buVkcode = KEY_6;
        (void)SAL_MemSet(&buttons2[0], 0x00, sizeof(KEYButton_t)); //QAC
        buttons2[0].buSscancode = 0x00;
        buttons2[0].buEscancode = 0x83;
        buttons2[0].buVkcode = KEY_ENTER;
    }
 #endif
    return lRet;
}


/*
***************************************************************************************************
*                                          KEY_AdcScan
*
*
* @param    piKeyVal [in]
* @param    piKPressed [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_AdcScan(int32 * piKeyVal, int32 * piKPressed, uint8 ucModNum)
{
    uint32 read_buf[MAX_ADC_KEY]= {0, }; //CS : Uninitialized Variable
    const uint32 *dev_read_buf = NULL;
    SALRetCode_t ret_key = SAL_RET_FAILED;
    int32 key = -1;
    int32 i;
    int32 key_checked = 0;

    if((piKeyVal == ((void *)0)) || (piKPressed == ((void *)0)))
    {
        ret_key = SAL_RET_FAILED;
    }
    else
    {
        dev_read_buf = ADC_AutoScan(ucModNum, siADCDebug);

        for (i=0; i<MAX_ADC_KEY; i++)
        {
            switch (i)
            {
                case 0:
                    read_buf[i] = (dev_read_buf[ADC_CHANNEL_1] & 0xFFFUL);
                    break;
                case 1:
                    read_buf[i] = (dev_read_buf[ADC_CHANNEL_7] & 0xFFFUL);
                    break;
                case 2:
                    read_buf[i] = (dev_read_buf[ADC_CHANNEL_8] & 0xFFFUL);
                    break;
                case 3:
                    read_buf[i] = (dev_read_buf[ADC_CHANNEL_9] & 0xFFFUL);
                    break;
                case 4:
                    read_buf[i] = (dev_read_buf[ADC_CHANNEL_10] & 0xFFFUL);
                    break;
                case 5:
                    read_buf[i] = (dev_read_buf[ADC_CHANNEL_11] & 0xFFFUL);
                    break;
                default:
                    ADCKEY_DBG("INVALID ADC KEY \n");
                    break;
            }
        }


        for (i = 0; i < MAX_ADC_KEY; i++) {
            key = KEY_AdcGetkeycodebyscancode(i, read_buf[i]);
            if (key > 0)
            {
#ifdef EXTERNAL_KEY
                if((tcc_key.key_pressed == (int32)KEY_ADC_RELEASED)  && (tcc_key.old_key != key)) //first pressed
                {
                    ADCKEY_DBG("Key info[%d] : [Pressed]old_key = %d / cur_key=%d\n", i, tcc_key.old_key, key);
                    tcc_key.old_key = key;
                    tcc_key.ch = i;
                    tcc_key.key_pressed = (int32)KEY_ADC_PRESSED;
                    tcc_key.key_status = (int32)KEY_ADC_CHECK; // to verify
                    tcc_key.key_chk_cnt = 0;
                    *piKPressed = 0;
                    ret_key = SAL_RET_FAILED;
                }
                else if ((tcc_key.key_pressed == (int32)KEY_ADC_PRESSED)  && (tcc_key.old_key == key)) //keep pressed
                {
                    if (tcc_key.key_status == (int32)KEY_ADC_CHECK)
                    {
                        if(tcc_key.key_chk_cnt > 5)
                        {
                            ADCKEY_DBG("Key info[%d] : [chk]ch = %d / old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                            tcc_key.key_status = (int32)KEY_ADC_PRESSED;
                            tcc_key.key_chk_cnt = 0;
                            *piKeyVal = tcc_key.old_key;
                            *piKPressed = 1;
                            ret_key = SAL_RET_SUCCESS;
                            key_checked = 1;
                            //break;
                        }
                        else
                        {
                            tcc_key.key_chk_cnt++;
                        }
                    }
                    else {
                        tcc_key.key_status = (int32)KEY_ADC_PRESSING;
                        *piKeyVal = tcc_key.old_key;
                        *piKPressed = 1;
                        ret_key = SAL_RET_FAILED;
                    }
                    ADCKEY_DBG("Key info[%d] : [Pressing]old_key = %d / cur_key=%d\n", i, tcc_key.old_key, key);
                }
                else
                {
                    if((tcc_key.key_status == (int32)KEY_ADC_PRESSED) || (tcc_key.key_status == (int32)KEY_ADC_PRESSING))
                    {
                        ADCKEY_DBG("Key info[%d] : [Released_1]ch = %d / old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                        tcc_key.key_pressed = (int32)KEY_ADC_RELEASED;
                        tcc_key.key_status = (int32)KEY_ADC_RELEASED;
                        tcc_key.key_chk_cnt = 0;
                        *piKeyVal = tcc_key.old_key;
                        *piKPressed = 0;
                        ret_key = SAL_RET_SUCCESS;
                        key_checked = 1;
                        //break;
                    }
                    else {
                        ADCKEY_DBG("Key info[%d](No pressed key) : [Released_1]ch = %d / old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                        tcc_key.old_key = -1;
                        tcc_key.key_pressed = (int32)KEY_ADC_RELEASED;
                        tcc_key.key_status = (int32)KEY_ADC_RELEASED;
                        key_checked = 0;
                    }
                }
                ADCKEY_DBG("Key info[%d] : ch = %d / old_key = %d / cur_key=%d / pressed=%d\n", i, tcc_key.ch, tcc_key.old_key, key, tcc_key.key_pressed);
#endif  // EXTERNAL_KEY
            }
            else
            {
                if(tcc_key.key_status == NULL)
                {
                    KEY_AdcResetKeystatus();
                    ret_key = SAL_RET_FAILED;
                }
                else if((tcc_key.key_status > (int32)KEY_ADC_CHECK) && (tcc_key.old_key > (int32)0) && (tcc_key.ch == i)) //released
                {
                    ADCKEY_DBG("Key info[%d] : [Released_2]ch = %d /old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                    *piKeyVal = tcc_key.old_key;
                    *piKPressed = 0;
                    KEY_AdcResetKeystatus();
                    ret_key = SAL_RET_SUCCESS;
                    key_checked = 1;
                    //break;
                }
                else
                {
                    ret_key = SAL_RET_FAILED; //none key
                }
            }

            if (key_checked == 1) //stop scaning key
            {
                break;
            }
            else //keep scaning key
            {
                ; //none key
            }
        }
    }

    return ret_key;
}

#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key_demo_adc.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_KEY_DEMO == 1 )

#include <app_cfg.h>

#include "key_adc.h"
#include "adc.h"
#include "gpio.h"
#include "key.h"
#include <bsp.h>
#include "debug.h"
#include "key_demo_adc.h"
#include "spu_test.h"

static int32 keyStatus[MAX_ADC_KEY][MAX_ADC_BTN];

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#ifdef INTERNAL_KEY
static struct KEYButton demo_buttons0[6] =
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
static struct KEYButton demo_buttons1[8] = {
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
static struct KEYButton demo_buttons2[5] = {
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
static struct KEYButton demo_buttons3[4] = {
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
static struct KEYButton demo_buttons4[4] = {
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
static struct KEYButton demo_buttons5[6] = {
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
*                                             VARIABLES
***************************************************************************************************
*/

static KEYKeyInfo_t                     demo_key[MAX_ADC_KEY] = {{-1, 0, 0, 0, 0},{-1, 0, 0, 0, 0},{-1, 0, 0, 0, 0},{-1, 0, 0, 0, 0},{-1, 0, 0, 0, 0},{-1, 0, 0, 0, 0}};

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void KEY_DemoAdcResetKeystatus
(
    void
);

static int32 KEY_DemoAdcGetkeycodebyscancode
(
    int32 iCh,
    uint32 uiAdcdata,
    int32 (*piKeyStatus)[MAX_ADC_BTN]

);

static void KEY_DemoAdcToggle
(
    uint32 iKeyCode,
    KEYKeyInfo_t sKeyInfo,
    int32 (*piKeyStatus)[MAX_ADC_BTN]
);


/*
***************************************************************************************************
*                                          KEY_AdcResetKeystatus_demo
*
*
* Notes
*
***************************************************************************************************
*/
static void KEY_DemoAdcResetKeystatus (void)
{
    int32 i;
    for(i=0;i<MAX_ADC_KEY;i++)
    {
        demo_key[i].old_key = -1;
        demo_key[i].key_pressed=(int32)KEY_ADC_RELEASED;
        demo_key[i].key_status=(int32)KEY_ADC_RELEASED;
        demo_key[i].key_chk_cnt=0;
        demo_key[i].ch = 0;
    }
}

/*
***************************************************************************************************
*                                          KEY_AdcGetkeycodebyscancode_demo
*
*
* @param    uiCh [in]
* @param    uiAdcdata [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static int32 KEY_DemoAdcGetkeycodebyscancode(int32 iCh, uint32 uiAdcdata, int32 (*piKeyStatus)[MAX_ADC_BTN])
{
    uint32 i;
    int32 key = -1;
    uint32 data=0;

    data = (uiAdcdata & 0x00000FFFUL);

    switch(iCh)
    {
#ifdef INTERNAL_KEY
        case 0: //ch 1 //ADKEY0
            for (i = 0; i < (sizeof(demo_buttons0)/sizeof(KEYButton_t)); i++) {
                if ((data >= demo_buttons0[i].buSscancode) && (data <= demo_buttons0[i].buEscancode))
                {
                    key = (int32) demo_buttons0[i].buVkcode;
                    ADCKEY_DBG("ADC KEY : SW%x \n", key);
                    if(piKeyStatus[iCh][i] != 0)
                    {
                        piKeyStatus[iCh][i] = 0;
                    }
                    else
                    {
                        piKeyStatus[iCh][i] = 1;
                    }
                    break;
                }
            }
            break;
#endif // INTERNAL_KEY

#ifdef EXTERNAL_KEY
        case 1: //ch 2 //MC_FRT_ADKEY1
            for (i = 0; i < (sizeof(demo_buttons1)/sizeof(KEYButton_t)); i++) {
                if ((data >= demo_buttons1[i].buSscancode) && (data <= demo_buttons1[i].buEscancode))
                {
                    key = (int32)demo_buttons1[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    if(piKeyStatus[iCh][i] != 0)
                    {
                        piKeyStatus[iCh][i] = 0;
                    }
                    else
                    {
                        piKeyStatus[iCh][i] = 1;
                    }
                    break;
                }
            }
            break;
        case 2: //ch 4 //MC_FRT_ADKEY3
            for (i = 0; i < (sizeof(demo_buttons2)/sizeof(KEYButton_t)); i++) {
                if ((data >= demo_buttons2[i].buSscancode) && (data <= demo_buttons2[i].buEscancode))
                {
                    key = (int32)demo_buttons2[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    if(piKeyStatus[iCh][i] != 0)
                    {
                        piKeyStatus[iCh][i] = 0;
                    }
                    else
                    {
                        piKeyStatus[iCh][i] = 1;
                    }
                    break;
                }
            }
            break;
        case 3: //ch 5 //MC_FRT_ADKEY4
            for (i = 0; i < (sizeof(demo_buttons3)/sizeof(KEYButton_t)); i++) {
                if ((data >= demo_buttons3[i].buSscancode) && (data <= demo_buttons3[i].buEscancode))
                {
                    key = (int32)demo_buttons3[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    if(piKeyStatus[iCh][i] != 0)
                    {
                        piKeyStatus[iCh][i] = 0;
                    }
                    else
                    {
                        piKeyStatus[iCh][i] = 1;
                    }
                    break;
                }
            }
            break;
        case 4: //ch 4 //MC_FRT_ADKEY3
            for (i = 0; i < (sizeof(demo_buttons4)/sizeof(KEYButton_t)); i++) {
                if ((data >= demo_buttons4[i].buSscancode) && (data <= demo_buttons4[i].buEscancode))
                {
                    key = (int32)demo_buttons4[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    if(piKeyStatus[iCh][i] != 0)
                    {
                        piKeyStatus[iCh][i] = 0;
                    }
                    else
                    {
                        piKeyStatus[iCh][i] = 1;
                    }
                    break;
                }
            }
            break;
        case 5: //ch 5 //MC_FRT_ADKEY4
            for (i = 0; i < (sizeof(demo_buttons5)/sizeof(KEYButton_t)); i++) {
                if ((data >= demo_buttons5[i].buSscancode) && (data <= demo_buttons5[i].buEscancode))
                {
                    key = (int32)demo_buttons5[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", iCh, key);
                    if(piKeyStatus[iCh][i] != 0)
                    {
                        piKeyStatus[iCh][i] = 0;
                    }
                    else
                    {
                        piKeyStatus[iCh][i] = 1;
                    }
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
*                                          KEY_AdcScan_demo
*
*
* @param    piKeyVal [in]
* @param    piKPressed [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_DemoAdcScan(uint8 ucModNum)
{
    uint32 read_buf[MAX_ADC_KEY]= {0, }; //CS : Uninitialized Variable
    const uint32 *dev_read_buf = NULL;
    SALRetCode_t ret_key = SAL_RET_FAILED;
    int32 key = -1;
    int32 i;

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
        key = KEY_DemoAdcGetkeycodebyscancode(i, read_buf[i], keyStatus);
        if(key > 0)
        {
            KEY_DemoAdcToggle((uint32)key, demo_key[i], keyStatus);
        }
    }
    ret_key = SAL_RET_SUCCESS;

    return ret_key;
}

static void KEY_DemoAdcToggle(uint32 iKeyCode, KEYKeyInfo_t sKeyInfo, int32 (*piKeyStatus)[MAX_ADC_BTN])
{
    (void)(*piKeyStatus);
    (void)sKeyInfo;
#if ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )
    SPU_Test_SSL_With_Key((uint32)iKeyCode);
#endif  //( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )
}

#endif  // ( MCU_BSP_SUPPORT_APP_KEY_DEMO == 1 )


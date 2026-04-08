// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : fwupdate.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : Firmware update
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <app_cfg.h>

#include "bsp.h"
#include "debug.h"
#include "uart.h"
#include "eflash.h"
#include "fwupdate.h"
#include "clock.h"
#include "clock_dev.h"
#include "snor_mio.h"
#include "hsm_manager.h"


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

FWUDInfo_t FWUpdateInfo;

static const uint32   CRC32_TABLE[256] = {
    0x00000000, 0x90910101, 0x91210201, 0x01B00300,
    0x92410401, 0x02D00500, 0x03600600, 0x93F10701,
    0x94810801, 0x04100900, 0x05A00A00, 0x95310B01,
    0x06C00C00, 0x96510D01, 0x97E10E01, 0x07700F00,
    0x99011001, 0x09901100, 0x08201200, 0x98B11301,
    0x0B401400, 0x9BD11501, 0x9A611601, 0x0AF01700,
    0x0D801800, 0x9D111901, 0x9CA11A01, 0x0C301B00,
    0x9FC11C01, 0x0F501D00, 0x0EE01E00, 0x9E711F01,
    0x82012001, 0x12902100, 0x13202200, 0x83B12301,
    0x10402400, 0x80D12501, 0x81612601, 0x11F02700,
    0x16802800, 0x86112901, 0x87A12A01, 0x17302B00,
    0x84C12C01, 0x14502D00, 0x15E02E00, 0x85712F01,
    0x1B003000, 0x8B913101, 0x8A213201, 0x1AB03300,
    0x89413401, 0x19D03500, 0x18603600, 0x88F13701,
    0x8F813801, 0x1F103900, 0x1EA03A00, 0x8E313B01,
    0x1DC03C00, 0x8D513D01, 0x8CE13E01, 0x1C703F00,
    0xB4014001, 0x24904100, 0x25204200, 0xB5B14301,
    0x26404400, 0xB6D14501, 0xB7614601, 0x27F04700,
    0x20804800, 0xB0114901, 0xB1A14A01, 0x21304B00,
    0xB2C14C01, 0x22504D00, 0x23E04E00, 0xB3714F01,
    0x2D005000, 0xBD915101, 0xBC215201, 0x2CB05300,
    0xBF415401, 0x2FD05500, 0x2E605600, 0xBEF15701,
    0xB9815801, 0x29105900, 0x28A05A00, 0xB8315B01,
    0x2BC05C00, 0xBB515D01, 0xBAE15E01, 0x2A705F00,
    0x36006000, 0xA6916101, 0xA7216201, 0x37B06300,
    0xA4416401, 0x34D06500, 0x35606600, 0xA5F16701,
    0xA2816801, 0x32106900, 0x33A06A00, 0xA3316B01,
    0x30C06C00, 0xA0516D01, 0xA1E16E01, 0x31706F00,
    0xAF017001, 0x3F907100, 0x3E207200, 0xAEB17301,
    0x3D407400, 0xADD17501, 0xAC617601, 0x3CF07700,
    0x3B807800, 0xAB117901, 0xAAA17A01, 0x3A307B00,
    0xA9C17C01, 0x39507D00, 0x38E07E00, 0xA8717F01,
    0xD8018001, 0x48908100, 0x49208200, 0xD9B18301,
    0x4A408400, 0xDAD18501, 0xDB618601, 0x4BF08700,
    0x4C808800, 0xDC118901, 0xDDA18A01, 0x4D308B00,
    0xDEC18C01, 0x4E508D00, 0x4FE08E00, 0xDF718F01,
    0x41009000, 0xD1919101, 0xD0219201, 0x40B09300,
    0xD3419401, 0x43D09500, 0x42609600, 0xD2F19701,
    0xD5819801, 0x45109900, 0x44A09A00, 0xD4319B01,
    0x47C09C00, 0xD7519D01, 0xD6E19E01, 0x46709F00,
    0x5A00A000, 0xCA91A101, 0xCB21A201, 0x5BB0A300,
    0xC841A401, 0x58D0A500, 0x5960A600, 0xC9F1A701,
    0xCE81A801, 0x5E10A900, 0x5FA0AA00, 0xCF31AB01,
    0x5CC0AC00, 0xCC51AD01, 0xCDE1AE01, 0x5D70AF00,
    0xC301B001, 0x5390B100, 0x5220B200, 0xC2B1B301,
    0x5140B400, 0xC1D1B501, 0xC061B601, 0x50F0B700,
    0x5780B800, 0xC711B901, 0xC6A1BA01, 0x5630BB00,
    0xC5C1BC01, 0x5550BD00, 0x54E0BE00, 0xC471BF01,
    0x6C00C000, 0xFC91C101, 0xFD21C201, 0x6DB0C300,
    0xFE41C401, 0x6ED0C500, 0x6F60C600, 0xFFF1C701,
    0xF881C801, 0x6810C900, 0x69A0CA00, 0xF931CB01,
    0x6AC0CC00, 0xFA51CD01, 0xFBE1CE01, 0x6B70CF00,
    0xF501D001, 0x6590D100, 0x6420D200, 0xF4B1D301,
    0x6740D400, 0xF7D1D501, 0xF661D601, 0x66F0D700,
    0x6180D800, 0xF111D901, 0xF0A1DA01, 0x6030DB00,
    0xF3C1DC01, 0x6350DD00, 0x62E0DE00, 0xF271DF01,
    0xEE01E001, 0x7E90E100, 0x7F20E200, 0xEFB1E301,
    0x7C40E400, 0xECD1E501, 0xED61E601, 0x7DF0E700,
    0x7A80E800, 0xEA11E901, 0xEBA1EA01, 0x7B30EB00,
    0xE8C1EC01, 0x7850ED00, 0x79E0EE00, 0xE971EF01,
    0x7700F000, 0xE791F101, 0xE621F201, 0x76B0F300,
    0xE541F401, 0x75D0F500, 0x7460F600, 0xE4F1F701,
    0xE381F801, 0x7310F900, 0x72A0FA00, 0xE231FB01,
    0x71C0FC00, 0xE151FD01, 0xE0E1FE01, 0x7070FF00
};

static int32 is_init_snor = 0;
static int32 is_init_eflash = 0;


/**************************************************************************************************
*                                  STATIC FUNCTIONS DECLARATION
**************************************************************************************************/

static int32 FWUD_SetUpdateFlag
(
    uint32                              storage_type
);

static sint32 FWUD_OpenDev
(
    uint8                               ucDev
);

static sint32 FWUD_ReceiveData
(
    uint8                               ucDev,
    uint8 *                             pucBuf,
    uint32                              uiSize
);

static int32 FWUD_SendData
(
    uint8                               ucDev,
    void                                *pBuf,
    uint32                              uiSize
);

static uint32 FWUD_CalcCRC8
(
    uint8                               *base,
    uint32                              length
);

static uint32 FWUD_CalcCRC8In
(
    uint32                              uCRCIN,
    uint8                              *base,
    uint32                              length
);

static int32 FWUD_InitStorage
(
    uint32                              storage_type
);

static int32 FWUD_WriteToStorage
(
    uint32                              addr,
    uint32                              size,
    uint8                               *data_buffer,
    uint32                              storage,
    uint32                              type
);

static int32 FWUD_ReadFromStorage
(
    uint32                              addr,
    uint32                              size,
    uint8                              *data_buffer,
    uint32                              storage,
    uint32                              type
);

static int32 FWUD_CmdWriteHandler
(
    uint8                               ucDev,
    FWUDPacket_t                        *pfwdn_cmd
);

static int32 FWUD_CmdReadyToUpdate
(
    uint8                               ucDev,
    FWUDPacket_t *                      pfwdn_cmd
);


/**************************************************************************************************
*                                        STATIC FUNCTIONS
**************************************************************************************************/

static int32 FWUD_SetUpdateFlag
(
    uint32                              storage_type
)
{
    (void) storage_type;

    return 0;
}

static sint32 FWUD_OpenDev
(
    uint8                               ucDev
)
{
    UartParam_t sUartCfg;
    sint32  iRet;

    iRet    = 0;

    sUartCfg.sCh          = FWUD_UART_CH;
    sUartCfg.sPriority    = GIC_PRIORITY_NO_MEAN;
    sUartCfg.sBaudrate    = FWUD_UART_BAUDRATE;
    sUartCfg.sMode        = UART_POLLING_MODE;
    sUartCfg.sCtsRts      = UART_CTSRTS_OFF;
    sUartCfg.sPortCfg     = 2U;
    sUartCfg.sWordLength  = WORD_LEN_8;
    sUartCfg.sFIFO        = ENABLE_FIFO;
    sUartCfg.s2StopBit    = TWO_STOP_BIT_OFF;
    sUartCfg.sParity      = PARITY_SPACE;
    sUartCfg.sFnCallback  = NULL_PTR;

    if(ucDev == FWUD_DEV_UART)
    {
        iRet = UART_Open(&sUartCfg);
    }
    else
    {
        /* not supported */
        iRet = -1;
    }

    return iRet;

}

void FWUD_Init
(
    void
)
{
    sint32  iRet;

    iRet = FWUD_OpenDev(FWUD_DEV_UART);

    if(iRet != -1)
    {
        mcu_printf("Success to open FWUD UART device\n");
    }
    else
    {
        mcu_printf("Fail to open FWUD UART device\n");
    }

    iRet = HSM_Init();
    if (iRet != HSM_OK) {
        mcu_printf("HSM_Init Fail\n");
    }
}

static sint32 FWUD_ReceiveData
(
    uint8                               ucDev,
    uint8 *                             pucBuf,
    uint32                              uiSize
)
{
    uint32 i;
    sint8 ret;
    uint32 uiRxData;

    ret     = 0;

    if(ucDev == FWUD_DEV_UART)
    {
        for ( i = 0; i < uiSize; i++ )
        {

            uiRxData    = UART_GetData(FWUD_UART_CH, (sint32)10000000, (sint8 *)&ret); /* polling */
            if (ret == -1)
            {
                break;
            }

            pucBuf[i] = (uint8)(uiRxData & 0xFFUL);
        }

        if( i > 0 )
        {
            ret = (sint8) i;
        }
    }
    else
    {
        /* not supported */
    }

    return ret;
}

static int32 FWUD_SendData
(
    uint8                               ucDev,
    void                                *pBuf,
    uint32                              uiSize
)
{
    int32  ret = 0;

    if(ucDev == FWUD_DEV_UART)
    {
	    ret = UART_Write(FWUD_UART_CH, pBuf, uiSize);
    }
    else
    {
        /* not supported */
    }

	return ret;
}


static uint32 FWUD_CalcCRC8
(
    uint8                               *base,
    uint32                              length
)
{
    uint32  crcout = 0;
    uint32  cnt;
    uint8   code;
    uint8   tmp;

    for (cnt = 0; cnt < length; cnt++)
    {
        code = base[cnt];
        tmp = code^crcout;
        crcout = (crcout>>8)^CRC32_TABLE[tmp&0xFF];
    }

    return crcout;
}

static uint32 FWUD_CalcCRC8In
(
    uint32                              uCRCIN,
    uint8                              *base,
    uint32                              length
)
{
	uint32  crcout = uCRCIN;
	uint32  cnt;
	uint8   code;
	uint8   tmp;

	for (cnt = 0; cnt < length; cnt++)
	{
		code = base[cnt];
		tmp = code^crcout;
		crcout = (crcout>>8)^CRC32_TABLE[tmp&0xFF];
	}

	return crcout;
}

static int32 FWUD_InitStorage
(
    uint32                              storage_type
)
{
    int32   ret = 0;

    if (storage_type == FWUD_STOR_TYPE_SNOR)
    {
        if (is_init_snor == 0)
        {
            mcu_printf("Init snor...\n");

            if (!SNOR_MIO_Init())
            {
                mcu_printf("Complete\n");
                is_init_snor = 1;
            }
            else
            {
                mcu_printf("Failed\n");
                ret = -1;
            }
        }
    }
    else if (storage_type == FWUD_STOR_TYPE_EFLASH)
    {
        if (is_init_eflash == 0)
        {
            mcu_printf("Init eflash...\n");
            if (!EFLASH_Init())
            {
                mcu_printf("Complete\n");
                is_init_eflash = 1;
            }
            else
            {
                mcu_printf("Failed\n");
                ret = -1;
            }
        }
    }
    else
    {
        //ERROR
        //Unknown storage type
        ret = -1;
    }

    return ret;
}

static int32 FWUD_WriteToStorage
(
    uint32                              addr,
    uint32                              size,
    uint8                               *data_buffer,
    uint32                              storage,
    uint32                              type
)
{
    int32   ret = -1;

    switch (storage)
    {
        case FWUD_STOR_TYPE_SNOR:
            ret = SNOR_MIO_FWDN_Write(addr, size, data_buffer);
            break;
        case FWUD_STOR_TYPE_EFLASH:
            ret = EFLASH_FWDN_Write(addr, size, data_buffer, type);
            break;
        default:
            //ERROR
            //Unknown storage type
            ret = -1;
            break;
    }

    return ret;
}

static int32 FWUD_ReadFromStorage
(
    uint32                              addr,
    uint32                              size,
    uint8                              *data_buffer,
    uint32                              storage,
    uint32                              type
)
{
    int32   ret = -1;

    switch (storage)
    {
        case FWUD_STOR_TYPE_SNOR:
            ret = SNOR_MIO_FWDN_Read(addr, size, data_buffer);
            break;
        case FWUD_STOR_TYPE_EFLASH:
            ret = EFLASH_FWDN_Read(addr, size, data_buffer, type);
            break;
        default:
            //ERROR
            //Unknown storage type
            ret = -1;
            break;
    }

    return ret;
}

static int32 FWUD_CmdWriteHandler
(
    uint8                               ucDev,
    FWUDPacket_t                        *pfwdn_cmd
)
{
    int32   ret = -1;
    uint8   data_buffer[BUNCH_SIZE];
    uint32  remain_size = 0;
    uint32  bunch_size;
    uint32  request_size = 0;
    uint32  storage_addr;
    uint32  storage_type;
    uint32  calc_crc = 0;
    uint32  pre_calc_crc;
    uint32  received_crc = 0;

    FWUDPacket_t    fwdn_ack = {};

    mcu_printf("%s:%d Handler Start !!\n", __func__, __LINE__);

    if (pfwdn_cmd == NULL)
    {
        //ERROR
        ret = -1;
    }
    else
    {
        storage_type = pfwdn_cmd->param0;
        request_size = pfwdn_cmd->param1;
        storage_addr = pfwdn_cmd->param2;   //offset
        received_crc = pfwdn_cmd->param3;

        remain_size = request_size;

        if(FWUD_InitStorage(storage_type) != 0)
        {
            mcu_printf("%s:%d Failed storage init\n", __func__, __LINE__);
            ret = -1;
        }
        else
        {
            fwdn_ack.cmd_type = FWUD_RSP_ACK;
            fwdn_ack.param0 = pfwdn_cmd->cmd_type;
            mcu_printf("%s:%d Send ACK for CMD\n", __func__, __LINE__);
            FWUD_SendData(ucDev, &fwdn_ack, sizeof(FWUDPacket_t));

            while (remain_size != 0)
            {
                if (remain_size > BUNCH_SIZE)
                {
                    bunch_size = BUNCH_SIZE;
                }
                else
                {
                    bunch_size = remain_size;
                }

                FWUD_ReceiveData(ucDev, (uint8 *)&fwdn_ack, sizeof(FWUDPacket_t));
                received_crc = fwdn_ack.param0;

                FWUD_ReceiveData(ucDev, (uint8 *)data_buffer, bunch_size);

                ret = FWUD_WriteToStorage(storage_addr, bunch_size, data_buffer, storage_type, EF_PFLASH);
                if (ret != 0)
                {
                    break;
                }

                //to calculate crc
                ret = FWUD_ReadFromStorage(storage_addr, bunch_size, data_buffer, storage_type, EF_PFLASH);
                if (ret != 0)
                {
                    break;
                }

                pre_calc_crc = calc_crc;
                calc_crc = FWUD_CalcCRC8In(calc_crc, data_buffer, bunch_size);
                if (received_crc != calc_crc)
                {
                    mcu_printf("%s:%d CRC fail \n", __func__, __LINE__);

                    fwdn_ack.cmd_type = FWUD_RSP_NACK;
                    fwdn_ack.param1 = FWUD_FAIL_CALC_CRC_FILE;
                    fwdn_ack.param2 = calc_crc;

                    calc_crc = pre_calc_crc;    //for retry
                }
                else
                {
                    fwdn_ack.cmd_type = FWUD_RSP_NYET;
                    storage_addr += bunch_size;
                    remain_size -= bunch_size;
                }
                fwdn_ack.param0 = pfwdn_cmd->cmd_type;
                FWUD_SendData(ucDev, &fwdn_ack, sizeof(FWUDPacket_t));

                //mcu_printf("%s:%d Data Transfer Complete !!\n", __func__, __LINE__);
            }

            if (ret == 0)
            {
                if (received_crc == calc_crc)
                {
                    fwdn_ack.cmd_type = FWUD_RSP_ACK;
                    fwdn_ack.param0 = pfwdn_cmd->cmd_type;
                    fwdn_ack.param1 = calc_crc;
                    FWUD_SendData(ucDev, &fwdn_ack, sizeof(FWUDPacket_t));
                    mcu_printf("%s:%d Handler Complete !!\n", __func__, __LINE__);
                    ret = 0;
                }
                else
                {
                    mcu_printf("%s:%d Failed crc (received crc : %x, calc crc : %x\n", __func__, __LINE__, received_crc, calc_crc);
                    fwdn_ack.param0 = FWUD_FAIL_CALC_CRC_FILE;
                    fwdn_ack.param1 = calc_crc;
                    fwdn_ack.cmd_type = FWUD_RSP_NACK;
                    //fwdn_ack.param0 = //resons
                    FWUD_SendData(ucDev, &fwdn_ack, sizeof(FWUDPacket_t));
                    mcu_printf("%s:%d Handler Failed !!\n", __func__, __LINE__);
                    ret = -1;
                }
            }
        }
    }

    return ret;
}

static int32 FWUD_CmdReadyToUpdate
(
    uint8                               ucDev,
    FWUDPacket_t *                      pfwdn_cmd
)
{
    mcu_printf("%s:%d Send ACK for Ready \n", __func__, __LINE__);

    FWUDPacket_t    res;

    res.cmd_type = FWUD_RSP_ACK;
    res.param0 = FWUD_CMD_READY;
    res.param4 = FWUD_CalcCRC8((unsigned char *)&res, sizeof(FWUDPacket_t) - 4);

    FWUD_SendData(ucDev, (void *)&res, sizeof(FWUDPacket_t));

    return 0;
}

void FWUD_Process
(
    void
)
{
    FWUDPacket_t    fwdn_cmd;
    sint32          iReadSize;

    uint8           ucDev;
    uint8           ucExit;
    sint32          iRet;

    ucDev           = FWUD_DEV_UART;
    ucExit          = 0U;

    iRet = FWUD_CmdReadyToUpdate(ucDev, &fwdn_cmd);

    iRet = HSM_SBVerifyStop();
    if (iRet != HSM_OK) {
        mcu_printf("%s:%d Verify Stop Fail \n", __func__, __LINE__);
		return;
    };
    mcu_printf("F/W Verify Stop\n");

    while(ucExit == 0U)
    {
        iReadSize = FWUD_ReceiveData(ucDev, (uint8 *)&fwdn_cmd, sizeof(FWUDPacket_t));

        if(iReadSize > 0)
        {
            switch (fwdn_cmd.cmd_type)
            {
                case FWUD_CMD_WRITE:
                    mcu_printf("%s:%d ***** Received Command : FWUD_WRITE_CMD ***** \n", __func__, __LINE__);

                    iRet = FWUD_CmdWriteHandler(ucDev, &fwdn_cmd);

                    break;

                default:
                    #if 0
                    iRet = -1;
                    mcu_printf("%s:%d Unknown Command\n", __func__, __LINE__);
                    FWUDPacket_t res;
                    res.cmd_type = FWUD_RSP_NACK;
                    res.param0 = fwdn_cmd.cmd_type;
                    res.param1 = FWUD_INVALID_CMD;
                    res.param4 = FWUD_CalcCRC8((unsigned char *)&res, sizeof(FWUDPacket_t) - 4);
                    FWUD_SendData((void *)&res, sizeof(FWUDPacket_t));
                    #endif
                    break;
            }
        }
    }

    (void)iRet;
}

#endif  // ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )


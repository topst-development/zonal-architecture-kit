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

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <app_cfg.h>
#include "bsp.h"
#include "debug.h"
#include "uart.h"
#include "eflash.h"
#include "spi_eccp.h"
#include "fwupdate.h"
#include "clock.h"
#include "clock_dev.h"
#include "snor_mio.h"



/**************************************************************************************************
*                                             DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static int32 is_init_snor = 0;
static int32 is_init_eflash = 0;

static FWUDInfo_t sFWUDInfo;

ECCPUnMarMsg_t sEccpMsg;


/**************************************************************************************************
*                                  STATIC FUNCTIONS DECLARATION
**************************************************************************************************/
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

static int32 FWUD_SetUpdateFlag
(
    uint32                              storage_type
);

static int32 FWUD_SendACK
(
    FWUDCmdType_t                       cmdID,
    FWUDAckIDType_t                     ack,
    FWUDNackType_t                      nackType,
    uint32                              uiReserved
);

static void FWUD_Delay
(
    uint32                              uiUs
);

/**************************************************************************************************
*                                        STATIC FUNCTIONS
**************************************************************************************************/
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

static int32 FWUD_SetUpdateFlag
(
    uint32                              storage_type
)
{
    (void) storage_type;

    return 0;
}

void FWUD_Init
(
    void
)
{
    ECCP_InitSPIManager();

    //FWUD_InitStorage(FWUD_STOR_TYPE_EFLASH);

    sFWUDInfo.iDev = ECCP_Open( ( const uint8 * ) "FWUD\0\0\0", ECCP_UPDATE_APP );

    if( sFWUDInfo.iDev != NULL_PTR )
    {
        mcu_printf("Success to open ECCP-FWUD device\n");

        sFWUDInfo.iCurrentCmd.ciStatus      = FWUD_STATUS_READY;
        sFWUDInfo.iCurrentCmd.ciCmd         = FWUD_CMD_UPDATE_START;
        sFWUDInfo.iCurrentCmd.ciFwDataCnt   = 0UL;
        sFWUDInfo.iCurrentCmd.ciError       = ECCP_SUCCESS;
    }
    else
    {
        mcu_printf("Fail to open ECCP-FWUD device\n");
    }
}

static int32 FWUD_SendACK
(
    FWUDCmdType_t                       cmdID,
    FWUDAckIDType_t                     ack,
    FWUDNackType_t                      nackType,
    uint32                              uiReserved
)
{
    int32 ret;
    uint8 ucData[ 2 ];

    ucData[ 0 ] = ( uint8 ) ack;
    ucData[ 1 ] = ( uint8 ) nackType;

    mcu_printf("\n FWUD_SendACK - CMD : 0x%02x, ACK : 0x%02x, NACK Type : 0x%02x \n", cmdID, ack, nackType);

    ret = ECCP_Send( sFWUDInfo.iDev, ECCP_A72_FRAME, ( uint16 ) cmdID, ( const uint8 * ) ucData, ( uint32 ) 2UL);

    return ret;
}

static void FWUD_Delay
(
    uint32                              uiUs
)
{
    uint32  i;
    uiUs = uiUs * 500;

    for (i = 0; i < uiUs; i++)
    {
         asm("nop");
    }
}


void FWUD_Process
(
    void
)
{
    uint16 usWriteCnt;
    uint32 uiStartAddr = 0UL;

    FWUDAckIDType_t ack = FWUD_ACK;
    FWUDNackType_t  nackType = FWUD_NACK_NO;

    uint8 ucExit = 0U;
    uint16 usCmd1;

    ECCPMsg_t sChkRxMsg;

    while(ucExit == 0U)
    {
        if(sFWUDInfo.iCurrentCmd.ciStatus == FWUD_STATUS_READY)
        {
            switch(sFWUDInfo.iCurrentCmd.ciCmd)
            {
                /******************* Main F/W ************************/
                case FWUD_CMD_UPDATE_START: /* AP -> VCP */
                    {
                        mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_START\n");

                        ack = FWUD_ACK;
                        nackType = FWUD_NACK_NO;

                        FWUD_SendACK(FWUD_CMD_UPDATE_READY, ack, nackType, 0);
                    }
                    break;

                /******************* Updater F/W *********************/
                case FWUD_CMD_UPDATE_READY: /* VCP -> AP */
                    break;

                case FWUD_CMD_UPDATE_FW_START: /* AP -> VCP */
                    {
                        /* Check address and size
                        * data[3:0]: Image start address
                        * data[7:4]: Image partition size
                        * data[11:8]: Image total size */

                        uint32 uiImgSectorSize;
                        uint32 uiImgTotalSize;
                        uint32 uiImgStartAddr;

                        mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_FW_START\n");

                        uiImgStartAddr  = (sEccpMsg.msgPtr[0] << 24) | (sEccpMsg.msgPtr[1] << 16) | (sEccpMsg.msgPtr[2] << 8) | (sEccpMsg.msgPtr[3]);
                        uiImgSectorSize = (sEccpMsg.msgPtr[4] << 24) | (sEccpMsg.msgPtr[5] << 16) | (sEccpMsg.msgPtr[6] << 8) | (sEccpMsg.msgPtr[7]);
                        uiImgTotalSize  = (sEccpMsg.msgPtr[8] << 24) | (sEccpMsg.msgPtr[9] << 16) | (sEccpMsg.msgPtr[10] << 8) | (sEccpMsg.msgPtr[11]);

                        mcu_printf("\nuiImgStartAddr : 0x%08x, uiImgSectorSize : 0x%08x, uiImgTotalSize : 0x%08x\n", uiImgStartAddr, uiImgSectorSize, uiImgTotalSize);

                        if((uiImgStartAddr % FWUD_FLASH_SECTOR_SIZE)
                            || (uiImgSectorSize % FWUD_FLASH_SECTOR_SIZE)
                            || (uiImgTotalSize > FWUD_MAX_PFLASH_SIZE))
                        {
                            ack = FWUD_NACK;
                            nackType = FWUD_NACK_FLASH_ACCESS_FAIL;
                        }
                        else
                        {
                            uint32 uiRet;

                            uiStartAddr = uiImgStartAddr;

                            //uiRet = EFLASH_Erase(uiStartAddr, uiImgTotalSize, EF_PFLASH);

                            (void) uiRet;

                            ack = FWUD_ACK;
                            nackType = FWUD_NACK_NO;
                        }

                        FWUD_SendACK(FWUD_CMD_UPDATE_FW_READY, ack, nackType, 0); //R5->A72
                    }
                    break;

                case FWUD_CMD_UPDATE_FW_READY: /* VCP -> AP */
                    break;

                case FWUD_CMD_UPDATE_FW_SEND: /* AP -> VCP */
                    {
                        /* Check address and size
                        * data[3:0]: Write address(current image address)
                        * data[5:4]: Current command count
                        * data[7:6]: Total command count
                        * data[9:8]: Data size
                        * data[11:10]: Data CRC
                        * data[267~12]: Data */

                        uint32  uiWriteAddr;
                        uint16  usCurentCmdCnt;
                        uint16  usTotalCmdCnt;
                        uint16  usDataSize;
                        uint16  usCrc1, usCrc2;
                        int32   ret;

                        mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_FW_SEND\n");

                        uiWriteAddr     = (sEccpMsg.msgPtr[0] << 24) | (sEccpMsg.msgPtr[1] << 16) | (sEccpMsg.msgPtr[2] << 8) | (sEccpMsg.msgPtr[3]);

                        usCurentCmdCnt  = (sEccpMsg.msgPtr[4] << 8) | (sEccpMsg.msgPtr[5]);
                        usTotalCmdCnt   = (sEccpMsg.msgPtr[6] << 8) | (sEccpMsg.msgPtr[7]);

                        usDataSize      = (sEccpMsg.msgPtr[8] << 8) | (sEccpMsg.msgPtr[9]);
                        usCrc1          = (sEccpMsg.msgPtr[10] << 8) | (sEccpMsg.msgPtr[11]);

                        //mcu_printf("uiWriteAddr : 0x%08x\n", uiWriteAddr);

                        //mcu_printf("usCurentCmdCnt : %d, usTotalCmdCnt : %d\n", usCurentCmdCnt, usTotalCmdCnt);

                        //mcu_printf("usDataSize : %d, usCrc1 : %d\n", usDataSize, usCrc1);

                        ack = FWUD_ACK;
                        nackType = FWUD_NACK_NO;


                        /* Continuity check */
                        if (usCurentCmdCnt == 1) {
                            mcu_printf("Start Data Program\n");
                            usWriteCnt = usCurentCmdCnt;
                        }
                        else if (usCurentCmdCnt != (usWriteCnt + 1)) {
                            mcu_printf("Data is not continuous\n");
                            ack = FWUD_NACK;
                            nackType = FWUD_NACK_COUNT_ERR;
                            break;
                        }
                        else {
                            usWriteCnt++;
                        }

                        /* Address check */
                        if (uiWriteAddr != (uiStartAddr + (MAX_FW_BUF_SIZE * (usWriteCnt-1)))) {
                            mcu_printf("Address is not continuous\n");
                            ack = FWUD_NACK;
                            nackType = FWUD_NACK_COUNT_ERR;
                            FWUD_SendACK(FWUD_CMD_UPDATE_FW_SEND_ACK, ack, nackType, sFWUDInfo.iCurrentCmd.ciFwDataCnt);
                            break;
                        }


                        /* Program Data */
                        if(uiWriteAddr < 0x00200000) /* Eflash(2M) : 0x01000000 ~ 0x011FFFFF */
                        {
                            ret = FWUD_InitStorage(FWUD_STOR_TYPE_EFLASH);
                            if(ret != 0) {
                                ack = FWUD_NACK;
                                nackType = FWUD_NACK_FLASH_INIT_FAIL;
                            } else {
                                FWUD_WriteToStorage(uiWriteAddr, usDataSize, &(sEccpMsg.msgPtr[12]), FWUD_STOR_TYPE_EFLASH, EF_PFLASH);
                            }
                        }
                        else if(uiWriteAddr < 0x00400000) /* S-NOR(2M) : 0x01200000 ~ 0x013FFFFF */
                        {
                            ret = FWUD_InitStorage(FWUD_STOR_TYPE_SNOR);
                            if(ret != 0) {
                                ack = FWUD_NACK;
                                nackType = FWUD_NACK_FLASH_INIT_FAIL;
                            } else {
                                FWUD_WriteToStorage(uiWriteAddr, usDataSize, &(sEccpMsg.msgPtr[12]), FWUD_STOR_TYPE_SNOR, 0);
                                SNOR_MIO_AutoRead(1);
                            }
                        }
                        else
                        {
                            mcu_printf("Unexpected address\n");
                        }


                        /* Compare with written and recieved data */
                        usCrc2 = ECCP_CalcCrc16((unsigned char *)(0x01000000 + uiWriteAddr), usDataSize);

                        if (usCrc1 != usCrc2) {
                            mcu_printf("CRC failure!!! Received CRC : (0x%08X) / Calculate CRC : (0x%08X) \n", usCrc1, usCrc2);

                            ack = FWUD_NACK;
                            nackType = FWUD_NACK_CRC_ERR;
                        }

                        if ((usTotalCmdCnt == usWriteCnt) && (ack == FWUD_ACK)) {
                            mcu_printf("Last Data Program Success\n");
                        }

                        //FWUD_Delay(1000);
                        FWUD_SendACK(FWUD_CMD_UPDATE_FW_SEND_ACK, ack, nackType, sFWUDInfo.iCurrentCmd.ciFwDataCnt);
                    }
                    break;

                case FWUD_CMD_UPDATE_FW_SEND_ACK: /* VCP -> AP */
                    break;

                case FWUD_CMD_UPDATE_FW_DONE: /* AP -> VCP */
                    mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_FW_DONE\n");

                    ack = FWUD_ACK;
                    nackType = FWUD_NACK_NO;

                    FWUD_SendACK(FWUD_CMD_UPDATE_FW_COMPLETE, FWUD_ACK, FWUD_NACK_NO, 0); //R5->A72
                    break;

                case FWUD_CMD_UPDATE_FW_COMPLETE: /* VCP -> AP */
                    break;

                case FWUD_CMD_UPDATE_DONE: /* AP -> VCP */
                    mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_DONE\n");

                    ack = FWUD_ACK;
                    nackType = FWUD_NACK_NO;

                    FWUD_SendACK(FWUD_CMD_UPDATE_COMPLETE, FWUD_ACK, FWUD_NACK_NO, 0); //R5->A72

                    ucExit = 1;

                    break;

                case FWUD_CMD_UPDATE_COMPLETE: /* VCP -> AP */
                    break;

                default:
                    break;
            }

            sFWUDInfo.iCurrentCmd.ciStatus = FWUD_STATUS_IDLE;
        }

        while(1)
        {
            SAL_MemSet(&sChkRxMsg, 0x00, sizeof(ECCPMsg_t));
            SAL_MemSet(&sEccpMsg, 0x00, sizeof(ECCPUnMarMsg_t));


            ECCP_CheckRx(&sChkRxMsg); /* check rx message */

            usCmd1 = (sChkRxMsg.msgPtr[CMD1_H_IDX] << 8) | sChkRxMsg.msgPtr[CMD1_L_IDX];

            if( usCmd1 == CMD1_ID_FWUD )
            {
                sFWUDInfo.iCurrentCmd.ciCmd = (FWUDCmdType_t) ((sChkRxMsg.msgPtr[CMD2_H_IDX] << 8) | sChkRxMsg.msgPtr[CMD2_L_IDX]);

                mcu_printf("FWUD CMD 2: 0x%02x \n", sFWUDInfo.iCurrentCmd.ciCmd);

                sFWUDInfo.iCurrentCmd.ciStatus = FWUD_STATUS_READY;
                sFWUDInfo.iCurrentCmd.ciError = ECCP_SUCCESS;

                break;
            }
            else
            {
                mcu_printf("Unknown CMD 1 : 0x%02x \n", usCmd1);
            }
        }

    }
}


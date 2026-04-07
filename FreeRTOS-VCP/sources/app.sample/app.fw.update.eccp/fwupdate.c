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
#include "sal_internal.h"
#include "debug.h"
#include "uart.h"
#include "eflash.h"
#include "pmu_reg.h"
#include "spi_eccp.h"
#include "fwupdate.h"
#include "wdt.h"
#include "clock.h"
#include "clock_dev.h"

#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "hsm_manager.h"


/***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static FWUDDev_t    gsFWUDDevHandle     = { NULL_PTR };


/**************************************************************************************************
*                                  STATIC FUNCTIONS DECLARATION
**************************************************************************************************/

static void FWUD_CopyAndJump
(
    void
);

static void FWUpdateTask
(
    void                                *pArg
);

/**************************************************************************************************
*                                        STATIC FUNCTIONS
**************************************************************************************************/


/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

static void FWUD_CopyAndJump
(
    void
)
{
    uint32 uiSrcAddr;

    uint32 uiAlignSize;
    uint32 uiAlignedAddr;

    uint32 uiTempAddr;
    uint32 uiImage_size;

#if 0
    /* HSM Init */
    if (HSM_Init() != HSM_OK) {
        mcu_printf("%s, HSM Init failed \n", __func__ );
        goto out;
    }
#endif

    /* disable interrupt */
    ( void ) GIC_IntSrcDis( ( uint32 ) GIC_TIMER_0 + SAL_OS_TIMER_ID );
    ( void ) SAL_CoreCriticalEnter();

    /* disable watchdog timer */
    WDT_Stop();

    /* disable CAN */
    (void) CAN_Deinit();

    SAL_MemCopy(&uiSrcAddr, (uint32 *)(REMAP_MEMORY_ADDR + FWUD_MCU_FW_BODY_SIZE_OFFSET), sizeof(uint32));

    uiAlignSize = uiSrcAddr % FWUD_ADDR_ALIGN_SIZE;

    if( uiAlignSize != 0 )
    {
        uiAlignSize = FWUD_ADDR_ALIGN_SIZE - uiAlignSize;
    }

    #ifdef SECURE_UPDATE
    uiAlignedAddr = REMAP_MEMORY_ADDR + FWUD_MCU_FW_START_OFFSET + uiSrcAddr + uiAlignSize;
    #else
    uiAlignedAddr = REMAP_MEMORY_ADDR + FWUD_MCU_FW_START_OFFSET + uiSrcAddr + CRT_SIZE + uiAlignSize;
    #endif

    mcu_printf( "%s, Copy F/W upgrade code from 0x%08x \n", __func__, uiAlignedAddr );
    mcu_printf( "%s, Jump to F/W upgrade code address : 0x00000200UL\n", __func__ );


    /* Copy updater F/W from temporary address to SRAM entry address */
    {
        register uint32 *   uiTempSrc   = ( uint32 * ) uiAlignedAddr;
        register uint32 *   uiTempDst   = ( uint32 * ) FWUD_UPDATER_FW_START_ADDR;
        register uint32     uiCnt       = 0;
        register uint32     uiTempSize  = 0;

        uiTempAddr = ((uint32)uiTempSrc) + 0x104UL;
        uiImage_size = *( volatile uint32 * ) ( uiTempAddr );

#if 0
        if (HSM_VerifyFw(((uint32)uiTempSrc), FWUD_UPDATER_HEADER_SIZE,
            (((uint32)uiTempSrc) + FWUD_UPDATER_HEADER_SIZE), uiImage_size, FWUD_UPDATER_ID) == HSM_OK)
        {
            mcu_printf("%s, Verify success F/W upgrade code \n", __func__ );
        }
        else
        {
            mcu_printf("%s, Verify fail F/W upgrade code \n", __func__ );
            goto out;
        }
#endif

        uiTempSize = FWUD_UPDATER_FW_SIZE - FWUD_UPDATER_HEADER_SIZE;

        uiTempSrc += (FWUD_UPDATER_HEADER_SIZE / 4);

        for( uiCnt = 0; uiCnt < ( uiTempSize / 4 ); uiCnt++ )
        {
            *uiTempDst++ = *uiTempSrc++;
        }

        FWUD_JUMP_TO_ADDR; /* jump & execute r5_sub_fw.rom */
    }
out:
    while( TRUE ) /* never reach */
    {
        BSP_NOP_DELAY();
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

    ret = ECCP_Send( gsFWUDDevHandle.dDev, ECCP_A72_FRAME, ( uint16 ) cmdID, ( const uint8 * ) ucData, ( uint32 ) 2UL);

    return ret;
}

static void FWUpdateTask
(
    void                                *pArg
)
{
#if 0
    uint8   ucSrc;
    uint16  usCmd;
    uint8   ucData[ ECCP_MAX_PACKET ];
    int32   siLen;

    FWUDAckIDType_t ack = FWUD_ACK;
    FWUDNackType_t  nackType = FWUD_NACK_NO;

    while( TRUE )
    {
        if ( gsFWUDDevHandle.dDev != NULL )
        {
            ( void ) SAL_MemSet( ucData, 0x00, ECCP_MAX_PACKET );

            siLen = ECCP_Recv( gsFWUDDevHandle.dDev, &ucSrc, &usCmd, ucData );

            if( siLen >= ECCP_SUCCESS )
            {
                mcu_printf( "%s Recived Src [0x%02X] Cmd[0x%04X]\n", __func__, ucSrc, usCmd );

                switch (usCmd)
                {
                    /******************* Main F/W ************************/
                    case FWUD_CMD_UPDATE_START: /* AP -> VCP */
                        {
                            mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_START\n");
                            ECCP_Dump("FWUpdateTask", ucData, (uint32) siLen);

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
                            mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_FW_START\n");
                            ECCP_Dump("FWUpdateTask", ucData, (uint32) siLen);

                            ack = FWUD_ACK;
                            nackType = FWUD_NACK_NO;

                            FWUD_SendACK(FWUD_CMD_UPDATE_FW_READY, ack, nackType, 0); //R5->A72
                        }
                        break;

                    case FWUD_CMD_UPDATE_FW_READY: /* VCP -> AP */
                        break;

                    case FWUD_CMD_UPDATE_FW_SEND: /* AP -> VCP */
                        {
                            mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_FW_SEND\n");
                            ECCP_Dump("FWUpdateTask", ucData, (uint32) siLen);

                            ack = FWUD_ACK;
                            nackType = FWUD_NACK_NO;

                            FWUD_SendACK(FWUD_CMD_UPDATE_FW_SEND_ACK, ack, nackType, 0);
                        }
                        break;

                    case FWUD_CMD_UPDATE_FW_SEND_ACK: /* VCP -> AP */
                        break;

                    case FWUD_CMD_UPDATE_FW_DONE: /* AP -> VCP */
                        {
                            mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_FW_DONE\n");
                            ECCP_Dump("FWUpdateTask", ucData, (uint32) siLen);

                            ack = FWUD_ACK;
                            nackType = FWUD_NACK_NO;

                            FWUD_SendACK(FWUD_CMD_UPDATE_FW_COMPLETE, FWUD_ACK, FWUD_NACK_NO, 0); //R5->A72
                        }
                        break;

                    case FWUD_CMD_UPDATE_FW_COMPLETE: /* VCP -> AP */
                        break;

                    case FWUD_CMD_UPDATE_DONE: /* AP -> VCP */
                        {
                            mcu_printf("\n[CMD] : FWUD_CMD_UPDATE_DONE\n");
                            ECCP_Dump("FWUpdateTask", ucData, (uint32) siLen);

                            ack = FWUD_ACK;
                            nackType = FWUD_NACK_NO;

                            FWUD_SendACK(FWUD_CMD_UPDATE_COMPLETE, FWUD_ACK, FWUD_NACK_NO, 0); //R5->A72
                        }

                        break;

                    case FWUD_CMD_UPDATE_COMPLETE: /* VCP -> AP */
                        break;

                    default:
                        break;
                }
            }
            else
            {
                mcu_printf( "%s ECCP_Recv Fail %d\n", __func__, siLen );
            }
        }
        else
        {
            mcu_printf( "%s ECCP device is NULL\n", __func__ );
        }

        ( void ) SAL_TaskSleep( 100 );
    }
#else
    uint8   ucSrc;
    uint16  usCmd;
    uint8   ucData[ ECCP_MAX_PACKET ];
    int32   siLen;

    while( TRUE )
    {
        if ( gsFWUDDevHandle.dDev != NULL )
        {
            ( void ) SAL_MemSet( ucData, 0x00, ECCP_MAX_PACKET );

            siLen = ECCP_Recv( gsFWUDDevHandle.dDev, &ucSrc, &usCmd, ucData );

            if( siLen >= ECCP_SUCCESS )
            {
                mcu_printf( "%s Recived Src [0x%02X] Cmd[0x%04X]\n", __func__, ucSrc, usCmd );

                FWUD_Start( usCmd );
            }
            else
            {
                mcu_printf( "%s ECCP_Recv Fail %d\n", __func__, siLen );
            }
        }
        else
        {
            mcu_printf( "%s ECCP device is NULL\n", __func__ );
        }

        ( void ) SAL_TaskSleep( 100 );
    }
#endif
}

void FWUD_Start
(
    uint16                              usCmd
)
{
    if( usCmd == FWUD_CMD_UPDATE_START )
    {
		( void ) HSM_Init();

        FWUD_CopyAndJump();
    }
    else
    {
        mcu_printf( "%s, Invalid command \n", __func__ );
    }
}

void CreateFWUDTask
(
    void
)
{
    uint8   ucDevName[ 7 ];

    static uint32   FWUDTaskID = 0UL;
    static uint32   FWUDTaskStk[ FWUD_TASK_STK_SIZE ];

    ucDevName[ 0 ] = 'F';
    ucDevName[ 1 ] = 'W';
    ucDevName[ 2 ] = 'U';
    ucDevName[ 3 ] = 'D';
    ucDevName[ 4 ] = ( uint8 ) 0x00;
    ucDevName[ 5 ] = ( uint8 ) 0x00;
    ucDevName[ 6 ] = ( uint8 ) 0x00;

    gsFWUDDevHandle.dDev = ECCP_Open( ( const uint8 * ) ucDevName, ECCP_UPDATE_APP );

    if( gsFWUDDevHandle.dDev != NULL_PTR )
    {
        (void) SAL_TaskCreate
        (
            &FWUDTaskID,
            (const uint8 *)"Firmware Update Task",
            (SALTaskFunc)&FWUpdateTask,
            &FWUDTaskStk[0],
            FWUD_TASK_STK_SIZE,
            SAL_PRIO_FWUD_APP,
            NULL
        );
    }
    else
    {
        mcu_printf( "%s:%d Fail to create FWUpdateTask \n", __func__, __LINE__ );
    }
}


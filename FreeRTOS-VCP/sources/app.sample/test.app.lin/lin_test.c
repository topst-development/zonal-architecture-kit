// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : lin_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <app_cfg.h>
#include <bsp.h>
#include <uart.h>
#include <lin.h>
#include <lin_test.h>
#include <gpio.h>
#include <debug.h>
#include <gdma.h>
#include <uart_drv.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/


#define LINTEST_CH                      UART_CH3
#define LINTEST_BAUDRATE                19200U
#define LINTEST_PORTSEL                 9U
#define LINTEST_SLP                     GPIO_GPK(14UL);
#define LIN_GIC_PRIORITY                (10U)

#define MSEND_TEST_FID   0x12UL
#define MREAD_TEST_FID   0x21UL
#define SSEND_TEST_FID   0x21UL
#define SREAD_TEST_FID   0x12UL

#define MSEND_TEST_DLC   3UL
#define MREAD_TEST_DLC   4UL
#define SSEND_TEST_DLC   4UL
#define SREAD_TEST_DLC   3UL

typedef enum LinTestStatus
{
    LIN_IDLE_TEST = 0,
    LIN_SEND_TEST,
    LIN_READ_TEST
} LinTestStatus_t;

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

uint8 LIN_TEST_STATUS;

/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static void Lin_AppTask
(
    void *                              pArg
);

void LIN_UserResponse
(
    void *                              pArg,
    uint8                               ucPid
);

uint8 LIN_GetDLC
(
    uint8                               ucFid
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

void LIN_TestUsage(void)
{
    mcu_printf("Lin Command Usage \n");
    mcu_printf("    : lin init mpol \n");
    mcu_printf("    : lin init mintr \n");
    mcu_printf("    : lin init sintr \n");
    mcu_printf("    : lin init mdma \n");
    mcu_printf("    : lin msend \n");
    mcu_printf("    : lin mread \n");
    mcu_printf("\r\n");
}

uint8 LIN_GetDLC
(
    uint8                               ucFid
)
{
    uint8 ret_DLC = 0;

    switch (ucFid)
    {
        case MSEND_TEST_FID : /* MSEND_TEST_FID = SREAD_TEST_FID */
        {
            ret_DLC = MSEND_TEST_DLC;
            break;
        }
        case MREAD_TEST_FID : /* MREAD_TEST_FID = SSEND_TEST_FID */
        {
            ret_DLC = MREAD_TEST_DLC;
            break;
        }
        default             :
        {
            mcu_printf("[ERROR] Check LIN_GetDLC parameter ");
            break;
        }
    }

    return ret_DLC;
}

void LIN_UserResponse
(
    void *                              pArg,
    uint8                               ucPid
)
{
    UartStatus_t *  uartStat;
    uint8           ucFid;
    int             i;

    uint32          uiData;

    //Slave ID 0x21
    uint8   ucData[4] = {0x1A, 0x2B, 0x3C, 0x4D};
    uint8   unCSum = 0xCF;

    uint8   ucSize = sizeof(ucData);

    uartStat = (UartStatus_t *)pArg;
    LIN_CalcFrameId(ucPid, &ucFid);

    if(ucFid == (uint8)SSEND_TEST_FID)
    {
        for (i = 0; i < ucSize; i++)
        {
            UART_PutChar(uartStat->sCh, ucData[i]);

            if((SAL_ReadReg(uartStat->sBase + UART_REG_FR) & UART_FR_RXFE) != UART_FR_RXFE)
            {
                uiData = (SAL_ReadReg(uartStat->sBase + UART_REG_DR) & 0xFFUL );
                uartStat->sRxIntr.iXmitBuf[uartStat->sRxIntr.iHead++] = uiData;
                if(uartStat->sRxIntr.iHead >= uartStat->sRxIntr.iSize)
                {
                    uartStat->sRxIntr.iHead = 0;
                }
            }
        }

        UART_PutChar(uartStat->sCh, unCSum);
    }
}


void LIN_CreateAppTask
(
    void*                        pArg
)
{
    static uint32 uiLinTestAppTaskID;
    static uint32 uiLinTestAppTaskStk[ACFG_TASK_USER_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &uiLinTestAppTaskID,
        (const uint8 *)"Lin_Task",
        (SALTaskFunc)&Lin_AppTask,
        &uiLinTestAppTaskStk[0],
        ACFG_TASK_USER_STK_SIZE,
        SAL_PRIO_LIN_TEST,
        pArg
    );
}

static void Lin_AppTask
(
    void *                              pArg
)
{

    LINConfig_t sLinConfig  = *(LINConfig_t *)pArg;

    sint8 cRet = LIN_OK;
    uint8 ucFid;
    uint8 j;

    uint8 var_ID;
    uint8 ucSendData[3] = {0xAA, 0xBB, 0xCC};

    LINData_t sLINRxData = {0,};
    LINData_t sLINData = {0,};

    while( 1 )
    {

        cRet = LIN_ReadHeader(sLinConfig, &sLINRxData.ldPid);

        if(cRet == LIN_NO_RESPONSE)
        {
            LIN_CalcFrameId(sLINRxData.ldPid, &ucFid);
            sLINRxData.ldDLC = LIN_GetDLC(ucFid);

            cRet = LIN_ReadResponse(sLinConfig, &sLINRxData);

            if(cRet == LIN_OK)
            {
                mcu_printf("[ Response ][ FID : 0x%02x ]", ucFid);

                if(sLINRxData.ldSize > 0 )
                {
                    for(j = 0; j < sLINRxData.ldSize; j++)
                    {
                        mcu_printf("0x%02X ", sLINRxData.ldData[j]);
                    }
                    mcu_printf("\n");
                }

                SAL_MemSet(&sLINRxData, 0, sizeof(LINData_t));
            }else{

                LIN_CalcFrameId(sLINRxData.ldPid, &ucFid);
                mcu_printf("Response Fail [cRet : %d] \n", cRet);

                if(sLINRxData.ldSize > 0 )
                {
                    for(j = 0; j < sLINRxData.ldSize; j++)
                    {
                        mcu_printf("0x%02X ", sLINRxData.ldData[j]);
                    }
                    mcu_printf("\n");
                }
                LIN_FlushRxData(sLinConfig);
            }

        }
        else if(cRet == LIN_HEADER_ERROR)
        {
            mcu_printf("[LIN TASK]FAIL to READ HEADER \n");
            LIN_FlushRxData(sLinConfig);
        }

        switch (LIN_TEST_STATUS)
        {

            case LIN_SEND_TEST :

                var_ID = (uint8)MSEND_TEST_FID;
                sLINData.ldSize = 3;

                SAL_MemCopy(sLINData.ldData, ucSendData, sLINData.ldSize);

                /* Calculate Protected identifier  */
                LIN_CalcProtectId(var_ID, &sLINData.ldPid);

                /* Calculate Checksum */
                if ((var_ID == LIN_DIAGNOSIC_ID1) || (var_ID == LIN_DIAGNOSIC_ID2))
                {
                    sLINData.ldCSum = LIN_CalClassicChecksum(sLINData.ldData, sLINData.ldSize);
                }
                else
                {
                    sLINData.ldCSum = LIN_CalEnhancedChecksum(sLINData.ldPid, sLINData.ldData, sLINData.ldSize);
                }

                LIN_MasterTx(sLinConfig, &sLINData);

                break;

            case LIN_READ_TEST :

                var_ID = (uint8)MREAD_TEST_FID;

                /* Calculate Protected identifier  */
                LIN_CalcProtectId(var_ID, &sLINData.ldPid);
                LIN_MasterRx(sLinConfig, &sLINData);

                break;

            default :

                break;
       }

        ( void ) SAL_TaskSleep( 50 );
    }
}

void LIN_SampleTest(uint8 ucArgc, void * pArgv[])
{
    const uint8 *       pucStr;
    LINStatus_t         uiStatus;
    sint32              ret;
    static LINConfig_t  sLinConfig;

    pucStr = NULL_PTR;
    uiStatus = LIN_OK;

    //Unused Parameter
    (void)ucArgc;

    if(pArgv != NULL_PTR)
    {
        pucStr = (const uint8 *)pArgv[0];

        if(pucStr != NULL_PTR)
        {
            if ((SAL_StrNCmp(pucStr, (const uint8 *)"init", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                if(ucArgc >= 2)
                {
                    pucStr = (const uint8 *)pArgv[1];
                    if ((SAL_StrNCmp(pucStr, (const uint8 *)"mintr", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                    {
                        sLinConfig.cfgMode          = UART_INTR_MODE;
                        sLinConfig.cfgLinMode       = LIN_MASTER;
                        sLinConfig.cfgGicPriority   = LIN_GIC_PRIORITY;
                    }
                    else if ((SAL_StrNCmp(pucStr, (const uint8 *)"sintr", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                    {
                        sLinConfig.cfgMode          = UART_INTR_MODE;
                        sLinConfig.cfgLinMode       = LIN_SLAVE;
                        sLinConfig.cfgGicPriority   = LIN_GIC_PRIORITY;

                        LIN_RegisterCB(LIN_UserResponse);
                    }
                    else if ((SAL_StrNCmp(pucStr, (const uint8 *)"mpol", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                    {
                        sLinConfig.cfgMode          = UART_POLLING_MODE;
                        sLinConfig.cfgLinMode       = LIN_MASTER;
                        sLinConfig.cfgGicPriority   = GIC_PRIORITY_NO_MEAN;
                    }
                    else if ((SAL_StrNCmp(pucStr, (const uint8 *)"mdma", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                    {
                        sLinConfig.cfgMode          = UART_DMA_MODE;
                        sLinConfig.cfgLinMode       = LIN_MASTER;
                        sLinConfig.cfgGicPriority   = LIN_GIC_PRIORITY;
                    }
                    else
                    {
                        mcu_printf("Wrong argument : %s\n", pucStr);
                        LIN_TestUsage();

                        return;
                    }

                    sLinConfig.cfgLinCh      = LINTEST_CH;
                    sLinConfig.cfgBaud       = LINTEST_BAUDRATE;
                    sLinConfig.cfgPortSel    = LINTEST_PORTSEL;
                    sLinConfig.cfgLinSlpPin  = LINTEST_SLP; //TCC701x

                    uiStatus = LIN_Init(sLinConfig);
                    if (uiStatus != LIN_OK)
                    {
                        mcu_printf("Fail LIN_Init");
                    }
                    else
                    {
                        mcu_printf("Create task\n");
                        LIN_CreateAppTask((void*)&sLinConfig);
                    }
                }
                else
                {
                    mcu_printf("Wrong argument : %s\n", pucStr);
                    LIN_TestUsage();
                }

            }
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"msend", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                if(sLinConfig.cfgLinMode == LIN_MASTER)
                {
                    LIN_TEST_STATUS = LIN_SEND_TEST;
                }
            }
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"mread", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                if(sLinConfig.cfgLinMode == LIN_MASTER)
                {
                    LIN_TEST_STATUS = LIN_READ_TEST;
                }
            }
            else
            {
                LIN_TEST_STATUS = LIN_IDLE_TEST;
                mcu_printf("Wrong argument : %s\n", pucStr);
                LIN_TestUsage();
            }
        }
    }
}

#endif // ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )


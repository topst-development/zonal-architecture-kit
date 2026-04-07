// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eccp_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#include "debug.h"
#include "sal_internal.h"
#include "sal_com.h"
#include "spi_eccp.h"
#include "eccp_test.h"

#define ECCP_TEST_STK_SIZE              512

static uint8                            receiving;
static uint8                            sending;
static uint8                            echoing;

static void TEST_EccpRecvTask
(
    void *                              pArg
);

static void TEST_EccpSendTask
(
    void *                              pArg
);

static void TEST_EccpEchoTask
(
    void *                              pArg
);

void TEST_EccpRecv
(
    void
)
{
    static uint32   ECCPRecvTaskID = 0UL;
    static uint32   ECCPRecvTaskStk[ECCP_TEST_STK_SIZE];

    if ((echoing == 0) && (sending == 0))
    {
        if (receiving == 0)
        {
            receiving = 1;
            (void)SAL_TaskCreate
            (
                &ECCPRecvTaskID,
                (const uint8 *)"ECCP Recv Task",
                (SALTaskFunc)&TEST_EccpRecvTask,
                &ECCPRecvTaskStk[0],
                ECCP_TEST_STK_SIZE,
                SAL_PRIO_IPC_PARSER,
                NULL
            );
        }
    }
    else
    {
        mcu_printf("%s Another Test is running\n", __func__);
        SAL_TaskSleep(1000UL);
    }
}

void TEST_EccpSend
(
    void
)
{
    static uint32   ECCPSendTaskID = 0UL;
    static uint32   ECCPSendTaskStk[ECCP_TEST_STK_SIZE];

    if ((echoing == 0) && (sending == 0))
    {
        if (receiving == 0)
        {
            receiving = 1;
            (void)SAL_TaskCreate
            (
                &ECCPSendTaskID,
                (const uint8 *)"ECCP Send Task",
                (SALTaskFunc)&TEST_EccpSendTask,
                &ECCPSendTaskStk[0],
                ECCP_TEST_STK_SIZE,
                SAL_PRIO_IPC_PARSER,
                NULL
            );
        }
    }
    else
    {
        mcu_printf("%s Another Test is running\n", __func__);
        SAL_TaskSleep(1000UL);
    }
}

void TEST_EccpEcho
(
    void
)
{
    static uint32   ECCPEchoTaskID = 0UL;
    static uint32   ECCPEchoTaskStk[ECCP_TEST_STK_SIZE];

    if ((sending != 0) && (receiving != 0))
    {
        if (echoing == 0)
        {
            echoing = 1;
            (void)SAL_TaskCreate
            (
                &ECCPEchoTaskID,
                (const uint8 *)"ECCP Echo Task",
                (SALTaskFunc)&TEST_EccpEchoTask,
                &ECCPEchoTaskStk[0],
                ECCP_TEST_STK_SIZE,
                SAL_PRIO_IPC_PARSER,
                NULL
            );
        }
    }
    else
    {
        mcu_printf("%s Another Test is running\n", __func__);
        SAL_TaskSleep(1000UL);
    }

}

static void TEST_EccpRecvTask
(
    void *                              pArg
)
{
    int32  siLen;
    uint8  ucSrc;
    uint16 uiCmd;
    uint8  pucDat[ECCP_MAX_PACKET];

    mcu_printf("%s Created\n");

    ECCPDev_t * dev = ECCP_Open((const uint8 *) "TEST_SYSMON", ECCP_SYSMON_APP); 

    while (TRUE)
    {
        if (dev != NULL)
        {
            (void) SAL_MemSet(pucDat, 0x00, ECCP_MAX_PACKET);
            siLen = ECCP_Recv(dev, &ucSrc, &uiCmd, pucDat); 

            if (siLen == ECCP_SUCCESS)      // Only Cmd Received
            {
                mcu_printf("%s Recived Src [0x%02X] Cmd[0x%04X]\n", __func__, ucSrc, uiCmd);
                if (ECCP_SUCCESS != ECCP_Send(dev, ucSrc, uiCmd + 1, pucDat, siLen))
                {
                    mcu_printf("%s ECCP_Send Fail\n",__func__);
                }
            }
            else if (siLen > ECCP_SUCCESS)  // Both Cmd and Dat Received
            {
                if (ECCP_SUCCESS != ECCP_Send(dev, ucSrc, uiCmd + 1, pucDat, siLen))
                {
                    mcu_printf("%s ECCP_Send Fail\n",__func__);
                }
                mcu_printf("%s Recived Src [0x%02X] Cmd[0x%04X]\n", __func__, ucSrc, uiCmd);
#ifdef ECCP_DUMP_ENABLE
                ECCP_Dump("TEST_EccpRecvTask", pucDat, (uint32) siLen);
#endif
            }
            else
            {
                mcu_printf("%s ECCP_Recv Fail %d\n", __func__, siLen);
            }
        }
        else
        {
            mcu_printf("%s ECCP_Open NULL\n", __func__);
            SAL_TaskSleep(1000UL);
        }
    }
}

static void TEST_EccpSendTask
(
    void *                              pArg
)
{
    mcu_printf("%s Created\n");

    ECCPDev_t * dev = ECCP_Open((const uint8 *) "TEST_CONTROL", ECCP_CONTROL); 

    while (TRUE)
    {
        if (dev != NULL)
        {
            ECCP_Send(dev, 0, 0x01, 0x01, 0x01);
        }
        else
        {
            mcu_printf("%s ECCP_Open NULL\n", __func__);
            SAL_TaskSleep(1000UL);
        }
    }
}

static void TEST_EccpEchoTask
(
    void *                              pArg
)
{
    mcu_printf("%s Created\n");

    ECCPDev_t * dev = ECCP_Open((const uint8 *) "TEST_CONTROL", ECCP_CONTROL); 

    while (TRUE)
    {
        if (dev != NULL)
        {
        }
        else
        {
            mcu_printf("%s ECCP_Open NULL\n", __func__);
            SAL_TaskSleep(1000UL);
        }
    }
}


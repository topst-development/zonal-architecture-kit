// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : echo_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_SOCKET == 1 )

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "sal_com.h"
#include "echo_test.h"
#ifdef OS_FREERTOS
#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"
#endif

void SOCKTEST_IPInit
(
    const uint8*                        pucMacAddr,
    const uint8*                        pucIpAddr
)
{


    uint8 ucMacAddress[SIZEOFMACADDR] = {0, };
    uint8 ucIpAddress[SIZEOFIPV4ADDR] = {0, };
    uint8 ucNetMask[SIZEOFIPV4ADDR] = {255, 255, 255, 0}; // C Class For Testing.
    uint8 ucGatewayAddress[SIZEOFIPV4ADDR] = {192, 168, 0, 1 };
    uint8 ucDNSServerAddress[SIZEOFIPV4ADDR] = {10, 10, 10, 1};

    mcu_printf("%s mac[%s] ip[%s]\n", __func__, pucMacAddr, pucIpAddr);
    if (SIZEOFMACADDR != sscanf((char *) pucMacAddr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
                                &ucMacAddress[0], &ucMacAddress[1], &ucMacAddress[2], &ucMacAddress[3], &ucMacAddress[4], &ucMacAddress[5]))
    {
        mcu_printf("%s invalid mac address format %s\n", __func__, pucMacAddr);
        return;
    }
    mcu_printf("%s mac %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
                ucMacAddress[0], ucMacAddress[1], ucMacAddress[2], ucMacAddress[3], ucMacAddress[4], ucMacAddress[5]);

    if (SIZEOFIPV4ADDR != sscanf((char *) pucIpAddr, "%3hhd.%3hhd.%3hhd.%3hhd",
                                &ucIpAddress[0], &ucIpAddress[1], &ucIpAddress[2], &ucIpAddress[3]))
    {
        mcu_printf("%s Invalid ip address format %s\n", __func__, pucIpAddr);
        return;
    }
    mcu_printf("%s ip %d.%d.%d.%d\n", __func__, ucIpAddress[0], ucIpAddress[1], ucIpAddress[2], ucIpAddress[3]);

    // For Test. Network Layer checks the gateway address according to net mask
    ucGatewayAddress[0] = ucIpAddress[0];
    ucGatewayAddress[1] = ucIpAddress[1];
    ucGatewayAddress[2] = ucIpAddress[2];
    ucGatewayAddress[3] = 1;

    FreeRTOS_IPInit(ucIpAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMacAddress);

    mcu_printf("%s Success IP Config\n", __func__);
}

void SOCKTEST_EchoSend
(
    const uint8*                        pucIpAddr,
    const uint8*                        pucMsg
)
{
    Socket_t xSocket;
    uint8 ucIpAddress[SIZEOFIPV4ADDR] = {0, };
    struct freertos_sockaddr xRemoteAddress;
    BaseType_t xTotalLengthToSend = 0, xAlreadyTransmitted = 0, xByteSent = 0;
    size_t xLenToSend;
    char* pcRecvBuffer[3];
    BaseType_t lBytesReceived = 0;


    if (SIZEOFIPV4ADDR != sscanf((char *) pucIpAddr, "%3hhd.%3hhd.%3hhd.%3hhd",
                                &ucIpAddress[0], &ucIpAddress[1], &ucIpAddress[2], &ucIpAddress[3]))
    {
        mcu_printf("%s Invalid ip address format %s\n", __func__, pucIpAddr);
        return;
    }
    mcu_printf("%s ip %d.%d.%d.%d\n", __func__, ucIpAddress[0], ucIpAddress[1], ucIpAddress[2], ucIpAddress[3]);

    (void) SAL_StrLength((const int8 *) pucMsg, (SALSize *) &xTotalLengthToSend);
    if (xTotalLengthToSend <= 0)
    {
        mcu_printf("%s Invalid msg %s\n", __func__, pucMsg);
        return;
    }
    mcu_printf("%s msg %s\n", __func__, (char *) pucMsg);

    xRemoteAddress.sin_port = FreeRTOS_htons(7000);
    xRemoteAddress.sin_addr = FreeRTOS_inet_addr_quick(ucIpAddress[0], ucIpAddress[1], ucIpAddress[2], ucIpAddress[3]);

    if (FREERTOS_INVALID_SOCKET != (xSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP)))
    {
        if (0 == FreeRTOS_connect(xSocket, &xRemoteAddress, sizeof(xRemoteAddress)))
        {
            while (xAlreadyTransmitted < xTotalLengthToSend)
            {
                xLenToSend = xTotalLengthToSend - xAlreadyTransmitted;
                xByteSent = FreeRTOS_send(xSocket, &pucMsg[xAlreadyTransmitted], xLenToSend, 0);
                if (xByteSent >= 0)
                {
                    xAlreadyTransmitted += xByteSent;
                }
                else
                {
                    break;
                }
            }

            while (1)
            {
                lBytesReceived =  FreeRTOS_recv(xSocket, pcRecvBuffer, 3, 0);
                if (lBytesReceived > 0)
                {
                    mcu_printf("%s recved %s\n", __func__, pcRecvBuffer);
                    break;
                }
                else if (lBytesReceived == 0)
                {
                    SAL_TaskSleep(100);
                }
                else
                {
                    mcu_printf("%s Fail recving\n", __func__);
                }
            }

            FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
            FreeRTOS_closesocket(xSocket);
        }
        else
        {
            mcu_printf("%s Fail to connect socket\n", __func__);
        }
    }
    else
    {
        mcu_printf("%s Fail to create socket\n", __func__);
    }

}

void SOCKTEST_EchoRecv
(
    void
)
{
    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket;
    socklen_t xSize = sizeof( xClient );
    static const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const BaseType_t xBacklog = 20;
    BaseType_t lBytesReceived = 0, xByteSent = 0;
    static char cRxedData[512];
    Socket_t xSocket;

    if (FREERTOS_INVALID_SOCKET != (xListeningSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP)))
    {
        FreeRTOS_setsockopt( xListeningSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );

        xBindAddress.sin_port = ( uint16_t ) 7000;
        xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );

        FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );

        mcu_printf("%s socket bound now listening\n", __func__);
        FreeRTOS_listen( xListeningSocket, xBacklog );

        if (FREERTOS_INVALID_SOCKET != (xSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize )))
        {
            mcu_printf("%s socket accepted\n", __func__);
            while(1)
            {
                SAL_MemSet(cRxedData, 0x00, 512);
                lBytesReceived = FreeRTOS_recv( xSocket, &cRxedData, 512, 0 );
                if (lBytesReceived > 0)
                {
                    mcu_printf("%s received %s\n", __func__, cRxedData);
                    if (0 < (xByteSent = FreeRTOS_send(xSocket, "OK\0", 3, 0)))
                    {
                        mcu_printf("%s OK msg sent\n", __func__);
                    }
                    else
                    {
                        mcu_printf("%s Fail send OK msg\n", __func__);
                    }
                }
                else if (lBytesReceived == 0)
                {
                    SAL_TaskSleep(100);
                }
                else
                {
                    FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );
                    break;
                }
            }

            FreeRTOS_closesocket( xSocket );
            FreeRTOS_closesocket( xListeningSocket );
        }
        else
        {
            mcu_printf("%s Fail accept socket\n", __func__);
        }
    }
    else
    {
        mcu_printf("%s Fail create socket\n", __func__);
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SOCKET == 1 )


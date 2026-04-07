/*
FreeRTOS+TCP V2.0.11
Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.

Modified by Copyright 2022 Telechips Inc.
Modified date : 2022.12.10
Description : Implemented platform dependent code.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 http://aws.amazon.com/freertos
 http://www.FreeRTOS.org
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "NetworkInterface.h"

#include "NetworkBufferManagement.h"
#include "FreeRTOS_IP_Private.h"

#include "eth.h"
#include "debug.h"
#include "sal_freertos_impl.h"

/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
driver will filter incoming packets and only pass the stack those packets it
considers need processing. */
#if( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0 )
#define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer ) eProcessBuffer
#else
#define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer ) eConsiderFrameForProcessing( ( pucEthernetBuffer ) )
#endif

#define BUFFER_SIZE ( ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING )
#define BUFFER_SIZE_ROUNDED_UP ( ( BUFFER_SIZE + 7 ) & ~0x07UL )
#define GMACRXTASKSTACKSIZE 1024U

static uint8_t                          ucBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ][ BUFFER_SIZE_ROUNDED_UP ];
TaskHandle_t                            xTask = NULL;

static void prvGMACReceiveTask(void *pvPArameters);

static uint8_t * pucGetRXBuffer( size_t uxSize )
{
	TickType_t uxBlockTimeTicks = ( 10U );
	NetworkBufferDescriptor_t * pxBufferDescriptor;
	uint8_t * pucReturn = NULL;

	pxBufferDescriptor = pxGetNetworkBufferWithDescriptor(uxSize, uxBlockTimeTicks );

	if( pxBufferDescriptor != NULL )
	{
		pucReturn = pxBufferDescriptor->pucEthernetBuffer;
	}

	return pucReturn;
}

BaseType_t xNetworkInterfaceInitialise( void )
{
    BaseType_t ret;
	uint32 idx;
	uint8 * pucbuf;

    ETH_Prepare(ETH_MODE_NORMAL);
    ETH_SetMacAddress((uint8 *) ipLOCAL_MAC_ADDRESS);

	for(idx = 0; idx < ETH_RDES_NUM ; idx++)
	{
		pucbuf = pucGetRXBuffer(BUF_SIZE_2KiB);
	    ETH_SetRecvBuff(pucbuf, idx);
	}

    if (pdPASS == (ret = xTaskCreate(prvGMACReceiveTask, "GMACRecvTask", GMACRXTASKSTACKSIZE, (void *) 1, (configMAX_PRIORITIES - 1), &xTask)))
    {
        ETH_RegisterTask(xTask);
    }
    else
    {
        mcu_printf("%s GMACRecvTask Create Fail!!\n", __func__);
        ret = pdFALSE;
    }

    //mcu_printf("%s GMAC Init Success!\n", __func__);
    return ret;
}

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend )
{
    ETH_Send(pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);

    iptraceNETWORK_INTERFACE_TRANSMIT();

    if (xReleaseAfterSend != pdFALSE)
    {
        vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
    }

    return pdPASS;
}

void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    BaseType_t x;

    for( x = 0; x < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; x++ )
    {
        pxNetworkBuffers[ x ].pucEthernetBuffer = &( ucBuffers[ x ][ ipBUFFER_PADDING ] );
        *( ( uint32_t * ) &ucBuffers[ x ][ 0 ] ) = ( uint32_t ) &( pxNetworkBuffers[ x ] );
    }
}

BaseType_t xGetPhyLinkStatus( void )
{
    // 1 : link up , 0 : link down
    return ETH_GetLinkStatus();
}

static void prvGMACReceiveTask(void *pvPArameters)
{
    NetworkBufferDescriptor_t *pxBufferDescriptor;
    ETHRxDescriptor_t *pxETHRxDescriptor;
    xIPStackEvent_t xRxEvent;

    pxBufferDescriptor = NULL;
    pxETHRxDescriptor = NULL;

    //mcu_printf("%s GMAC Resc Task Created!\n", __func__);
    while (1)
    {
    	// To reduce CPU load, link update is performed only once.
    	if(ETH_GetLinkStatus() == 0)
   		{
	        (void) ETH_UpdateLink();
			vTaskDelay(1);
   		}
        //if (ETH_GetLinkStatus() == 1U)
        else
        {
        	ulTaskNotifyTake(pdFALSE, 1);
			pxETHRxDescriptor = ETH_Recv();

            if (pxETHRxDescriptor->xDataLength > 0)
            {
                if (NULL != (pxBufferDescriptor = pxGetNetworkBufferWithDescriptor((size_t) pxETHRxDescriptor->xDataLength, 0)))
				//if (NULL != (pxBufferDescriptor = pxGetNetworkBufferWithDescriptor((size_t) 1536, 0)))
                {
                    pxBufferDescriptor->pucEthernetBuffer = pxETHRxDescriptor->pucEthernetBuffer;
                    pxBufferDescriptor->xDataLength = (size_t) pxETHRxDescriptor->xDataLength;

                    if (eProcessBuffer == eConsiderFrameForProcessing(pxBufferDescriptor->pucEthernetBuffer))
                    {
                        xRxEvent.eEventType = eNetworkRxEvent;
                        xRxEvent.pvData = (void *) pxBufferDescriptor;

                        if (pdFALSE == xSendEventStructToIPTask(&xRxEvent, 0))
                        {
                            vReleaseNetworkBufferAndDescriptor(pxBufferDescriptor);
                            iptraceETHERNET_RX_EVENT_LOST();
                        }
                        else
                        {
                            iptraceNETWORK_INTERFACE_RECEIVE();
                        }
                    }
                    else
                    {
                        vReleaseNetworkBufferAndDescriptor(pxBufferDescriptor);
                    }

					(void)ETH_Rx();
                }
                else
                {
                	iptraceETHERNET_RX_EVENT_LOST();
                }
            }
            else
            {
            	if(pxETHRxDescriptor->overflow == 1)
               		{
               			(void)ETH_Rx();
               		}
                //mcu_printf("%s ETH_Recv fail\n", __func__);
            }
        }

    }

    mcu_printf("%s dead!!\n", __func__);
}

// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eth.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : Ethernet Driver
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_ETH == 1 )

// for GMAC 5.1a (DWC-ETHERNET-QOS)
#include <eth.h>
#include "debug.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"

#if ( RTL_8211E == 1 )
#include <eth_mii_rtl8211e.h>
#else
#include <eth_mii.h>
#endif

//#define ETH_RAW_THP_TEST
// raw packet throughput test (between MAC Layer)
// Call ETH_CreateSendTask(), ETH_CreateRecvTask() in main.c

/*************************************************************************************************/
/*                                      LOCAL VARIABLES                                          */
/*************************************************************************************************/

#define DIRECTION_RX                    (0U)
#define DIRECTION_TX                    (1U)

ETHDev_t                                h_mac;
ETHRxDescriptor_t                       RxDesc;

volatile uint32 *                       rdes_base;
volatile uint32 *                       tdes_base;

uint32 overflow_err = 0;
uint32 overflow_cnt = 0;

uint32 backup_rxbuf_addr[ETH_RDES_NUM];

uint8 initialized = 0;
uint8 setup_flag = 0;

/*
***************************************************************************************************
*                                      FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void delay1us
(
   uint32                               uiUs
);

static void ETH_PhyReset
(
    void
);

static void ETH_SetFilter
(
    void
);

static void ETH_DmaReset
(
    void
);

static void ETH_DmaStartTx
(
    uint32                              uiChan
);

static void ETH_DmaStopTx
(
    uint32                              uiChan
);

static void ETH_DmaStartRx
(
    uint32                              uiChan
);

static void ETH_DmaStopRx
(
    uint32                              uiChan
);

static void ETH_DmaInit
(
    void
);

static void ETH_DmaInterrupt
(
    void
);

static void ETH_Isr
(
    void *                              pArg
);

static void ETH_Tx
(
    uint8 *                             pBuff,
    uint32                              uiLen
);

static uint32 ETH_GetIPVersion
(
    uint32                              uiId
);

static void ETH_SetDefaultInfo
(
    void
);

static void ETH_SetPeriClock
(
    void
);

static void ETH_CoreInit
(
    uint32                              uiMode
);

static void ETH_SetPort
(
    void
);

static void ETH_SetSignalDelay
(
    void
);

static void ETH_SetOpmode
(
    void
);

static void ETH_DescInit
(
    void
);

static void ETH_TxRxEnable
(
    void
);

static void ETH_TxRxDisable
(
    void
);

static uint32 ETH_CheckRxOwn
(
    void
);

static void ETH_PlatformInit
(
    void
);

static void ETH_RecvTask
(
    void *                              pArg
);

/*
***************************************************************************************************
*                                          delay1us
* Function to generate delay.
*
* @param    [In] uiUs  :  time value
* @return   None
* Notes
***************************************************************************************************
*/

static void delay1us
(
   uint32                               uiUs
)
{
   uint32                               uiCnt;
   uint32                               uiSec;

   uiCnt = 0xffUL;
   uiSec = 0;

   uiSec = uiUs * (uint32)500UL;

   for (uiCnt = 0; uiCnt < uiSec; uiCnt++)
   {
       BSP_NOP_DELAY();
   }
}

/*
***************************************************************************************************
*                                          pucGetRXBuffer
* Function to alllocate network buffer.
*
* @param    [In] uxSize  :  buffer size
* @return   None
* Notes
***************************************************************************************************
*/

static uint8_t * pucGetRXBuffer
(
    size_t uxSize
)
{
    TickType_t                          uxBlockTimeTicks = ( 10U );
    NetworkBufferDescriptor_t *         pxBufferDescriptor;
    uint8_t *                           pucReturn = NULL;

    pxBufferDescriptor = pxGetNetworkBufferWithDescriptor(uxSize, uxBlockTimeTicks );

    if( pxBufferDescriptor != NULL )
    {
        pucReturn = pxBufferDescriptor->pucEthernetBuffer;
    }

    return pucReturn;
}

#ifdef ETH_RAW_THP_TEST

/*
***************************************************************************************************
*                                          ETH_SendTask
* Function to send packet for throughput test.
*
* @param    None
* @return   None
* Notes
***************************************************************************************************
*/

static void ETH_SendTask
(
    void *                              pArg
)
{
    uint32                              packet_length = 1536;
    const uint8                         test_buf[1536] =
                                        {
                                            0x00, 0x11, 0x22, 0x33, 0x44, 0x55, // dst addr 6 byte
                                            0x00, 0x12, 0x34, 0x56, 0x78, 0x66, // src addr 6 byte
                                            0x08, 0x06,                         // Ether type 2 byte
                                            0x00, 0x01, 0x08, 0x00, 0xee, 0xee, 0xee, 0xee, //data
                                            0x00, 0x12, 0x34, 0x56, 0xff, 0xff, 0xff, 0xff,
                                            0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0xc0, 0xa8, 0x01, 0x64,
                                        };

    while(1)
    {
        if(ETH_GetLinkStatus() == 0)
        {
            mcu_printf("Link Down.. waiting for link up.. \n");
            (void) ETH_UpdateLink();
            vTaskDelay(1);
        }
        else
        {
            ETH_Send(&test_buf[0], packet_length);
        }
        // There is no delay to send the packet as much as possible.
    }
}

/*
***************************************************************************************************
*                                          ETH_CreateSendTask
* Function to create send task for throughput test.
*
* @param    None
* @return   None
* Notes
***************************************************************************************************
*/

void ETH_CreateSendTask
(
    void
)
{
    uint8                               ucMacAddress[6] = {0, };
    uint32                              idx = 0;
    uint8 *                             pucbuf;
    uint32                              ret = 0;

    uint32                              EthTxTaskId = 0;
    uint32                              EthTxTaskStack[ETH_TASK_STACK_SIZE];

    ucMacAddress[0] = 0x00;
    ucMacAddress[1] = 0x11;
    ucMacAddress[2] = 0x22;
    ucMacAddress[3] = 0x33;
    ucMacAddress[4] = 0x44;
    ucMacAddress[5] = 0x66;

    ETH_Prepare(ETH_MODE_NORMAL);
    ETH_SetMacAddress((uint8 *) &ucMacAddress[0]);

    if((SAL_TaskCreate(&EthTxTaskId,  (const uint8 *)"ETH Send Task", \
                             (SALTaskFunc)&ETH_SendTask, \
                             &(EthTxTaskStack[0]), \
                             (uint32)ETH_TASK_STACK_SIZE, \
                             2, \ // High priority
                             NULL)) != SAL_RET_SUCCESS)
    {
        ETH_D("Create ETH Send Task : FAIL.\n");
    }
}

#endif
/*
***************************************************************************************************
*                                          ETH_PrintPkt
* Function to debug packet data.
*
* @param    [In] pBuf  : Buffer pointer
*           [In] uiLen : Data Length
*           [In] uiDir : TX/RX Direction
* @return   None
* Notes
***************************************************************************************************
*/

void ETH_PrintPkt
(
    uint8 *                             pBuf,
    uint32                              uiLen,
    uint32                              uiDir
)
{
    uint32                              uiCnt;

    mcu_printf("Print Ethernet Packet to debug . . .\n");
    mcu_printf("%s desc index : %d, len = %d byte, buf addr: 0x%x , %s total len : %d byte\n", uiDir == DIRECTION_RX ? "rx" : "tx",
                                        h_mac.dCurRx, uiLen, pBuf, uiDir == DIRECTION_RX ? "rx" : "tx", h_mac.dRxTotalLen);

    for(uiCnt = 0; uiCnt < uiLen; uiCnt++)
    {
        if ((uiCnt % 16) == 0)
        {
            mcu_printf("\n %03x:", uiCnt);
        }

        mcu_printf(" %02x", pBuf[uiCnt]);
    }

    mcu_printf("\n\n");
}

/*
***************************************************************************************************
*                                          ETH_PhyReset
* Function to reset phy device via GPIO control. (hardware reset)
*
* @param    None
* @return   None
* Notes
***************************************************************************************************
*/

static void ETH_PhyReset
(
    void
)
{
    (void)GPIO_Config(ETH_PHY_RST, GPIO_FUNC(0)|GPIO_OUTPUT|GPIO_DS(3));

    (void)GPIO_Set(ETH_PHY_RST, 0);

    delay1us(1000);

    (void)GPIO_Set(ETH_PHY_RST, 1);
}

/*
***************************************************************************************************
*                                          ETH_SetFilter
* Function to set RX packet filter type.
*
* @param    None
* @return   None
* Notes

***************************************************************************************************
*/

static void ETH_SetFilter
(
    void
)
{
    // Packet filtter setting.
    // default : receive all
    (void)SAL_WriteReg(ETH_PKT_RECV_ALL, ETH_MAC_BASE + ETH_MAC_PACKET_FILTER);
}

/*
***************************************************************************************************
*                                          ETH_DmaReset
* Function to reset dma channel.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_DmaReset
(
    void
)
{
    uint32                              uiValue;
    uint32                              uiLimit;

    uiLimit = 100;

    uiValue = SAL_ReadReg(ETH_MAC_BASE + DMA_BUS_MODE);

    /* DMA SW reset */
    uiValue |= DMA_BUS_MODE_SFT_RESET;
    (void)SAL_WriteReg(uiValue, ETH_MAC_BASE + DMA_BUS_MODE);

    while (uiLimit--)
    {
        if (!(SAL_ReadReg(ETH_MAC_BASE + DMA_BUS_MODE) & DMA_BUS_MODE_SFT_RESET))
        {
            break;
        }

        delay1us(10);
    }

    if (uiLimit == 0)
    {
        mcu_printf("GMAC] SW RESET FAIL. \n");
    }
    else
    {
        ETH_D("GMAC] DMA SW RESET DONE. \n");
    }
}

/*
***************************************************************************************************
*                                          ETH_DmaStartTx
* Function to start dma channel for tx.
*
* @param    [In] uiChan : Channel index [0~1]
* @return   None
* Notes

*
***************************************************************************************************
*/

static void ETH_DmaStartTx
(
    uint32                              uiChan
)
{
    uint32                              uiReg;
    uiReg = 0;

    // Start TX DMA
    uiReg = SAL_ReadReg(ETH_MAC_BASE + DMA_CHAN_TX_CONTROL(uiChan));
    uiReg |= DMA_CONTROL_ST;
    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + DMA_CHAN_TX_CONTROL(uiChan));

    // Interrupt Enable
    uiReg = SAL_ReadReg(ETH_MAC_BASE + DMA_CHAN_INTR_ENA(uiChan));
    uiReg = DMA_CHAN_INTR_DEFAULT_MASK_4_10;
    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + DMA_CHAN_INTR_ENA(uiChan));
}

/*
***************************************************************************************************
*                                          ETH_DmaStopTx
* Function to stop dma channel for tx.
*
* @param    [In] uiChan : Channel index [0~1]
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_DmaStopTx
(
    uint32                              uiChan
)
{
    uint32                              uiReg;

    // Stop TX DMA
    uiReg = SAL_ReadReg(ETH_MAC_BASE + DMA_CHAN_TX_CONTROL(uiChan));
    uiReg &= ~DMA_CONTROL_ST;
    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + DMA_CHAN_TX_CONTROL(uiChan));

    // Interrupt Disable
    uiReg = 0;
    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + DMA_CHAN_INTR_ENA(uiChan));
}

/*
***************************************************************************************************
*                                          ETH_DmaStartRx
* Function to start dma channel for rx.
*
* @param    [In] uiChan : Channel index [0~1]
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_DmaStartRx
(
    uint32                              uiChan
)
{
    uint32                              uiReg;

    // Start RX DMA
    uiReg = SAL_ReadReg(ETH_MAC_BASE + DMA_CHAN_RX_CONTROL(uiChan));
    uiReg |= DMA_CONTROL_SR;
    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + DMA_CHAN_RX_CONTROL(uiChan));
}

/*
***************************************************************************************************
*                                          ETH_DmaStopRx
* Function to stop dma channel for rx.
*
* @param    [In] uiChan : Channel index [0~1]
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_DmaStopRx
(
    uint32                              uiChan
)
{
    uint32                              uiReg;

    // Stop RX DMA
    uiReg = SAL_ReadReg(ETH_MAC_BASE + DMA_CHAN_RX_CONTROL(uiChan));
    uiReg &= ~DMA_CONTROL_SR;
    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + DMA_CHAN_RX_CONTROL(uiChan));
}

/*
***************************************************************************************************
*                                          ETH_DmaInit
* Function to intialize dma channel.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_DmaInit
(
    void
)
{
    uint32                              uiValue;

    uiValue = SAL_ReadReg(ETH_MAC_BASE + DMA_SYS_BUS_MODE);

    /* Set the Fixed burst mode */
    uiValue |= DMA_SYS_BUS_FB;

    /* Mixed Burst has no effect when fb is set */
    uiValue |= DMA_SYS_BUS_MB;

    uiValue |= DMA_SYS_BUS_AAL;

    (void)SAL_WriteReg(uiValue, ETH_MAC_BASE + DMA_SYS_BUS_MODE);

    uiValue = SAL_ReadReg(ETH_MAC_BASE + DMA_BUS_MODE);
}

/*
***************************************************************************************************
*                                          ETH_DmaInterrupt
* Function to process dma interrupt.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_DmaInterrupt
(
    void
)
{
    uint32                              uiChan;
    uint32                              uiStatus;
#ifdef OS_FREERTOS
    BaseType_t xHigherPriorityTaskWoken;
#endif

    uiChan = 0;
    uiStatus = SAL_ReadReg(ETH_MAC_BASE + DMA_CHAN_STATUS(uiChan));

    ETH_D("intr_status: %08x\n", uiStatus);

    /* ABNORMAL interrupts */
    if ((uiStatus & DMA_CHAN_STATUS_AIS))
    {
        if((uiStatus & DMA_CHAN_STATUS_RBU))
        {
            mcu_printf("[%s] status : %x (DMA_CHAN_STATUS_RBU) \n", __func__, uiStatus);
        }

        if((uiStatus & DMA_CHAN_STATUS_RPS))
        {
            mcu_printf("[%s] status : %x (DMA_CHAN_STATUS_RPS) \n", __func__, uiStatus);
        }

        if((uiStatus & DMA_CHAN_STATUS_RWT))
        {
            mcu_printf("[%s] status : %x (DMA_CHAN_STATUS_RWT) \n", __func__, uiStatus);
        }

        if((uiStatus & DMA_CHAN_STATUS_ETI))
        {
            mcu_printf("[%s] status : %x (DMA_CHAN_STATUS_ETI) \n", __func__, uiStatus);
        }

        if((uiStatus & DMA_CHAN_STATUS_TPS))
        {
            mcu_printf("[%s] status : %x (DMA_CHAN_STATUS_TPS) \n", __func__, uiStatus);
        }

        if((uiStatus & DMA_CHAN_STATUS_FBE))
        {
            mcu_printf("[%s] status : %x (DMA_CHAN_STATUS_FBE) \n", __func__, uiStatus);
        }
    }


    /* TX/RX NORMAL interrupts */
    if((uiStatus & DMA_CHAN_STATUS_NIS))
    {
        if((uiStatus & DMA_CHAN_STATUS_RI))
        {
            ETH_D("--RI interrupt\n");
#ifndef ETH_RAW_THP_TEST
#ifdef OS_FREERTOS
            if(xIPIsNetworkTaskReady() == pdTRUE)
            {
                xHigherPriorityTaskWoken = pdFALSE;
                vTaskNotifyGiveFromISR(h_mac.xTask, &xHigherPriorityTaskWoken);
            }
#endif
#endif
        }

        if((uiStatus & DMA_CHAN_STATUS_TI))
        {
            ETH_D("--TI interrupt\n");
        }
    }

    /* Clear the interrupt by writing a logic 1 to the chanX interrupt
     * status [21-0] expect reserved bits [5-3]
     */
    (void)SAL_WriteReg(0xffff, ETH_MAC_BASE + DMA_CHAN_STATUS(uiChan));
}

/*
***************************************************************************************************
*                                          ETH_Isr
* Function to handle ethernet interrupt.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_Isr
(
    void *                              pArg
)
{

    ETH_D("ETH_Isr\n");
    (void)pArg;

    /* To handle DMA interrupts */
    (void)ETH_DmaInterrupt();
}

/*
***************************************************************************************************
*                                          ETH_DmaStopRx
* Function to update link information from PHY status register.
*
* @param    None
* @return   None
*
***************************************************************************************************
*/

void ETH_UpdateLink
(
    void
)
{
    uint32                              uiNewState;
    uiNewState = 0;

    (void)PHY_ReadStatus();
#if ( RTL_8211E == 0 )
    if(h_mac.dMode > ETH_MODE_NORMAL)
    {
        h_mac.dLinkInfo.lLink = 1;
    }
#endif
    if(h_mac.dLinkInfo.lLink)
    {
        uint32                          uiCtrl;
        uiCtrl = SAL_ReadReg(ETH_MAC_BASE + MAC_CTRL_REG);

        /* Now we make sure that we can be in full duplex mode.
         * If not, we operate in half-duplex mode. */
        if (h_mac.dLinkInfo.lCurDuplex != h_mac.dLinkInfo.lOldDuplex)
        {
            uiNewState = 1;

            if(!h_mac.dLinkInfo.lCurDuplex)
            {
                uiCtrl &= ~h_mac.dLinkInfo.lDuplex;
            }
            else
            {
                uiCtrl |= h_mac.dLinkInfo.lDuplex;
            }

            h_mac.dLinkInfo.lOldDuplex = h_mac.dLinkInfo.lCurDuplex;
        }

        if (h_mac.dLinkInfo.lOldSpeed != h_mac.dLinkInfo.lCurSpeed)
        {
            uiNewState = 1;
            uiCtrl &= ~h_mac.dLinkInfo.lSpeedMask;

            switch(h_mac.dLinkInfo.lCurSpeed)
            {
                case SPEED_1000:
                    uiCtrl |= h_mac.dLinkInfo.lSpeed1000;
                    break;
                case SPEED_100:
                    uiCtrl |= h_mac.dLinkInfo.lSpeed100;
                    break;
                case SPEED_10:
                    uiCtrl |= h_mac.dLinkInfo.lSpeed10;
                    break;
                default:
                    h_mac.dLinkInfo.lCurSpeed = 0;
                    break;
            }

            h_mac.dLinkInfo.lOldSpeed = h_mac.dLinkInfo.lCurSpeed;
        }

        (void)SAL_WriteReg(uiCtrl, ETH_MAC_BASE + MAC_CTRL_REG);

        if (!h_mac.dLinkInfo.lOldlink)
        {
            uiNewState = 1;
            h_mac.dLinkInfo.lOldlink = h_mac.dLinkInfo.lLink;
        }
    }
    else if(h_mac.dLinkInfo.lOldlink)
    {
        uiNewState = 1;
        h_mac.dLinkInfo.lOldlink = 0;
        h_mac.dLinkInfo.lOldSpeed = 0;
        h_mac.dLinkInfo.lOldDuplex = 0;
    }

    if(uiNewState)
    {
        mcu_printf("Ethernet Link %s [%d Mbps %s-Duplex]\n"
                        , (h_mac.dLinkInfo.lOldlink == 1) ? "Up" : "Down"
                        , h_mac.dLinkInfo.lOldSpeed
                        , (h_mac.dLinkInfo.lOldDuplex) == 1 ? "Full" : "Half");
    }

}

/*
***************************************************************************************************
*                                          ETH_GetLinkStatus
* Function to get current link status.
*
* @param    None
* @return   LinkStatus (1 : up, 0 : down)
*
***************************************************************************************************
*/

uint32 ETH_GetLinkStatus
(
    void
)
{
    return h_mac.dLinkInfo.lLink;
}


/*
***************************************************************************************************
*                                          ETH_Rx
* Function to control RX descriptor.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_Rx
(
    void
)
{
    volatile uint32 *                   pRdes_ptr;

    pRdes_ptr = (volatile uint32 *)rdes_base + (h_mac.dCurRx * ETH_DESC_SIZE);

    ETH_D("DMA Rx start, Rdes ptr : %x \n" , pRdes_ptr);

    pRdes_ptr[0] = (uint32)backup_rxbuf_addr[h_mac.dCurRx];
    pRdes_ptr[1] = 0;
    pRdes_ptr[2] = 0;
    pRdes_ptr[3] = 0;
    pRdes_ptr[3] |= RDES3_INT_ON_COMPLETION_EN | RDES3_BUFFER1_VALID_ADDR;
    pRdes_ptr[3] |= RDES3_OWN;

    if(h_mac.dCurRx == (ETH_RDES_NUM -1))
    {
        // Ring tail pointer
        (void)SAL_WriteReg(rdes_base, ETH_MAC_BASE + DMA_CHAN_RX_END_ADDR(0));
    }
    else
    {
        // Ring tail pointer
        (void)SAL_WriteReg(rdes_base + (h_mac.dCurRx * ETH_DESC_SIZE), ETH_MAC_BASE + DMA_CHAN_RX_END_ADDR(0));
    }

    h_mac.dCurRx++;

    if(h_mac.dCurRx >= ETH_RDES_NUM)
    {
        h_mac.dCurRx = h_mac.dCurRx - ETH_RDES_NUM;
    }
}


/*
***************************************************************************************************
*                                          ETH_Tx
* Function to control TX descriptor to process transmit packet.
*
* @param    [In] pBuff : Buffer pointer
*           [In] uiLen : Data Length
* @return
* Notes
*
***************************************************************************************************
*/

static void ETH_Tx
(
    uint8 *                             pBuff,
    uint32                              uiLen
)
{
    volatile uint32 *                   pTdes_ptr;
    uint32                              uiSize;

    tdes_base = rdes_base + ETH_RDES_NUM * ETH_DESC_SIZE;
    pTdes_ptr = (volatile uint32 *)(tdes_base + h_mac.dCurTx * ETH_DESC_SIZE);

    pTdes_ptr[0] = (uint32)&pBuff[0];
    uiSize = uiLen;

    pTdes_ptr[1] = 0x0;
    pTdes_ptr[2] = (uiSize);
    pTdes_ptr[2] |= TDES2_INTERRUPT_ON_COMPLETION; // Interrupt Enabled on Completion
    pTdes_ptr[3] |= TDES3_OWN |  TDES3_FIRST_DESCRIPTOR | TDES3_LAST_DESCRIPTOR | uiSize;

    if(h_mac.dCurTx == (ETH_TDES_NUM -1))
    {
        // Ring tail pointer
        (void)SAL_WriteReg(tdes_base, ETH_MAC_BASE + DMA_CHAN_TX_END_ADDR(0));
    }
    else
    {
        // Ring tail pointer
        (void)SAL_WriteReg(tdes_base + (h_mac.dCurTx * ETH_DESC_SIZE), ETH_MAC_BASE + DMA_CHAN_TX_END_ADDR(0));
    }
#if 0 // Dump TX/RX Descriptors for Debug
    for(int i = 0; i < ETH_TDES_NUM; i++)
    {
        pTdes_ptr = (volatile uint32 *)(tdes_base + i * ETH_DESC_SIZE);
        mcu_printf("TDesc %d : %x %x %x %x\n ",i, pTdes_ptr[0], pTdes_ptr[1],pTdes_ptr[2], pTdes_ptr[3]);
    }
    for(int i = 0; i < ETH_RDES_NUM; i++)
    {
        volatile uint32 * pRdes_ptr;
        pRdes_ptr = (volatile uint32 *)(rdes_base + i * ETH_DESC_SIZE);
        mcu_printf("RDesc %d : %x %x %x %x\n ",i, pRdes_ptr[0], pRdes_ptr[1],pRdes_ptr[2], pRdes_ptr[3]);
    }
#endif
    h_mac.dCurTx++;

    if(h_mac.dCurTx == ETH_TDES_NUM)
    {
        h_mac.dCurTx = 0;
    }

}

/*
***************************************************************************************************
*                                          ETH_GetIPVersion
* Function to get IP version value.
*
* @param    [In] uiId : Value of GMAC Version register value
* @return   Synopsys ID
* Notes
*
***************************************************************************************************
*/

static uint32 ETH_GetIPVersion
(
    uint32                              uiId
)
{
    /* Check Synopsys Id (not available on old chips) */
    uint32                              uiUserId;
    uint32                              uiSynpsId;

    uiUserId = ((uiId & 0x0000ff00) >> 8);
    uiSynpsId = (uiId & 0x000000ff);

    mcu_printf("user ID: 0x%x, Synopsys ID (version): 0x%x\n", uiUserId, uiSynpsId);

    return uiSynpsId;
}

/*
***************************************************************************************************
*                                          ETH_SetDefaultInfo
* Function to set default value of structure.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_SetDefaultInfo
(
    void
)
{
    h_mac.dCurTx                = 0;
    h_mac.dCurRx                = 0;
    h_mac.dMacAddrPtr           = NULL_PTR;
    h_mac.dRxBuff               = NULL_PTR;
    h_mac.dRxBuffSize           = 0;
    h_mac.dRecvCallback         = NULL;

    h_mac.dLinkInfo.lOldDuplex  = 0;
    h_mac.dLinkInfo.lCurDuplex  = 0;

    h_mac.dLinkInfo.lCurSpeed   = 0;
    h_mac.dLinkInfo.lOldSpeed   = 0;

    h_mac.dLinkInfo.lDuplex     = ETH_MAC_CONFIG_DM;
    h_mac.dLinkInfo.lSpeed10    = ETH_MAC_CONFIG_PS;
    h_mac.dLinkInfo.lSpeed100   = ETH_MAC_CONFIG_FES | ETH_MAC_CONFIG_PS;
    h_mac.dLinkInfo.lSpeed1000  = 0;
    h_mac.dLinkInfo.lSpeedMask  = ETH_MAC_CONFIG_FES | ETH_MAC_CONFIG_PS;
    h_mac.dMiiInfo.mAddr        = ETH_MAC_MDIO_ADDR;
    h_mac.dMiiInfo.mData        = ETH_MAC_MDIO_DATA;
    h_mac.dMiiInfo.mAddrShift   = 21;
    h_mac.dMiiInfo.mAddrMask    = GENMASK(25, 21);
    h_mac.dMiiInfo.mRegShift    = 16;
    h_mac.dMiiInfo.mRegMask     = GENMASK(20, 16);
    h_mac.dMiiInfo.mCsrShift    = 8;
    h_mac.dMiiInfo.mCsrMask     = GENMASK(11, 8);
    h_mac.dMiiInfo.mCsr         = CSR_150_250M;
}

/*
***************************************************************************************************
*                                          ETH_SetMacAddress
* Function to set MAC ADDRESS.
*
* @param    [In] pAddr : Pointer of MAC address data buffer
* @return
* Notes
*
***************************************************************************************************
*/

void ETH_SetMacAddress
(
    uint8 *                             pAddr
)
{
    uint8                               default_mac[6] = {0x00, 0x12, 0x34, 0x56, 0x78, 0x90};
    uint32                              uiData;
    uiData = 0;

    if(pAddr == NULL_PTR)
    {
        h_mac.dMacAddrPtr = default_mac;
    }
    else
    {
        h_mac.dMacAddrPtr = pAddr;
    }

    uiData = (h_mac.dMacAddrPtr[5] << 8) | h_mac.dMacAddrPtr[4];

    /* For MAC Addr registers se have to set the Address Enable (AE)
     * bit that has no effect on the High Reg 0 where the bit 31 (MO)
     * is RO.
     */

    uiData |= (0 << GMAC_HI_DCS_SHIFT);
    (void)SAL_WriteReg(uiData | GMAC_HI_REG_AE, ETH_MAC_BASE + ETH_MAC_ADDR_HIGH(0));

    uiData = (h_mac.dMacAddrPtr[3] << 24) | (h_mac.dMacAddrPtr[2] << 16) | (h_mac.dMacAddrPtr[1] << 8) | h_mac.dMacAddrPtr[0];
    (void)SAL_WriteReg(uiData, ETH_MAC_BASE + ETH_MAC_ADDR_LOW(0));
}

/*
***************************************************************************************************
*                                          ETH_SetPeriClock
* Function to set peri clock rate and enable peri clock.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_SetPeriClock
(
    void
)
{
    uint32                              uiPhyInf;

    uiPhyInf = PHY_INF_SEL;

    // Peri SW reset
    (void)CLOCK_SetSwReset(CLOCK_IOBUS_GMAC, 1);
    (void)CLOCK_SetSwReset(CLOCK_IOBUS_GMAC, 0);
    (void)CLOCK_EnableIobus(CLOCK_IOBUS_GMAC, SALEnabled);

    if((uiPhyInf == RMII_MODE))
    {
        (void)CLOCK_SetPeriRate(CLOCK_PERI_GMAC0, RMII_PERICLK_RATE);
    }
    else if(uiPhyInf == MII_MODE)
    {
        (void)CLOCK_SetPeriRate(CLOCK_PERI_GMAC0, MII_PERICLK_RATE);
    }
    else
    {
        (void)CLOCK_SetPeriRate(CLOCK_PERI_GMAC0, RGMII_PERICLK_RATE);
    }

    (void)CLOCK_EnablePeri(CLOCK_PERI_GMAC0);
}

/*
***************************************************************************************************
*                                          ETH_CoreInit
* Function to set MAC configuration.
*
* @param    [In] uiMode : Mode
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_CoreInit
(
    uint32                              uiMode
)
{
    uint32                              uiValue;
    uiValue = ETH_DEFAULT_CORE_INIT;

    if(uiMode == ETH_MODE_MAC_LOOPBACK)
    {
        mcu_printf("Set MAC loopback Mode ! \n");
        uiValue |= ETH_MAC_CONFIG_LM;
    }

    (void)SAL_WriteReg(uiValue, ETH_MAC_BASE + ETH_MAC_CONFIG);
    ETH_D("MAC_CONFIG set value: %08x\n", SAL_ReadReg(ETH_MAC_BASE + ETH_MAC_CONFIG));
}

/*
***************************************************************************************************
*                                          ETH_SetPort
* Function to set HW Port config.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_SetPort
(
    void
)
{
    uint32                              uiPhyInf;
    uint32                              uiReg;

    uiReg = 0;
    uiPhyInf = PHY_INF_SEL;

    // 3.3V power enable
    (void)GPIO_Config(GPIO_GPC(5), GPIO_FUNC(0) | GPIO_OUTPUT);
    (void)GPIO_Set(GPIO_GPC(5), 1);

    (void)GPIO_Config(ETH_MDC   , GPIO_FUNC(1)|GPIO_OUTPUT|GPIO_DS(3));
    (void)GPIO_Config(ETH_MDIO  , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN|GPIO_DS(3));

    if(uiPhyInf == RMII_MODE)
    {
        (void)GPIO_Config(ETH_TXCLK , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN|GPIO_OUTPUT|GPIO_DS(3));
    }
    else
    {
        (void)GPIO_Config(ETH_TXCLK , GPIO_FUNC(1)|GPIO_OUTPUT|GPIO_DS(3));
    }

    (void)GPIO_Config(ETH_TXD0  , GPIO_FUNC(1)|GPIO_OUTPUT|GPIO_DS(3));
    (void)GPIO_Config(ETH_TXD1  , GPIO_FUNC(1)|GPIO_OUTPUT|GPIO_DS(3));

    if((uiPhyInf == RGMII_MODE) || (uiPhyInf == MII_MODE))
    {
        (void)GPIO_Config(ETH_TXD2  , GPIO_FUNC(1)|GPIO_OUTPUT|GPIO_DS(3));
        (void)GPIO_Config(ETH_TXD3  , GPIO_FUNC(1)|GPIO_OUTPUT|GPIO_DS(3));
    }

    (void)GPIO_Config(ETH_TXEN  , GPIO_FUNC(1)|GPIO_OUTPUT|GPIO_DS(3));

    (void)GPIO_Config(ETH_RXCLK , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN);
    (void)GPIO_Config(ETH_RXD0  , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN);
    (void)GPIO_Config(ETH_RXD1  , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN);

    if((uiPhyInf == RGMII_MODE) || (uiPhyInf == MII_MODE))
    {
        (void)GPIO_Config(ETH_RXD2  , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN);
        (void)GPIO_Config(ETH_RXD3  , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN);
    }

    (void)GPIO_Config(ETH_RXDV  , GPIO_FUNC(1)|GPIO_INPUT|GPIO_INPUTBUF_EN);

    (void)ETH_WriteReg_16(0x0, ETH_CFG_BASE + ETH_CFG1_OFFSET_H);
    (void)ETH_WriteReg_16(0x0, ETH_CFG_BASE);
    (void)ETH_WriteReg_16(0x0, ETH_CFG_BASE + ETH_CFG0_OFFSET_H);

    // PHY INF SEL write.
    uiReg |= ETH_CFG1_H_TCO;
    uiReg |= (uiPhyInf & ETH_CFG1_H_PHY_INFSEL_MASK) << ETH_CFG1_H_PHY_INFSEL_SHIFT;
    uiReg |= ETH_CFG1_H_CE;

    (void)ETH_WriteReg_16(uiReg, ETH_CFG_BASE + ETH_CFG1_OFFSET_H);

    (void)ETH_PhyReset();
}

/*
***************************************************************************************************
*                                          ETH_SetSignalDelay
* Function to modify RGMII/RMII/MII signal delay.
*
* @param    None
* @return   None
* Notes
*           delay [value] ns
*
***************************************************************************************************
*/

static void ETH_SetSignalDelay
(
    void
)
{
    uint32                              uiReg;

    // Set Delay : TXC , TXEN
    uiReg = 0;
    uiReg |= (h_mac.dDelayInfo.dTxc << ETH_DLY_TXC_O_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dTxcInv << ETH_DLY_TXC_O_INV_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dTxen << ETH_DLY_TXEN_SHIFT);

    (void)SAL_WriteReg(uiReg, ETH_DLY_BASE + ETH_DLY0_OFFSET);

    // Set Delay : TXD[0:3]
    uiReg = 0;
    uiReg |= (h_mac.dDelayInfo.dTxd0 << ETH_DLY_TXD0_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dTxd1 << ETH_DLY_TXD1_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dTxd2 << ETH_DLY_TXD2_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dTxd3 << ETH_DLY_TXD3_SHIFT);

    (void)SAL_WriteReg(uiReg, ETH_DLY_BASE + ETH_DLY1_OFFSET);

    // Set Delay : RXC, RXDV
    uiReg = 0;
    uiReg |= (h_mac.dDelayInfo.dRxc << ETH_DLY_RXC_I_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dRxcInv << ETH_DLY_RXC_I_INV_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dRxdv << ETH_DLY_RXDV_SHIFT);

    (void)SAL_WriteReg(uiReg, ETH_DLY_BASE + ETH_DLY3_OFFSET);

    // Set Delay : RXD[0:3]
    uiReg = 0;
    uiReg |= (h_mac.dDelayInfo.dRxd0 << ETH_DLY_RXD0_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dRxd1 << ETH_DLY_RXD1_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dRxd2 << ETH_DLY_RXD2_SHIFT);
    uiReg |= (h_mac.dDelayInfo.dRxd3 << ETH_DLY_RXD3_SHIFT);

    (void)SAL_WriteReg(uiReg, ETH_DLY_BASE + ETH_DLY4_OFFSET);
}

/*
***************************************************************************************************
*                                          ETH_SetOpmode
* Function to set operation mode for HW Queue & DMA channel.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_SetOpmode
(
    void
)
{
    uint32                              uiValue;

    // MTL RxQ0 operation mode.
    // Threshold mode. full packet will be transferred
    //uiValue = 0x1F00000;
    uiValue = 0x1F00020; // +SF mode
    (void)SAL_WriteReg(uiValue, ETH_MAC_BASE + ETH_MAC_RXQ0_OP_MODE);

    // MTL TxQ0 operation mode
    // Threshold mode. Full packet will be transferred
    //uiValue = 0xF0078; // TX Threshold : max 512 byte
    uiValue = 0xF000a; // +SF mode
    (void)SAL_WriteReg(uiValue, ETH_MAC_BASE + ETH_MAC_TXQ0_OP_MODE);

    // DMA Channel operation mode
    // Set PBL
    (void)SAL_WriteReg((1 << DMA_BUS_MODE_PBL_SHIFT), ETH_MAC_BASE + DMA_CHAN_CONTROL(0));
    (void)SAL_WriteReg((DEFAULT_DMA_PBL << DMA_BUS_MODE_PBL_SHIFT), ETH_MAC_BASE + DMA_CHAN_TX_CONTROL(0));
    (void)SAL_WriteReg((DEFAULT_DMA_PBL << DMA_BUS_MODE_RPBL_SHIFT), ETH_MAC_BASE + DMA_CHAN_RX_CONTROL(0));

    // RXQ0-2EN ( EN_AV )
    (void)SAL_WriteReg(0x2, ETH_MAC_BASE + ETH_MAC_RXQ_CTRL0);

}

/*
***************************************************************************************************
*                                          ETH_DescInit
* Function to initialize TX/RX descriptors.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_DescInit
(
    void
)
{
    // Set TX/RX Descriptor Base Address
    // RDES
    rdes_base = (volatile uint32 *)MPU_GetDMABaseAddress();
    (void)SAL_WriteReg((unsigned int)rdes_base, ETH_MAC_BASE + DMA_CHAN_RX_BASE_ADDR(0));
    ETH_D("RDES Base Address : %08x\n", rdes_base);

    // TDES
    tdes_base = rdes_base + ETH_RDES_NUM * ETH_DESC_SIZE;
    (void)SAL_WriteReg((uint32)tdes_base, ETH_MAC_BASE + DMA_CHAN_TX_BASE_ADDR(0));
    ETH_D("TDES Base Address : %08x\n", tdes_base);

    // Set Tx/Rx Ring Length
    (void)SAL_WriteReg((ETH_RDES_NUM - 1), ETH_MAC_BASE + DMA_CHAN_RX_RING_LEN(0));
    (void)SAL_WriteReg((ETH_TDES_NUM - 1), ETH_MAC_BASE + DMA_CHAN_TX_RING_LEN(0));

    // Set Tail Address
    (void)SAL_WriteReg(rdes_base + (ETH_DESC_SIZE * ETH_RDES_NUM) , ETH_MAC_BASE + DMA_CHAN_RX_END_ADDR(0));
    ETH_D("RDES Tail Address: %x\n", SAL_ReadReg(ETH_MAC_BASE + DMA_CHAN_RX_END_ADDR(0)));
}

/*
***************************************************************************************************
*                                          ETH_TxRxEnable
* Function to set TX/RX enable bit in MAC configuration register.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_TxRxEnable
(
    void
)
{
    uint32                              uiReg;

    uiReg = SAL_ReadReg(ETH_MAC_BASE + ETH_MAC_CONFIG);
    uiReg |= (ETH_MAC_CONFIG_RE) | (ETH_MAC_CONFIG_TE);

    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + ETH_MAC_CONFIG);
}

/*
***************************************************************************************************
*                                          ETH_TxRxDisable
* Function to clear TX/RX enable bit in MAC configuration register.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_TxRxDisable
(
    void
)
{
    uint32                              uiReg;

    uiReg = SAL_ReadReg(ETH_MAC_BASE + ETH_MAC_CONFIG);
    uiReg &= ~((ETH_MAC_CONFIG_RE) | (ETH_MAC_CONFIG_TE));

    (void)SAL_WriteReg(uiReg, ETH_MAC_BASE + ETH_MAC_CONFIG);
}

/*
***************************************************************************************************
*                                          ETH_CheckRxOwn
* Function to check Rx descriptor OWN bit.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static uint32 ETH_CheckRxOwn
(
    void
)
{
    volatile uint32 *                   pRdes_ptr;
    uint32                              uiRet;

    uiRet = 0;
    pRdes_ptr = (volatile uint32 *)rdes_base + (h_mac.dCurRx * ETH_DESC_SIZE);

    uiRet = (pRdes_ptr[3] & RDES3_OWN);

    if(uiRet == 0)
    {
        h_mac.dRxFrameLen = pRdes_ptr[3] & RDES3_PACKET_SIZE_MASK;
        if((pRdes_ptr[3] & RDES3_OVERFLOW_ERROR) != 0)
        {
            overflow_err = 1;
        }
    }
    else
    {
        h_mac.dRxFrameLen = 0;
    }

    return uiRet;
}

/*
***************************************************************************************************
*                                          ETH_PlatformInit
* Function to initailze platform data.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_PlatformInit
(
    void
)
{
    uint32                              uiReg;

    uiReg = 0;

    (void)ETH_SetPeriClock();
    (void)ETH_SetPort();
    (void)ETH_SetSignalDelay();

    // MAC Version Check
    uiReg = SAL_ReadReg(ETH_MAC_BASE + ETH_MAC_VERSION);
    (void)ETH_GetIPVersion(uiReg);

    // GMAC interrupt
    (void)GIC_IntVectSet((uint32)GIC_GMAC, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_EDGE_RISING, &ETH_Isr, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GMAC);
}

/*
***************************************************************************************************
*                                          ETH_RecvTask
* Function to process received packet. (for test)
* To use the TCP/IP Layer, use the task implemented in NetworkInterface.c.
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void ETH_RecvTask
(
    void *                              pArg
)
{
    (void)pArg;

    ETHRxDescriptor_t *                 pxETHRxDescriptor;
#ifdef ETH_RAW_THP_TEST
    uint32                              diff_time   = 0;
    uint32                              old_rx_len  = 0;
    uint32                              diff_len    = 0;
    uint32                              first_tick  = 0;
    uint32                              cur_tick    = 0;
    uint32                              old_tick    = 0;
    uint32                              cnt_sec     = 0;
#endif

    while (1)
    {
        if(ETH_GetLinkStatus() == 0)
        {
            mcu_printf("Link Down.. waiting for link up.. \n");
            (void) ETH_UpdateLink();
            vTaskDelay(1);
        }
        else
        {
            pxETHRxDescriptor = ETH_Recv();

            if (pxETHRxDescriptor->xDataLength > 0)
            {
#ifdef ETH_RAW_THP_TEST
                if(first_tick == 0)
                {
                    SAL_GetTickCount(&first_tick);
                    old_rx_len = 0;
                    old_tick = first_tick;
                }
                else
                {
                    SAL_GetTickCount(&cur_tick);
                }

                diff_time = cur_tick - old_tick;
                if((diff_time > 1000)) // 1 sec ( 1ms tick * 1000 )
                {
                    old_tick = cur_tick;
                    diff_len = h_mac.dRxTotalLen - old_rx_len;
                    old_rx_len = h_mac.dRxTotalLen;
                    cnt_sec++;

                    mcu_printf("Ethernet throughput (%d s) : %d bps. (overflow err : %d) \n",
                                cnt_sec, (diff_len * 8) , overflow_cnt);
                }
#else
                ETH_PrintPkt((uint8 *)pxETHRxDescriptor->pucEthernetBuffer, pxETHRxDescriptor->xDataLength, DIRECTION_RX);
#endif
                (void)ETH_Rx();
            }
            else
            {
                if(pxETHRxDescriptor->overflow == 1)
                {
                    (void)ETH_Rx();
                }
            }
#ifndef ETH_RAW_THP_TEST
            SAL_TaskSleep(1);
#endif
        }
    }
}

/*
***************************************************************************************************
*                                          ETH_Send
* Function to transmit ethernet packet.
*
* @param    [In] pBuff : Tx data buffer ptr
* @param    [In] uiLen : Tx data length
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_Send
(
    uint8 *                             pBuff,
    uint32                              uiLen
)
{
    if((pBuff != NULL_PTR) && (uiLen != 0))
    {
        ETH_Tx(pBuff, uiLen);
    }
}

/*
***************************************************************************************************
*                                          ETH_Recv
* Function to transmit ethernet packet.
*
* @param    None
* @return   ETHRxDescriptor_t * : Pointer of Descriptor
* Notes
*
***************************************************************************************************
*/

ETHRxDescriptor_t *ETH_Recv
(
    void
)
{
    ETHRxDescriptor_t *                 ret;

    ret = NULL;

    if(ETH_CheckRxOwn() == 0)
    {
        if(h_mac.dRxFrameLen > 0)
        {
            h_mac.dRxTotalLen += h_mac.dRxFrameLen;

            RxDesc.xDataLength = h_mac.dRxFrameLen;
            RxDesc.pucEthernetBuffer = (uint8 *)backup_rxbuf_addr[h_mac.dCurRx];
        }
        else
        {
            RxDesc.xDataLength = 0;
            RxDesc.pucEthernetBuffer = NULL;

            if(overflow_err == 1)
            {
                overflow_cnt++;
                RxDesc.overflow = 1;
                overflow_err = 0;
            }
        }

        //ETH_PrintPkt((uint8 *)RxDesc.pucEthernetBuffer, h_mac.dRxFrameLen, DIRECTION_RX);
    }
    else
    {
        RxDesc.xDataLength = 0;
        RxDesc.pucEthernetBuffer = NULL;
    }

    ret = &RxDesc;

    return ret;
}

/*
***************************************************************************************************
*                                          ETH_SetRecvBuff
* Function to set ptr of recv buffer.
*
* @param    [In] pBuff : Rx data buffer ptr
* @param    [In] uiSize : Rx buffer size
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_SetRecvBuff
(
    uint8 *                             pBuff,
    uint32                              index
)
{
    volatile uint32 * pRdes_ptr = rdes_base + (index * ETH_DESC_SIZE);

    pRdes_ptr[0] = (uint32)pBuff;
    backup_rxbuf_addr[index] = pRdes_ptr[0];
    pRdes_ptr[1] = 0;
    pRdes_ptr[2] = 0;
    pRdes_ptr[3] = 0;
    pRdes_ptr[3] |= RDES3_INT_ON_COMPLETION_EN | RDES3_BUFFER1_VALID_ADDR;
    pRdes_ptr[3] |= RDES3_OWN;

    (void)SAL_WriteReg(pRdes_ptr, ETH_MAC_BASE + DMA_CHAN_RX_END_ADDR(0));
}

/*
***************************************************************************************************
*                                          ETH_RegisterTask
* Function to store task info.
*
* @param    [In] xTask : task handle ptr
* @return   None
* Notes
*
***************************************************************************************************
*/

#ifdef OS_FREERTOS
void ETH_RegisterTask
(
    TaskHandle_t                        xTask
)
{
    h_mac.xTask = xTask;
}
#endif

/*
***************************************************************************************************
*                                          ETH_CreateRecvTask
* Function to create ethernet packet recv task.
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_CreateRecvTask
(
    uint32                              uiMode
)
{
    uint8                               ucMacAddress[6] = {0, };
    uint32                              idx = 0;
    uint8 *                             pucbuf;
#ifdef ETH_RAW_THP_TEST
    uint32                              ret = 0;
#endif
    static uint32                       EthRxTaskId = 0;
    static uint32                       EthRxTaskStack[ETH_TASK_STACK_SIZE];

    ucMacAddress[0] = 0x00;
    ucMacAddress[1] = 0x11;
    ucMacAddress[2] = 0x22;
    ucMacAddress[3] = 0x33;
    ucMacAddress[4] = 0x44;
    ucMacAddress[5] = 0x55;

    ETH_Prepare(uiMode);
    ETH_SetMacAddress((uint8 *) &ucMacAddress[0]);
    xNetworkBuffersInitialise();

    for(idx = 0; idx < ETH_RDES_NUM ; idx++)
    {
        pucbuf = pucGetRXBuffer(BUF_SIZE_2KiB);
        ETH_SetRecvBuff(pucbuf, idx);
    }

#ifdef ETH_RAW_THP_TEST
    if (pdPASS != (ret = xTaskCreate(ETH_RecvTask, "GMACRecvTask", 1024, (void *) 1, (15), NULL)))
    {
        ETH_D("Create ETH Recv Task : FAIL.\n");
    }
#else
    if((SAL_TaskCreate(&EthRxTaskId,  (const uint8 *)"ETH Recv Task", \
                         (SALTaskFunc)&ETH_RecvTask, \
                         &(EthRxTaskStack[0]), \
                         (uint32)ETH_TASK_STACK_SIZE, \
                         SAL_PRIO_ETH_TASK, \
                         NULL)) != SAL_RET_SUCCESS)
    {
        ETH_D("Create ETH Recv Task : FAIL.\n");
    }
#endif
}

/*
***************************************************************************************************
*                                          ETH_SetRecvCallback
* Function to set callback function pointer.
* @param    [In] function : callback function pointer
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_SetRecvCallback
(
    ETHRecvCallback                     function
)
{
    h_mac.dRecvCallback = function;
};

/*
***************************************************************************************************
*                                          ETH_Prepare
* Function to initailize ethernet.
* @param    [In] uiMode : mode
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_Prepare
(
    uint32                              uiMode
)
{
    if(initialized == 0)
    {
        (void)ETH_SetDefaultInfo();

        h_mac.dMode = uiMode;

        (void)ETH_PlatformInit();
        (void)ETH_SetMacAddress(NULL_PTR);
        (void)ETH_DmaReset();

        (void)ETH_DmaInit();
        (void)ETH_CoreInit(uiMode);
        (void)ETH_DescInit();

        (void)ETH_SetOpmode();
        (void)ETH_SetFilter();

        (void)ETH_TxRxEnable();

        (void)ETH_DmaStartTx(0);
        (void)ETH_DmaStartRx(0);

        if(uiMode == ETH_MODE_PHY_LOOPBACK)
        {
            (void)PHY_InitLoopback(&h_mac);
        }
        else
        {
            (void)PHY_Init(&h_mac);
        }

        initialized = 1;
    }
    else
    {
        mcu_printf("Controller already initialized ! Skip HW init.\n");
    }

}

/*
***************************************************************************************************
*                                          ETH_Init
* Function to initailize ethernet and start tasks.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_Init
(
    void
)
{
    ETH_CreateRecvTask(ETH_MODE_NORMAL);
}

#ifdef ETH_CORNER_SAMPLE_TEST

/*
***************************************************************************************************
*                                          ETH_CornerSample_Test
* Function to check result of signal tunning test.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

void ETH_CornerSample_Test
(
    void
)
{
	uint8                               ucMacAddress[6] = {0, };
	uint32                              idx = 0;
	uint8 *                             pucbuf;

    uint32                              packet_length = ETH_MAX_PACKET_SIZE;
    uint8                               test_buf[ETH_MAX_PACKET_SIZE] =
                                        {
                                            0x00, 0x11, 0x22, 0x33, 0x44, 0x55, // dst addr 6 byte
                                            0x00, 0x12, 0x34, 0x56, 0x78, 0x66, // src addr 6 byte
                                            0x08, 0x06,                         // Ether type 2 byte
                                            0x00, 0x01, 0x08, 0x00, 0xee, 0xee, 0xee, 0xee, //data
                                            0x00, 0x12, 0x34, 0x56, 0xff, 0xff, 0xff, 0xff,
                                            0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0xc0, 0xa8, 0x01, 0x64,
                                        };
	ETHRxDescriptor_t *                 pxETHRxDescriptor;

    uint32 loop_cnt = 0;
    uint32 fail_cnt = 0;
    uint32 result = 0;
    uint32 i;
    uint32 j;
    uint32 d4;
    uint32 ext1;
    uint32 uiValue = 0;

    static uint8 test_result[64][64] = {0,};

    ucMacAddress[0] = 0x00;
	ucMacAddress[1] = 0x11;
	ucMacAddress[2] = 0x22;
	ucMacAddress[3] = 0x33;
	ucMacAddress[4] = 0x44;
	ucMacAddress[5] = 0x55;

    if(setup_flag == 0)
    {
        ETH_Prepare(ETH_MODE_PHY_LOOPBACK);
        ETH_SetMacAddress((uint8 *) &ucMacAddress[0]);
        xNetworkBuffersInitialise();

        for(idx = 0; idx < ETH_RDES_NUM ; idx++)
        {
            pucbuf = pucGetRXBuffer(BUF_SIZE_2KiB);
            ETH_SetRecvBuff(pucbuf, idx);
        }

        while(1)
        {
            if(ETH_GetLinkStatus() == 0)
            {
                mcu_printf("Link Down.. waiting for link up.. \n");
                (void) ETH_UpdateLink();
            }
            else
            {
                delay1us(30000);
                break;
            }
        }

        // MTL RxQ0 operation mode.
        // Threshold mode. full packet will be transferred
        uiValue = 0x1F00000;
        //uiValue = 0x1F00020; // +SF mode -> Cornertest fail in invert TX/RX signal
        (void)SAL_WriteReg(uiValue, ETH_MAC_BASE + ETH_MAC_RXQ0_OP_MODE);

        // MTL TxQ0 operation mode
        // Threshold mode. Full packet will be transferred
        uiValue = 0xF0078; // TX Threshold : max 512 byte
        //uiValue = 0xF000a; // +SF mode -> Cornertest fail in invert TX/RX signal
        (void)SAL_WriteReg(uiValue, ETH_MAC_BASE + ETH_MAC_TXQ0_OP_MODE);

        // VCP EVB v1.1.1
        d4 = 10;
        ext1 = 0;
        setup_flag = 1;
    }

    mcu_printf("Ethernet Corner Sample Test Start ... \n");
    mcu_printf("Loopback path : MAC TX - PHY - MAC RX \n");

    for(i = 0 ; i < 64 ; i ++)
    {
        for(j = 0 ; j < 64 ; j++)
        {

            if(i > 31)
            {
                h_mac.dDelayInfo.dTxcInv = 1;
                h_mac.dDelayInfo.dTxc = i - 32;
            }
            else
            {
                h_mac.dDelayInfo.dTxcInv = 0;
                h_mac.dDelayInfo.dTxc = i;
            }

            if(j > 31)
            {
                h_mac.dDelayInfo.dRxcInv = 1;
                h_mac.dDelayInfo.dRxc = j - 32;
            }
            else
            {
                h_mac.dDelayInfo.dRxcInv = 0;
                h_mac.dDelayInfo.dRxc = j;
            }

            h_mac.dDelayInfo.dRxd0 = d4;
            h_mac.dDelayInfo.dRxd1 = d4;
            h_mac.dDelayInfo.dRxd2 = d4;
            h_mac.dDelayInfo.dRxd3 = d4;
            h_mac.dDelayInfo.dRxdv = ext1;

            (void)ETH_SetSignalDelay();

            delay1us(10);

            while(loop_cnt < 10)
            {
                ETH_Send(&test_buf[0], packet_length);
                delay1us(20);
                pxETHRxDescriptor = ETH_Recv();

                if (pxETHRxDescriptor->xDataLength > 0)
                {
                    // TX packet length + CRC 4 byte = RX packet length
                    if(pxETHRxDescriptor->xDataLength != (packet_length + 4))
                    {
                        fail_cnt++;
                    }
                    else
                    {
                        //mcu_printf("Receive correct data.\n");
                    }
                    (void)ETH_Rx();
                }
                else
                {
                    fail_cnt++;

                    if(pxETHRxDescriptor->overflow == 1)
                    {
                        mcu_printf("RX overflow!! \n");
                        (void)ETH_Rx();
                    }
                }

                loop_cnt++;
            }

            // data loss count
            if(fail_cnt > 4)
            {
                test_result[i][j] = 1;
                mcu_printf("X ");
            }
            else
            {
                mcu_printf(". ");
            }

            loop_cnt = 0;
            fail_cnt = 0;
            if(j == 31)
            {
                mcu_printf("| ");
            }
        } // j
        mcu_printf("\n");
    }//i


    for(i = 0 ; i < 64 ; i++)
    {
        for(j = 0 ; j < 64 ; j++)
        {
            if(test_result[i][j] != 0)
            {
                result++;

                //mcu_printf("Fail info] Txc : %d , Rxc : %d \n", i , j);
            }

            test_result[i][j] = 0;
        }
    }
    mcu_printf("========== Test Result\n");

    if(result > (2048))
    {
        mcu_printf("========== FAIL\n");
    }
    else
    {
        mcu_printf("========== PASS\n");
    }

    mcu_printf("\n");

    mcu_printf("Ethernet Corner Sample Test End ... \n");

    loop_cnt = 0;
    result = 0;
    fail_cnt = 0;

    //PMU_ColdReset();
}
#endif

#endif  // ( MCU_BSP_SUPPORT_DRIVER_ETH == 1 )


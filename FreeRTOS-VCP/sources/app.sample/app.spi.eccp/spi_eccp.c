// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spi_eccp.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : Demo application for controlling of "TLC5925 LED Sink Driver" using GPSB driver on the tcc70xx evb.
*
*
***************************************************************************************************
*/


#if ( APLT_LINUX_SUPPORT_SPI_DEMO == 1 )
#include "debug.h"
#include "sal_internal.h"
#include "sal_com.h"
#include "gpsb_reg.h"
#include "gpsb.h"
#include "mpu.h"
#include "spi_eccp.h"
#include "gic.h"
#include "gpio.h"
#include <bsp.h>


#define ECCP_INIT_STEP(V, S)            ((V) |= (S))
#define ECCP_NO_ERR(V)                  (V == 0UL)
#define RING_BUFFER_SIZE           64
#define MAX_HEADSIZE (RING_BUFFER_SIZE - 1)

static uint32                           cs = 0;
static uint32                           eccpIsInit = 0;
static uint32                           eccpTxTask[ECCP_TASK_SIZE];
static uint32                           eccpRxTask[ECCP_TASK_SIZE];
static ECCPManager_t                    eccpMgr = {0, };
static ECCPDev_t                        eccpDevSlot[ECCP_MAX_DEV_SLOT] = {0, };
static uint32 * tx_dma;
static uint32 * rx_dma;


static void ECCP_TxTask
(
    void *                              pArgs
);

static void ECCP_RxTask
(
    void *                              pArgs
);

static int32 ECCP_MakeFrame
(
    uint8                               ch,
    ECCPMsg_t *                         msg,
    uint16                              puiCmd1,
    uint16                              puiCmd2,
    const uint8 *                       pucBuff,
    uint32                              uiLength
);

static uint16 ECCP_CalcCrc16
(
    const uint8 *                       pucBuffer,
    uint32                              uiLength
);

static void ECCP_RecvISR
(
    void *                              arg
);

static void ECCP_Complete
(
    uint32                              uiCh,
    uint32                              iEvent,
    void *                              pArg
);

static void ECCP_DoRx
(
    uint8 *                             pucBuff,
    uint32                              flag
);

static int32 ECCP_CheckHeader
(
    uint8 *                             pucDummy,
    uint8 *                             pucBuff,
    uint32                              uiHeadLen
);

static int32 ECCP_DeployTxData
(
    ECCPMsg_t *                         txMsg,
    ECCPMsg_t *                         rxMsg,
    uint32                              txSize
);

static int32 ECCP_ReadPayload
(
    uint8 *                             pucBuff,
    uint8 *                             pucDummy,
    uint32                              uiHeadLen
);

static int32 ECCP_DeployRxData
(
    uint16                              dev,
    ECCPUnMarMsg_t *                    rxMsg,
    uint8 *                             pucBuff,
    uint32                              uiHeadLen
);

static int32  ECCP_Rx_in_Tx
(
    uint8 *                             pucDummy,
    uint8 *                             pucBuff,
    uint32                              TxMsgLen
);

static void delay1us
(
   uint32                               uiUs
);

void ECCP_InitSPIManager
(
    void
)
{

    uint32 sdi, sdo, sclk;

    GPSBOpenParam_t                     OpenParam;
    uint32 err = 0;

    if (SAL_RET_SUCCESS != SAL_QueueCreate(&eccpMgr.mgrTxQueue, (const uint8 *) "ECCP_SPIManager", ECCP_MAX_QUEUE_DEPTH, sizeof(ECCPMsg_t)))
    {
        ECCP_INIT_STEP(err, 1UL);
    }

    if (SAL_RET_SUCCESS != SAL_SemaphoreCreate(&eccpMgr.mgrLock, (const uint8 *) "ECCP_TXLock", 1UL, SAL_OPT_BLOCKING))
    {
        ECCP_INIT_STEP(err, 2UL);
    }

    if (SAL_RET_SUCCESS != SAL_EventCreate(&eccpMgr.mgrRxEvent, (const uint8 *) "ECCP_RXEvent", 0UL))
    {
        ECCP_INIT_STEP(err, 3UL);
    }

    if (SAL_RET_SUCCESS != SAL_TaskCreate(&eccpMgr.mgrTxTask, (const uint8 *) "ECCP_TXTask", &ECCP_TxTask, eccpTxTask, ECCP_TASK_SIZE, SAL_PRIO_IPC_CONTROL, NULL))
    {
        ECCP_INIT_STEP(err, 4UL);
    }

    if (SAL_RET_SUCCESS != SAL_TaskCreate(&eccpMgr.mgrRxTask, (const uint8 *) "ECCP_RXTask", &ECCP_RxTask, eccpRxTask, ECCP_TASK_SIZE, SAL_PRIO_IPC_CONTROL, NULL))
    {
        ECCP_INIT_STEP(err, 5UL);
    }

    if (SAL_RET_SUCCESS != SAL_EventCreate(&eccpMgr.mgrTxEvent, (const uint8 *) "ECCP_TXEvent", 0UL))
    {
        ECCP_INIT_STEP(err, 6UL);
    }

    if (SAL_RET_SUCCESS != SAL_EventCreate(&eccpMgr.mgrRxDoneEvent, (const uint8 *) "ECCP_RXDoneEvent", 0UL))
    {
        ECCP_INIT_STEP(err, 7UL);
    }

    (void) GPIO_Config(GPIO_GPA(23), (uint32)(GPIO_FUNC((uint32)0UL) | GPIO_OUTPUT));
    // External Interrupt
    (void) GPIO_Config(GPIO_GPA(22), (uint32)(GPIO_FUNC((uint32)0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN));
    (void) GPIO_IntExtSet(GIC_EXT8, GPIO_GPA(22));
    (void) GIC_IntVectSet(GIC_EXT8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, &ECCP_RecvISR, (void*)NULL);
    (void) GIC_IntSrcEn(GIC_EXT8);

    // SPI Open
    sclk   = gpsbport[ECCP_GPSB_PORT][GPSB_SCLK];
    cs     = gpsbport[ECCP_GPSB_PORT][GPSB_CS];
    sdo    = gpsbport[ECCP_GPSB_PORT][GPSB_SDO];
    sdi    = gpsbport[ECCP_GPSB_PORT][GPSB_SDI];

    tx_dma = (uint32 *)MPU_GetDMABaseAddress();
    rx_dma = (uint32 *) ((uint32 *)MPU_GetDMABaseAddress()+ ECCP_GPSB_DMA_SIZE);

    OpenParam.uiSdo = sdo;
    OpenParam.uiSdi = sdi;
    OpenParam.uiSclk = sclk;
    OpenParam.pDmaAddrTx = tx_dma;
    OpenParam.pDmaAddrRx = rx_dma;
    OpenParam.uiDmaBufSize = ECCP_GPSB_DMA_SIZE;
    OpenParam.fbCallback    = ECCP_Complete;
    OpenParam.pArg          = NULL_PTR;
    OpenParam.uiIsSlave     = GPSB_MASTER_MODE;

    (void) GPSB_Open(ECCP_GPSB_CH,OpenParam);
    (void) GPSB_SetSpeed(ECCP_GPSB_CH, ECCP_GPSB_SPEED);
    (void) GPSB_SetBpw(ECCP_GPSB_CH, ECCP_GPSB_BPW);
    (void) GPSB_CsInit(ECCP_GPSB_CH, cs, 1UL);
    (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);

    if (err == 0UL)
    {
        eccpIsInit = 1UL;
    }
}

ECCPDev_t* ECCP_Open
(
    const uint8 *                       pucDevName,
    uint16                              ucFeature
)
{
    ECCPDev_t *ret = NULL;

    if (eccpIsInit == 1UL)
    {
        if (ECCP_CHECK_UNUSED_DEVICE(ucFeature))
        {
            if (SAL_RET_SUCCESS != SAL_QueueCreate(&eccpDevSlot[ucFeature].devQueue, pucDevName, ECCP_MAX_QUEUE_DEPTH, sizeof(ECCPUnMarMsg_t)))
            {
                //mcu_printf("%s Queue Creation Fail For %d\n", __func__, ucFeature);
            }

            if (SAL_RET_SUCCESS != SAL_SemaphoreCreate(&eccpDevSlot[ucFeature].devLock, pucDevName, 1UL, SAL_OPT_BLOCKING))
            {
                //mcu_printf("%s Tx Lock Creation Fail For %d\n", __func__, ucFeature);
            }

            eccpDevSlot[ucFeature].devName   = pucDevName;
            eccpDevSlot[ucFeature].devSpiCh  = ECCP_GPSB_CH;
            eccpDevSlot[ucFeature].devDevId  = ucFeature;
            eccpDevSlot[ucFeature].devInUsed = 1UL;

            ret = &eccpDevSlot[ucFeature];
        }
        else
        {
            //mcu_printf("%s No Slot For %d\n", __func__, ucFeature);
        }
    }
    else
    {
        //mcu_printf("%s No SPIManager For %d\n", __func__, ucFeature);
    }

    return ret;
}

int32 ECCP_Send
(
    ECCPDev_t *                         eccpDev,
    uint8                               ucTarget,
    uint16                              puiCmd2,
    const uint8 *                       pucBuff,
    uint32                              uiLength
)
{
    int32 ret = ECCP_SUCCESS;

    if ((eccpDev != NULL) && (eccpDev->devInUsed == 1UL))
    {
        (void) SAL_SemaphoreWait(eccpDev->devLock, 0UL, SAL_OPT_BLOCKING);
        if (ECCP_SUCCESS == (ret = ECCP_MakeFrame(ucTarget, &eccpDev->devTxMsg, eccpDev->devDevId, puiCmd2, pucBuff, uiLength)))
        {
            //mcu_printf("%s %d:%d:%d sent\n", __func__, sizeof(ECCPMsg_t), eccpDev->devTxMsg.msgLen, uiLength);
            if (ECCP_CHECK_QUEUEPUT(ret, eccpMgr.mgrTxQueue, &eccpDev->devTxMsg, ECCPMsg_t))
            {
                mcu_printf("%s QueuePut Fail\n", __func__);
                ret = ECCP_QUEUE_FAIL;
            }
            else
            {
                //mcu_printf("QueuePut end \n");
            }
        }

        (void) SAL_SemaphoreRelease(eccpDev->devLock);
    }

    else
    {
        //mcu_printf("%s %s is not ready to be used\n", __func__, (eccpDev == NULL ? (uint8 *)"" : eccpDev->devName));
        ret = ECCP_NOT_OPENED;
    }
    // mcu_printf("eccp send end \n");
    return ret;

}

int32 ECCP_Recv
(
    ECCPDev_t *                         eccpDev,
    uint8 *                             pucSource,
    uint16 *                            puiCmd,
    uint8 *                             pucBuff
)
{
    int32 ret = ECCP_SUCCESS;
    uint32 rxSize = 0UL;

    if (ECCP_CHECK_OPEN_DEVICE(eccpDev))
    {
        if (ECCP_CHECK_QUEUEGET(ret, eccpDev->devQueue, &eccpDev->devRxMsg, &rxSize))
        {
            if (ECCP_CHECK_VALID_DATA(rxSize))
            {
                *pucSource = eccpDev->devRxMsg.msgSource;
                *puiCmd    = eccpDev->devRxMsg.msgCmd;

                if (ECCP_CHECK_VALID_DATA(eccpDev->devRxMsg.msgLen))
                {
                    SAL_MemCopy(pucBuff, eccpDev->devRxMsg.msgPtr, eccpDev->devRxMsg.msgLen);
                    ret = eccpDev->devRxMsg.msgLen;
                }
            }
            else
            {
                //mcu_printf("%s parser problem\n", __func__);
                ret = ECCP_INVALID_DATA;
            }
        }
    }
    else
    {
        //mcu_printf("%s %s is not ready to be used\n", __func__, (eccpDev == NULL ? NULL : eccpDev->devName));
        ret = ECCP_NOT_OPENED;
    }

    return ret;
}

static int32 ECCP_MakeFrame
(
    uint8                               ch,
    ECCPMsg_t *                         msg,
    uint16                              puiCmd1,
    uint16                              puiCmd2,
    const uint8 *                       pucBuff,
    uint32                              uiLength
)
{
    uint16 uiCrc;
    uint8 *pucCrc  = (uint8 *) &uiCrc;
    uint8 *pucCmd1 = (uint8 *) &puiCmd1;
    uint8 *pucCmd2 = (uint8 *) &puiCmd2;
    uint8 *pucSize = (uint8 *) &uiLength;
    int32 ret = ECCP_SUCCESS;

    if ((uiLength + ECCP_DATA + 2UL) <= ECCP_GPSB_DMA_SIZE)
    {
        SAL_MemSet(msg->msgPtr, 0x00, sizeof(msg->msgPtr));
        msg->msgPtr[ECCP_FF]    = ECCP_SF0;
        msg->msgPtr[ECCP_55]    = ECCP_SF1;
        msg->msgPtr[ECCP_AA]    = ECCP_SF2;
        msg->msgPtr[ECCP_SRC]   = ECCP_VCP_FRAME;
        msg->msgPtr[ECCP_DES]   = ch;
        msg->msgPtr[ECCP_CMD1H] = pucCmd1[1];
        msg->msgPtr[ECCP_CMD1L] = pucCmd1[0];
        msg->msgPtr[ECCP_CMD2H] = pucCmd2[1];
        msg->msgPtr[ECCP_CMD2L] = pucCmd2[0];
        msg->msgPtr[ECCP_LENH]  = pucSize[1];
        msg->msgPtr[ECCP_LENL]  = pucSize[0];

        uiCrc = ECCP_CalcCrc16(msg->msgPtr, ECCP_DATA);
        msg->msgPtr[ECCP_DATA]     = pucCrc[1];
        msg->msgPtr[ECCP_DATA + 1] = pucCrc[0];

        if (pucBuff != NULL)
        {
            (void) SAL_MemCopy(&msg->msgPtr[ECCP_DATA + 2], pucBuff, uiLength);

            uiCrc = ECCP_CalcCrc16(&msg->msgPtr[ECCP_DATA + 2], ECCP_DATASIZE_64 + ECCP_DUMMY_SIZE);
            msg->msgPtr[ECCP_PACKET_SIZE_64 - ECCP_CRC_SIZE] = pucCrc[1];
            msg->msgPtr[ECCP_PACKET_SIZE_64 - ECCP_CRC_SIZE + 1UL] = pucCrc[0];

            msg->msgLen = ECCP_PACKET_SIZE_64;
        }

    }
    else
    {
        //mcu_printf("%s data limit %d\n", __func__, ECCP_GPSB_DMA_SIZE - ECCP_DATA - 2);
        ret = ECCP_TOO_MUCH_DATA;
    }

    return ret;
}

static uint16 ECCP_CalcCrc16
(
    const uint8 *                       pucBuffer,
    uint32                              uiLength
)
{
    uint32 i;
    uint32 temp;
    uint16 crcCode = 0;
    static const uint16 crc16Table[256] =
    {
        0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50a5U, 0x60c6U, 0x70e7U,
        0x8108U, 0x9129U, 0xa14aU, 0xb16bU, 0xc18cU, 0xd1adU, 0xe1ceU, 0xf1efU,
        0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52b5U, 0x4294U, 0x72f7U, 0x62d6U,
        0x9339U, 0x8318U, 0xb37bU, 0xa35aU, 0xd3bdU, 0xc39cU, 0xf3ffU, 0xe3deU,
        0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64e6U, 0x74c7U, 0x44a4U, 0x5485U,
        0xa56aU, 0xb54bU, 0x8528U, 0x9509U, 0xe5eeU, 0xf5cfU, 0xc5acU, 0xd58dU,
        0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76d7U, 0x66f6U, 0x5695U, 0x46b4U,
        0xb75bU, 0xa77aU, 0x9719U, 0x8738U, 0xf7dfU, 0xe7feU, 0xd79dU, 0xc7bcU,
        0x48c4U, 0x58e5U, 0x6886U, 0x78a7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
        0xc9ccU, 0xd9edU, 0xe98eU, 0xf9afU, 0x8948U, 0x9969U, 0xa90aU, 0xb92bU,
        0x5af5U, 0x4ad4U, 0x7ab7U, 0x6a96U, 0x1a71U, 0x0a50U, 0x3a33U, 0x2a12U,
        0xdbfdU, 0xcbdcU, 0xfbbfU, 0xeb9eU, 0x9b79U, 0x8b58U, 0xbb3bU, 0xab1aU,
        0x6ca6U, 0x7c87U, 0x4ce4U, 0x5cc5U, 0x2c22U, 0x3c03U, 0x0c60U, 0x1c41U,
        0xedaeU, 0xfd8fU, 0xcdecU, 0xddcdU, 0xad2aU, 0xbd0bU, 0x8d68U, 0x9d49U,
        0x7e97U, 0x6eb6U, 0x5ed5U, 0x4ef4U, 0x3e13U, 0x2e32U, 0x1e51U, 0x0e70U,
        0xff9fU, 0xefbeU, 0xdfddU, 0xcffcU, 0xbf1bU, 0xaf3aU, 0x9f59U, 0x8f78U,
        0x9188U, 0x81a9U, 0xb1caU, 0xa1ebU, 0xd10cU, 0xc12dU, 0xf14eU, 0xe16fU,
        0x1080U, 0x00a1U, 0x30c2U, 0x20e3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
        0x83b9U, 0x9398U, 0xa3fbU, 0xb3daU, 0xc33dU, 0xd31cU, 0xe37fU, 0xf35eU,
        0x02b1U, 0x1290U, 0x22f3U, 0x32d2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
        0xb5eaU, 0xa5cbU, 0x95a8U, 0x8589U, 0xf56eU, 0xe54fU, 0xd52cU, 0xc50dU,
        0x34e2U, 0x24c3U, 0x14a0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
        0xa7dbU, 0xb7faU, 0x8799U, 0x97b8U, 0xe75fU, 0xf77eU, 0xc71dU, 0xd73cU,
        0x26d3U, 0x36f2U, 0x0691U, 0x16b0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
        0xd94cU, 0xc96dU, 0xf90eU, 0xe92fU, 0x99c8U, 0x89e9U, 0xb98aU, 0xa9abU,
        0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18c0U, 0x08e1U, 0x3882U, 0x28a3U,
        0xcb7dU, 0xdb5cU, 0xeb3fU, 0xfb1eU, 0x8bf9U, 0x9bd8U, 0xabbbU, 0xbb9aU,
        0x4a75U, 0x5a54U, 0x6a37U, 0x7a16U, 0x0af1U, 0x1ad0U, 0x2ab3U, 0x3a92U,
        0xfd2eU, 0xed0fU, 0xdd6cU, 0xcd4dU, 0xbdaaU, 0xad8bU, 0x9de8U, 0x8dc9U,
        0x7c26U, 0x6c07U, 0x5c64U, 0x4c45U, 0x3ca2U, 0x2c83U, 0x1ce0U, 0x0cc1U,
        0xef1fU, 0xff3eU, 0xcf5dU, 0xdf7cU, 0xaf9bU, 0xbfbaU, 0x8fd9U, 0x9ff8U,
        0x6e17U, 0x7e36U, 0x4e55U, 0x5e74U, 0x2e93U, 0x3eb2U, 0x0ed1U, 0x1ef0U
    };

    if (pucBuffer != NULL_PTR)
    {
        for (i = 0; i < uiLength; i++)
        {
            temp    = ((((uint32) crcCode) >> 8U) ^ pucBuffer[i]) & ((uint32) 0xff);
            crcCode = crc16Table[temp] ^ ((uint16) (((uint32) crcCode) << 8U));
        }
    }

    return crcCode;
}

static void ECCP_TxTask
(
    void *                              pArgs
)
{
    int32  ret = SAL_RET_SUCCESS;
    uint32 txSize = 0UL;
    static ECCPMsg_t txMsgInTask;
    static ECCPMsg_t rxMsgIntxTask;

    //mcu_printf("%s Created\n", __func__);

    while(TRUE)
    {
        SAL_MemSet(&txMsgInTask, 0x00, sizeof(txMsgInTask));

        if (ECCP_CHECK_QUEUEGET(ret, eccpMgr.mgrTxQueue, &txMsgInTask, &txSize))
        {
 //           mcu_printf("ECCP_CHECK_QUEUEGET\n");
            if (ECCP_SUCCESS != ECCP_DeployTxData(&txMsgInTask, &rxMsgIntxTask, txSize))
            {
                mcu_printf("%s tx to channel fail\n", __func__);
            }
        }
        else
        {
            mcu_printf("%s QueueFail\n", __func__);
        }
    }

    //mcu_printf("%s ended\n", __func__);
}

static void ECCP_RxTask
(
    void *                              pArgs
)
{
    uint32 flag;
    static ECCPMsg_t rxMsgInTask;
    SAL_MemSet(&rxMsgInTask, 0x00, sizeof(rxMsgInTask));

    //mcu_printf("%s Created\n", __func__);

    while(TRUE)
    {
        if (ECCP_CHECK_WAIT_EVENT(eccpMgr.mgrRxEvent, ECCP_READ_EVENT, &flag))
        {
            /* //origin
            ECCP_DoRx(rxMsgInTask.msgPtr, flag);
            ECCP_CLEAR_EVENT(eccpMgr.mgrRxEvent, ECCP_READ_EVENT);
            */
            ECCP_CLEAR_EVENT(eccpMgr.mgrRxEvent, ECCP_READ_EVENT);
            ECCP_DoRx(rxMsgInTask.msgPtr, flag);
            ECCP_NOTIFY_READ_DONE
        }
        else
        {
            ECCP_D("EventGet Fail");
        }
    }

    //mcu_printf("%s ended\n", __func__);
}

static void ECCP_DoRx
(
    uint8 *                             pucBuff,
    uint32                              flag
)
{
    uint32 rdFlag = 0;

    if (ECCP_CHECK_TIMEOUT(flag))
    {
        (void) SAL_SemaphoreWait(eccpMgr.mgrLock, 0UL, SAL_OPT_BLOCKING);
        (void) GPSB_CsDeactivate(ECCP_GPSB_CH, cs, 1UL);

        if (ECCP_CHECK_GPSB_XFER(NULL, pucBuff, (ECCP_HEADER_SIZE + 2UL)))
        {
            if (ECCP_CHECK_WAIT_EVENT(eccpMgr.mgrRxDoneEvent, ECCP_READ_DONE_EVENT, &rdFlag))
            {
                ECCP_CLEAR_EVENT(eccpMgr.mgrRxDoneEvent, ECCP_READ_DONE_EVENT);
            }
            else
            {
                ECCP_D("EventGet Fail");
            }

            (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);

#ifdef ECCP_DUMP_ENABLE
            //ECCP_Dump(__func__, pucBuff, ECCP_HEADER_SIZE + 2UL);
            ECCP_Dump(__func__, &pucBuff[ECCP_CMD2H], 6);
#endif

            if (ECCP_SUCCESS != ECCP_CheckHeader(NULL, pucBuff, ECCP_HEADER_SIZE))
            {
                ECCP_D("push buff error");
            }
        }
        else
        {
            (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);
            ECCP_D("Xfer Rx Fail");
        }
        (void) SAL_SemaphoreRelease(eccpMgr.mgrLock);
    }
    else
    {
        ECCP_D("Event Timeout\n");
    }
}

static int32 ECCP_CheckHeader
(
    uint8 *                             pucDummy,
    uint8 *                             pucBuff,
    uint32                              uiHeadLen
)
{
    uint16  uiCrc1;
    uint16  uiCrc2;
    int32   ret = ECCP_SUCCESS;

    if (ECCP_CHECK_START_FRAME(pucBuff) && ECCP_CHECK_VCP_FRAME(pucBuff))
    {
        uiCrc1 = (pucBuff[uiHeadLen] << 8) | pucBuff[uiHeadLen + 1UL];
        uiCrc2  = ECCP_CalcCrc16(pucBuff, uiHeadLen);

        if (ECCP_CHECK_CRC(uiCrc1, uiCrc2))
        {
            ret = ECCP_ReadPayload(pucBuff, pucDummy, uiHeadLen);
        }
        else
        {
            ECCP_D("Invalid ECCP Header");
            ret = ECCP_INVALID_HEADER;
        }
    }
    else
    {
        ECCP_D("Not ECCP frame %02X %02X %02X %02X", pucBuff[ECCP_FF], pucBuff[ECCP_55], pucBuff[ECCP_AA], pucBuff[ECCP_DES]);
        ret = ECCP_INVALID_FRAME;
    }

    return ret;
}

static int32  ECCP_Rx_in_Tx
(
    uint8 *                             pucDummy,
    uint8 *                             pucBuff,
    uint32                              TxMsgLen
)

{
    uint32  rdFlag = 0;
    uint16  uiLen = 0;
    int32   ret = ECCP_SUCCESS;
    uint16  dev;
    uint16  uiCrc1, uiCrc2;
    static  ECCPUnMarMsg_t rxMsg;

    if (ECCP_CHECK_START_FRAME(pucBuff) && ECCP_CHECK_VCP_FRAME(pucBuff))
    {
        ECCP_RECEIVED_DEVICE(dev, pucBuff);
        if (ECCP_CHECK_VALID_DEVICE(dev))
        {
            ECCP_RECEIVED_LENGTH(&rxMsg, pucBuff);

            if(dev == ECCP_UPDATE_APP)
            {
                ECCP_D("ECCP_UPDATE Case");
                if(TxMsgLen == ECCP_PACKET_SIZE_64)
                {
                    uiLen = ECCP_PACKET_SIZE_492 - ECCP_PACKET_SIZE_64;
                }

                (void) GPSB_CsDeactivate(ECCP_GPSB_CH, cs, 1UL);

                if (ECCP_CHECK_GPSB_XFER(pucDummy, &pucBuff[TxMsgLen], uiLen))
                {
                    if (ECCP_CHECK_WAIT_EVENT(eccpMgr.mgrRxDoneEvent, ECCP_READ_DONE_EVENT, &rdFlag))
                    {
                        ECCP_CLEAR_EVENT(eccpMgr.mgrRxDoneEvent, ECCP_READ_DONE_EVENT);
                    }
                    else
                    {
                        ECCP_D("EventGet Fail");
                    }

                    (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);

#ifdef ECCP_DUMP_ENABLE
                    ECCP_Dump(__func__, pucBuff, ECCP_PACKET_SIZE_492);
#endif
                }
                else
                {
                    (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);
                    ECCP_D("Xfer Rx Header Fail");
                    ret = ECCP_GPSB_TRANSFER_FAIL;
                }
            }

            //ECCP_D("uiLen : %d , TxMsgLen : %d", uiLen, TxMsgLen);
#ifdef ECCP_DUMP_ENABLE
            ECCP_Dump(__func__, &pucBuff[ECCP_CMD2H], 8);
#endif

            if (ECCP_CHECK_ONLY_HEADER(&rxMsg) && (dev == ECCP_STR_APP) && (ECCP_CHECK_BL_CMD(pucBuff)))
            {
                ECCP_D("BL");

                ECCP_EXTRACT_CRC(uiCrc1, ECCP_HEADER_SIZE, &pucBuff[0]);
                ECCP_RECEIVED_CRC(uiCrc2, ECCP_HEADER_SIZE, pucBuff);

                if (ECCP_CHECK_CRC(uiCrc1, uiCrc2))
                {
                    ECCP_RECEIVED_COMMAND(&rxMsg, pucBuff);
                    ECCP_RECEIVED_SOURCE(&rxMsg, pucBuff);
                }
                else
                {
                    ECCP_D("[STR, BL cmd] CRC Fail - Crc1 : %x Crc2 : %x ", uiCrc1, uiCrc2);
                    ret = ECCP_INVALID_DATA;
                }
            }
            else
            {
                if( dev == ECCP_UPDATE_APP )
                {
                    ECCP_EXTRACT_CRC(uiCrc1, ECCP_DATASIZE_492 + ECCP_DUMMY_SIZE, &pucBuff[ECCP_HEADER_SIZE+ECCP_CRC_SIZE]);
                    ECCP_RECEIVED_CRC(uiCrc2, ECCP_PACKET_SIZE_492 - ECCP_CRC_SIZE, pucBuff);
                }
                else
                {
                    ECCP_EXTRACT_CRC(uiCrc1, ECCP_DATASIZE_64 + ECCP_DUMMY_SIZE, &pucBuff[ECCP_HEADER_SIZE+ECCP_CRC_SIZE]);
                    ECCP_RECEIVED_CRC(uiCrc2, ECCP_PACKET_SIZE_64 - ECCP_CRC_SIZE, pucBuff);
                }

                if (ECCP_CHECK_CRC(uiCrc1, uiCrc2))
                {
                    ECCP_RECEIVED_COMMAND(&rxMsg, pucBuff);
                    ECCP_RECEIVED_SOURCE(&rxMsg, pucBuff);
                    ECCP_RECEIVED_DATA(&rxMsg, &pucBuff[ECCP_HEADER_SIZE+2]);
                }
                else
                {
                    ECCP_D("CRC Fail - Crc1 : %x Crc2 : %x ", uiCrc1, uiCrc2);
                    ret = ECCP_INVALID_DATA;
                }

               //mcu_printf("dev: %d, msgCmd : %x, msgSource : %x, msgLen : %x \n", dev, rxMsg.msgCmd ,rxMsg.msgSource, rxMsg.msgLen);
            }

            if (ECCP_CHECK_NO_ERROR(ret))
            {
                if (ECCP_CHECK_QUEUEPUT(ret, eccpDevSlot[dev].devQueue, &rxMsg, ECCPUnMarMsg_t))
                {
                    //ECCP_D("Queue Put Fail");
                    ret = ECCP_QUEUE_FAIL;
                }
                else
                {
                    ECCP_D("Queue Put Success");
                }
            }

            ECCP_D("ret : %d", ret);
        }
        else
        {
            ECCP_D("Invaild dev : %d , IsUsed : %d", dev, eccpDevSlot[dev].devInUsed);
        }
    }
    else
    {
        //mcu_printf("%s Not ECCP frame %02X %02X %02X %02X\n", __func__, pucBuff[ECCP_FF], pucBuff[ECCP_55], pucBuff[ECCP_AA], pucBuff[ECCP_DES]);
        //ECCP_D("ECCP Invalid frame");
        ret = ECCP_INVALID_FRAME;
    }

    return ret;
}


static int32 ECCP_DeployTxData
(
    ECCPMsg_t *                         txMsg,
    ECCPMsg_t *                         rxMsg,
    uint32                              txSize
)
{
    int32 ret = ECCP_SUCCESS;
    uint32 flag = 1;

    static uint8 rx_buff[ECCP_MAX_PACKET] = {0,};
 //   mcu_printf("%s Xfer Tx \n", __func__);
    if ((txMsg != NULL) && (txSize > 0UL))
    {
        (void) SAL_SemaphoreWait(eccpMgr.mgrLock, 0UL, SAL_OPT_BLOCKING);
        (void) GPSB_CsDeactivate(ECCP_GPSB_CH, cs, 1UL);
        //delay1us(1000);

        if (ECCP_CHECK_GPSB_XFER(txMsg->msgPtr, rx_buff, txMsg->msgLen))  //rxMsg->msgPtr NULL
        {
            if (ECCP_CHECK_WAIT_EVENT(eccpMgr.mgrTxEvent, ECCP_WRITE_EVENT, &flag))
            {
                ECCP_CLEAR_EVENT(eccpMgr.mgrTxEvent, ECCP_WRITE_EVENT);
            }
            else
            {
                ECCP_D("EventGet Fail");
            }

            (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);

#ifdef ECCP_DUMP_ENABLE
            //ECCP_Dump(__func__, txMsg->msgPtr, txMsg->msgLen);
            ECCP_Dump(__func__, &txMsg->msgPtr[ECCP_CMD2H], 8);
#endif

            GPSB_ClearDMABuffer(ECCP_GPSB_CH, ECCP_GPSB_DMA_SIZE);

#ifdef ECCP_DUMP_ENABLE
            //ECCP_Dump(__func__, &rx_buff, txMsg->msgLen);
#endif

            ECCP_Rx_in_Tx(NULL, rx_buff, txMsg->msgLen);
        }
        else
        {
            (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);
            mcu_printf("%s Xfer Tx Fail\n", __func__);
            ret = ECCP_GPSB_TRANSFER_FAIL;
        }

        //(void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);
        (void) SAL_SemaphoreRelease(eccpMgr.mgrLock);
    }
    else
    {
        mcu_printf("%s size error[%d]\n", __func__, txSize);
    }

    return ret;
}

static int32 ECCP_ReadPayload
(
    uint8 *                             pucBuff,
    uint8 *                             pucDummy,
    uint32                              uiHeadLen
)
{
    uint16 dev;
    uint32 flag = 0;
    int32  ret  = ECCP_SUCCESS;
    static ECCPUnMarMsg_t rxMsg;
    uint32 dataframe_size;

    ECCP_RECEIVED_DEVICE(dev, pucBuff);
    //mcu_printf("%s dev %d \n", __func__, dev);
    if (ECCP_CHECK_VALID_DEVICE(dev))
    {
        ECCP_RECEIVED_LENGTH(&rxMsg, pucBuff);

        if( (dev == ECCP_STR_APP) && (ECCP_CHECK_BL_CMD(pucBuff)) )
        {
            ECCP_D("BL");
            ret = ECCP_DeployRxData(dev, &rxMsg, pucBuff, uiHeadLen);
        }
        else if (ECCP_CHECK_VALID_LENGTH(rxMsg.msgLen, uiHeadLen))
        {
            //mcu_printf("%s Xfer Rx len : %d \n", __func__, rxMsg.msgLen + 2UL);
            if (dev == ECCP_UPDATE_APP)
            {
                dataframe_size = ECCP_DATASIZE_492;
            }
            else
            {
                dataframe_size = ECCP_DATASIZE_64;
            }

            (void) GPSB_CsDeactivate(ECCP_GPSB_CH, cs, 1UL);

            if (ECCP_CHECK_GPSB_XFER(pucDummy, &pucBuff[uiHeadLen], dataframe_size + ECCP_DUMMY_SIZE + ECCP_CRC_SIZE ))
            {
                if (ECCP_CHECK_WAIT_EVENT(eccpMgr.mgrRxDoneEvent, ECCP_READ_DONE_EVENT, &flag))
                {
                    ECCP_CLEAR_EVENT(eccpMgr.mgrRxDoneEvent, ECCP_READ_DONE_EVENT);
                }
                else
                {
                    ECCP_D("EventGet Fail");
                }

                (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);

#ifdef ECCP_DUMP_ENABLE
                //ECCP_Dump(__func__, &pucBuff[uiHeadLen], dataframe_size + ECCP_DUMMY_SIZE + ECCP_CRC_SIZE );
                ECCP_Dump(__func__, &pucBuff[uiHeadLen], 4);
#endif
                ret = ECCP_DeployRxData(dev, &rxMsg, pucBuff, uiHeadLen);
            }
            else
            {
                (void) GPSB_CsActivate(ECCP_GPSB_CH, cs, 1UL);
                //mcu_printf("%s Xfer Rx Header Fail\n", __func__);
                ret = ECCP_GPSB_TRANSFER_FAIL;
            }
        }
        else
        {
            //mcu_printf("%s Invalid data size[%d]\n", __func__, rxMsg.msgLen);
            ret = ECCP_INVALID_DATA_SIZE;
        }
    }
    else
    {
        //mcu_printf("%s Invalid cmd[%d] or Not Open [%d]\n", __func__, dev, eccpDevSlot[dev].devInUsed);
        ret = ECCP_GPSB_APP_NOT_REGISTERED;
    }

    return ret;
}

static int32 ECCP_DeployRxData
(
    uint16                              dev,
    ECCPUnMarMsg_t *                    rxMsg,
    uint8 *                             pucBuff,
    uint32                              uiHeadLen
)
{
    uint16 uiCrc1, uiCrc2;
    int32 ret = ECCP_SUCCESS;

    //mcu_printf("%s line %d\n", __func__, __LINE__);
    if (ECCP_CHECK_ONLY_HEADER(rxMsg))
    {
        ECCP_RECEIVED_COMMAND(rxMsg, pucBuff);
        ECCP_RECEIVED_SOURCE(rxMsg, pucBuff);
    }
    else
    {
        if( dev == ECCP_UPDATE_APP )
        {
            ECCP_EXTRACT_CRC(uiCrc1, ECCP_DATASIZE_492 + ECCP_DUMMY_SIZE, &pucBuff[uiHeadLen]);
            ECCP_RECEIVED_CRC(uiCrc2, ECCP_HEADER_SIZE + ECCP_DATASIZE_492 + ECCP_DUMMY_SIZE, pucBuff);
        }
        else
        {
            ECCP_EXTRACT_CRC(uiCrc1, ECCP_DATASIZE_64 + ECCP_DUMMY_SIZE, &pucBuff[uiHeadLen]);
            ECCP_RECEIVED_CRC(uiCrc2, ECCP_HEADER_SIZE + ECCP_DATASIZE_64 + ECCP_DUMMY_SIZE, pucBuff);
        }


        if (ECCP_CHECK_CRC(uiCrc1, uiCrc2))
        {
            ECCP_RECEIVED_COMMAND(rxMsg, pucBuff);
            ECCP_RECEIVED_SOURCE(rxMsg, pucBuff);
            ECCP_RECEIVED_DATA(rxMsg, &pucBuff[uiHeadLen]);
        }
        else
        {
            ECCP_D("Crc1 : %x Crc2 : %x ", uiCrc1, uiCrc2);
            mcu_printf("[%s] CRC Fail\n", __func__);
            ret = ECCP_INVALID_DATA;
        }

    }

    if (ECCP_CHECK_NO_ERROR(ret))
    {
        //mcu_printf("%s rxMsg.Len : %x \n", __func__, rxMsg->msgLen);
        if (ECCP_CHECK_QUEUEPUT(ret, eccpDevSlot[dev].devQueue, rxMsg, ECCPUnMarMsg_t))
        {
            //mcu_printf("%s Queue Put Fail. dev %d\n", __func__, dev);
            ret = ECCP_QUEUE_FAIL;
        }
        else
        {
            //mcu_printf("%s Queue Put Success. dev %d \n", __func__, dev);
        }
    }

    return ret;
}

static void ECCP_RecvISR
(
    void *                              arg
)
{
    ECCP_D("");
    if (SAL_RET_SUCCESS != SAL_EventSet(eccpMgr.mgrRxEvent, ECCP_READ_EVENT, SAL_EVENT_OPT_FLAG_SET))
    {
        ECCP_D("Event Set Fail");
    }
}

static void ECCP_Complete
(
    uint32                              uiCh,
    uint32                              iEvent,
    void *                              pArg
)
{
    if (uiCh == ECCP_GPSB_CH)
    {
  //          mcu_printf("%s event %d\n", __func__, iEvent);
        if (iEvent == GPSB_EVENT_TXCOMPLETE)
        {
            if (SAL_RET_SUCCESS != SAL_EventSet(eccpMgr.mgrTxEvent, ECCP_WRITE_EVENT, SAL_EVENT_OPT_FLAG_SET))
            {
                mcu_printf("%s Event Set Fail\n", __func__);
            }
        }
        else if (iEvent == GPSB_EVENT_COMPLETE)
        {
            if (SAL_RET_SUCCESS != SAL_EventSet(eccpMgr.mgrRxDoneEvent, ECCP_READ_DONE_EVENT, SAL_EVENT_OPT_FLAG_SET))
            {
                //mcu_printf("%s Event Set Fail\n", __func__);
            }
        }
    }
}

#ifdef ECCP_DUMP_ENABLE
void ECCP_Dump
(
    const int8 *                        pucFunc,
    uint8 *                             pucBuff,
    uint32                              uiLen
)
{
    uint32 idx;
//    mcu_printf("\n");
    mcu_printf("%s len[%d] 0x", pucFunc, uiLen);
    for (idx = 0; idx < uiLen; idx++)
    {
        mcu_printf("%02X ", pucBuff[idx]);
    }
    mcu_printf("\n");
}
#endif

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

#endif

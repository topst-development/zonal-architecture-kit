// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_msg.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CAN_MSG_HEADER
#define MCU_BSP_CAN_MSG_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

typedef struct CANRxRingBuffer
{
    boolean                             rrbInitFlag;
    uint32                              rrbMsgBufferMax;
    uint32                              rrbHeadIdx;
    uint32                              rrbTailIdx;
    CANMessage_t                        rrbMsg[ CAN_RX_MSG_RING_BUFFER_MAX ];
} CANRxRingBuffer_t;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

CANErrorType_t CAN_MsgInit
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_MsgPutRxMessage
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
);

uint32 CAN_MsgGetCountOfRxMessage
(
    uint8                               ucCh
);

CANErrorType_t CAN_MsgGetRxMessage
(
    uint8                               ucCh,
    CANMessage_t *                      psRxMsg
);

CANErrorType_t CAN_MsgSetTxMessage
(
    const CANController_t *             psControllerInfo,
    const CANMessage_t *                psMsg,
    uint8 *                             pucTxBufferIndex
);

/* Not used function */
/*
CANErrorType_t CAN_MsgRequestTxAllMessage
(
    CANController_t *                   psControllerInfo,
    uint32                              uiTxBufferAllIndex
);
*/

CANErrorType_t CAN_MsgRequestTxMessageCancellation
(
    const CANController_t *             psControllerInfo,
    uint8                               ucTxBufferIndex
);

uint32 CAN_MsgGetCountOfTxEvent
(
    uint8                               ucCh
);

CANErrorType_t CAN_MsgGetTxEventMessage
(
    uint8                               ucCh,
    CANTxEvent_t *                      psTxEvtBuffer
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

#endif  // MCU_BSP_CAN_MSG_HEADER


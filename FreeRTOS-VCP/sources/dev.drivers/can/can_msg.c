// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_msg.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "can_par.h"
#include "can_drv.h"
#include "can_msg.h"

#include <sal_internal.h>


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static CANRxRingBuffer_t RxRingBufferManager[ CAN_CONTROLLER_NUMBER ]; /* Rx Message */


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static uint8 CAN_MsgGetDataSizeBySizeType
(
    CANDataLength_t                     uiDataLengthCode
);

static CANDataLength_t CAN_MsgGetSizeTypebyDataSize
(
    uint8                               ucDataSize
);

static CANErrorType_t CAN_MsgCopyRxMessage
(
    const CANRamRxBuffer_t *            psSrcReceivedMsg,
    CANMessage_t *                      psRxBuffer
);

static CANMessage_t *CAN_MsgGetRxBufferFromRingBuffer
(
    uint8                               ucCh,
    CANRingBufferType_t                 ucType
);

static CANErrorType_t CAN_MsgPutRxMessageToRxDBuffer
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
);

static CANErrorType_t CAN_MsgPutRxMessageToRxFIFO
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
);

static CANErrorType_t CAN_MsgCopyTxMessage
(
    CANRamTxBuffer_t *                  psTxMsg,
    const CANMessage_t *                psSrcMsg
);

static CANErrorType_t CAN_MsgRequestTxMessage
(
    const CANController_t *             psControllerInfo,
    uint8                               ucReqIndex
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANErrorType_t CAN_MsgInit
(
    const CANController_t *             psControllerInfo
)
{
    uint8           ucCh;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        ucCh = psControllerInfo->cChannelHandle;

        RxRingBufferManager[ ucCh ].rrbTailIdx = 0;
        RxRingBufferManager[ ucCh ].rrbHeadIdx = 0;
        RxRingBufferManager[ ucCh ].rrbMsgBufferMax = CAN_RX_MSG_RING_BUFFER_MAX;

        ( void ) SAL_MemSet( &RxRingBufferManager[ ucCh ].rrbMsg[ 0 ], 0, ( sizeof( CANMessage_t ) * CAN_RX_MSG_RING_BUFFER_MAX ) );

        RxRingBufferManager[ ucCh ].rrbInitFlag = TRUE;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static uint8 CAN_MsgGetDataSizeBySizeType
(
    CANDataLength_t                     uiDataLengthCode
)
{
    uint8 ucDataSize;

    ucDataSize = 0;

    switch( uiDataLengthCode )
    {
        case CAN_DATA_LENGTH_12:    // 9,
        {
            ucDataSize = 12;

            break;
        }

        case CAN_DATA_LENGTH_16:    // 10,
        {
            ucDataSize = 16;

            break;
        }

        case CAN_DATA_LENGTH_20:    // 11,
        {
            ucDataSize = 20;

            break;
        }

        case CAN_DATA_LENGTH_24:    // 12,
        {
            ucDataSize = 24;

            break;
        }

        case CAN_DATA_LENGTH_32:    // 13,
        {
            ucDataSize = 32;

            break;
        }

        case CAN_DATA_LENGTH_48:    // 14,
        {
            ucDataSize = 48;

            break;
        }

        case CAN_DATA_LENGTH_64:    // 15,
        {
            ucDataSize = 64;

            break;
        }

        default:
        {
            if( uiDataLengthCode <= CAN_DATA_LENGTH_8 )
            {
                ucDataSize = ( uint8 ) uiDataLengthCode;
            }
            else
            {
                ucDataSize = 64;
            }

            break;
        }
    }

    return ucDataSize;
}

static CANDataLength_t CAN_MsgGetSizeTypebyDataSize
(
    uint8                               ucDataSize
)
{
    CANDataLength_t uiDataLengthCode;

    uiDataLengthCode = CAN_DATA_LENGTH_0;

    if( ucDataSize <= 8U )
    {
        uiDataLengthCode = ( CANDataLength_t ) ucDataSize;
    }
    else if( ucDataSize <= 12U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_12;
    }
    else if( ucDataSize <= 16U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_16;
    }
    else if( ucDataSize <= 20U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_20;
    }
    else if( ucDataSize <= 24U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_24;
    }
    else if( ucDataSize <= 32U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_32;
    }
    else if( ucDataSize <= 48U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_48;
    }
    else if( ucDataSize <= 64U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_64;
    }
    else
    {
        uiDataLengthCode = CAN_DATA_LENGTH_64;
    }

    return uiDataLengthCode;
}

static CANErrorType_t CAN_MsgCopyRxMessage
(
    const CANRamRxBuffer_t *            psSrcReceivedMsg,
    CANMessage_t *                      psRxBuffer
)
{
    uint8           i;
    uint8           j;
    uint8           ucRemainLength;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ( psSrcReceivedMsg != NULL_PTR ) && ( psRxBuffer != NULL_PTR ) )
    {
        psRxBuffer->mErrorStateIndicator = ( uint8 ) psSrcReceivedMsg->rR0.rFReg.rfESI;
        psRxBuffer->mRemoteTransmitRequest = ( uint8 ) psSrcReceivedMsg->rR0.rFReg.rfRTR;
        psRxBuffer->mFDFormat = ( uint8 ) psSrcReceivedMsg->rR1.rFReg.rfFDF;
        psRxBuffer->mExtendedId = ( uint8 ) psSrcReceivedMsg->rR0.rFReg.rfXTD;

        if( psRxBuffer->mExtendedId == ( uint8 ) TRUE )
        {
            psRxBuffer->mId = ( uint32 ) psSrcReceivedMsg->rR0.rFReg.rfID;
        }
        else
        {
            psRxBuffer->mId = ( uint32 ) ( ( ( uint32 ) ( psSrcReceivedMsg->rR0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
        }

        psRxBuffer->mDataLength = CAN_MsgGetDataSizeBySizeType( ( CANDataLength_t ) psSrcReceivedMsg->rR1.rFReg.rfDLC );

        if( 0U < psRxBuffer->mDataLength )
        {
            ucRemainLength = psRxBuffer->mDataLength % 4U;

            for( i = 0U ; i < ( psRxBuffer->mDataLength / 4U ) ; i++ )
            {
                for( j = 0U ; j < 4U ; j++ )
                {
                    psRxBuffer->mData[ j + ( 4U * i ) ] = ( uint8 ) ( ( ( uint32 ) ( ( uint32 ) psSrcReceivedMsg->rData[ i ] >> ( j * 8U ) ) ) & 0xFFU );
                }
            }

            if( 0U < ucRemainLength )
            {
                for( j = 0U ; j < ucRemainLength ; j++ )
                {
                    psRxBuffer->mData[ j + ( 4U * i ) ] = ( uint8 ) ( ( ( uint32 ) ( ( uint32 )psSrcReceivedMsg->rData[ i ] >> ( j * 8U ) ) ) & 0xFFU );
                }
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

static CANMessage_t * CAN_MsgGetRxBufferFromRingBuffer
(
    uint8                               ucCh,
    CANRingBufferType_t                 ucType
)
{
    uint32          uiTotalNum;
    CANMessage_t *  psBuffer;

    uiTotalNum      = 0;
    psBuffer        = NULL_PTR;

    if( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE )
    {
        uiTotalNum = CAN_MsgGetCountOfRxMessage( ucCh );

        if( uiTotalNum < CAN_RX_MSG_RING_BUFFER_MAX )
        {
            if( ( uiTotalNum == 0UL ) || ( ucType == CAN_RING_BUFFER_LAST ) )
            {
                psBuffer = &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbTailIdx ];

                RxRingBufferManager[ ucCh ].rrbTailIdx++;

                if( RxRingBufferManager[ ucCh ].rrbMsgBufferMax <= RxRingBufferManager[ ucCh ].rrbTailIdx )
                {
                    RxRingBufferManager[ ucCh ].rrbTailIdx = 0;
                }
            }
            else
            {
                if( 0UL < RxRingBufferManager[ ucCh ].rrbHeadIdx )
                {
                    RxRingBufferManager[ ucCh ].rrbHeadIdx--;
                }
                else
                {
                    RxRingBufferManager[ ucCh ].rrbHeadIdx = RxRingBufferManager[ ucCh ].rrbMsgBufferMax - 1UL;
                }

                psBuffer = &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbHeadIdx ];
            }
        }
    }

    return psBuffer;
}

static CANErrorType_t CAN_MsgPutRxMessageToRxDBuffer
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
)
{
    boolean                     uiSavedFlag;
    uint8                       ucIndex;
    uint8                       i;
    uint8                       j;
    uint32                      uiRxMsgAddr;
    uint32                      uiCheckFlag;
    uint32                      uiNewMsgFlag[ 2 ];
    CANMessage_t *              psRxBuffer;
    const CANRamRxBuffer_t *    psReceivedMsg;
    const CANController_t *     psControllerInfo;
    CANErrorType_t              ret;

    uiSavedFlag                 = FALSE;
    uiCheckFlag                 = 0;
    psRxBuffer                  = NULL_PTR;
    psReceivedMsg               = NULL_PTR;
    ret                         = CAN_ERROR_NONE;

    psControllerInfo            = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];
    uiNewMsgFlag[ 0 ]           = psControllerInfo->cRegister->crNewData1.rNReg;
    uiNewMsgFlag[ 1 ]           = psControllerInfo->cRegister->crNewData2.rNReg;

    for( i = 0U ; i < 2U ; i++ )
    {
        for( j = 0U ; j < 32U ; j++ )
        {
            uiCheckFlag = ( uint32 ) 1UL << j;

            if( ( uiNewMsgFlag[ i ] & uiCheckFlag ) != 0UL )
            {
                ucIndex = ( j + ( 32U * i ) ) & 0xFFU;

                psRxBuffer = CAN_MsgGetRxBufferFromRingBuffer( ucCh, CAN_RING_BUFFER_LAST );

                if( ( psRxBuffer != NULL_PTR ) && ( psControllerInfo->cRamAddressInfo.raRxBufferAddr < CAN_NON_CACHE_MEMORY_END ) )
                {
                    psRxBuffer->mBufferIndex = ucIndex;
                    uiRxMsgAddr = psControllerInfo->cRamAddressInfo.raRxBufferAddr + ( sizeof( CANRamRxBuffer_t ) * ( ucIndex ) );
                    ( void ) SAL_MemCopy( &psReceivedMsg, &uiRxMsgAddr, sizeof( CANRamRxBuffer_t * ) );
                    psRxBuffer->mBufferType = CAN_RX_BUFFER_TYPE_DBUFFER;

                    ret = CAN_MsgCopyRxMessage( psReceivedMsg, psRxBuffer );

                    if( ret == CAN_ERROR_NONE )
                    {
                        uiSavedFlag = TRUE;
                    }
                }
                else if( psControllerInfo->cRamAddressInfo.raRxBufferAddr >= CAN_NON_CACHE_MEMORY_END )
                {
                    ret = CAN_ERROR_NOT_INIT;
                }
                else
                {
                    ret = CAN_ERROR_NO_BUFFER;
                }

                break;
            }
        }

        if( uiSavedFlag == TRUE )
        {
            break;
        }
    }

    if( uiSavedFlag == TRUE )
    {
        if( i == 0UL )
        {
            psControllerInfo->cRegister->crNewData1.rNReg = uiNewMsgFlag[ 0 ];
        }
        else
        {
            psControllerInfo->cRegister->crNewData2.rNReg = uiNewMsgFlag[ 1 ];
        }

        CAN_DrvCallbackNotifyRxEvent( ucCh, ( uint32 )psRxBuffer->mBufferIndex, uiBufferType, CAN_ERROR_NONE );
    }

    return ret;
}

static CANErrorType_t CAN_MsgPutRxMessageToRxFIFO
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
)
{
    boolean                     uiSavedFlag;
    uint8                       ucGetIdx;
    uint8                       ucFillLevel;
    uint8                       ucCnt;
    uint32                      uiRxFIFOSize;
    uint32                      uiRxMsgAddr;
    uint32                      uiRxFIFOAddr;
    CANRegFieldRXFxS_t          sRxFIFOStatus_bReg;
    CANRegFieldRXFxA_t *        psRxFIFOAcknowledge_bReg;
    CANMessage_t *              psRxBuffer;
    const CANRamRxBuffer_t *    psReceivedMsg;
    const CANController_t *     psControllerInfo;
    CANRingBufferType_t         ucRingBufferType;
    CANErrorType_t              ret;

    uiSavedFlag                 = FALSE;
    psRxBuffer                  = NULL_PTR;
    psReceivedMsg               = NULL_PTR;
    ret                         = CAN_ERROR_NONE;

    psControllerInfo            = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

    if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 )
    {
        sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO0Status.rFReg;
        psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO0Acknowledge.rFReg;
        uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO0Addr;
        uiRxFIFOSize = CAN_RX_FIFO_0_MAX;
    }
    else if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 )
    {
        sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO1Status.rFReg;
        psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO1Acknowledge.rFReg;
        uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO1Addr;
        uiRxFIFOSize = CAN_RX_FIFO_1_MAX;
    }
    else
    {
        /* unknown buffer type */
        ret = CAN_ERROR_NO_MESSAGE;
    }

    if( ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 ) || ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 ) )
    {
        ucGetIdx = ( uint8 ) sRxFIFOStatus_bReg.rfFxGI;
        ucFillLevel =  ( uint8 ) sRxFIFOStatus_bReg.rfFxFL;

        for(ucCnt = 0; ucCnt < ucFillLevel; ucCnt++)
        {
            if( ( uint8 ) psControllerInfo->cRegister->crHighPriorityMessageStatus.rFReg.rfBIDX == ucGetIdx )
            {
                ucRingBufferType = CAN_RING_BUFFER_FIRST;
            }
            else
            {
                ucRingBufferType = CAN_RING_BUFFER_LAST;
            }

            psRxBuffer = CAN_MsgGetRxBufferFromRingBuffer( ucCh, ucRingBufferType );

            if( ( psRxBuffer != NULL_PTR ) && ( uiRxFIFOAddr < CAN_NON_CACHE_MEMORY_END ) )
            {
                uiRxMsgAddr = uiRxFIFOAddr + ( sizeof( CANRamRxBuffer_t ) * ucGetIdx );
                (void) SAL_MemCopy( &psReceivedMsg, &uiRxMsgAddr, sizeof( CANRamRxBuffer_t * ) );
                psRxBuffer->mBufferType = uiBufferType;
                psRxBuffer->mBufferIndex = ucGetIdx;

                ret = CAN_MsgCopyRxMessage( psReceivedMsg, psRxBuffer );

                if( ret == CAN_ERROR_NONE )
                {
                    uiSavedFlag = TRUE;
                    psRxFIFOAcknowledge_bReg->rfFxAI = ucGetIdx;

                    ucGetIdx++;
                    if(ucGetIdx >= uiRxFIFOSize)
                    {
                        ucGetIdx = 0;
                    }
                }
            }
            else
            {
                ret = CAN_ERROR_NO_BUFFER;
            }
        }
    }

    if( uiSavedFlag == TRUE )
    {
        CAN_DrvCallbackNotifyRxEvent( ucCh, ( uint32 )psRxBuffer->mBufferIndex, uiBufferType, CAN_ERROR_NONE );
    }

    return ret;
}


CANErrorType_t CAN_MsgPutRxMessage
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
)
{
    CANErrorType_t  ret;

    ret             = CAN_ERROR_NOT_INIT;

    if( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE )
    {
        if( uiBufferType == CAN_RX_BUFFER_TYPE_DBUFFER )
        {
            ret = CAN_MsgPutRxMessageToRxDBuffer( ucCh, uiBufferType );
        }
        else
        {
            ret = CAN_MsgPutRxMessageToRxFIFO( ucCh, uiBufferType );
        }
    }

    return ret;
}

uint32 CAN_MsgGetCountOfRxMessage
(
    uint8                               ucCh
)
{
    uint32 ret;

    ret = 0;

    if( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE )
    {
        if( RxRingBufferManager[ ucCh ].rrbHeadIdx <= RxRingBufferManager[ ucCh ].rrbTailIdx )
        {
            ret = RxRingBufferManager[ ucCh ].rrbTailIdx - RxRingBufferManager[ ucCh ].rrbHeadIdx;
        }
        else
        {
            ret = ( CAN_RX_MSG_RING_BUFFER_MAX - RxRingBufferManager[ ucCh ].rrbHeadIdx ) + RxRingBufferManager[ ucCh ].rrbTailIdx;
        }
    }

    return ret;
}

CANErrorType_t CAN_MsgGetRxMessage
(
    uint8                               ucCh,
    CANMessage_t *                      psRxMsg
)
{
    uint32          uiMsgCount;
    CANErrorType_t  ret;

    uiMsgCount      = 0;
    ret             = CAN_ERROR_NONE;

    if( ( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE ) && ( psRxMsg != NULL_PTR ) )
    {
        uiMsgCount = CAN_MsgGetCountOfRxMessage( ucCh );

        if( 0UL < uiMsgCount )
        {
            ( void ) SAL_MemCopy( psRxMsg, &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbHeadIdx ], sizeof( CANMessage_t ) );

            RxRingBufferManager[ ucCh ].rrbHeadIdx++;

            if( CAN_RX_MSG_RING_BUFFER_MAX <= RxRingBufferManager[ ucCh ].rrbHeadIdx )
            {
                RxRingBufferManager[ ucCh ].rrbHeadIdx = 0;

                ret = CAN_ERROR_BAD_PARAM;
            }
        }
        else
        {
            ret = CAN_ERROR_NO_MESSAGE;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Tx Message */
static CANErrorType_t CAN_MsgCopyTxMessage
(
    CANRamTxBuffer_t *                  psTxMsg,
    const CANMessage_t *                psSrcMsg
)
{
    uint8           i;
    uint8           j;
    uint8           ucRemainLength;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ( psTxMsg != NULL_PTR ) && ( psSrcMsg != NULL_PTR ) )
    {
        psTxMsg->rT0.rFReg.rfXTD = psSrcMsg->mExtendedId;

        if( psSrcMsg->mExtendedId == ( uint8 ) TRUE )
        {
            psTxMsg->rT0.rFReg.rfID = psSrcMsg->mId;
        }
        else
        {
            psTxMsg->rT0.rFReg.rfID = ( SALReg32 ) ( ( psSrcMsg->mId & 0x7FFUL ) << 18UL );
        }

        psTxMsg->rT0.rFReg.rfRTR    = psSrcMsg->mRemoteTransmitRequest;
        psTxMsg->rT0.rFReg.rfESI    = psSrcMsg->mErrorStateIndicator;
        psTxMsg->rT1.rFReg.rfBRS    = psSrcMsg->mBitRateSwitching;
        psTxMsg->rT1.rFReg.rfEFC    = psSrcMsg->mEventFIFOControl;
        psTxMsg->rT1.rFReg.rfFDF    = psSrcMsg->mFDFormat;
        psTxMsg->rT1.rFReg.rfMM     = psSrcMsg->mMessageMarker;
        psTxMsg->rT1.rFReg.rfDLC    = (SALReg32)CAN_MsgGetSizeTypebyDataSize(psSrcMsg->mDataLength);

        if( 0U < psSrcMsg->mDataLength )
        {
            for( i = 0U ; i < ( psSrcMsg->mDataLength / 4U ) ; i++ )
            {
                psTxMsg->rData[ i ] = 0;

                for( j = 0U ; j < 4U ; j++)
                {
                    psTxMsg->rData[ i ] |= ( ( uint32 ) ( psSrcMsg->mData[ j + ( i * 4U ) ] ) << ( 8U * j ) );
                }
            }

            if( 0U < ( psSrcMsg->mDataLength % 4U ) )
            {
                psTxMsg->rData[ i ] = 0UL;

                ucRemainLength = psSrcMsg->mDataLength % 4U;

                for( j = 0U ; j < ucRemainLength ; j++ )
                {
                    psTxMsg->rData[ i ] |= ( ( uint32 ) ( psSrcMsg->mData[ j + ( i * 4U ) ] ) << ( 8U * j ) );
                }
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_MsgSetTxMessage
(
    const CANController_t *             psControllerInfo,
    const CANMessage_t *                psMsg,
    uint8 *                             pucTxBufferIndex
)
{
    uint8                           ucReqIndex;
    uint32                          uiTxMsgAddr;
    CANRamTxBuffer_t *              psTxMsg;
    const CANControllerRegister_t * psControllerReg;
    CANErrorType_t                  ret;

    psTxMsg = NULL_PTR;
    ret     = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL_PTR ) && ( psMsg != NULL_PTR ) )
    {
        if( ( psControllerInfo->cRegister != NULL_PTR ) && ( psControllerInfo->cRamAddressInfo.raTxBufferAddr < CAN_NON_CACHE_MEMORY_END ) )
        {
            psControllerReg = psControllerInfo->cRegister;

            if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_FIFO )
            {
                ucReqIndex  = ( uint8 ) psControllerReg->crTxFIFOOrQueueStatus.rFReg.rfTFQPI;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ucReqIndex );
                ( void ) SAL_MemCopy( &psTxMsg, &uiTxMsgAddr, sizeof( CANRamTxBuffer_t * ) );

                *pucTxBufferIndex = ucReqIndex;
            }
            else if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_QUEUE )
            {
                ucReqIndex  = (psMsg->mBufferIndex + ( uint8 ) psControllerReg->crTxFIFOOrQueueStatus.rFReg.rfTFQPI) & 0xFFU;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ucReqIndex );
                ( void ) SAL_MemCopy( &psTxMsg, &uiTxMsgAddr, sizeof( CANRamTxBuffer_t * ) );

                *pucTxBufferIndex = ucReqIndex;
            }
            else if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_DBUFFER )
            {
                ucReqIndex  = psMsg->mBufferIndex;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ucReqIndex );
                ( void ) SAL_MemCopy( &psTxMsg, &uiTxMsgAddr, sizeof( CANRamTxBuffer_t * ) );

                *pucTxBufferIndex = ucReqIndex;
            }
            else
            {
                /* unknown buffer type */
                ret = CAN_ERROR_NO_MESSAGE;
            }

            if( ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_FIFO ) || ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_QUEUE ) || ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_DBUFFER ) )
            {
                ( void ) CAN_MsgCopyTxMessage( psTxMsg, psMsg );

                if( psControllerInfo->cTxBufferInfo->tbTxFIFOorQueueMode != CAN_TX_BUFFER_MODE_QUEUE ) {
                    ret = CAN_MsgRequestTxMessage( psControllerInfo, ucReqIndex );
                } else {
                    ret = CAN_ERROR_NONE;
                }
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_MsgRequestTxMessage
(
    const CANController_t *             psControllerInfo,
    uint8                               ucReqIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crTxBufferAddRequest |= ( uint32 ) ( ( uint32 ) 1UL << ( ucReqIndex & 0x1FU ) );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Not used function */
/*
CANErrorType_t CAN_MsgRequestTxAllMessage
(
    CANController_t *                   psControllerInfo,
    uint32                              uiTxBufferAllIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crTxBufferAddRequest |= uiTxBufferAllIndex;
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}
*/

CANErrorType_t CAN_MsgRequestTxMessageCancellation
(
    const CANController_t *             psControllerInfo,
    uint8                               ucTxBufferIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crTxBufferCancellationRequest |= ( uint32 ) ( ( uint32 ) 1UL << ( ucTxBufferIndex & 0x1FU ) );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

uint32 CAN_MsgGetCountOfTxEvent
(
    uint8                               ucCh
)
{
    const CANController_t * psControllerInfo;
    uint32                  ret;

    ret = 0;

    psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

    if( psControllerInfo->cRegister != NULL_PTR )
    {
        ret = psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFFL;
    }

    return ret;
}

CANErrorType_t CAN_MsgGetTxEventMessage
(
    uint8                               ucCh,
    CANTxEvent_t *                      psTxEvtBuffer
)
{
    uint32                      uiIndex;
    uint32                      uiTxEvtMsgAddr;
    const CANController_t *     psControllerInfo;
    const CANRamTxEventFIFO_t * psSavedTxEvt;
    CANErrorType_t              ret;

    uiIndex                 = 0;
    psControllerInfo        = NULL_PTR;
    psSavedTxEvt            = NULL_PTR;
    ret                     = CAN_ERROR_NONE;

    if( psTxEvtBuffer != NULL_PTR )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFFL != 0UL )
            {
                uiIndex = psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFGI;
                uiTxEvtMsgAddr = psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr + ( sizeof( CANRamTxEventFIFO_t ) * uiIndex );
                ( void ) SAL_MemCopy( &psSavedTxEvt, &uiTxEvtMsgAddr, sizeof( CANRamTxEventFIFO_t * ) );

                if( psSavedTxEvt != NULL_PTR )
                {
                    psTxEvtBuffer->teExtendedId = ( uint8 ) psSavedTxEvt->rE0.rFReg.rfXTD;

                    if( psTxEvtBuffer->teExtendedId == 0U )
                    {
                        psTxEvtBuffer->teId = ( uint32 ) ( ( ( uint32 ) ( psSavedTxEvt->rE0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
                    }
                    else
                    {
                        psTxEvtBuffer->teId = ( uint32 ) ( psSavedTxEvt->rE0.rFReg.rfID );
                    }

                    psTxEvtBuffer->teRemoteTransmitRequest = ( uint8 ) psSavedTxEvt->rE0.rFReg.rfRTR;
                    psTxEvtBuffer->teFDFormat = ( uint8 ) psSavedTxEvt->rE1.rFReg.rfFDF;
                    psTxEvtBuffer->teBitRateSwitching = ( uint8 ) psSavedTxEvt->rE1.rFReg.rfBRS;
                    psTxEvtBuffer->teDataLength = CAN_MsgGetDataSizeBySizeType( ( CANDataLength_t ) psSavedTxEvt->rE1.rFReg.rfDLC );
                }

                psControllerInfo->cRegister->crTxEventFIFOAcknowledge.rFReg.rfEFAI = uiIndex;
            }
            else
            {
                ret = CAN_ERROR_NO_MESSAGE;
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )


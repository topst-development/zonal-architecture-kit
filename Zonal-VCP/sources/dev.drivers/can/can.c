// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can.c
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
#include "can_drv.h"
#include "can_msg.h"
#include "can_porting.h"
#include "can_par.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/



/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/



/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/



/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANErrorType_t CAN_Init
(
    void
)
{
    uint8               ucCh;
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL_PTR;
    ret                 = CAN_ERROR_NONE;

    for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
    {
        psControllerInfo = &CANDriverInfo.dControllerInfo[ ucCh ];
        psControllerInfo->cChannelHandle = ucCh;

        ( void ) CAN_PortingDisableControllerInterrupts( psControllerInfo->cChannelHandle );

        ret = CAN_DrvInitChannel( psControllerInfo );

        if( ret != CAN_ERROR_NONE )
        {
            CAN_D( "[CAN]CAN_Init Failed \r\n", psControllerInfo->cChannelHandle );
            break;
        }

        ( void ) CAN_PortingEnableControllerInterrupts( psControllerInfo->cChannelHandle );
    }

    if( ret == CAN_ERROR_NONE )
    {
        CAN_D( "[CAN]CAN_Init Completed \r\n", psControllerInfo->cChannelHandle );
    }

    return ret;
}

CANErrorType_t CAN_Deinit
(
    void
)
{
    uint8               ucCh;
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL_PTR;
    ret                 = CAN_ERROR_NONE;

    for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
    {
        psControllerInfo = &CANDriverInfo.dControllerInfo[ ucCh ];
        psControllerInfo->cChannelHandle = ucCh;

        ( void ) CAN_PortingDisableControllerInterrupts( psControllerInfo->cChannelHandle );

        ( void ) CAN_PortingResetDriver( psControllerInfo );

        ret = CAN_DrvDeinitChannel( psControllerInfo );

        if( ret != CAN_ERROR_NONE )
        {
            CAN_D( "[CAN]CAN_Deinit Failed \r\n", psControllerInfo->cChannelHandle );
            break;
        }
    }

    return ret;
}

CANErrorType_t CAN_InitMessage
(
    uint8                               ucCh
)
{
    const CANController_t * psControllerInfo;
    CANErrorType_t          ret;

    psControllerInfo        = NULL_PTR;
    ret                     = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];
        ( void ) CAN_MsgInit( psControllerInfo );
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

uint32 CAN_CheckNewRxMessage
(
    uint8                               ucCh
)
{
    uint32  ret;

    ret = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = CAN_MsgGetCountOfRxMessage( ucCh );
    }

    return ret;
}

CANErrorType_t CAN_GetNewRxMessage
(
    uint8                               ucCh,
    CANMessage_t *                      psMsg
)
{
    const CANController_t * psControllerInfo;
    CANErrorType_t          ret;

    psControllerInfo        = NULL_PTR;
    ret                     = CAN_ERROR_NONE;

    if( ( ucCh < CAN_CONTROLLER_NUMBER ) && ( psMsg != NULL_PTR ) )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        if( ( psControllerInfo->cMode == CAN_MODE_OPERATION )
         || ( psControllerInfo->cMode == CAN_MODE_MONITORING )
         || ( psControllerInfo->cMode == CAN_MODE_INTERNAL_TEST )
         || ( psControllerInfo->cMode == CAN_MODE_EXTERNAL_TEST ) )
        {
            ret = CAN_MsgGetRxMessage( ucCh, psMsg );
        }
        else
        {
            ret = CAN_ERROR_CONTROLLER_MODE;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_SendMessage
(
    uint8                               ucCh,
    const CANMessage_t *                psMsg,
    uint8 *                             pucTxBufferIndex
)
{
    const CANController_t * psControllerInfo;
    CANErrorType_t          ret;

    psControllerInfo        = NULL_PTR;
    ret                     = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        if( psMsg != NULL_PTR )
        {
            if( ( psControllerInfo->cMode == CAN_MODE_OPERATION )
             || ( psControllerInfo->cMode == CAN_MODE_MONITORING )
             || ( psControllerInfo->cMode == CAN_MODE_INTERNAL_TEST )
             || ( psControllerInfo->cMode == CAN_MODE_EXTERNAL_TEST ) )
            {
                ret = CAN_MsgSetTxMessage( psControllerInfo, psMsg, pucTxBufferIndex );
            }
            else
            {
                ret = CAN_ERROR_CONTROLLER_MODE;
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_RequestTxMessageCancellation
(
    uint8                               ucCh,
    uint8                               ucTxIndex
)
{
    const CANController_t * psControllerInfo;
    CANErrorType_t          ret;

    psControllerInfo        = NULL_PTR;
    ret                     = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        if( psControllerInfo->cMode == CAN_MODE_OPERATION )
        {
            ret = CAN_MsgRequestTxMessageCancellation( psControllerInfo, ucTxIndex );
        }
        else
        {
            ret = CAN_ERROR_CONTROLLER_MODE;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

uint32 CAN_CheckNewTxEvent
(
    uint8                               ucCh
)
{
    uint32  ret;

    ret = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = CAN_MsgGetCountOfTxEvent( ucCh );
    }

    return ret;
}

CANErrorType_t CAN_GetNewTxEvent
(
    uint8                               ucCh,
    CANTxEvent_t *                      psTxEvent
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = CAN_MsgGetTxEventMessage( ucCh, psTxEvent );
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_SetControllerMode
(
    uint8                               ucCh,
    CANMode_t                           ucControllerMode
)
{
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL_PTR;
    ret                 = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDriverInfo.dControllerInfo[ ucCh ];

        switch( ucControllerMode )
        {
            case CAN_MODE_NO_INITIALIZATION:
            {
                /* nothing */

                break;
            }

            case CAN_MODE_OPERATION:
            {
                ret = CAN_DrvSetNormalOperationMode( psControllerInfo );

                break;
            }

            case CAN_MODE_SLEEP:
            {
                ret = CAN_DrvSetSleepMode();

                break;
            }

            case CAN_MODE_WAKE_UP:
            {
                ret = CAN_DrvSetWakeUpMode();

                break;
            }

            case CAN_MODE_MONITORING:
            {
                ret = CAN_DrvSetMonitoringMode( psControllerInfo );

                break;
            }

            case CAN_MODE_RESET_CONTROLLER:
            {
                ret = CAN_DrvResetController( psControllerInfo );

                break;
            }

            case CAN_MODE_INTERNAL_TEST:
            {
                ret = CAN_DrvSetInternalTestMode( psControllerInfo );

                break;
            }

            case CAN_MODE_EXTERNAL_TEST:
            {
                ret = CAN_DrvSetExternalTestMode( psControllerInfo );

                break;
            }

            default:
            {
                ret = CAN_ERROR_BAD_PARAM;

                break;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANMode_t CAN_GetControllerMode
(
    uint8                               ucCh
)
{
    const CANController_t * psControllerInfo;
    CANMode_t               ret;

    psControllerInfo        = NULL_PTR;
    ret                     = CAN_MODE_NO_INITIALIZATION;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        ret = psControllerInfo->cMode;
    }

    return ret;
}

uint32 CAN_GetProtocolStatus
(
    uint8                               ucCh
)
{
    const CANController_t * psControllerInfo;
    uint32                  ret;

    psControllerInfo        = NULL_PTR;
    ret                     = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        ret = CAN_DrvGetProtocolStatus( psControllerInfo );
    }

    return ret;
}

CANErrorType_t CAN_RegisterCallbackFunctionTx
(
    CANNotifyTxEventCB                  pCbFnTx
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( pCbFnTx != NULL_PTR )
    {
        CANCallbackFunctions.cbNotifyTxEvent = pCbFnTx;
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_RegisterCallbackFunctionRx
(
    CANNotifyRxEventCB                  pCbFnRx
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( pCbFnRx != NULL_PTR )
    {
        CANCallbackFunctions.cbNotifyRxEvent = pCbFnRx;
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_RegisterCallbackFunctionError
(
    CANNotifyErrorEventCB               pCbFnError
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( pCbFnError != NULL_PTR )
    {
        CANCallbackFunctions.cbNotifyErrorEvent = pCbFnError;
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )


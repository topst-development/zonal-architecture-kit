// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_drv.c
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

#include <gic.h>
#include <clock.h>
#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "can_par.h"
#include "can_drv.h"
#include "can_msg.h"
#include "can_porting.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

CANDriver_t CANDriverInfo;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static void CAN_DrvInitChannelHW
(
    CANController_t *                   psControllerInfo
);

static uint32 CAN_DrvInitChannelRamBaseAddr
(
    const CANController_t *             psControllerInfo,
    uint32 *                            puiMemSize
);

static void CAN_DrvInitChannelRamAndBuffer
(
    CANController_t *                   psControllerInfo,
    uint32                              uiMemAddr,
    uint32                              uiMemSize
);

static void CAN_DrvInitChannelController
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetTimeStamp
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvSetTimeoutValue
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvSetWatchDog
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvSetControlConfig
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvCopyFilterList
(
    CANController_t *                   psControllerInfo,
    CANIdType_t                         uiFilterType
);

static CANErrorType_t CAN_DrvSetFilterConfig
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetBitrate
(
    const CANController_t *             psControllerInfo,
    CANBitRateTiming_t                  ucTimingType
);

static CANErrorType_t CAN_DrvInitTiming
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvStartConfigSetting
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvFinishConfigSetting
(
    const CANController_t *             psControllerInfo
);

static uint32 CAN_DrvGetSizeOfRamMemory
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvRegisterParameterAll
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvInitBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiMemory,
    uint32                              uiMemorySize
);

static CANErrorType_t CAN_DrvDeinitBuffer
(
    CANController_t *                   psControllerInfo
);

static uint32 CAN_DrvSetRamStandardIDFilter
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
);

static uint32 CAN_DrvSetRamExtendedIDFilter
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
);

static uint32 CAN_DrvSetRamRxFIFO0
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
);

static uint32 CAN_DrvSetRamRxFIFO1
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
);

static uint32 CAN_DrvSetRamRxBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
);

static uint32 CAN_DrvSetRamTxBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
);

static void CAN_DrvSetRamTxEventFIFO
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
);

static void CAN_DrvIRQHandler
(
    void *                              pArg
);

static CANErrorType_t CAN_DrvSetInterruptConfig
(
    CANController_t *                   psControllerInfo
);

static void CAN_DrvProcessIRQ
(
    uint8                               ucCh
);

static void CAN_DrvCallbackNotifyTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
);

static void CAN_DrvCallbackNotifyErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANController_t * CAN_DrvGetControllerInfo
(
    uint8                               ucCh
)
{
    CANController_t *   psControllerInfo;

    psControllerInfo    = NULL_PTR;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDriverInfo.dControllerInfo[ ucCh ];
    }

    return psControllerInfo;
}

static void CAN_DrvInitChannelHW
(
    CANController_t *                   psControllerInfo
)
{
    psControllerInfo->cRegister = CAN_PortingGetControllerRegister( psControllerInfo->cChannelHandle );

    ( void ) CAN_PortingInitHW( psControllerInfo );
    ( void ) CAN_PortingSetControllerClock( psControllerInfo, CAN_CONTROLLER_CLOCK );
    ( void ) CAN_PortingResetDriver( psControllerInfo );
}

static uint32 CAN_DrvInitChannelRamBaseAddr
(
    const CANController_t *             psControllerInfo,
    uint32 *                            puiMemSize
)
{
    uint8               ucCh;
    uint32              uiMemAddr;
    uint32              uiMemSize;
    CANRegBaseAddr_t *  psConfigBaseRegAddr;

    uiMemAddr           = 0UL;
    ucCh                = psControllerInfo->cChannelHandle;

    uiMemSize = CAN_DrvGetSizeOfRamMemory( psControllerInfo );

    if( 0UL < uiMemSize )
    {
        uiMemAddr = CAN_PortingAllocateNonCacheMemory( ucCh, uiMemSize );
    }

    if( uiMemAddr != 0UL )
    {
        psConfigBaseRegAddr = CAN_PortingGetMessageRamBaseAddr( ucCh );

        if( psConfigBaseRegAddr != NULL_PTR )
        {
            psConfigBaseRegAddr->rFReg.rfBASE_ADDR = ( SALReg32 ) ( ( uint32 ) ( uiMemAddr >> ( uint32 ) 16UL ) & ( uint32 ) 0xFFFFUL );
        }

        *puiMemSize = uiMemSize;
    }

    return uiMemAddr;
}

static void CAN_DrvInitChannelRamAndBuffer
(
    CANController_t *                   psControllerInfo,
    uint32                              uiMemAddr,
    uint32                              uiMemSize
)
{
    void *  pAddr;
    uint32  uiAddr;
    uint32  uiSize;

    pAddr   = NULL_PTR;
    uiAddr  = uiMemAddr;
    uiSize  = uiMemSize;

    ( void ) SAL_MemCopy( &pAddr, &uiAddr, sizeof( void * ) );

    ( void ) SAL_MemSet( ( void * ) pAddr, 0, uiSize );
    ( void ) CAN_DrvInitBuffer( psControllerInfo, uiAddr, uiSize );
    ( void ) CAN_MsgInit( psControllerInfo );
}

static void CAN_DrvInitChannelController
(
    CANController_t *                   psControllerInfo
)
{
    /* Set CAN FD */
    ( void ) CAN_DrvSetControlConfig( psControllerInfo );

    /* Set Filter */
    ( void ) CAN_DrvSetFilterConfig( psControllerInfo );

    /* Set Timing */
    ( void ) CAN_DrvInitTiming( psControllerInfo );

    /* Set Interrupt */
    ( void ) CAN_DrvSetInterruptConfig( psControllerInfo );

    /* Set TimeStamp */
    ( void ) CAN_DrvSetTimeStamp( psControllerInfo );

    /* Set TimeOut */
    ( void ) CAN_DrvSetTimeoutValue( psControllerInfo );

    /* Set WatchDog */
    ( void ) CAN_DrvSetWatchDog( psControllerInfo );
}


CANErrorType_t CAN_DrvInitChannel
(
    CANController_t *                   psControllerInfo
)
{
    uint32              uiMemAddr;
    uint32              uiMemSize;
    CANErrorType_t      ret;

    uiMemAddr           = 0UL;
    ret                 = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cMode == CAN_MODE_NO_INITIALIZATION ) )
    {
        CAN_DrvInitChannelHW( psControllerInfo );
        uiMemAddr = CAN_DrvInitChannelRamBaseAddr( psControllerInfo, &uiMemSize );

        if( uiMemAddr != 0UL )
        {
            ( void ) CAN_DrvRegisterParameterAll( psControllerInfo );

            /* Start configuration */
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            CAN_DrvInitChannelRamAndBuffer( psControllerInfo, uiMemAddr, uiMemSize );

            CAN_DrvInitChannelController( psControllerInfo );

            /* End configuration */
            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_OPERATION;
        }
        else
        {
            ret = CAN_ERROR_ALLOC;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_DrvDeinitChannel
(
    CANController_t *                   psControllerInfo
)
{
    uint8               ucCh;
    CANRegBaseAddr_t *  psConfigBaseRegAddr;
    CANErrorType_t      ret;

    psConfigBaseRegAddr = NULL_PTR;
    ret                 = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        /* Set Config */
        ucCh = psControllerInfo->cChannelHandle;

        /* Set HW_Init */
        ret = CAN_DrvStartConfigSetting( psControllerInfo );

        /* Set Buffer */
        if( ret == CAN_ERROR_NONE )
        {
            ( void ) CAN_MsgInit( psControllerInfo );
        }

        /* Free Memory */
        if( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr != 0UL )
        {
            CAN_PortingDeallocateNonCacheMemory( ucCh, ( uint32 * ) &( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr ) );
        }

        ( void ) CAN_DrvDeinitBuffer( psControllerInfo );

        /* Set Message RAM base address */
        psConfigBaseRegAddr = CAN_PortingGetMessageRamBaseAddr( ucCh );

        if( psConfigBaseRegAddr != NULL_PTR )
        {
            psConfigBaseRegAddr->rFReg.rfBASE_ADDR = 0;
        }

        psControllerInfo->cMode = CAN_MODE_NO_INITIALIZATION;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Config */
CANErrorType_t CAN_DrvSetNormalOperationMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            if( psControllerInfo->cMode == CAN_MODE_MONITORING )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfMON = CAN_BIT_DISABLE;
            }
            else if( psControllerInfo->cMode == CAN_MODE_INTERNAL_TEST )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = CAN_BIT_DISABLE;
                psControllerInfo->cRegister->crTest.rFReg.rfLBCK = CAN_BIT_DISABLE;
                psControllerInfo->cRegister->crCCControl.rFReg.rfMON = CAN_BIT_DISABLE;
            }
            else if( psControllerInfo->cMode == CAN_MODE_EXTERNAL_TEST )
            {
                psControllerInfo->cRegister->crTest.rFReg.rfLBCK = CAN_BIT_DISABLE;
                psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = CAN_BIT_DISABLE;
            }
            else
            {
                ; /* Nothing to do */
            }

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_OPERATION;

            CAN_D( "[CAN][Ch:%d] Set OPERATION Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvSetMonitoringMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfMON = CAN_BIT_ENABLE;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_MONITORING;

            CAN_D( "[CAN][Ch:%d] Set MONITORING Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvResetController
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    ( void ) CAN_Deinit();
    ( void ) CAN_Init();

    if( psControllerInfo != NULL_PTR )
    {
        psControllerInfo->cMode = CAN_MODE_OPERATION;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

CANErrorType_t CAN_DrvSetInternalTestMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = CAN_BIT_ENABLE;
            psControllerInfo->cRegister->crTest.rFReg.rfLBCK = CAN_BIT_ENABLE;
            psControllerInfo->cRegister->crCCControl.rFReg.rfMON = CAN_BIT_ENABLE;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_INTERNAL_TEST;

            CAN_D( "[CAN][Ch:%d] Set INTERNAL_TEST Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvSetExternalTestMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = CAN_BIT_ENABLE;
            psControllerInfo->cRegister->crTest.rFReg.rfLBCK = CAN_BIT_ENABLE;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_EXTERNAL_TEST;

            CAN_D( "[CAN][Ch:%d] Set External_TEST Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvSetWakeUpMode
(
    void
)
{
    uint8               ucCh;
    sint32              iTimeOutValue;
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    iTimeOutValue       = 400;
    ret                 = CAN_ERROR_NONE;

    for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
    {
        psControllerInfo = &CANDriverInfo.dControllerInfo[ ucCh ];
        psControllerInfo->cChannelHandle = ucCh;

        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( psControllerInfo->cMode == CAN_MODE_NO_INITIALIZATION )
            {
                ret = CAN_Init();
            }
            else if( psControllerInfo->cMode == CAN_MODE_SLEEP )
            {
                ( void ) CAN_PortingSetControllerClock( psControllerInfo, CAN_CONTROLLER_CLOCK );
                ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

                psControllerInfo->cRegister->crCCControl.rFReg.rfCSR = CAN_BIT_DISABLE;

                ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

                while( psControllerInfo->cRegister->crCCControl.rFReg.rfCSA == CAN_BIT_ENABLE )
                {
                    CAN_PortingDelay( 1 );

                    if( iTimeOutValue == 0 )
                    {
                        ret = CAN_ERROR_TIMEOUT;

                        break;
                    }

                    iTimeOutValue--;
                }
            }
            else
            {
                ; /* Nothing to do */
            }

            ( void ) CAN_PortingEnableControllerInterrupts( psControllerInfo->cChannelHandle );

            psControllerInfo->cMode = CAN_MODE_WAKE_UP;
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;

            break;
        }
    }

    return ret;
}

CANErrorType_t CAN_DrvSetSleepMode
(
    void
)
{
    uint8               ucCh;
    sint32              iTimeOutValue;
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    iTimeOutValue       = 200;
    ret                 = CAN_ERROR_NONE;

    for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
    {
        psControllerInfo = &CANDriverInfo.dControllerInfo[ ucCh ];
        psControllerInfo->cChannelHandle = ucCh;

        ( void ) CAN_PortingDisableControllerInterrupts( psControllerInfo->cChannelHandle );

        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( psControllerInfo->cMode == CAN_MODE_OPERATION )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfCSR = CAN_BIT_ENABLE;

                while( psControllerInfo->cRegister->crCCControl.rFReg.rfCSA == CAN_BIT_DISABLE )
                {
                    CAN_PortingDelay( 1 );

                    if( iTimeOutValue == 0 )
                    {
                        break;
                    }

                    iTimeOutValue--;
                }
            }
            else
            {
                ; /* Nothing to do */
            }

            psControllerInfo->cMode = CAN_MODE_SLEEP;
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;

            break;
        }
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetTimeStamp
(
    const CANController_t *             psControllerInfo
)
{
    CANRegExtTSCtrl0_t *    psExtTSCtrl0;
    CANRegExtTSCtrl1_t *    psEXTTSCtrl1;
    CANErrorType_t          ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( CAN_PortingGetTimeStampEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            if( psControllerInfo->cRegister != NULL_PTR )
            {
                psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTSS = 0x2;
                psExtTSCtrl0 = CAN_PortingGetConfigEXTS0Addr( psControllerInfo->cChannelHandle );
                if( psExtTSCtrl0 != NULL_PTR )
                {
                    psExtTSCtrl0->rFReg.rfENABLE = CAN_BIT_ENABLE;
                }

                psEXTTSCtrl1 = CAN_PortingGetConfigEXTS1Addr( psControllerInfo->cChannelHandle );
                if( psEXTTSCtrl1 != NULL_PTR )
                {
                    psEXTTSCtrl1->rFReg.rfEXT_TIMESTAMP_DIV_RATIO = CAN_TIMESTAMP_RATIO;
                    psEXTTSCtrl1->rFReg.rfEXT_TIMESTAMP_COMP = CAN_TIMESTAMP_COMP;
                }

                CAN_D( "[CAN]Set External TimeStamp \r\n", psControllerInfo->cChannelHandle );

            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetTimeoutValue
(
    const CANController_t *             psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( CAN_PortingGetTimeOutEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            if( psControllerInfo->cRegister != NULL_PTR )
            {
                if( psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTCP == 0UL )
                {
                    psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTCP = CAN_TIMESTAMP_PRESCALER;
                }

                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfTOP = CAN_TIMEOUT_VALUE;
                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfTOS = CAN_TIMEOUT_TYPE;
                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfETOC = CAN_BIT_ENABLE;
            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetWatchDog
(
    const CANController_t *             psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( CAN_PortingGetWatchDogEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            if( psControllerInfo->cRegister != NULL_PTR )
            {
                psControllerInfo->cRegister->crRAMWatchdog.rFReg.rfWDC = CAN_WATCHDOG_VALUE;
            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetControlConfig
(
    const CANController_t *             psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( CAN_PortingGetFDEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfFDOE = CAN_BIT_ENABLE;
            }
            if( CAN_PortingGetBitRateSwitchEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfBRSE = CAN_BIT_ENABLE;
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

static CANErrorType_t CAN_DrvCopyFilterList
(
    CANController_t *                   psControllerInfo,
    CANIdType_t                         uiFilterType
)
{
    uint8                           ucCh;
    uint8                           ucNumOfFilter;
    uint32                          uiTempAddr;
    const CANIdFilterList_t *       psFilterList;
    CANRamStdIDFilterElement_t *    psStdFilterElement;
    CANRamExtIDFilterElement_t *    psExtFilterElement;
    CANErrorType_t                  ret;

    ucCh                = 0;
    uiTempAddr          = 0;
    psFilterList        = NULL_PTR;
    psStdFilterElement  = NULL_PTR;
    psExtFilterElement  = NULL_PTR;
    ret                 = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        ucCh = psControllerInfo->cChannelHandle;

        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( uiFilterType == CAN_ID_TYPE_STANDARD )
            {
                if( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr != 0UL )
                {
                    ( void ) SAL_MemCopy( &psStdFilterElement, ( const void * ) &psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr, sizeof( uint32 ) );
                    psFilterList = ( CANIdFilterList_t * ) StandardIDFilterPar.idfList[ ucCh ];

                    if( ( psFilterList != NULL_PTR ) && ( psStdFilterElement != NULL_PTR ) )
                    {
                        for( ucNumOfFilter = 0U ; ucNumOfFilter < CAN_STANDARD_ID_FILTER_NUMBER ; ucNumOfFilter++ )
                        {
                            psStdFilterElement->rFReg.rfSFT      = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterType ) & 0x3UL );
                            psStdFilterElement->rFReg.rfSFEC     = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterElementConfiguration ) & 0x7UL );
                            psStdFilterElement->rFReg.rfSFID1    = ( SALReg32 ) ( psFilterList->idflFilterID1 & 0x7FFUL );
                            psStdFilterElement->rFReg.rfSFID2    = ( SALReg32 ) ( psFilterList->idflFilterID2 & 0x7FFUL );

                            uiTempAddr  = ( uint32 ) psStdFilterElement;
                            uiTempAddr  += sizeof( CANRamStdIDFilterElement_t );
                            ( void ) SAL_MemCopy( &psStdFilterElement, ( const void * ) &uiTempAddr, sizeof( uint32 ) );

                            uiTempAddr  = ( uint32 ) psFilterList;
                            uiTempAddr  += sizeof( CANIdFilterList_t );
                            ( void ) SAL_MemCopy( &psFilterList, ( const void * ) &uiTempAddr, sizeof( uint32 ) );
                        }

                        psControllerInfo->cNumOfStdIDFilterList = ucNumOfFilter;
                        psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = ucNumOfFilter;
                    }
                }
                else
                {
                    ret = CAN_ERROR_NOT_INIT;
                }
            }
            else if( uiFilterType == CAN_ID_TYPE_EXTENDED )
            {
                if( psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr != 0UL )
                {
                    ( void ) SAL_MemCopy( &psExtFilterElement, ( const void * ) &psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr, sizeof( uint32 ) );
                    psFilterList = ( CANIdFilterList_t * ) ExtendedIDFilterPar.idfList[ ucCh ];

                    if( ( psFilterList != NULL_PTR ) && ( psExtFilterElement != NULL_PTR ) )
                    {
                        for( ucNumOfFilter = 0U ; ucNumOfFilter < CAN_EXTENDED_ID_FILTER_NUMBER ; ucNumOfFilter++ )
                        {
                            psExtFilterElement->rF0.rFReg.rfEFEC    = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterElementConfiguration ) & 0x7UL );
                            psExtFilterElement->rF1.rFReg.rfEFT     = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterType ) & 0x3UL );
                            psExtFilterElement->rF0.rFReg.rfEFID1   = ( SALReg32 ) ( psFilterList->idflFilterID1 & 0x1FFFFFFFUL );
                            psExtFilterElement->rF1.rFReg.rfEFID2   = ( SALReg32 ) ( psFilterList->idflFilterID2 & 0x1FFFFFFFUL );

                            uiTempAddr  = ( uint32 ) psExtFilterElement;
                            uiTempAddr  += sizeof( CANRamExtIDFilterElement_t );
                            ( void ) SAL_MemCopy( &psExtFilterElement, ( const void * ) &uiTempAddr, sizeof( uint32 ) );

                            uiTempAddr  = ( uint32 ) psFilterList;
                            uiTempAddr  += sizeof( CANIdFilterList_t );
                            ( void ) SAL_MemCopy( &psFilterList, ( const void * ) &uiTempAddr, sizeof( uint32 ) );
                        }

                        psControllerInfo->cNumOfExtIDFilterList = ucNumOfFilter;
                        psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = ucNumOfFilter;
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

static CANErrorType_t CAN_DrvSetFilterConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                ret = CAN_DrvCopyFilterList( psControllerInfo, CAN_ID_TYPE_STANDARD );
            }

            if( ret == CAN_ERROR_NONE )
            {
                if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
                {
                    ret = CAN_DrvCopyFilterList( psControllerInfo, CAN_ID_TYPE_EXTENDED );
                }
            }

            /* Reject Remote Frames Standard */
            if( CAN_PortingGetStandardIDRemoteRejectEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFS = CAN_BIT_ENABLE;
            }
            else
            {
                psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFS = CAN_BIT_DISABLE;
            }

            /* Reject Remote Frames Extended */
            if( CAN_PortingGetExtendedIDRemoteRejectEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFE = CAN_BIT_ENABLE;
            }
            else
            {
                psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFE = CAN_BIT_DISABLE;
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

/* Timing */
static CANErrorType_t CAN_DrvSetBitrate
(
    const CANController_t *             psControllerInfo,
    CANBitRateTiming_t                  ucTimingType
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( ucTimingType == CAN_BIT_RATE_TIMING_ARBITRATION )
        {
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNSJW    = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpSJW ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNBRP    = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpBRP ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNTSEG1  = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPROP ) + ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPhaseSeg1 ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNTSEG2  = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPhaseSeg2 ) - 1UL;
        }
        else if( ucTimingType == CAN_BIT_RATE_TIMING_DATA )
        {
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDSJW   = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpSJW ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDBRP   = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpBRP ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDTSEG1 = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpPROP ) + ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpPhaseSeg1 ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDTSEG2 = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpPhaseSeg2 ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfTDC    = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDC );
            psControllerInfo->cRegister->crTransmitterDelayCompensation.rFReg.rfTDCO = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDCOffset );
            psControllerInfo->cRegister->crTransmitterDelayCompensation.rFReg.rfTDCF = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDCFilterWindow );

            psControllerInfo->cRegister->crCCControl.rFReg.rfBRSE = CAN_BIT_ENABLE;
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvInitTiming
(
    const CANController_t *             psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cArbiPhaseTimeInfo != NULL_PTR )
        {
            /* Arbitration Phase Bit Timing */
            ( void ) CAN_DrvSetBitrate( psControllerInfo, CAN_BIT_RATE_TIMING_ARBITRATION );

            /* Data Phase Bit Timing */
            if( CAN_PortingGetBitRateSwitchEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                if( psControllerInfo->cDataPhaseTimeInfo != NULL_PTR )
                {
                    ( void ) CAN_DrvSetBitrate( psControllerInfo, CAN_BIT_RATE_TIMING_DATA );
                }
            }
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvStartConfigSetting
(
    const CANController_t *             psControllerInfo
)
{
    uint32          uiCnt;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crCCControl.rFReg.rfINIT = 1;

            for( uiCnt = 0UL ; uiCnt < 0x100UL ; uiCnt++ ) {
                ;
            }

            psControllerInfo->cRegister->crCCControl.rFReg.rfCCE = 1;
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

static CANErrorType_t CAN_DrvFinishConfigSetting
(
    const CANController_t *             psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crCCControl.rFReg.rfCCE = 0;
            psControllerInfo->cRegister->crCCControl.rFReg.rfINIT = 0;
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

static uint32 CAN_DrvGetSizeOfRamMemory
(
    const CANController_t *             psControllerInfo
)
{
    uint32 uiStdIDFilterListSize;
    uint32 uiExtIDFilterListSize;
    uint32 uiRxFIFO0Size;
    uint32 uiRxFIFO1Size;
    uint32 uiRxBufferSize;
    uint32 uiTxEvtFIFOSize;
    uint32 uiTxBufferSize;
    uint32 ret;

    ret = 0;

    if( psControllerInfo != NULL_PTR )
    {
        /* StandardIDFilterListSize */
        uiStdIDFilterListSize = CAN_PortingGetSizeofStandardIDFilterList( psControllerInfo->cChannelHandle );

        /* ExtendedIDFilterListSize */
        uiExtIDFilterListSize = CAN_PortingGetSizeofExtendedIDFilterList( psControllerInfo->cChannelHandle );

        /* RxFIFO0Size */
        uiRxFIFO0Size = CAN_PortingGetSizeofRxFIFO0( psControllerInfo->cChannelHandle );

        /* RxFIFO1Size */
        uiRxFIFO1Size = CAN_PortingGetSizeofRxFIFO1( psControllerInfo->cChannelHandle );

        /* RxBufferSize */
        uiRxBufferSize = CAN_PortingGetSizeofRxBuffer( psControllerInfo->cChannelHandle );

        /* TxEventFIFOSize */
        uiTxEvtFIFOSize = CAN_PortingGetSizeofTxEventBuffer( psControllerInfo->cChannelHandle );

        /* TxBufferSize */
        uiTxBufferSize = CAN_PortingGetSizeofTxBuffer( psControllerInfo->cChannelHandle );

        ret = uiStdIDFilterListSize + uiExtIDFilterListSize + uiRxFIFO0Size + uiRxFIFO1Size + uiRxBufferSize + uiTxEvtFIFOSize + uiTxBufferSize;
    }

    return ret;
}

static CANErrorType_t CAN_DrvRegisterParameterAll
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        /* ArbitrationPhaseTiming */
        psControllerInfo->cArbiPhaseTimeInfo = ( CANTimingParam_t * ) &ArbitrationPhaseTimingPar[ psControllerInfo->cChannelHandle ];

        /* DATAPhaseTiming */
        psControllerInfo->cDataPhaseTimeInfo = ( CANTimingParam_t * ) &DataPhaseTimingPar[ psControllerInfo->cChannelHandle ];

        /* TxBufferInfo */
        psControllerInfo->cTxBufferInfo = ( CANTxBuffer_t * ) &TxBufferInfoPar[ psControllerInfo->cChannelHandle ];

        /* RxDedicatedBufferInfo */
        psControllerInfo->cDedicatedBufferInfo = ( CANRxBuffer_t * ) &RxBufferInfoPar[ psControllerInfo->cChannelHandle ];

        /* RxBufferFIFO0Info */
        psControllerInfo->cFIFO0BufferInfo = ( CANRxBuffer_t * ) &RxBufferFIFO0InfoPar[ psControllerInfo->cChannelHandle ];

        /* RxBufferFIFO1Info */
        psControllerInfo->cFIFO1BufferInfo = ( CANRxBuffer_t * ) &RxBufferFIFO1InfoPar[ psControllerInfo->cChannelHandle ];

        psControllerInfo->cCallbackFunctions = ( CANCallBackFunc_t * ) &CANCallbackFunctions;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Set Buffer */
static CANErrorType_t CAN_DrvInitBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiMemory,
    uint32                              uiMemorySize
)
{
    uint32          uiStartAddr;
    CANErrorType_t  ret;

    uiStartAddr     = 0UL;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( ( uiMemory != 0UL ) && ( 0UL < uiMemorySize ) )
        {
            uiStartAddr = uiMemory;

            if( psControllerInfo->cRegister != NULL_PTR )
            {
                /* StandardIDFilterList */
                uiStartAddr += CAN_DrvSetRamStandardIDFilter(psControllerInfo, uiStartAddr);

                /* ExtendedIDFilterList */
                uiStartAddr += CAN_DrvSetRamExtendedIDFilter(psControllerInfo, uiStartAddr);

                /* Rx FIFO0 */
                uiStartAddr += CAN_DrvSetRamRxFIFO0(psControllerInfo, uiStartAddr);

                /* Rx FIFO1 */
                uiStartAddr += CAN_DrvSetRamRxFIFO1(psControllerInfo, uiStartAddr);

                /* Rx Dedicated Buffer */
                uiStartAddr += CAN_DrvSetRamRxBuffer(psControllerInfo, uiStartAddr);

                /* Tx Buffer */
                uiStartAddr += CAN_DrvSetRamTxBuffer(psControllerInfo, uiStartAddr);

                /* Tx Event FIFO */
                CAN_DrvSetRamTxEventFIFO(psControllerInfo, uiStartAddr);
            }
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvDeinitBuffer
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cRegister != NULL_PTR ) )
    {
        /* StandardIDFilterListSize */
        if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr = 0;

            psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = 0;
            psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfFLSSA = 0;
        }

        /* ExtendedIDFilterListSize */
        if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr = 0;

            psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = 0;
            psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfFLESA = 0;
        }

        /* RxBufferFIFO0Info */
        if( psControllerInfo->cFIFO0BufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raRxFIFO0Addr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF0DS = 0;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO0BufferInfo->rbOperationMode;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxWM = 0xA; //Watermark
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxS = 0;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxSA = 0;
        }
        /* RxBufferFIFO1Info */
        if( psControllerInfo->cFIFO1BufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raRxFIFO1Addr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF1DS = 0;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO1BufferInfo->rbOperationMode;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxWM = 0xA; //Watermark
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxS = 0;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxSA = 0;
        }

        /* RxDedicatedBufferInfo */
        if( psControllerInfo->cDedicatedBufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raRxBufferAddr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfRBDS = 0;
            psControllerInfo->cRegister->crRxBufferConfiguration.rFReg.rfRBSA = 0;
        }

        /* TxBufferInfo */
        if( psControllerInfo->cTxBufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raTxBufferAddr = 0;

            psControllerInfo->cRegister->crTxBufferElementSizeConfiguration.rFReg.rfTBDS = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQM = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQS = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfNDTB = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTBSA = 0;

            if( CAN_PortingGetTxEventFIFOEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr = 0;

                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFWM = 0;
                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFS = 0;
                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFSA = 0;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static uint32 CAN_DrvSetRamStandardIDFilter
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
)
{
    uint32  uiSize;

    uiSize  = 0UL;

    if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
    {
        psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr = uiRamAddr;

        psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = 0;
        psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfFLSSA = ( uiRamAddr & 0xFFFFUL ) >> 2UL;

        uiSize = CAN_PortingGetSizeofStandardIDFilterList( psControllerInfo->cChannelHandle );
    }

    return uiSize;
}

static uint32 CAN_DrvSetRamExtendedIDFilter
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
)
{
    uint32  uiSize;

    uiSize  = 0UL;

    if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
    {
        psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr = uiRamAddr;

        psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = 0;
        psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfFLESA = ( uiRamAddr & 0xFFFFUL ) >> 2UL;

        uiSize = CAN_PortingGetSizeofExtendedIDFilterList( psControllerInfo->cChannelHandle );
    }

    return uiSize;
}

static uint32 CAN_DrvSetRamRxFIFO0
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
)
{
    uint32  uiSize;

    uiSize  = 0UL;

    if( psControllerInfo->cFIFO0BufferInfo != NULL_PTR )
    {
        psControllerInfo->cRamAddressInfo.raRxFIFO0Addr = uiRamAddr;

        psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF0DS = ( SALReg32 ) psControllerInfo->cFIFO0BufferInfo->rbDataFieldSize;
        psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO0BufferInfo->rbOperationMode;
        psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxWM = 0xA; //Watermark
        psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxS = psControllerInfo->cFIFO0BufferInfo->rbNumberOfElement;
        psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxSA = ( uiRamAddr & 0xFFFFUL ) >> 2UL;

        uiSize = CAN_PortingGetSizeofRxFIFO0( psControllerInfo->cChannelHandle );
    }

    return uiSize;
}

static uint32 CAN_DrvSetRamRxFIFO1
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
)
{
    uint32  uiSize;

    uiSize  = 0UL;

    if( psControllerInfo->cFIFO1BufferInfo != NULL_PTR )
    {
        psControllerInfo->cRamAddressInfo.raRxFIFO1Addr = uiRamAddr;

        psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF1DS = ( SALReg32 ) psControllerInfo->cFIFO1BufferInfo->rbDataFieldSize;
        psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO1BufferInfo->rbOperationMode;
        psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxWM = 0xA; //Watermark
        psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxS = psControllerInfo->cFIFO1BufferInfo->rbNumberOfElement;
        psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxSA = ( uiRamAddr & 0xFFFFUL ) >> 2UL;

        uiSize = CAN_PortingGetSizeofRxFIFO1( psControllerInfo->cChannelHandle );
    }

    return uiSize;
}

static uint32 CAN_DrvSetRamRxBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
)
{
    uint32  uiSize;

    uiSize  = 0UL;

    if( psControllerInfo->cDedicatedBufferInfo != NULL_PTR )
    {
        psControllerInfo->cRamAddressInfo.raRxBufferAddr = uiRamAddr;

        psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfRBDS = ( SALReg32 ) psControllerInfo->cDedicatedBufferInfo->rbDataFieldSize;
        psControllerInfo->cRegister->crRxBufferConfiguration.rFReg.rfRBSA = ( uiRamAddr & 0xFFFFUL ) >> 2UL;

        uiSize = CAN_PortingGetSizeofRxBuffer( psControllerInfo->cChannelHandle );
    }

    return uiSize;
}

static uint32 CAN_DrvSetRamTxBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
)
{
    uint32  uiSize;

    uiSize  = 0UL;

    if( psControllerInfo->cTxBufferInfo != NULL_PTR )
    {
        psControllerInfo->cRamAddressInfo.raTxBufferAddr = uiRamAddr;

        psControllerInfo->cRegister->crTxBufferElementSizeConfiguration.rFReg.rfTBDS = psControllerInfo->cTxBufferInfo->tbInterruptConfig;
        psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQM = ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbTxFIFOorQueueMode;
        psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQS = psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
        psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfNDTB = psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer;
        psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTBSA = ( uiRamAddr & 0xFFFFUL ) >> 2UL;

        uiSize = CAN_PortingGetSizeofTxBuffer( psControllerInfo->cChannelHandle );
    }

    return uiSize;
}

static void CAN_DrvSetRamTxEventFIFO
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiRamAddr
)
{
    if( ( psControllerInfo->cTxBufferInfo != NULL_PTR ) && ( CAN_PortingGetTxEventFIFOEnable( psControllerInfo->cChannelHandle ) == TRUE ) )
    {
        psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr = uiRamAddr;

        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFWM = ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer + ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFS = ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer + ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFSA = ( uiRamAddr & 0xFFFFUL ) >> 2UL;

        ( void ) CAN_PortingGetSizeofTxEventBuffer( psControllerInfo->cChannelHandle );
    }
}

/* ERROR */
uint32 CAN_DrvGetProtocolStatus
(
    const CANController_t *             psControllerInfo
)
{
    uint32 ret;

    ret = 0;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ret = ( uint32 ) psControllerInfo->cRegister->crProtocolStatusRegister.rNReg;
        }
    }

    return ret;
}

/*  IRQ  */
static void CAN_DrvIRQHandler
(
    void *                              pArg
)
{
    uint8 ucCh;
    const CANController_t * psControllerInfo;

    ucCh                    = 0U;
    psControllerInfo        = NULL_PTR;

    if( pArg != NULL_PTR )
    {
        ( void ) SAL_MemCopy( &ucCh, ( const void * ) pArg, sizeof( uint8 ) );

        if( ucCh < CAN_CONTROLLER_NUMBER )
        {
            psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

            if( ( psControllerInfo->cMode != CAN_MODE_NO_INITIALIZATION ) && ( psControllerInfo->cMode != CAN_MODE_SLEEP ) )
            {
                CAN_DrvProcessIRQ( ucCh );
            }
        }
    }
}

static CANErrorType_t CAN_DrvSetInterruptConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        psControllerInfo->cRegister->crInterruptEnable.rNReg = CAN_INTERRUPT_ENABLE;
        psControllerInfo->cRegister->crInterruptLineEnable.rNReg = CAN_INTERRUPT_LINE_ENABLE;
        psControllerInfo->cRegister->crInterruptLineSelect.rNReg = CAN_INTERRUPT_LINE_SEL;

        psControllerInfo->cRegister->crTxBufferTransmissionInterruptEnable = 0xFFFFFFFFUL;
        psControllerInfo->cRegister->crTxBufferCancellationFinishedInterruptEnable = 0xFFFFFFFFUL;

        ( void ) CAN_PortingSetInterruptHandler( psControllerInfo, &CAN_DrvIRQHandler );
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static void CAN_DrvProcessIRQ
(
    uint8                               ucCh
)
{
    const CANController_t * psControllerInfo;
    CANRegFieldIR_t         sIRStatus;

    psControllerInfo = NULL_PTR;

    psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

    sIRStatus = psControllerInfo->cRegister->crInterruptRegister.rFReg;

    // 29 : Access to Reserved Address
    if( sIRStatus.rfARA != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Access to Reserved Address \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS );
    }

    // 28 : Protocol Error in Data Phase
    if( sIRStatus.rfPED != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Protocol Error in Data Phase \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );
    }

    // 27 : Protocol Error in Arbitration Phase
    if( sIRStatus.rfPEA != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Protocol Error in Arbitration Phase \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );
    }

    // 26 : Watchdog Interrupt
    if( sIRStatus.rfWDI != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Watchdog Interrupt \r\n", ucCh );
    }

    // 25 : Bus_Off Status
    if( sIRStatus.rfBO != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Bus_Off Status \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BUS_OFF );
    }

    // 24 : Warning Status
    if( sIRStatus.rfEW != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Warning Status \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_WARNING );
    }

    // 23 : Error Passive
    if( sIRStatus.rfEP != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Error Passive \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PASSIVE );
    }

    // 22 : Error Logging Overflow
    if( sIRStatus.rfELO != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Error Logging Overflow \r\n", ucCh );
    }

    // 21 : Bit Error Uncorrected
    if( sIRStatus.rfBEU != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Bit Error Uncorrected \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );
    }

    // 20 : Bit Error Corrected
    if( sIRStatus.rfBEC != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Bit Error Corrected \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );
    }

    // 19 : Message stored to Dedicated Rx Buffer
    if( sIRStatus.rfDRX != 0UL )
    {
        ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_DBUFFER );
    }

    // 18 : Timeout Occurred
    if( sIRStatus.rfTOO != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Timeout Occurred \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TIMEOUT );
    }

    // 17 : Message RAM Access Failure
    if( sIRStatus.rfMRAF != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Message RAM Access Failure \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_RAM_ACCESS_FAIL );
    }

    // 16 : Timestamp Wraparound
    if( sIRStatus.rfTSW != 0UL )
    {
        /* Nothing to do */
    }

    // 15 : Tx Event FIFO Element Lost
    if( sIRStatus.rfTEFL != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Tx Event FIFO Element Lost \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_LOST );
    }

    // 14 : Tx Event FIFO Full
    if( sIRStatus.rfTEFF != 0UL )
    {
        CAN_D( "[CAN Channel : %d] Tx Event FIFO Full \r\n", ucCh );

        CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_FULL );
    }

    // 13 : Tx Event FIFO Watermark Reached
    if( sIRStatus.rfTEFW != 0UL )
    {
        /* Nothing to do */
    }

    // 12 : Tx Event FIFO New Entry
    if( sIRStatus.rfTEFN != 0UL )
    {
        CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_NEW_EVENT );
    }

    // 11 : Tx FIFO Empty
    if( sIRStatus.rfTFE != 0UL )
    {
        /* Nothing to do */
    }

    // 10 : Transmission Cancellation Finished
    if( sIRStatus.rfTCF != 0UL )
    {
        CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_CANCEL_FINISHED );
    }

    // 9 : Transmission Completed
    if( sIRStatus.rfTC != 0UL )
    {
        CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_COMPLETED );
    }

    // 8 : High Priority Message
    if( sIRStatus.rfHPM != 0UL )
    {
        CAN_D( "[CAN Channel : %d] High Priority Message \r\n", ucCh );
    }

    // 7 : Rx FIFO 1 Message Lost
    if( sIRStatus.rfRF1L != 0UL )
    {
        CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );
    }

    // 6 : Rx FIFO 1 Full
    if( sIRStatus.rfRF1F != 0UL )
    {
        CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );
    }

    // 5 : Rx FIFO 1 Watermark Reached
    if( sIRStatus.rfRF1W != 0UL )
    {
        /* Nothing to do */
    }

    // 4 : Rx FIFO 1 New Message
    if( sIRStatus.rfRF1N != 0UL )
    {
        ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO1 );
    }

    // 3 : Rx FIFO 0 Message Lost
    if( sIRStatus.rfRF0L != 0UL )
    {
        CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );
    }

    // 2 : Rx FIFO 0 Full
    if( sIRStatus.rfRF0F != 0UL )
    {
        CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );
    }

    // 1 : Rx FIFO 0 Watermark Reached
    if( sIRStatus.rfRF0W != 0UL )
    {
        /* Nothing to do */
    }

    // 0 : Rx FIFO 0 New Message
    if( sIRStatus.rfRF0N != 0UL )
    {
        ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO0 );
    }

    psControllerInfo->cRegister->crInterruptRegister.rFReg = sIRStatus;
}

static void CAN_DrvCallbackNotifyTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
)
{
    const CANController_t * psControllerInfo;

    psControllerInfo = NULL_PTR;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        if( psControllerInfo->cCallbackFunctions != NULL_PTR )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyTxEvent != NULL_PTR )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyTxEvent( ucCh, uiIntType );
            }
        }
    }
}

void CAN_DrvCallbackNotifyRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
)
{
    const CANController_t * psControllerInfo;

    psControllerInfo = NULL_PTR;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        if( psControllerInfo->cCallbackFunctions != NULL_PTR )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyRxEvent != NULL_PTR )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyRxEvent( ucCh, uiRxIndex, uiRxBufferType, uiError );
            }
        }
    }
}

static void CAN_DrvCallbackNotifyErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
)
{
    const CANController_t * psControllerInfo;

    psControllerInfo = NULL_PTR;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = ( const CANController_t * ) &CANDriverInfo.dControllerInfo[ ucCh ];

        if( psControllerInfo->cCallbackFunctions != NULL_PTR )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyErrorEvent != NULL_PTR )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyErrorEvent( ucCh, uiError );
            }
        }
    }
}

/* Not used function */
/*
CANErrorType_t CAN_DrvSetDisableAutomaticRetransmission
(
    CANController_t *                   psControllerInfo,
    boolean bDisable
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfDAR = bDisable;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}
*/

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )


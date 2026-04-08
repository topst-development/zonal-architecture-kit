// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_drv.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CAN_DRV_HEADER
#define MCU_BSP_CAN_DRV_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

#define CAN_BIT_DISABLE                 (0U)
#define CAN_BIT_ENABLE                  (1U)


typedef struct CANRamAddress
{
    uint32                              raStandardIDFilterListAddr;
    uint32                              raExtendedIDFilterListAddr;
    uint32                              raRxFIFO0Addr;
    uint32                              raRxFIFO1Addr;
    uint32                              raRxBufferAddr;
    uint32                              raTxEventFIFOAddr;
    uint32                              raTxBufferAddr;
} CANRamAddress_t;

typedef struct CANController
{
    /* Config */
    CANMode_t                           cMode;
    uint8                               cChannelHandle;
    uint32                              cFrequency;

    /* Timing */
    CANTimingParam_t *                  cArbiPhaseTimeInfo;
    CANTimingParam_t *                  cDataPhaseTimeInfo;

    /* Buffer */
    CANTxBuffer_t *                     cTxBufferInfo;
    CANRxBuffer_t *                     cDedicatedBufferInfo;
    CANRxBuffer_t *                     cFIFO0BufferInfo;
    CANRxBuffer_t *                     cFIFO1BufferInfo;

    /*Filter */
    uint8                               cNumOfStdIDFilterList;
    CANIdFilterList_t *                 cStdIDFilterList;
    uint8                               cNumOfExtIDFilterList;
    CANIdFilterList_t *                 cExtIDFilterList;

    /* Register & Address*/
    CANRamAddress_t                     cRamAddressInfo;
    CANControllerRegister_t *           cRegister;
    CANCallBackFunc_t *                 cCallbackFunctions;
} CANController_t;

typedef struct CANDriver
{
    uint8                               dIsInitDriver;
    uint8                               dNumOfController;
    CANController_t                     dControllerInfo[ CAN_CONTROLLER_NUMBER ];
} CANDriver_t;


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

extern CANDriver_t CANDriverInfo;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

CANController_t * CAN_DrvGetControllerInfo
(
    uint8                               ucCh
);

CANErrorType_t CAN_DrvInitChannel
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvDeinitChannel
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetNormalOperationMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetMonitoringMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvResetController
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetInternalTestMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetExternalTestMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetWakeUpMode
(
    void
);

CANErrorType_t CAN_DrvSetSleepMode
(
    void
);

uint32 CAN_DrvGetProtocolStatus
(
    const CANController_t *             psControllerInfo
);

void CAN_DrvCallbackNotifyRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
);

/* Not used function */
/*
CANErrorType_t CAN_DrvSetDisableAutomaticRetransmission
(
    CANController_t *                   psControllerInfo,
    boolean                             bDisable
);
*/

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

#endif  // MCU_BSP_CAN_DRV_HEADER


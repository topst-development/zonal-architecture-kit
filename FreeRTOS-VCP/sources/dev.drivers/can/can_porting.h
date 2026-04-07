// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_porting.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CAN_PORTING_HEADER
#define MCU_BSP_CAN_PORTING_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include "gic.h"
#include "pmio.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


#define CAN_0_FS                        (PMIO_GPK(13)|PMIO_GPK(8)|PMIO_GPK(7)|PMIO_GPK(1))
#define CAN_0_TX                        (GPIO_GPK(8UL))
#define CAN_0_RX                        (GPIO_GPK(1UL))
#define CAN_0_STB                       (GPIO_GPK(13UL))
#define CAN_0_INH                       (GPIO_GPK(7UL))

#define CAN_1_FS                        (PMIO_GPK(9)|PMIO_GPK(2))
#define CAN_1_TX                        (GPIO_GPK(9UL))
#define CAN_1_RX                        (GPIO_GPK(2UL))
#define CAN_1_STB                       (GPIO_GPB(18UL))
#define CAN_1_INH                       (0UL) //No INH for CAN1

#define CAN_2_FS                        (PMIO_GPK(10)|PMIO_GPK(3))
#define CAN_2_TX                        (GPIO_GPK(10UL))
#define CAN_2_RX                        (GPIO_GPK(3UL))
#define CAN_2_STB                       (GPIO_GPB(19UL))
#define CAN_2_INH                       (0UL) //No INH for CAN2


#define CAN_SAL_DELAY_SUPPORT           (0)

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

void CAN_PortingDelay
(
    uint32                              uiMsDelay
);

CANErrorType_t CAN_PortingInitHW
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_PortingSetControllerClock
(
    CANController_t *                   psControllerInfo,
    uint32                              uiFreq
);

CANErrorType_t CAN_PortingResetDriver
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_PortingSetInterruptHandler
(
    CANController_t *                   psControllerInfo,
    GICIsrFunc                          fnIsr
);

CANErrorType_t CAN_PortingDisableControllerInterrupts
(
    uint8                               ucCh
);

CANErrorType_t CAN_PortingEnableControllerInterrupts
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofStandardIDFilterList
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofExtendedIDFilterList
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofRxFIFO0
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofRxFIFO1
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofRxBuffer
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofTxEventBuffer
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofTxBuffer
(
    uint8                               ucCh
);

boolean CAN_PortingGetBitRateSwitchEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetFDEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetStandardIDFilterEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetExtendedIDFilterEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetStandardIDRemoteRejectEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetExtendedIDRemoteRejectEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetTxEventFIFOEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetWatchDogEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetTimeOutEnable
(
    uint8                               ucCh
);

boolean CAN_PortingGetTimeStampEnable
(
    uint8                               ucCh
);

uint32 CAN_PortingAllocateNonCacheMemory
(
    uint8                               ucCh,
    uint32                              uiMemSize
);

void CAN_PortingDeallocateNonCacheMemory
(
    uint8                               ucCh,
    uint32 *                            puiMemAddr
);

CANControllerRegister_t * CAN_PortingGetControllerRegister
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigBaseAddr
(
    void
);

CANRegBaseAddr_t * CAN_PortingGetMessageRamBaseAddr
(
    uint8                               ucCh
);

CANRegExtTSCtrl0_t * CAN_PortingGetConfigEXTS0Addr
(
    uint8                               ucCh
);

CANRegExtTSCtrl1_t * CAN_PortingGetConfigEXTS1Addr
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigWritePasswordAddr
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigWriteLockAddr
(
    uint8                               ucCh
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

#endif  // MCU_BSP_CAN_PORTING_HEADER


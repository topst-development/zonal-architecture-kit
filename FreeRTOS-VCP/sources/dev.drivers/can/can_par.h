// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_par.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CAN_PAR_HEADER
#define MCU_BSP_CAN_PAR_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

extern CANTimingParam_t ArbitrationPhaseTimingPar[ CAN_CONTROLLER_NUMBER ];

extern CANTimingParam_t DataPhaseTimingPar[ CAN_CONTROLLER_NUMBER ];

extern CANTxBuffer_t TxBufferInfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANRxBuffer_t RxBufferInfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANRxBuffer_t RxBufferFIFO0InfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANRxBuffer_t RxBufferFIFO1InfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANCallBackFunc_t CANCallbackFunctions;

extern CANIdFilter_t StandardIDFilterPar;

extern CANIdFilter_t ExtendedIDFilterPar;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

#endif  // MCU_BSP_CAN_PAR_HEADER


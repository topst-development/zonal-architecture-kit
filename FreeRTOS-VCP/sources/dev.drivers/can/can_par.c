// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_par.c
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


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

#ifdef CAN_CHANNEL_0
static CANIdFilterList_t StandardIDFilterPar_CH0[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH0[ CAN_EXTENDED_ID_FILTER_NUMBER ];
#endif
#ifdef CAN_CHANNEL_1
static CANIdFilterList_t StandardIDFilterPar_CH1[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH1[ CAN_EXTENDED_ID_FILTER_NUMBER ];
#endif
#ifdef CAN_CHANNEL_2
static CANIdFilterList_t StandardIDFilterPar_CH2[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH2[ CAN_EXTENDED_ID_FILTER_NUMBER ];
#endif


/**************************************************************************************************
*
*                                      CAN Timing Information
*
**************************************************************************************************/

/* Arbitration Phase Timing Information */
CANTimingParam_t ArbitrationPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
#ifdef CAN_CHANNEL_0
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 0, speed : 500Kbps / sample point : 80%
#endif
#ifdef CAN_CHANNEL_1
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 1, speed : 500Kbps / sample point : 80%
#endif
#ifdef CAN_CHANNEL_2
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 2, speed : 500Kbps / sample point : 80%
#endif
};

/* Data Phase Timing Information */
CANTimingParam_t DataPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
#ifdef CAN_CHANNEL_0
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 0, speed : 2Mbps / sample point : 70%
#endif
#ifdef CAN_CHANNEL_1
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 1, speed : 2Mbps / sample point : 70%
#endif
#ifdef CAN_CHANNEL_2
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 2, speed : 2Mbps / sample point : 70%
#endif
};


/**************************************************************************************************
*
*                                      CAN Buffer Information
*
**************************************************************************************************/

/* TX Buffer */
CANTxBuffer_t TxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Buffer count,  FIFO,QUEUE count, FIFO,QUEUE mode,          Buffer size,                Interrupt Config */
#ifdef CAN_CHANNEL_0
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
#endif
#ifdef CAN_CHANNEL_1
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
#endif
#ifdef CAN_CHANNEL_2
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
#endif
};

/* RX Dedicated Buffer */
CANRxBuffer_t RxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Don't care,    Data Field Size,                Don't care */
#ifdef CAN_CHANNEL_0
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
#endif
#ifdef CAN_CHANNEL_1
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
#endif
#ifdef CAN_CHANNEL_2
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
#endif
};

/* RX FIFO 0 */
CANRxBuffer_t RxBufferFIFO0InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Buffer count,  Data Field Size,            Operation mode */
#ifdef CAN_CHANNEL_0
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
#endif
#ifdef CAN_CHANNEL_1
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
#endif
#ifdef CAN_CHANNEL_2
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
#endif
};

/* RX FIFO 1 */
CANRxBuffer_t RxBufferFIFO1InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Buffer count,  Data Field Size,            Operation mode */
#ifdef CAN_CHANNEL_0
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
#endif
#ifdef CAN_CHANNEL_1
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
#endif
#ifdef CAN_CHANNEL_2
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
#endif
};


/**************************************************************************************************
*
*                                      CAN Standard ID Filter
*
**************************************************************************************************/
#ifdef CAN_CHANNEL_0
static CANIdFilterList_t StandardIDFilterPar_CH0[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF },
};
#endif
#ifdef CAN_CHANNEL_1
static CANIdFilterList_t StandardIDFilterPar_CH1[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF },
};
#endif
#ifdef CAN_CHANNEL_2
static CANIdFilterList_t StandardIDFilterPar_CH2[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF },
};
#endif

CANIdFilter_t StandardIDFilterPar =
{
    {
#ifdef CAN_CHANNEL_0
    &StandardIDFilterPar_CH0[ 0 ],
#endif
#ifdef CAN_CHANNEL_1
    &StandardIDFilterPar_CH1[ 0 ],
#endif
#ifdef CAN_CHANNEL_2
    &StandardIDFilterPar_CH2[ 0 ],
#endif
    }
};


/**************************************************************************************************
*
*                                      CAN Extended ID Filter
*
**************************************************************************************************/
#ifdef CAN_CHANNEL_0
static CANIdFilterList_t ExtendedIDFilterPar_CH0[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF },
};
#endif
#ifdef CAN_CHANNEL_1
static CANIdFilterList_t ExtendedIDFilterPar_CH1[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF },
};
#endif
#ifdef CAN_CHANNEL_2
static CANIdFilterList_t ExtendedIDFilterPar_CH2[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF },
};
#endif

CANIdFilter_t ExtendedIDFilterPar =
{
    {
#ifdef CAN_CHANNEL_0
    &ExtendedIDFilterPar_CH0[ 0 ],
#endif
#ifdef CAN_CHANNEL_1
    &ExtendedIDFilterPar_CH1[ 0 ],
#endif
#ifdef CAN_CHANNEL_2
    &ExtendedIDFilterPar_CH2[ 0 ],
#endif
    }
};


/**************************************************************************************************
*
*                                      CAN Call Back Function
*
**************************************************************************************************/

CANCallBackFunc_t CANCallbackFunctions =
{
    NULL_PTR,  //cbNotifyRxEvent
    NULL_PTR,  //cbNotifyTxEvent
    NULL_PTR,  //cbNotifyErrorEvent
};


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )


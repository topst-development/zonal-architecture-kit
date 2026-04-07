// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : lin.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_LIN_HEADER
#define MCU_BSP_LIN_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_LIN == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_UART value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
#include "debug.h"

#define LIN_D(fmt, args...)             {LOGD(DBG_TAG_LIN, fmt, ## args)}
#define LIN_E(fmt, args...)             {LOGE(DBG_TAG_LIN, fmt, ## args)}
#else
#define LIN_D(fmt, args...)
#define LIN_E(fmt, args...)
#endif

/* Used to carry diagnostic and configuration data. Use only classic checksum */
#define LIN_DIAGNOSIC_ID1               (0x3CU)
#define LIN_DIAGNOSIC_ID2               (0x3DU)

#define LIN_MAX_DATA_SIZE               (8U)


typedef void (*LINResponseIsrFunc)
(
    void *                              pArg,
    uint8                               ucPid
);

typedef enum LINMode
{
    LIN_MASTER                          = 0,
    LIN_SLAVE
}LINMode_t;

typedef enum LINStatus
{
    LIN_ERROR                           = -1,
    LIN_OK                              = 0,
    LIN_HEADER_ERROR,
    LIN_NO_RESPONSE,
    LIN_CSUM_ERROR,
    LIN_NO_DATA_INBUS,
} LINStatus_t;

typedef struct LINData{
    uint8                               ldPid;
    uint8                               ldData[LIN_MAX_DATA_SIZE];
    uint8                               ldCSum;
    uint8                               ldSize;
    uint8                               ldDLC;  //Data Length Size
}LINData_t;

typedef struct LINConfig
{
    uint32                              cfgLinCh;
    uint32                              cfgPortSel;
    uint32                              cfgBaud;
    uint32                              cfgLinSlpPin;
    uint8                               cfgMode;     //Polling, Interrupt
    uint8                               cfgGicPriority;
    LINMode_t                           cfgLinMode;
} LINConfig_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

sint8 LIN_Init
(
    LINConfig_t                         sLinCfg
);

sint8 LIN_MasterTx
(
    LINConfig_t                         sLinCfg,
    LINData_t *                         psLinTxData
);

sint8 LIN_MasterRx
(
    LINConfig_t                         sLinCfg,
    LINData_t *                         psLinTxData
);

LINStatus_t LIN_GetStatus
(
    LINConfig_t                         sLinCfg,
    LINData_t *                         psLinRxData
);

sint8 LIN_ReadHeader
(
    LINConfig_t                         sLinCfg,
    uint8 *                             pucPid
);

sint8 LIN_ReadResponse
(
    LINConfig_t                         sLinCfg,
    LINData_t *                         psLinRxData
);

sint8 LIN_CalcProtectId
(
    uint8                               ucFid,
    uint8 *                             pucPid
);

sint8 LIN_CalcFrameId
(
    uint8                               ucPid,
    uint8 *                             pucFid
);

uint8 LIN_CalClassicChecksum
(
    uint8 *                             pucData,
    uint8                               ucSize
);

uint8 LIN_CalEnhancedChecksum
(
    uint8                               ucPid,
    uint8 *                             pucData,
    uint8                               ucSize
);

sint8 LIN_FlushRxData
(
    LINConfig_t                         sLinCfg
);

void LIN_RegisterCB
(
    LINResponseIsrFunc                  pLinResFunc
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_LIN == 1 )

#endif  // MCU_BSP_LIN_HEADER


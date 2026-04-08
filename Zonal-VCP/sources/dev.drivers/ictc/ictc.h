// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : ictc.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_ICTC_HEADER
#define MCU_BSP_ICTC_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_ICTC == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include "sal_com.h"
#include "ictc_dev.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define ICTC_D(fmt, args...)        {LOGD(DBG_TAG_ICTC, fmt, ## args)}
    #define ICTC_Err(fmt, args...)      {LOGE(DBG_TAG_ICTC, fmt, ## args)}
#else
    #define ICTC_D(fmt, args...)
    #define ICTC_Err(fmt, args...)
#endif

#define ICTC_USER_CUS_FUNCTION

#define ICTC_SUB_ICTC_ENABLED

#define ICTC_TOTAL_CHANNEL_NUM          (6UL)
#define ICTC_DIFF_MARGIN                (0x800UL)
#define ICTC_DiffABSValue(X, Y)         (((X) > (Y)) ? ((X) - (Y)) : ((Y) - (X)))

#define ICTC_REG_MAX_VALUE              (0xFFFFFFFFUL)
#define ICTC_REG_ZERO_VALUE             (0x00000000UL)

/******************************************************************************
 * Record Time-stamp
 ******************************************************************************/

#define ICTC_TIMESTAMP_RECORD_ENABLE
#define ICTC_TIMESTAMP_RECORD_MAXCNT    (15UL)

/******************************************************************************
 * CLOCK
 ******************************************************************************/

/* ICTC MAX clock 150Mhz */
#define ICTC_PERI_CLOCK                 ((150UL) * (1000UL) * (1000UL))

#define ICTC_CH_0                       (0UL)
#define ICTC_CH_1                       (1UL)
#define ICTC_CH_2                       (2UL)
#define ICTC_CH_3                       (3UL)
#define ICTC_CH_4                       (4UL)
#define ICTC_CH_5                       (5UL)

typedef void (*ICTCCallback)(uint32 uiChannel, uint32 uiPeriod, uint32 uiDuty);

/******************************************************************************
* struct
******************************************************************************/

typedef struct ICTCCallBack
{
    uint32                              cbChannel;
    ICTCCallback                        cbCallBackFunc;
} ICTCCallBack_t;

typedef struct ICTCModeConfig
{
    uint32                              mcTimeout;
    uint32                              mcREdgeMat;
    uint32                              mcFEdgeMat;
    uint32                              mcEdgeCntMat;
    uint32                              mcPrdRound;
    uint32                              mcDutyRound;
    uint32                              mcEnableCounter;
    uint32                              mcEnableIrq;
    uint32                              mcOperationMode;
} ICTCModeConfig_t;

#ifdef ICTC_USER_CUS_FUNCTION
typedef struct ICTCIsrResult
{
    uint32                              irNoisefltFullCnt;
    uint32                              irTocntFullCnt;
    uint32                              irEcntFullCnt;
    uint32                              irDtErrCnt;
    uint32                              irPrdErrCnt;
    uint32                              irDtPrdCmpFullCnt;
    uint32                              irRedgeNotiCnt;
    uint32                              irFedgeNotiCnt;
}ICTCIsrResult_t;
#endif

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

#ifdef ICTC_USER_CUS_FUNCTION
/*
***************************************************************************************************
*                                          ICTC_ClearIsrResult
*
* Function to check the interrupt result (Can be changed by user)
* @param
* Notes
*
***************************************************************************************************
*/
void ICTC_ClearIsrResult
(
    void
);

/*
***************************************************************************************************
*                                           ICTC_GetIsrResult
*
* Function to check the interrupt result (Can be changed by user)
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
void ICTC_GetIsrResult
(
    ICTCIsrResult_t *                   psIsrResult
);

#endif  /* End of ICTC_USER_CUS_FUNCTION */

/*
***************************************************************************************************
*                                          ICTC_Init
*
* Initialize register map
* Notes
*
***************************************************************************************************
*/

void ICTC_Init
(
    void
);


/*
***************************************************************************************************
*                                          ICTC_Deinit
*
* De-Initialize register map
* Notes
*
***************************************************************************************************
*/

void ICTC_Deinit
(
    void
);

/*
***************************************************************************************************
*                                          ICTC_GetPrevPeriodCnt
*
* ICTC get previous period Count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetPrePeriodCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetPreDutyCnt
*
* ICTC get previous Duty Count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPreDutyCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetCurEdgeCnt
*
* ICTC get current edge count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetCurEdgeCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetPrvEdgeCnt
*
* ICTC get previous edge count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPrvEdgeCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetREdgeTstmpCnt
*
* ICTC get rising edge timestamp count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetREdgeTstmpCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetFEdgeTstmpCnt
*
* ICTC get falling edge timestamp count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetFEdgeTstmpCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_EnableCapture
*
* ICTC Enable Operation, Enable TCLK and ICTC EN
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

void ICTC_EnableCapture
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_DisableCapture
*
* ICTC Diable Operation, Clear TCLK and ICTC EN
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

void ICTC_DisableCapture
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_SetOpEnCtrlCounter
*
* ICTC set OP_EN_CTRL Counter Register
* @param    channel number 0 ~ 5.
* @param    counter value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetOpEnCtrlCounter
(
    uint32                              uiChannel,
    uint32                              uiCntEnValue
);

/*
***************************************************************************************************
*                                          ICTC_SetOpModeCtrlReg
*
* ICTC set OP_MODE_CTRL Register
* @param    channel number 0 ~ 5.
* @param    OP_MODE value.
* Notes
*
***************************************************************************************************
*/
void ICTC_SetOpModeCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiModeValue
);

/*
***************************************************************************************************
*                                          ICTC_SetIRQCtrlReg
*
* ICTC set IRQ_CTRL Register
* @param    channel number 0 ~ 5.
* @param    IRQ_EN value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetIRQCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiIRQValue
);

/*
***************************************************************************************************
*                                          ICTC_SetCallBackFunc
*
* ICTC set CallbackFunction
* @param    channel number 0 ~ 5.
* @param    CallBack Function
* Notes
*
***************************************************************************************************
*/

void ICTC_SetCallBackFunc
(
    uint32                              uiChannel,
    ICTCCallback                        pCallbackFunc
);

/*
***************************************************************************************************
*                                          ICTC_SetTimeoutValue
*
* ICTC set Time-Out Value Register
* @param    channel number 0 ~ 5.
* @param    timeout value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetTimeoutValue
(
    uint32                              uiChannel,
    uint32                              uiTimeoutValue
);

/*
***************************************************************************************************
*                                          ICTC_SetEdgeMatchingValue
*
* ICTC set Edge Matching Value, R_EDGE, F_EDGE, EDGE_COUNTER
* @param    channel number 0 ~ 5.
* @param    rising edge matching value.
* @param    falling edge matching value.
* @param    edge counter matching value.
* @return
*
* Notes
*
***************************************************************************************************
*/

void ICTC_SetEdgeMatchingValue
(
    uint32                              uiChannel,
    uint32                              uiRisingEdgeMat,
    uint32                              uiFallingEdgeMat,
    uint32                              uiEdgeCounterMat
);

/*
***************************************************************************************************
*                                          ICTC_SetCompareRoundValue
*
* ICTC set Period/Duty compare round value configuration register
* @param    channel number 0 ~ 5.
* @param    period compare round value.
* @param    duty compare round value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetCompareRoundValue
(
    uint32                              uiChannel,
    uint32                              uiPeriodCompareRound,
    uint32                              uiDutyCompareRound
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_ICTC == 1 )

#endif  // MCU_BSP_ICTC_HEADER


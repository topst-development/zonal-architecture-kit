// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : power_com.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef POWER_COM_HEADER
#define POWER_COM_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/
#include <app_cfg.h>

#if (APLT_LINUX_SUPPORT_POWER_CTRL == 1)
#include <sal_com.h>
#include <spi_eccp.h>



/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

#define POWER_COM_MSG_BY_ECCP

typedef enum
{
    POWER_COM_AP_CH_NOT_SET           = (0xFF),
    POWER_COM_AP_CH_ALL               = ECCP_ALL_FRAME,
    POWER_COM_AP_CH_A72               = ECCP_A72_FRAME,
    POWER_COM_AP_CH_A53               = ECCP_A53_FRAME,
} POWERCOMApCh_t;

typedef enum
{
    POWER_COM_NOTI_NO_ERROR   = 0,
    POWER_COM_NOTI_TIME_OUT   = 1
}POWERCOMNoti_t;

typedef void                            (* POWERCOMNotiCbk)
(
    POWERCOMNoti_t                      tNotiType,
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2,
    uint8*                              pucDat
);

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

SALRetCode_t POWER_COM_Start
(
    POWERCOMApCh_t                      tMain,
    POWERCOMApCh_t                      tSub,
    POWERCOMNotiCbk                     fNotiCbk
);

SALRetCode_t POWER_COM_HaltCh
(
    POWERCOMApCh_t                      tCh,
    uint8                               ucIsHalt
);

SALRetCode_t POWER_COM_SendMsg
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2,
    uint8                               ucDat1,
    uint8                               ucDat2
);

void POWER_COM_RecvMsg
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2,
    uint32                              uiTimeout
);
#endif

#endif


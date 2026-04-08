// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : power_com.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/
#if (APLT_LINUX_SUPPORT_POWER_CTRL == 1)
#include "power_com.h"
#include <debug.h>
#include <gic.h>
#include <timer.h>
#include <bsp.h>

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


#if 1
#define POWER_COM_D(args...)
#else
#define POWER_COM_D(fmt, args...)            {LOGD(DBG_TAG_PMIO, fmt, ## args)}
#endif

#define POWER_COM_E(args...)            { \
                                            mcu_printf("[%s:%d] Error ! ",__func__, __LINE__); \
                                            mcu_printf(args); \
                                        }

#define POWER_COM_VA_MSG_TASK_STACK_SIZE         (2048UL)
#define POWER_COM_VA_MSG_DATA_LIST_MAX_SIZE    (30UL)
#define POWER_COM_VA_MSG_ACK_CMD2           ((uint16)0xFF01)
#define POWER_COM_VA_MSG_ACK_TIME           (5000UL)
#define POWER_COM_VA_TIMER_CH               (2UL)

typedef struct
{
    uint16          usCmd2;

    uint8           ucNotiByAll;
    uint8           ucNotiByA72;
    uint8           ucNotiByA53;

    uint8           ucRecvByA72;
    uint8           ucRecvByA53;
}POWERCOMRecvMsgInfo_t;

typedef struct
{
    uint32 uiTaskId;
    uint32 uiTaskStack[POWER_COM_VA_MSG_TASK_STACK_SIZE];
    uint32 uiEventId;
    uint32 uiEventNum;

}POWERCOMTask_t;

typedef struct
{
    POWERCOMTask_t  tTask;

    POWERCOMNotiCbk fNotiCbk;

    POWERCOMNoti_t                      tRetNoti;
    POWERCOMApCh_t                      tRetMsgCh;
    uint16                              usCmd2;
    uint8                               ucDat[2];
}POWERCOMNotiInfo_t;

typedef struct
{
    int8            cMsgOpenName[7];

#ifdef POWER_COM_MSG_BY_ECCP
   ECCPDev_t * tMsgDev;  //for real spi eccp
#endif

    boolean           bMsgA72Run;
    boolean           bMsgA53Run;

    POWERCOMTask_t      tRecvMsgTask;
    uint32              uiNotiLock;
    POWERCOMNotiInfo_t  tNoti;


    POWERCOMRecvMsgInfo_t   tRecvMsgList[POWER_COM_VA_MSG_DATA_LIST_MAX_SIZE];
    uint32              uiRecvMsgListMaxNum;

    POWERCOMApCh_t      tAckCh;
    uint16              usAckCmd2;
    uint16              usAckCmd2Wait;

    POWERCOMApCh_t      tTimeoutMsgCh;
    uint16              usTimeoutMsgCmd2;
}POWERCOMInfo_t;


static uint32           guiFzEvtMainId;

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static POWERCOMInfo_t * ptPOWERCOMInfos;
/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static POWERCOMApCh_t POWER_COM_GetMsgValidCh
(
    POWERCOMApCh_t                      tReqCh
);

#ifdef POWER_COM_MSG_BY_ECCP
static void POWER_COM_RecvMsgTask_Eccp
(
    void *                              pArg
);
#endif

static void POWER_COM_NotiTask
(
    void *                              pArg
);

static void POWER_COM_CheckNoti
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd,
    uint8                               *pucDat
);

static void POWER_COM_SetMsgTimer
(
    POWERCOMApCh_t                          tMsgCh,
    uint16                              uiCmd,
    uint32                              uiUsec
);

static sint32 POWER_COM_MsgTimeoutHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static void POWER_COM_AckSend
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2
);
static SALRetCode_t POWER_COM_AckRecvWait
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2
);
static void POWER_COM_AckRecvDone
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2
);
static SALRetCode_t POWER_COM_SetTask
(
    POWERCOMTask_t*                     tTask,
    uint32                              uiTaskSize,
    SALTaskFunc                         fFunc,
    const uint8*                        ucFuncName
);

static void delay1us
(
   uint32                               uiUs
);
/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

static POWERCOMApCh_t POWER_COM_GetMsgValidCh
(
    POWERCOMApCh_t                      tReqCh
)
{
    POWERCOMApCh_t tValidCh = POWER_COM_AP_CH_NOT_SET;

    if(tReqCh == POWER_COM_AP_CH_ALL)
    {
        if(
                (ptPOWERCOMInfos->bMsgA72Run == SALEnabled) &&
                (ptPOWERCOMInfos->bMsgA53Run == SALEnabled)
          )
        {
            tValidCh = POWER_COM_AP_CH_ALL;
        }
        else if(
                (ptPOWERCOMInfos->bMsgA72Run == SALEnabled) &&
                (ptPOWERCOMInfos->bMsgA53Run == SALDisabled)
               )
        {
            tValidCh = POWER_COM_AP_CH_A72;
        }
        else if(
                (ptPOWERCOMInfos->bMsgA72Run == SALDisabled) &&
                (ptPOWERCOMInfos->bMsgA53Run == SALEnabled)
               )
        {
            tValidCh = POWER_COM_AP_CH_A53;
        }
        else
        {
            tValidCh = POWER_COM_AP_CH_NOT_SET;
            POWER_COM_D("Eccp Communication is disconnected\n");
        }
    }
    else if(
            (tReqCh == POWER_COM_AP_CH_A72) &&
            (ptPOWERCOMInfos->bMsgA72Run == SALEnabled)
           )
    {
        tValidCh = POWER_COM_AP_CH_A72;
    }
    else if(
            (tReqCh == POWER_COM_AP_CH_A53) &&
            (ptPOWERCOMInfos->bMsgA53Run == SALEnabled)
           )
    {
        tValidCh = POWER_COM_AP_CH_A53;
    }
    else
    {
        tValidCh = POWER_COM_AP_CH_NOT_SET;
        POWER_COM_D("Unknown Source comm\n");
    }
    return tValidCh;
}

static void POWER_COM_RecvMsgTask_Eccp
(
    void *                              pArg
)
{
    uint32          uiFlag;
    uint8           ucFreeze;
    POWERCOMApCh_t tMsgProcCh;
    uint8  ucSrc;
    uint16 usCmd;
    uint8  pucDat[ECCP_MAX_PACKET];

    ucFreeze = 0U;
    (void)pArg;
 //   mcu_printf("spi send\n");
  //  POWER_COM_AckSend((POWERCOMApCh_t)POWER_COM_AP_CH_ALL, 0x01);
    while(TRUE)
    {
        if(ucFreeze == 0)
        {
            if(ptPOWERCOMInfos != NULL_PTR)
            {
                if(ptPOWERCOMInfos->tMsgDev != NULL_PTR)
                {
                    ucSrc = 0U;
                    usCmd = 0U;
                    pucDat[0] = 0U; /*Valid data of power msg is 2byte*/
                    pucDat[1] = 0U; /*Valid data of power msg is 2byte*/
                    tMsgProcCh = POWER_COM_AP_CH_NOT_SET;

                    if(
                            (ptPOWERCOMInfos->bMsgA72Run == SALEnabled) ||
                            (ptPOWERCOMInfos->bMsgA53Run == SALEnabled)
                      )
                    {
                        (void)ECCP_Recv(ptPOWERCOMInfos->tMsgDev, &ucSrc, &usCmd, pucDat);

                        if(usCmd == POWER_COM_VA_MSG_ACK_CMD2)
                        {
                            if(
                                    (ptPOWERCOMInfos->usAckCmd2Wait)  ==
                                    ( (((uint16)pucDat[0])<<8) | ((uint16)pucDat[1]) )
                                )
                            {
                                POWER_COM_D("ARxS%02x ARxD%04x\n", ucSrc, ( (((uint16)pucDat[0])<<8) | ((uint16)pucDat[1]) ));
                                POWER_COM_AckRecvDone(
                                        (POWERCOMApCh_t)ucSrc,
                                        ( (((uint16)pucDat[0])<<8) | ((uint16)pucDat[1]) )
                                        );
                            }
                            else
                            {
                                mcu_printf("Error ! Ack Drop x%04x\n", ( (((uint16)pucDat[0])<<8) | ((uint16)pucDat[1]) ));
                            }
                        }
                        else
                        {
                            POWER_COM_D("RxS%02x, RxC%04x, RxD%02x, RxD%02x\n", ucSrc, usCmd, pucDat[0], pucDat[1]);
                            POWER_COM_D("ATxS%02x ATxD%04x\n", ucSrc, usCmd);
                            POWER_COM_AckSend((POWERCOMApCh_t)ucSrc, usCmd);

                            tMsgProcCh = POWER_COM_GetMsgValidCh((POWERCOMApCh_t)ucSrc);

                            if(tMsgProcCh != POWER_COM_AP_CH_NOT_SET)
                            {
                                if(ptPOWERCOMInfos->usTimeoutMsgCmd2 == usCmd)
                                {
                                    if(tMsgProcCh == ptPOWERCOMInfos->tTimeoutMsgCh)
                                    {
                                        POWER_COM_SetMsgTimer(POWER_COM_AP_CH_NOT_SET, 0U, 0UL);
                                    }
                                    else
                                    {
                                        if(ptPOWERCOMInfos->tTimeoutMsgCh == POWER_COM_AP_CH_ALL)
                                        {
                                            if(tMsgProcCh == POWER_COM_AP_CH_A72)
                                            {
                                                ptPOWERCOMInfos->tTimeoutMsgCh = POWER_COM_AP_CH_A53;
                                            }

                                            if(tMsgProcCh == POWER_COM_AP_CH_A53)
                                            {
                                                ptPOWERCOMInfos->tTimeoutMsgCh = POWER_COM_AP_CH_A72;
                                            }
                                        }
                                    }
                                }

                                POWER_COM_CheckNoti(tMsgProcCh, usCmd, pucDat);
                            }
                            else
                            {
                                POWER_COM_D("Unknown Source comm\n");
                            }
                        }
                    }
                    else
                    {
                        ucFreeze = 1U;
                        POWER_COM_D("Eccp Communication is disconnected\n");
                    }

                }
                else
                {
                    ucFreeze = 1U;
                    POWER_COM_D("Eccp Communication is not open\n");
                }
            }
            else
            {
                ucFreeze = 1U;
                POWER_COM_E("POWERCOMInfos is null.\n");
            }
        }
        else
        {
            POWER_COM_D("Eccp Communication Freezes.\n");
            (void)SAL_EventGet(guiFzEvtMainId, 1UL, 0UL, \
                    (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)), &uiFlag);
        }
    }
}

static void POWER_COM_NotiTask
(
    void *                              pArg
)
{
    uint32 uiFlag;
    POWERCOMNoti_t                      tRetNoti;
    POWERCOMApCh_t                      tRetMsgCh;
    uint16                              usCmd2;
    uint8                               ucDat[2];

    uiFlag = 0UL;

    while(TRUE)
    {
        (void)SAL_EventGet(
                ptPOWERCOMInfos->tNoti.tTask.uiEventId,
                ptPOWERCOMInfos->tNoti.tTask.uiEventNum,
                0UL,
                (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)),
                &uiFlag
                );

        (void)SAL_EventSet(
                ptPOWERCOMInfos->tNoti.tTask.uiEventId,
                ptPOWERCOMInfos->tNoti.tTask.uiEventNum,
                SAL_EVENT_OPT_CLR_ALL
                );

        (void) SAL_SemaphoreWait(ptPOWERCOMInfos->uiNotiLock, 0UL, SAL_OPT_BLOCKING);
        tRetNoti  = ptPOWERCOMInfos->tNoti.tRetNoti;
        tRetMsgCh = ptPOWERCOMInfos->tNoti.tRetMsgCh;
        usCmd2    = ptPOWERCOMInfos->tNoti.usCmd2;
        ucDat[0]  = ptPOWERCOMInfos->tNoti.ucDat[0];
        ucDat[1]  = ptPOWERCOMInfos->tNoti.ucDat[1];
        (void) SAL_SemaphoreRelease(ptPOWERCOMInfos->uiNotiLock);

        if(uiFlag != 0UL)
        {
            ptPOWERCOMInfos->tNoti.fNotiCbk(tRetNoti, tRetMsgCh, usCmd2, ucDat);
        }
        else
        {
            POWER_COM_D("Event zero\n");
        }
    }
}

static void POWER_COM_CheckNoti
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd,
    uint8                               *pucDat
)
{
    uint32 uiIdx;
    uint8 ucRecvDone;
    uint8 ucDat[2];

    ucRecvDone = 0U;
    ucDat[0] = pucDat[0];
    ucDat[1] = pucDat[1];

    //POWER_COM_D("ch%d recvd x%04x\n", tMsgCh, usCmd);

    for (uiIdx = 0UL; uiIdx < ptPOWERCOMInfos->uiRecvMsgListMaxNum; uiIdx++)
    {
        if(ptPOWERCOMInfos->tRecvMsgList[uiIdx].usCmd2 == usCmd)
        {
            //mcu_printf("Rx Src x%02x, Cmd2 x%02x, Dat1 x%02x, Dat2 x%02x\n", (uint16)tMsgCh, usCmd, pucDat[0], pucDat[1]);
            break;
        }
    }

    if(uiIdx < ptPOWERCOMInfos->uiRecvMsgListMaxNum)
    {
        if((tMsgCh == POWER_COM_AP_CH_A72) || (tMsgCh == POWER_COM_AP_CH_ALL))
        {
            ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA72 = 1U;
        }

        if((tMsgCh == POWER_COM_AP_CH_A53) || (tMsgCh == POWER_COM_AP_CH_ALL))
        {
            ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA53 = 1U;
        }

        if(ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByAll > 0U)
        {
            if(
                    (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA72 > 0U) &&
                    (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA53 > 0U)
              )
            {
                ucRecvDone = 1U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA72 = 0U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA53 = 0U;
            }
        }

        if(
                (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA72 > 0U) &&
                (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA72 > 0U)
          )
        {
            ucRecvDone = 1U;
            ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA72 = 0U;
        }

        if(
                (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA53 > 0U) &&
                (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA53 > 0U)
          )
        {
            ucRecvDone = 1U;
            ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA53 = 0U;
        }

        if(ucRecvDone > 0U)
        {
            if(ptPOWERCOMInfos->tNoti.fNotiCbk != NULL_PTR)
            {
                (void) SAL_SemaphoreWait(ptPOWERCOMInfos->uiNotiLock, 0UL, SAL_OPT_BLOCKING);
                ptPOWERCOMInfos->tNoti.tRetNoti = POWER_COM_NOTI_NO_ERROR;
                ptPOWERCOMInfos->tNoti.tRetMsgCh = tMsgCh;
                ptPOWERCOMInfos->tNoti.usCmd2 = usCmd;
                ptPOWERCOMInfos->tNoti.ucDat[0] = ucDat[0];
                ptPOWERCOMInfos->tNoti.ucDat[1] = ucDat[1];
                (void) SAL_SemaphoreRelease(ptPOWERCOMInfos->uiNotiLock);

                (void)SAL_EventSet(
                        ptPOWERCOMInfos->tNoti.tTask.uiEventId,
                        ptPOWERCOMInfos->tNoti.tTask.uiEventNum,
                        SAL_EVENT_OPT_FLAG_SET
                        );
            }
        }
    }
}

static void POWER_COM_SetMsgTimer
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              uiCmd,
    uint32                              uiUsec
)
{
    if(ptPOWERCOMInfos != NULL_PTR)
    {
        if(uiUsec != 0UL)
        {
            POWER_COM_D("Set new msg timer %d\n", uiUsec);

            ptPOWERCOMInfos->tTimeoutMsgCh = tMsgCh;
            ptPOWERCOMInfos->usTimeoutMsgCmd2 = uiCmd;

            (void) TIMER_Enable((TIMERChannel_t)POWER_COM_VA_TIMER_CH, uiUsec, \
                    &POWER_COM_MsgTimeoutHandler, NULL_PTR);

            (void) GIC_IntSrcEn((uint32)GIC_TIMER_0 + POWER_COM_VA_TIMER_CH);
        }
        else
        {
            POWER_COM_D("Clear old msg timer.\n");
            (void) TIMER_Disable((TIMERChannel_t)POWER_COM_VA_TIMER_CH);
            ptPOWERCOMInfos->tTimeoutMsgCh = POWER_COM_AP_CH_NOT_SET;
            ptPOWERCOMInfos->usTimeoutMsgCmd2 = 0UL;
        }
    }
}

static sint32 POWER_COM_MsgTimeoutHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
)
{
    (void)pArgs;

    if((iChannel == (TIMERChannel_t)POWER_COM_VA_TIMER_CH))
    {
        if(ptPOWERCOMInfos != NULL_PTR)
        {
            (void) TIMER_Disable((TIMERChannel_t)POWER_COM_VA_TIMER_CH);
            if(ptPOWERCOMInfos->tNoti.fNotiCbk != NULL_PTR)
            {
                (void) SAL_SemaphoreWait(ptPOWERCOMInfos->uiNotiLock, 0UL, SAL_OPT_BLOCKING);
                ptPOWERCOMInfos->tNoti.tRetNoti = POWER_COM_NOTI_TIME_OUT;

                ptPOWERCOMInfos->tNoti.tRetMsgCh = ptPOWERCOMInfos->tTimeoutMsgCh;
                ptPOWERCOMInfos->tNoti.usCmd2 = ptPOWERCOMInfos->usTimeoutMsgCmd2;

                ptPOWERCOMInfos->tTimeoutMsgCh = POWER_COM_AP_CH_NOT_SET;
                ptPOWERCOMInfos->usTimeoutMsgCmd2 = 0UL;

                ptPOWERCOMInfos->tNoti.ucDat[0] = 0U;
                ptPOWERCOMInfos->tNoti.ucDat[1] = 0U;
                (void) SAL_SemaphoreRelease(ptPOWERCOMInfos->uiNotiLock);

                (void)SAL_EventSet(
                        ptPOWERCOMInfos->tNoti.tTask.uiEventId,
                        ptPOWERCOMInfos->tNoti.tTask.uiEventNum,
                        SAL_EVENT_OPT_FLAG_SET
                        );
            }
        }
    }
    else
    {
        POWER_COM_D("TimerIntr fail null or mismatched ch %d\n", (uint32)iChannel);
    }
    return (sint32)0;
}

static void POWER_COM_AckSend
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2
)
{
    uint8 ucSendDt[2];

    ucSendDt[0] = (uint8)((usCmd2 >> 8) & 0xFF);
    ucSendDt[1] = (uint8)((usCmd2 >> 0) & 0xFF);
//    mcu_printf("send 0x%X, 0x%X, usCMD2 0x%X\n",ucSendDt[0], ucSendDt[1], usCmd2);
#ifdef POWER_COM_MSG_BY_ECCP
    if(
        ECCP_Send(ptPOWERCOMInfos->tMsgDev, tMsgCh,  POWER_COM_VA_MSG_ACK_CMD2,  (uint8 *) &ucSendDt, 2UL)  != (uint32)0
      )
    {
        POWER_COM_E("sub send fail \n");
    }
#endif
}

static SALRetCode_t POWER_COM_AckRecvWait
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2
)
{
    SALRetCode_t tRet;
    uint32 uiFlag;

    tRet = SAL_RET_FAILED;

    //POWER_COM_D("wt ch %d, cmd x%04x\n", (uint32)tMsgCh, usCmd2);
    ptPOWERCOMInfos->tAckCh = POWER_COM_AP_CH_NOT_SET;
    ptPOWERCOMInfos->usAckCmd2 = (uint16)0;
    ptPOWERCOMInfos->usAckCmd2Wait = (uint16)usCmd2;

    (void)SAL_EventGet(ptPOWERCOMInfos->tRecvMsgTask.uiEventId,
            ptPOWERCOMInfos->tRecvMsgTask.uiEventNum,
            POWER_COM_VA_MSG_ACK_TIME,
            (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)),
            &uiFlag
            );

    //POWER_COM_D("ack event flag x%x\n", uiFlag);

    if(uiFlag != 0UL)
    {
        if(
                (ptPOWERCOMInfos->tAckCh == tMsgCh) &&
                (ptPOWERCOMInfos->usAckCmd2 == usCmd2)
          )
        {
            ptPOWERCOMInfos->tAckCh = POWER_COM_AP_CH_NOT_SET;
            ptPOWERCOMInfos->usAckCmd2 = (uint16)0;
            ptPOWERCOMInfos->usAckCmd2Wait = (uint16)0;
            tRet = SAL_RET_SUCCESS;
        }
        else
        {
            POWER_COM_E("Unknown Ack ch %d, cmd x%04x\n", ptPOWERCOMInfos->tAckCh, ptPOWERCOMInfos->usAckCmd2);
        }
    }
    else
    {
        POWER_COM_E("Ack timeout (x%04x==x%04x)\n", ptPOWERCOMInfos->usAckCmd2Wait, ptPOWERCOMInfos->usAckCmd2);
    }

    (void)SAL_EventSet(
            ptPOWERCOMInfos->tRecvMsgTask.uiEventId,
            ptPOWERCOMInfos->tRecvMsgTask.uiEventNum,
            SAL_EVENT_OPT_CLR_ALL
            );

    return tRet;
}

static void POWER_COM_AckRecvDone
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2
)
{
    ptPOWERCOMInfos->tAckCh = tMsgCh;
    ptPOWERCOMInfos->usAckCmd2 = usCmd2;

    (void)SAL_EventSet(
            ptPOWERCOMInfos->tRecvMsgTask.uiEventId,
            ptPOWERCOMInfos->tRecvMsgTask.uiEventNum,
            SAL_EVENT_OPT_FLAG_SET
            );
}

static SALRetCode_t POWER_COM_SetTask
(
    POWERCOMTask_t*                     tTask,
    uint32                              uiTaskSize,
    SALTaskFunc                         fFunc,
    const uint8*                        ucFuncName
)
{
    SALRetCode_t tRet;

    tRet = SAL_RET_SUCCESS;

    if((SAL_TaskCreate(&(tTask->uiTaskId), \
                    (const uint8 *)ucFuncName, \
                    fFunc, \
                    &(tTask->uiTaskStack[0]), \
                    (uint32)uiTaskSize, \
                    SAL_PRIO_POWER_MANAGER, \
                    NULL)) != SAL_RET_SUCCESS)
    {
        POWER_COM_E("Task creation FAIL.\n");
        tRet = SAL_RET_FAILED;
    }
    else
    {
        if ((SAL_EventCreate((uint32 *)&(tTask->uiEventId), \
                        (const uint8 *)"msg signal event created", 0)) != SAL_RET_SUCCESS)
        {
            tRet = SAL_RET_FAILED;
            POWER_COM_E("Task event creation FAIL.\n");
        }
        else
        {
            tTask->uiEventNum = 1UL;
        }
    }

    return tRet;
}

SALRetCode_t POWER_COM_Start
(
    POWERCOMApCh_t                      tMain,
    POWERCOMApCh_t                      tSub,
    POWERCOMNotiCbk                     fNotiCbk
)
{
    SALRetCode_t    tRet;
    static POWERCOMInfo_t gtPOWERCOMInfos;

    tRet = SAL_RET_SUCCESS;

    if(
            ((uint32)tMain != (uint32)POWER_COM_AP_CH_NOT_SET) ||
            ((uint32)tSub  != (uint32)POWER_COM_AP_CH_NOT_SET)
      )
    {
        if(ptPOWERCOMInfos == NULL_PTR)
        {
            gtPOWERCOMInfos.cMsgOpenName[0] = 'P';
            gtPOWERCOMInfos.cMsgOpenName[1] = 'O';
            gtPOWERCOMInfos.cMsgOpenName[2] = 'W';
            gtPOWERCOMInfos.cMsgOpenName[3] = 'E';
            gtPOWERCOMInfos.cMsgOpenName[4] = 'R';
            gtPOWERCOMInfos.cMsgOpenName[5] = '0';
            gtPOWERCOMInfos.cMsgOpenName[6] = '\0';

            gtPOWERCOMInfos.uiNotiLock     = 0UL;
            gtPOWERCOMInfos.tNoti.fNotiCbk = fNotiCbk;
            gtPOWERCOMInfos.tNoti.tRetNoti = POWER_COM_NOTI_NO_ERROR;
            gtPOWERCOMInfos.tNoti.tRetMsgCh = POWER_COM_AP_CH_NOT_SET;
            gtPOWERCOMInfos.tNoti.usCmd2 = (uint16)0;
            gtPOWERCOMInfos.tNoti.ucDat[0] = 0U;
            gtPOWERCOMInfos.tNoti.ucDat[1] = 0U;

            if(SAL_SemaphoreCreate(
                        &(gtPOWERCOMInfos.uiNotiLock),
                        (const uint8 *)"POWER_COM_ResourceLock",
                        1UL,
                        SAL_OPT_BLOCKING)
                    != SAL_RET_SUCCESS)
            {
                POWER_COM_D("semaphore is notworking\n");
                gtPOWERCOMInfos.uiNotiLock     = 0UL;
            }

            POWER_COM_SetTask(
                    &(gtPOWERCOMInfos.tNoti.tTask),
                    POWER_COM_VA_MSG_TASK_STACK_SIZE,
                    (SALTaskFunc)&POWER_COM_NotiTask,
                    (const uint8 *)"POWER_COM_NotiTask"
                    );

            gtPOWERCOMInfos.tRecvMsgTask.uiTaskId  = 0UL;

#ifdef POWER_COM_MSG_BY_ECCP
            gtPOWERCOMInfos.tMsgDev = ECCP_Open((const uint8 *)(gtPOWERCOMInfos.cMsgOpenName), ECCP_STR_APP);

            if(gtPOWERCOMInfos.tMsgDev != NULL_PTR)
            {
                tRet = POWER_COM_SetTask(
                        &(gtPOWERCOMInfos.tRecvMsgTask),
                        POWER_COM_VA_MSG_TASK_STACK_SIZE,
                        (SALTaskFunc)&POWER_COM_RecvMsgTask_Eccp,
                        (const uint8 *)"POWER_COM_RecvMsgTask"
                        );

                if(tRet == SAL_RET_SUCCESS)
                {
                    if ((SAL_EventCreate((uint32 *)&(guiFzEvtMainId), \
                                    (const uint8 *)"msg signal event created", 0)) != SAL_RET_SUCCESS)
                    {
                        POWER_COM_E("task freezer event creation FAIL.\n");
                    }

                    if((tMain == POWER_COM_AP_CH_A72) || (tSub == POWER_COM_AP_CH_A72))
                    {
                        gtPOWERCOMInfos.bMsgA72Run = SALEnabled;
                    }

                    if((tMain == POWER_COM_AP_CH_A53) || (tSub == POWER_COM_AP_CH_A53))
                    {
                        gtPOWERCOMInfos.bMsgA53Run = SALEnabled;
                    }
                }
                else
                {
                    POWER_COM_E("pmio create task FAIL.\n");
                }
            }
            else
            {
                POWER_COM_E("tMsgDev is NULL.\n");
            }
#endif
            gtPOWERCOMInfos.uiRecvMsgListMaxNum = 0UL;

            gtPOWERCOMInfos.tAckCh = POWER_COM_AP_CH_NOT_SET;
            gtPOWERCOMInfos.usAckCmd2 = 0U;
            gtPOWERCOMInfos.usAckCmd2Wait = 0U;

            gtPOWERCOMInfos.tTimeoutMsgCh = POWER_COM_AP_CH_NOT_SET;
            gtPOWERCOMInfos.usTimeoutMsgCmd2 = 0U;

            ptPOWERCOMInfos = &gtPOWERCOMInfos;
        }
        else
        {
            POWER_COM_E("pmio already start msg.\n");
            tRet = SAL_RET_FAILED;
        }
    }
    else
    {
        POWER_COM_E("parameter is unknown %d , %d\n", tMain, tSub);
        tRet = SAL_RET_FAILED;
    }

    return tRet;
}

SALRetCode_t POWER_COM_HaltCh
(
    POWERCOMApCh_t                      tCh,
    uint8                               ucIsHalt
)
{
    SALRetCode_t tRet;

    tRet = SAL_RET_FAILED;

    if(ptPOWERCOMInfos != NULL_PTR)
    {
        if(tCh == POWER_COM_AP_CH_A72)
        {
            if(ucIsHalt > 0U)
            {
                ptPOWERCOMInfos->bMsgA72Run             = SALDisabled;
            }
            else
            {
                ptPOWERCOMInfos->bMsgA72Run             = SALEnabled;
            }
            tRet = SAL_RET_SUCCESS;
        }
        else if(tCh == POWER_COM_AP_CH_A53)
        {
            if(ucIsHalt > 0U)
            {
                ptPOWERCOMInfos->bMsgA53Run             = SALDisabled;
            }
            else
            {
                ptPOWERCOMInfos->bMsgA53Run             = SALEnabled;
            }
            tRet = SAL_RET_SUCCESS;
        }
        else
        {
            POWER_COM_E("unknown channel %d\n", tCh);
        }
    }

    return tRet;
}

SALRetCode_t POWER_COM_SendMsg
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2,
    uint8                               ucDat1,
    uint8                               ucDat2
)
{
    SALRetCode_t tRet = SAL_RET_FAILED;

    POWERCOMApCh_t tMsgProcCh;
    POWERCOMApCh_t tMsgAllSeqCh[2] = {POWER_COM_AP_CH_NOT_SET, POWER_COM_AP_CH_NOT_SET};
    uint32 uiMsgSeqChIdx;
    uint8    ucSendDt[2] = {0U, 0U};
    uint32  uiSendDtLen = 0UL;

    if(ptPOWERCOMInfos != NULL_PTR)
    {
        if(
                (ptPOWERCOMInfos->bMsgA72Run == SALEnabled) ||
                (ptPOWERCOMInfos->bMsgA53Run == SALEnabled)
          )
        {
            tMsgProcCh = POWER_COM_GetMsgValidCh(tMsgCh);

            if(tMsgProcCh != POWER_COM_AP_CH_NOT_SET)
            {
                if(ucDat1 > 0U)
                {
                    ucSendDt[0] = ucDat1;
                    uiSendDtLen++;
                }
                if(ucDat2 > 0U)
                {
                    ucSendDt[1] = ucDat2;
                    uiSendDtLen++;
                }

                if(tMsgProcCh == POWER_COM_AP_CH_ALL)
                {
                    tMsgAllSeqCh[0] = POWER_COM_AP_CH_A72;
                    tMsgAllSeqCh[1] = POWER_COM_AP_CH_A53;
                }
                else
                {
                    tMsgAllSeqCh[0] = tMsgProcCh;
                    tMsgAllSeqCh[1] = POWER_COM_AP_CH_NOT_SET;
                }

                for(
                        uiMsgSeqChIdx = 0UL ;
                        ((tMsgAllSeqCh[uiMsgSeqChIdx] != POWER_COM_AP_CH_NOT_SET) &&
                        (uiMsgSeqChIdx<2)) ;
                        uiMsgSeqChIdx++
                    )
                {
                        POWER_COM_D("TxS%02x, TxC%04x, TxD%02x, TxD%02x\n",
                                tMsgAllSeqCh[uiMsgSeqChIdx], usCmd2, ucSendDt[0], ucSendDt[1]);

#ifdef POWER_COM_MSG_BY_ECCP

                    if(
                            ECCP_Send(ptPOWERCOMInfos->tMsgDev,
                                tMsgAllSeqCh[uiMsgSeqChIdx],
                                usCmd2,
                                (uint8 *) &ucSendDt,
                                uiSendDtLen)
                            == (uint32)0
                      )
                        {
                            tRet = POWER_COM_AckRecvWait(tMsgAllSeqCh[uiMsgSeqChIdx], usCmd2);
                        }
                        else
                        {
                            POWER_COM_E("sub send fail \n");
                        }
#endif
                    tMsgAllSeqCh[uiMsgSeqChIdx] = POWER_COM_AP_CH_NOT_SET;
                }
            }
            else
            {
                POWER_COM_D("Unknown Source comm\n");
            }
        }
        else
        {
            POWER_COM_D("Eccp Communication is disconnected\n");
        }
    }

    return tRet;
}

void POWER_COM_RecvMsg
(
    POWERCOMApCh_t                      tMsgCh,
    uint16                              usCmd2,
    uint32                              uiTimeout
)
{
    uint32 uiIdx;
    uint8 ucDropMsg = 0U;

    if(ptPOWERCOMInfos != NULL_PTR)
    {
        for (uiIdx = 0UL; uiIdx < ptPOWERCOMInfos->uiRecvMsgListMaxNum; uiIdx++)
        {
            if(ptPOWERCOMInfos->tRecvMsgList[uiIdx].usCmd2 == usCmd2)
            {
                break;
            }
        }

        if(uiIdx == ptPOWERCOMInfos->uiRecvMsgListMaxNum)
        {
            if((ptPOWERCOMInfos->uiRecvMsgListMaxNum+1UL) <= (POWER_COM_VA_MSG_DATA_LIST_MAX_SIZE))
            {
                ptPOWERCOMInfos->uiRecvMsgListMaxNum++;
                //POWER_COM_D("Listen start   cmd2:x%04x, noti ch:%d, timer:%d\n", usCmd2, (uint32)tMsgCh, uiTimeout);
            }
            else
            {
                POWER_COM_E("Msg List buffer is full. drop a msg ch:%d cmd2:x%04x, timer:%d\n",
                        (uint32)tMsgCh, usCmd2, uiTimeout);
                ucDropMsg = 1U;
            }
        }
        else
        {
            if(tMsgCh == POWER_COM_AP_CH_NOT_SET)
            {
                POWER_COM_D("Listen stop    cmd2:x%04x, noti_ch:%d, timer:%d\n", usCmd2, (uint32)tMsgCh, uiTimeout);
            }
            else
            {
                if(
                        (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA72 == 1U) &&
                        (tMsgCh==POWER_COM_AP_CH_A53)
                  )
                {
                    POWER_COM_D("Listen restart cmd2:x%04x, noti_ch:1 or 2, timer:%d\n",
                            usCmd2, uiTimeout);
                }
                else if(
                        (ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA53 == 1U) &&
                        (tMsgCh==POWER_COM_AP_CH_A72)
                  )
                {
                    POWER_COM_D("Listen restart cmd2:x%04x, noti_ch:1 or 2, timer:%d\n",
                            usCmd2, uiTimeout);
                }
                else
                {
                    POWER_COM_D("Listen restart cmd2:x%04x, noti_ch:%d, timer:%d\n",
                            usCmd2, (uint32)tMsgCh, uiTimeout);
                }
            }
        }

        if(ucDropMsg == 0U)
        {
            ptPOWERCOMInfos->tRecvMsgList[uiIdx].usCmd2 = usCmd2;

            if(tMsgCh == POWER_COM_AP_CH_ALL)
            {
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByAll = 1U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA72 = 0U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA53 = 0U;
            }
            else if(tMsgCh == POWER_COM_AP_CH_A72)
            {
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByAll = 0U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA72 = 1U;
            }
            else if(tMsgCh == POWER_COM_AP_CH_A53)
            {
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByAll = 0U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA53 = 1U;
            }
            else
            {
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByAll = 0U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA72 = 0U;
                ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucNotiByA53 = 0U;
            }

            ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA72 = 0U;
            ptPOWERCOMInfos->tRecvMsgList[uiIdx].ucRecvByA53 = 0U;

            if((uiTimeout > 0UL) && (tMsgCh != POWER_COM_AP_CH_NOT_SET))
            {
                if(ptPOWERCOMInfos->tTimeoutMsgCh != POWER_COM_AP_CH_NOT_SET)
                {
                    POWER_COM_SetMsgTimer(POWER_COM_AP_CH_NOT_SET, 0UL, 0UL);
                }
                POWER_COM_SetMsgTimer(tMsgCh, usCmd2, uiTimeout);
            }
            else
            {
                if(ptPOWERCOMInfos->usTimeoutMsgCmd2 == usCmd2)
                {
                    POWER_COM_SetMsgTimer(POWER_COM_AP_CH_NOT_SET, 0UL, 0UL);
                }
            }
        }
    }
    else
    {
        POWER_COM_E("Can not open message device.\n");
    }
}

static void delay1us
(
   uint32                               uiUs
)
{
   uint32                               uiCnt;
   uint32                               uiSec;

   uiCnt = 0xffUL;
   uiSec = 0;

   uiSec = uiUs * (uint32)500UL;

   for (uiCnt = 0; uiCnt < uiSec; uiCnt++)
   {
       BSP_NOP_DELAY();
   }
}

#endif

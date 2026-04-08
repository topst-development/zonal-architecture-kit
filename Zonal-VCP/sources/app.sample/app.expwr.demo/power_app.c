// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : power_app.c
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

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    #include "power_app.h"
    #include "power_com.h"
    #include <pmio.h>
#endif

#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
    #include <pmio_dev_extn_pw_ctl.h>
#endif

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)

#include <debug.h>
#include <gpio.h>
#include <gic.h>
#include <timer.h>
#include <rtc.h>


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/
#if 1
#define POWER_APP_MSG_D(args...)
#else
#define POWER_APP_MSG_D(args...)            { \
                                            mcu_printf("[POWER_APP] "); \
                                            mcu_printf(args); \
                                        }
#endif



#if 0
#define POWER_APP_D(args...)            { \
                                            mcu_printf("[POWER_APP] "); \
                                            mcu_printf(args); \
                                        }
#else
#define POWER_APP_D(fmt, args...)
//#define POWER_APP_D(fmt, args...)            {LOGD(DBG_TAG_PMIO, fmt, ## args)}
#endif

#define POWER_APP_E(args...)            { \
                                            mcu_printf("[POWER_APP][%s:%d] Error ! ",__func__, __LINE__); \
                                            mcu_printf(args); \
                                        }

#define POWER_APP_I(args...)            { \
                                            mcu_printf("[%s]",__func__); \
                                            mcu_printf(args); \
                                        }

#define POWER_APP_FUNC_MSEC_TO_USEC(x)       ((uint32)(x) * (uint32)(1000UL))
#define POWER_APP_FUNC_SEC_TO_USEC(x)        ((uint32)(x) * (uint32)(1000UL) * (uint32)(1000UL))
#define POWER_APP_VA_MSG_RECV_TIME_LIMIT     POWER_APP_FUNC_SEC_TO_USEC(60UL)
/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)

typedef struct{
    uint8 ucAppReady;
    uint8 ucPrepareStr;
    uint8 ucStr;
}POWERAPPMsgProceState_t;

typedef enum
{
    POWER_APP_MSG_NOT_SET                = (uint16)0x0000,
    POWER_APP_MSG_BOOT_REASON_REQ_RECV   = (uint16)0x0101, //used      (AP->R5)
    POWER_APP_MSG_BOOT_REASON            = (uint16)0x0102, //used      (R5->AP)
    POWER_APP_MSG_ACC_ON                 = (uint16)0x0201, //used      (R5->AP)
    POWER_APP_MSG_PREPARE_STR            = (uint16)0x0202, //used      (R5->AP)
    POWER_APP_MSG_APP_READY_RECV         = (uint16)0x0301, //used      (ALL)(AP->R5 : for first mbox ready check)
    POWER_APP_MSG_PREPARE_STR_DONE_RECV  = (uint16)0x0302, //used      (AP->R5)
    POWER_APP_MSG_STR                    = (uint16)0x0401, //used      (R5->AP)
    POWER_APP_MSG_PREPARE_STR_CANCLE     = (uint16)0x0402, //used      (R5->AP)
    POWER_APP_MSG_AP_AWAKE_RECV          = (uint16)0x0507, //used      (AP->R5)
    POWER_APP_MSG_STR_DONE_RECV          = (uint16)0x0504, //used      (AP->R5)
    POWER_APP_MSG_PWDN                   = (uint16)0x0601,
    POWER_APP_MSG_PWDN_DONE_RECV         = (uint16)0x0602,
} POWERAPPMsg_t;

POWERAPPMsgProceState_t gPOWERAPPMps;
#endif


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/
#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
static void POWER_APP_StartCommunication
(
    void
);

static void POWER_APP_ComSendHandlingScenario
(
    POWERCOMApCh_t                          tMsgCh,
    POWERAPPMsg_t                           tMsgType
);

static void POWER_APP_ComRecvHandlingScenario
(
    POWERCOMNoti_t                      tRetNoti,
    POWERCOMApCh_t                      tRetMsgCh,
    uint16                              usCmd2,
    uint8*                              pucDat
);

static void POWER_APP_IrqExtHandlerAccOff
(
    void
);

static void POWER_APP_IrqExtHandlerAccOn
(
    void
);

static void POWER_APP_StartNotiGpkIrq
(
    void
);


#endif
/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
static void POWER_APP_StartCommunication
(
    void
)
{
    /*
       Pass the main channel to communicate
       Pass the sub channel to communicate.
       Pass the POWERCOMMsgData_t array about all message data list. refer to POWERCOMMsgData_t in power_com.h
       Pass the number of POWERCOMMsgData_t array about message data lists.
       Pass a callback function that can be called when recv occurs.
    */
    (void)POWER_COM_Start(POWER_COM_AP_CH_A72, POWER_COM_AP_CH_A53, (POWERCOMNotiCbk)&POWER_APP_ComRecvHandlingScenario);

    /*
     * If POWER_COM_RecvMsg is called with POWER_COM_AP_CH_NOT_SET,
     * the callback function (POWER_APP_ComRecvHandlingScenario) is not called when each
     * channel receives a message.
     *
     * If POWER_COM_RecvMsg is called with POWER_COM_AP_CH_A72 or POWER_COM_AP_CH_A753,
     * the callback function (POWER_APP_ComRecvHandlingScenario) is called when each
     * channel receives a message.
     *
     * If POWER_COM_RecvMsg is called with POWER_COM_AP_CH_ALL
     * the callback function (POWER_APP_ComRecvHandlingScenario) is called when all channels
     * receive a message.

    */

    /*Early mesg may be AWAKe. This is only recv from CH72*/
    POWER_COM_RecvMsg(POWER_COM_AP_CH_A72, (uint16)POWER_APP_MSG_AP_AWAKE_RECV, 0UL);

    POWER_COM_RecvMsg(POWER_COM_AP_CH_ALL, (uint16)POWER_APP_MSG_APP_READY_RECV,
            POWER_APP_VA_MSG_RECV_TIME_LIMIT);

    POWER_COM_RecvMsg(POWER_COM_AP_CH_A72, (uint16)POWER_APP_MSG_BOOT_REASON_REQ_RECV, 0UL);
    POWER_COM_RecvMsg(POWER_COM_AP_CH_A53, (uint16)POWER_APP_MSG_BOOT_REASON_REQ_RECV, 0UL);

    POWER_COM_RecvMsg(POWER_COM_AP_CH_A72, (uint16)POWER_APP_MSG_PWDN_DONE_RECV, 0UL);

    /*
     * If POWER_COM_SendMsg is called with POWER_COM_AP_CH_NOT_SET,
     * Message is not sent.
     *
     * If POWER_COM_SendMsg is called with POWER_COM_AP_CH_A72 or POWER_COM_AP_CH_A753,
     * Message is sent to that channel.
     *
     * If POWER_COM_SendMsg is called with POWER_COM_AP_CH_ALL,
     * Message is sent to all channel.
    */
}

static void POWER_APP_ComSendHandlingScenario
(
    POWERCOMApCh_t                          tMsgCh,
    POWERAPPMsg_t                           tMsgType
)
{
    switch(tMsgType)
    {
        case POWER_APP_MSG_BOOT_REASON:
            {
                if(POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_BOOT_REASON, 1U, 0U) == SAL_RET_FAILED)
                {
                    POWER_APP_E("Send fail\n");
                    POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_BOOT_REASON, 1U, 0U);
                }
                break;
            }
        case POWER_APP_MSG_ACC_ON:
            {
                if(POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_ACC_ON, 0U, 0U) == SAL_RET_FAILED)
                {
                    POWER_APP_E("Send fail\n");
                    POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_ACC_ON, 0U, 0U);
                }
                break;
            }
        case POWER_APP_MSG_PREPARE_STR:
            {
                if(POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_PREPARE_STR, 0U, 0U) == SAL_RET_FAILED)
                {
                    POWER_APP_E("Send fail\n");
                    POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_PREPARE_STR, 0U, 0U);
                }
                else
                {
                    POWER_COM_RecvMsg(tMsgCh, (uint16)POWER_APP_MSG_PREPARE_STR_DONE_RECV, POWER_APP_VA_MSG_RECV_TIME_LIMIT);
                    gPOWERAPPMps.ucPrepareStr = 1U;
                }
                break;
            }
        case POWER_APP_MSG_PREPARE_STR_CANCLE:
            {
                /*Clear to recv PRE-STR-DONE*/
                POWER_COM_RecvMsg(POWER_COM_AP_CH_NOT_SET, (uint16)POWER_APP_MSG_PREPARE_STR_DONE_RECV, 0UL);

                if(POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_PREPARE_STR_CANCLE, 1U, 0U) == SAL_RET_FAILED)
                {
                    POWER_APP_E("Send fail\n");
                    POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_PREPARE_STR_CANCLE, 1U, 0U);
                }
                else
                {
                    gPOWERAPPMps.ucPrepareStr = 0U;
                }
                break;
            }
        case POWER_APP_MSG_STR:
            {
                if(POWER_COM_SendMsg(tMsgCh, (uint16)POWER_APP_MSG_STR, 0U, 0U) == SAL_RET_FAILED)
                {
                    POWER_APP_E("Send fail\n");
                }
                else
                {
                    /*Last mesg is STR-DONE. This is only recv from CH72*/
                    POWER_COM_RecvMsg(POWER_COM_AP_CH_A72, (uint16)POWER_APP_MSG_STR_DONE_RECV, POWER_APP_VA_MSG_RECV_TIME_LIMIT);
                    gPOWERAPPMps.ucStr = 1U;
                }
                break;
            }
        default:
            {
                POWER_APP_D("This is a MsgType without a scenario. %d", tMsgType);
                break;
            }
    }
}

static void POWER_APP_ComRecvHandlingScenario
(
    POWERCOMNoti_t                      tRetNoti,
    POWERCOMApCh_t                      tRetMsgCh,
    uint16                              usCmd2,
    uint8*                              pucDat
)
{
    uint8 ucCheckMsg;

    ucCheckMsg = 0U;

    POWER_APP_MSG_D("======================= Msg Handler ========================\n");

    switch(tRetNoti)
    {
        case POWER_COM_NOTI_NO_ERROR:
            {
                POWER_APP_MSG_D("AP Result : Msg recvd.\n");
                POWER_APP_MSG_D("Go to     : Msg processing mode\n");
                ucCheckMsg = 1U;
                break;
            }
        case POWER_COM_NOTI_TIME_OUT:
            {
                POWER_APP_MSG_D("AP Result : Recv Timeout. msg:x%04x\n", usCmd2);

                if(usCmd2 == POWER_APP_MSG_APP_READY_RECV)
                {
                    POWER_APP_MSG_D("Info      : %s Application NOT ready yet.\n",
                            (tRetMsgCh==POWER_COM_AP_CH_A72)?"A72":
                            ((tRetMsgCh==POWER_COM_AP_CH_A53)?"A53":
                             "A72 & A53"));
                    POWER_APP_MSG_D("===========================================================\n\n");
                }
                else
                {
#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
                    POWER_APP_E("Go to     : POWER DOWN mode\n");
                    PMIO_EXTN_SetStr(0U);
                    PMIO_EXTN_SetPower(0U);
#endif
                    PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
                }
                break;
            }
        default:
            {
                POWER_APP_MSG_D("AP Result : Unknown %d\n", (uint32)tRetNoti);
                break;
            }
    }

    if(ucCheckMsg > 0U)
    {
        POWER_APP_MSG_D("Channel   : %s\n", (tRetMsgCh==POWER_COM_AP_CH_A72)?"A72":((tRetMsgCh==POWER_COM_AP_CH_A53)?"A53":"ALL"));

        switch(usCmd2)
        {
            case (uint16)POWER_APP_MSG_APP_READY_RECV:
                {
                    POWER_APP_MSG_D("Type      : POWER_APP_MSG_APP_READY_RECV\n");
                    POWER_APP_MSG_D("Processing: communication is ready.\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                    gPOWERAPPMps.ucAppReady = 1U;
                    break;
                }
            case (uint16)POWER_APP_MSG_AP_AWAKE_RECV:
                {
                    POWER_APP_I("Type      : POWER_APP_MSG_AP_AWAKE_RECV\n");
                    POWER_APP_MSG_D("Processing: STR Low\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
                    PMIO_EXTN_SetStr(0U);
#endif
                    break;
                }
            case (uint16)POWER_APP_MSG_PREPARE_STR_DONE_RECV:
                {
                    POWER_APP_I("Type      : POWER_APP_MSG_PREPARE_STR_DONE_RECV\n");
                    POWER_APP_MSG_D("Processing: Send a STR msg\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                    POWER_APP_ComSendHandlingScenario(POWER_COM_AP_CH_ALL, POWER_APP_MSG_STR);
                    break;
                }
            case (uint16)POWER_APP_MSG_STR_DONE_RECV:
                {
#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
                    POWER_APP_I("Processing: power down control (str)\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                    PMIO_EXTN_SetStr(1U);
                    PMIO_EXTN_SetPower(0U);
#endif
//                    PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
                    gPOWERAPPMps.ucAppReady = 0U;
                    gPOWERAPPMps.ucPrepareStr = 0U;
                    gPOWERAPPMps.ucStr = 0U;
                    break;
                }
            case (uint16)POWER_APP_MSG_PWDN_DONE_RECV:
                {
                    POWER_APP_I("Type      : POWER_APP_MSG_PWDN_DONE_RECV\n");
#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
                    POWER_APP_MSG_D("Processing: power down control (pwdn)\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                    PMIO_EXTN_SetStr(0U);
                    PMIO_EXTN_SetPower(0U);
#endif
                    PMIO_PowerDown(PMIO_MODE_POWER_DEEP_DOWN);
                    gPOWERAPPMps.ucAppReady = 0U;
                    break;
                }
            case (uint16)POWER_APP_MSG_BOOT_REASON_REQ_RECV:
                {
                    POWER_APP_MSG_D("Type      : POWER_APP_MSG_BOOT_REASON_REQ_RECV\n");

                    if(pucDat[0] == 1U)
                    {
                        /*only disconnect to subcore*/
                        if(tRetMsgCh == POWER_COM_AP_CH_A72)
                        {
                            POWER_APP_MSG_D("Processing: Halt Ch A53. Single core mode.\n");
                            POWER_APP_MSG_D("            Send a BootReason msg\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                            (void)POWER_COM_HaltCh(POWER_COM_AP_CH_A53, 1);
                        }
                        else
                        {
                            POWER_APP_MSG_D("Processing: Halt Ch A72. Single core mode.\n");
                            POWER_APP_MSG_D("            Send a BootReason msg\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                            (void)POWER_COM_HaltCh(POWER_COM_AP_CH_A72, 1);
                        }
                    }
                    else
                    {
                        if(tRetMsgCh == POWER_COM_AP_CH_A72)
                        {
                            (void)POWER_COM_HaltCh(POWER_COM_AP_CH_A53, 0);
                            POWER_APP_MSG_D("Processing: Listen Ch A53. Multi core mode.\n");
                            POWER_APP_MSG_D("            Send a BootReason msg\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                        }
                        else
                        {
                            (void)POWER_COM_HaltCh(POWER_COM_AP_CH_A72, 0);
                            POWER_APP_MSG_D("Processing: Listen Ch A72. Multi core mode.\n");
                            POWER_APP_MSG_D("            Send a BootReason msg\n");
                    POWER_APP_MSG_D("===========================================================\n\n");
                        }
                    }

                    POWER_APP_ComSendHandlingScenario(tRetMsgCh, POWER_APP_MSG_BOOT_REASON);
                    break;
                }
            default:
                {
                    POWER_APP_MSG_D("Type      : UNKNOWN %d\n", usCmd2);
                    break;
                }
        }
    }
}
#endif

static void POWER_APP_IrqExtHandlerAccOff
(
    void
)
{

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    if(gPOWERAPPMps.ucAppReady == 0U)
#endif
    {
#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
        PMIO_EXTN_SetStr(0U);
        PMIO_EXTN_SetPower(0U);
#endif

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
        PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
#endif
    }
#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    else
    {
        /*start str message scenario*/
        POWER_APP_ComSendHandlingScenario(POWER_COM_AP_CH_ALL, POWER_APP_MSG_PREPARE_STR);
    }
#endif
}

static void POWER_APP_IrqExtHandlerAccOn
(
    void
)
{
    POWER_APP_D("IRQ EXTERNAL Handle. ACC On\n");

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    if(gPOWERAPPMps.ucAppReady == 0U)
#endif
    {
#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
        PMIO_EXTN_SetPower(1U);
#endif
    }
#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    else
    {
        if(gPOWERAPPMps.ucPrepareStr == 1U)
        {
            if(gPOWERAPPMps.ucStr == 1U)
            {
                POWER_APP_D("Already Entered STR Protection Section. Wait Msg STR_DONE_RECV.\n");
            }
            else
            {
                POWER_APP_ComSendHandlingScenario(POWER_COM_AP_CH_ALL, POWER_APP_MSG_PREPARE_STR_CANCLE);
            }
        }
        else
        {
            POWER_APP_ComSendHandlingScenario(POWER_COM_AP_CH_ALL, POWER_APP_MSG_ACC_ON);
        }
    }
#endif
}

static void POWER_APP_StartNotiGpkIrq
(
    void
)
{
#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    /*
        This function switches the GPK IRQ handling registered by the PMIO driver to the outside.
        ACC_DET(GPK03) IRQ will be controlled through the handler of POWER APP.
        The handler of POWER APP will decide to external AP's power down through message communication.
    */
    PMIO_SetNotiGpkIrq(
            PMIO_VA_INTERRUPT_SRC_ACC,
            GIC_INT_TYPE_EDGE_FALLING,
            (POWERAppHandler)&POWER_APP_IrqExtHandlerAccOff
            );

    PMIO_SetNotiGpkIrq(
            PMIO_VA_INTERRUPT_SRC_ACC,
            GIC_INT_TYPE_EDGE_RISING,
            (POWERAppHandler)&POWER_APP_IrqExtHandlerAccOn
            );
#endif
}

void POWER_APP_StartDemo
(
    void
)
{
#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1)
    PMIOEXTNExtPwrPins_t tExtPwrPins;

#if 0
    tExtPwrPins.uiPwrEnPin          = GPIO_GPK(10UL);
//    tExtPwrPins.uiAlivePwrConPin    = GPIO_GPB(4UL);
    tExtPwrPins.uiStrModePin        = GPIO_GPK(15UL);
    tExtPwrPins.uiPmicRstPin        = GPIO_GPA(15UL);
    tExtPwrPins.uiTccRstPin         = GPIO_GPA(19UL);
#endif
    tExtPwrPins.uiPwrEnPin          = GPIO_GPB(10UL);
 //   tExtPwrPins.uiAlivePwrConPin    = GPIO_GPK(10UL);
    tExtPwrPins.uiStrModePin        = GPIO_GPK(14UL);
    tExtPwrPins.uiPmicRstPin        = GPIO_GPA(15UL);
    tExtPwrPins.uiTccRstPin         = GPIO_GPA(19UL);

    PMIO_SetGpk(PMIO_GPK(14UL));

    PMIO_EXTN_Init(tExtPwrPins);

    PMIO_EXTN_SetPower(1U);
    PMIO_EXTN_SetStr(0U);
#endif

#if (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    gPOWERAPPMps.ucAppReady= 0U;
    gPOWERAPPMps.ucPrepareStr = 0U;
    gPOWERAPPMps.ucStr = 0U;
    POWER_APP_StartCommunication();
#endif


#if (ACFG_APP_POWER_EXT_AP_CTL_EN == 1) || (ACFG_APP_POWER_COMMUNICATION_EN == 1)
    POWER_APP_StartNotiGpkIrq();
#endif

}

#endif
#endif

// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : ictc_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )

#include "ictc.h"
#include "ictc_test.h"
#include "pdm_test.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"
#include "pdm.h"

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void ICTC_TestDutyPdmEnable
(
    PDMModeConfig_t *                   psPdmModeConfig
);

static void ICTC_TestEdgePdmEnable
(
    PDMModeConfig_t *                   psPdmModeConfig
);

/*************************************************************************************************/
/*                                  ICTC_GetPeriodAndDutyCallBack                                */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_GetPeriodAndDutyCallBack
(
    uint32                              uiChannel,
    uint32                              uiPeriod,
    uint32                              uiDuty
)
{
    if(uiChannel == (uint32)0UL)
    {
        mcu_printf("ICTC %d- SPEED DETECT period %d duty %d  \n\n", uiChannel, uiPeriod, uiDuty);
    }
    else if(uiChannel == (uint32)1UL)
    {
        mcu_printf("ICTC %d- ILL DETECT period %d duty %d  \n\n", uiChannel, uiPeriod, uiDuty);
    }
    else
    {
        mcu_printf("Invalid channel \n");
    }
}

/*************************************************************************************************/
/*                                  ICTC_TestSleepForSec                                         */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestSleepForSec
(
    uint32                              uiSec
)
{
    if((ICTC_REG_MAX_VALUE / 1000UL) > uiSec)
    {
        (void)SAL_TaskSleep(uiSec*(1000UL));
    }
}

/*************************************************************************************************/
/*                                  ICTC_TestSetGpio                                             */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestSetGpio
(
    void
)
{
    /* 70xx */
    /* VEHICLE ILL DETECT */
    (void)GPIO_Config(GPIO_GPC(6UL) , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_DS(0x3UL)));
}


/*************************************************************************************************/
/*                                  ICTC_TestSetIrq                                              */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestSetIrq
(
    uint32                              uiChannel,
    const ICTCModeConfig_t *            psIctcModeConfig
)
{
    ICTC_SetCallBackFunc(uiChannel, (ICTCCallback)&ICTC_GetPeriodAndDutyCallBack);

    ICTC_SetIRQCtrlReg(uiChannel, psIctcModeConfig->mcEnableIrq);
}

/*************************************************************************************************/
/*                                  ICTC_TestSetConfig                                           */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestSetConfig
(
    uint32                              uiChannel,
    const ICTCModeConfig_t *            psIctcModeConfig
)
{
    ICTC_SetTimeoutValue(uiChannel, psIctcModeConfig->mcTimeout);

    ICTC_SetEdgeMatchingValue(uiChannel,
                              psIctcModeConfig->mcREdgeMat,
                              psIctcModeConfig->mcFEdgeMat,
                              psIctcModeConfig->mcEdgeCntMat);

    ICTC_SetCompareRoundValue(uiChannel,
                              psIctcModeConfig->mcPrdRound,
                              psIctcModeConfig->mcDutyRound);
}

/*************************************************************************************************/
/*                                  ICTC_TestSetOperation                                        */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestSetOperation
(
    uint32                              uiChannel,
    const ICTCModeConfig_t *            psIctcModeConfig
)
{
    ICTC_SetOpEnCtrlCounter(uiChannel, psIctcModeConfig->mcEnableCounter);

    ICTC_SetOpModeCtrlReg(uiChannel, psIctcModeConfig->mcOperationMode);
}

/*************************************************************************************************/
/*                                  ICTC_TestStart                                               */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestStart
(
    uint32                          uiChannel,
    uint32                          uiTestMode
)
{
    PDMModeConfig_t pdmModeConfig   = {0,};

    ICTC_EnableCapture(uiChannel);

    if((uint32)ICTC_PRD_DUTY_USING_KEY == uiTestMode)
    {
        ICTC_TestSleepForSec(20UL);
    }

    else if (((uint32)ICTC_DUTY_USING_PDM == uiTestMode) || ((uint32)ICTC_EDGE_USING_PDM == uiTestMode))
    {
        PDM_Init();

        if((uint32)ICTC_DUTY_USING_PDM == uiTestMode)
        {
            ICTC_TestDutyPdmEnable(&pdmModeConfig);
        }
        else
        {
            ICTC_TestEdgePdmEnable(&pdmModeConfig);
        }
    }
    else
    {
        /* Other Test Mode */
    }

    ICTC_DisableCapture(uiChannel);
}

/*************************************************************************************************/
/*                                  ICTC_TestDutyPdmEnable                                       */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestDutyPdmEnable
(
    PDMModeConfig_t *                   psPdmModeConfig
)
{
#ifdef ICTC_USER_CUS_FUNCTION
    ICTCIsrResult_t IsrResult       = {0,};
#endif
    uint32          duty_rate       = 0;
    uint32          PDM_CH          = 0UL;

    mcu_printf("===== Start Phase Mode1 =====\n\n");

    psPdmModeConfig->mcOperationMode    = PDM_OUTPUT_MODE_PHASE_1;
    psPdmModeConfig->mcClockDivide      = 0UL;

    for(duty_rate = 1UL; duty_rate < 10UL; duty_rate++)
    {
        psPdmModeConfig->mcDutyNanoSec1     = duty_rate * 100UL * 1000UL ;
        psPdmModeConfig->mcPeriodNanoSec1   = 1000UL * 1000UL ;
        psPdmModeConfig->mcDutyNanoSec2     = 0UL;
        psPdmModeConfig->mcPeriodNanoSec2   = 0UL;
        psPdmModeConfig->mcInversedSignal   = 1UL;

        (void)PDM_SetConfig(PDM_CH, (PDMModeConfig_t *)psPdmModeConfig);

        (void)PDM_Enable(PDM_CH, PMM_OFF);

        ICTC_TestSleepForSec(3UL);

#ifdef ICTC_USER_CUS_FUNCTION
        ICTC_GetIsrResult(&IsrResult);

        mcu_printf(" ====================================\n");
        mcu_printf("  ictc >>> duty change irq cnt : %d \n", IsrResult.irDtErrCnt);
        mcu_printf("  ictc >>> freq change irq cnt : %d\n", IsrResult.irPrdErrCnt);
        mcu_printf(" ====================================\n\n");
#endif /* End of ICTC_USER_CUS_FUNCTION */
    }

    (void)PDM_Disable(PDM_CH, PMM_OFF);

    mcu_printf("===== End of Phase Mode Sample =====\n\n");
}

/*************************************************************************************************/
/*                                  ICTC_TestEdgePdmEnable                                       */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestEdgePdmEnable
(
    PDMModeConfig_t *                   psPdmModeConfig
)
{
#ifdef ICTC_USER_CUS_FUNCTION
    ICTCIsrResult_t IsrResult       = {0,};
#endif
    uint32          PDM_CH          = 0UL;
    uint32          loop_cnt        = 0xAU;

    mcu_printf("===== Start Phase Mode1 (Loop : 0x%X) =====\n\n", loop_cnt);

    psPdmModeConfig->mcOperationMode    = PDM_OUTPUT_MODE_PHASE_1;
    psPdmModeConfig->mcClockDivide      = 0UL;
    psPdmModeConfig->mcDutyNanoSec1     = 3UL * 100UL * 1000UL ;
    psPdmModeConfig->mcPeriodNanoSec1   = 1000UL * 1000UL ;
    psPdmModeConfig->mcLoopCount        = loop_cnt;

    (void)PDM_SetConfig((uint32)PDM_CH, (PDMModeConfig_t *)psPdmModeConfig);

    (void)PDM_Enable((uint32)PDM_CH, PMM_OFF);

    ICTC_TestSleepForSec(3UL);

    (void)PDM_Disable((uint32)PDM_CH, PMM_OFF);

#ifdef ICTC_USER_CUS_FUNCTION
    ICTC_GetIsrResult(&IsrResult);

    mcu_printf(" ====================================\n");
    mcu_printf("ictc >>>  edge counter full irq cnt : %d \n", IsrResult.irEcntFullCnt);
    mcu_printf("ictc >>>  rising edge: %d \n", IsrResult.irRedgeNotiCnt);
    mcu_printf("ictc >>>  falling edge : %d\n", IsrResult.irFedgeNotiCnt);
    mcu_printf(" ====================================\n\n");

    mcu_printf("===== End of Phase Mode Sample =====\n\n");
#endif /* End of ICTC_USER_CUS_FUNCTION */
}

/*************************************************************************************************/
/*                                  ICTC_TestPeriodDutyUsingKey                                  */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestPeriodDutyUsingKey
(
    void
)
{
    ICTCModeConfig_t    IctcModeConfig  = {0,};

    mcu_printf("===== ICTC TEST - GET DUTY/PERIOD USING KEY (CH : %d) ==== \n", ICTC_CH_0);

    ICTC_TestSetGpio();

    IctcModeConfig.mcTimeout    = 0x0FFFFFFFUL;
    IctcModeConfig.mcREdgeMat   = 0x00000100UL;
    IctcModeConfig.mcFEdgeMat   = 0x00000100UL;
    IctcModeConfig.mcEdgeCntMat = 0x0000FFFFUL;
    IctcModeConfig.mcPrdRound   = 0x0FFFF800UL;
    IctcModeConfig.mcDutyRound  = 0x0FFFF800UL;

    IctcModeConfig.mcEnableIrq      = ICTC_IRQ_CTRL_DT_ERR_IRQ | ICTC_IRQ_CTRL_PRD_ERR_IRQ;
    IctcModeConfig.mcEnableCounter  = ICTC_OPEN_CTRL_FLTCNT_EN | ICTC_OPEN_CTRL_PDCMPCNT_EN;
    IctcModeConfig.mcOperationMode  = ICTC_OPMODE_CTRL_ABS_SEL | ICTC_OPMODE_CTRL_RISING_EDGE | ICTC_OPMODE_CTRL_TCLK_BYPASS |
                                      ICTC_OPMODE_CTRL_FLT_IMM_F_MODE | ICTC_OPMODE_CTRL_FLT_IMM_R_MODE | ICTC_OPMODE_CTRL_DUTY_CMP_ISR |
                                      (66UL /* GPIO_C06 */ << ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET);

    ICTC_Init();

    ICTC_TestSetIrq(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestSetConfig(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestSetOperation(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestStart(ICTC_CH_0, ICTC_PRD_DUTY_USING_KEY);

    mcu_printf("===== FINISH ICTC TEST ==== \n");

}

/*************************************************************************************************/
/*                                  ICTC_TestDutyUsingPDM                                        */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestDutyUsingPDM
(
    void
)
{
    ICTCModeConfig_t                IctcModeConfig  = {0,};

    mcu_printf("===== GET DUTY/PERIOD TEST ICTC-%d ====\n\n", ICTC_CH_0);

    IctcModeConfig.mcTimeout    = 0x0FFFFFFFUL;
    IctcModeConfig.mcREdgeMat   = 0x00000100UL;
    IctcModeConfig.mcFEdgeMat   = 0x00000100UL;
    IctcModeConfig.mcEdgeCntMat = 0x0000FFFFUL;
    IctcModeConfig.mcPrdRound   = 0x0FFFFFF0UL;
    IctcModeConfig.mcDutyRound  = 0x0FFFFFF0UL;

    IctcModeConfig.mcEnableIrq      = ICTC_IRQ_CTRL_DT_ERR_IRQ | ICTC_IRQ_CTRL_PRD_ERR_IRQ;
    IctcModeConfig.mcEnableCounter  = ICTC_OPEN_CTRL_FLTCNT_EN | ICTC_OPEN_CTRL_PDCMPCNT_EN;
    IctcModeConfig.mcOperationMode  = ICTC_OPMODE_CTRL_ABS_SEL | ICTC_OPMODE_CTRL_RISING_EDGE | ICTC_OPMODE_CTRL_TCLK_BYPASS |
                                      ICTC_OPMODE_CTRL_FLT_IMM_F_MODE | ICTC_OPMODE_CTRL_FLT_IMM_R_MODE | ICTC_OPMODE_CTRL_PRDDT_CMP_ISR |
                                      (64UL /* GPIO_C04 */ << ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET);
    ICTC_Init();

    (void)GPIO_Config(GPIO_GPC(4UL) , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_DS(0x3UL))); /* Set GPIO */

    ICTC_TestSetIrq(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestSetConfig(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestSetOperation(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestStart(ICTC_CH_0, ICTC_DUTY_USING_PDM);

    mcu_printf("===== FINISH ICTC TEST ==== \n");
}

/*************************************************************************************************/
/*                                  ICTC_TestEdgeCountUsingPDM                                   */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*************************************************************************************************/
static void ICTC_TestEdgeUsingPDM
(
    void
)
{
    ICTCModeConfig_t                IctcModeConfig  = {0,};

    mcu_printf("===== EDGE TEST ICTC-%d ====\n", ICTC_CH_0);

    IctcModeConfig.mcTimeout    = 0x0FFFFFFFUL;
    IctcModeConfig.mcREdgeMat   = 0x00000003UL;
    IctcModeConfig.mcFEdgeMat   = 0x00000003UL;
    IctcModeConfig.mcEdgeCntMat = 0x00000004UL;
    IctcModeConfig.mcPrdRound   = 0x0FFFFFF0UL;
    IctcModeConfig.mcDutyRound  = 0x0FFFFFF0UL;

    IctcModeConfig.mcEnableIrq      = ICTC_IRQ_CTRL_REDGE_NOTI_IRQ | ICTC_IRQ_CTRL_FEDGE_NOTI_IRQ |
                                      ICTC_IRQ_CTRL_ECNT_FULL_IRQ;
    IctcModeConfig.mcEnableCounter  = ICTC_OPEN_CTRL_TSCNT_EN | ICTC_OPEN_CTRL_FLTCNT_EN |
                                      ICTC_OPEN_CTRL_EDGECNT_EN;
    IctcModeConfig.mcOperationMode  = ICTC_OPMODE_CTRL_ABS_SEL | ICTC_OPMODE_CTRL_FALLING_EDGE | ICTC_OPMODE_CTRL_TCLK_BYPASS |
                                      ICTC_OPMODE_CTRL_FLT_IMM_F_MODE | ICTC_OPMODE_CTRL_FLT_IMM_R_MODE | ICTC_OPMODE_CTRL_PRDDT_CMP_ISR |
                                      (64UL /* GPIO_C04 */ << ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET);
    ICTC_Init();

    (void)GPIO_Config(GPIO_GPC(4UL) , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_DS(0x3UL))); /* Set GPIO */

    ICTC_TestSetIrq(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestSetConfig(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestSetOperation(ICTC_CH_0, &IctcModeConfig);

    ICTC_TestStart(ICTC_CH_0, ICTC_EDGE_USING_PDM);

    mcu_printf("===== FINISH ICTC TEST ==== \n");
}


/*
***************************************************************************************************
*                                       ICTC_SelectTestCase
*
* @param    test case number
* @return
*
* Notes
*
***************************************************************************************************
*/

void ICTC_SelectTestCase
(
    uint32                              uiTestCase
)
{
    switch(uiTestCase)
    {
        case    1:
            ICTC_TestPeriodDutyUsingKey();
            break;
        case    2:
            ICTC_TestDutyUsingPDM();
            break;
        case    3:
            ICTC_TestEdgeUsingPDM();
            break;
        default :
            mcu_printf("== Invaild Test Case ==\n");
            break;
    }
}


#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )


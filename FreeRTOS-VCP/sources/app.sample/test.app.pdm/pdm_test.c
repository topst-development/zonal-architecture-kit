// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : pdm_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )

#include "pdm.h"
#include "pdm_test.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"
#include "pmio.h"

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                           PDM_TestSleepForSec
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void PDM_TestSleepForSec
(
    uint32                              uiSec
)
{
    (void)SAL_TaskSleep(uiSec*(1000UL));

    return;
}

/*
***************************************************************************************************
*                                           PDM_EnableLEDPower
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void PDM_EnableLEDPower
(
    void
)
{
    /* Enable LED power */
    (void) GPIO_Config(LED_PWR_EN, (uint32)(GPIO_FUNC(0U) | GPIO_OUTPUT));
    (void) GPIO_Set(LED_PWR_EN, 1UL);
}

/*
***************************************************************************************************
*                                           PDM_TestPhaseMode1
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void PDM_TestPhaseMode1
(
    void
)
{
    SALRetCode_t    ret             = SAL_RET_FAILED;
    uint32          uiDutyRate      = 0U;
    uint32          uiCnt           = 0U;
    PDMModeConfig_t sModeConfigInfo = {0U,};

    mcu_printf("\n== Start PDM Phase Mode1 Sample ==\n");

    (void)PDM_Init();

    PDM_TestSleepForSec(1U);

    sModeConfigInfo.mcOperationMode     = PDM_OUTPUT_MODE_PHASE_1;
    sModeConfigInfo.mcPortNumber        = GPIO_PERICH_CH0;

    for(uiDutyRate = 0; uiDutyRate <= 1000; uiDutyRate += 100)
    {
        mcu_printf("\n\t********** DUTY %d %% **********\n", uiDutyRate/10);

        sModeConfigInfo.mcPeriodNanoSec1    = 1000UL     * 1000UL;
        sModeConfigInfo.mcDutyNanoSec1      = uiDutyRate * 1000UL;

        (void)PDM_Disable(PDM_CH_0, PMM_ON);

        uiCnt = 0;
        while(PDM_GetChannelStatus(PDM_CH_0))
        {
            /* Wait 1 ms */
            SAL_TaskSleep(1U);

            uiCnt++;
            if(3000U < uiCnt)
            {
                mcu_printf("[%s:%d] ERROR! PDM is not disabled\n", __func__, __LINE__);
                break;
            }
        }

        ret = PDM_SetConfig((uint32)PDM_CH_0, &sModeConfigInfo);

        if(ret == SAL_RET_SUCCESS)
        {
            PDM_Enable((uint32)PDM_CH_0, PMM_ON);
        }

        PDM_TestSleepForSec(4UL);
    }

    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);

    mcu_printf("\n== End of PDM Phase Mode1 Sample ==\n");

    return;
}

/*
***************************************************************************************************
*                                           PDM_TestPhaseMode2
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/

static void PDM_TestPhaseMode2
(
    void
)
{
    SALRetCode_t    ret             = SAL_RET_FAILED;
    PDMModeConfig_t sModeConfigInfo = {0,};

    mcu_printf("\n== Start PDM Phase Mode2 Sample ==\n");

    (void)PDM_Init();

    PDM_TestSleepForSec(1UL);

    sModeConfigInfo.mcPortNumber        = GPIO_PERICH_CH0;
    sModeConfigInfo.mcOperationMode     = PDM_OUTPUT_MODE_PHASE_2;

    sModeConfigInfo.mcDutyNanoSec1      = ((350UL) * (1000UL));
    sModeConfigInfo.mcPeriodNanoSec1    = ((700UL) * (1000UL));
    sModeConfigInfo.mcDutyNanoSec2      = ((150UL) * (1000UL));
    sModeConfigInfo.mcPeriodNanoSec2    = ((300UL) * (1000UL));

    (void)PDM_Disable(PDM_CH_0, PMM_ON);

    PDM_TestSleepForSec(1UL);

    ret = PDM_SetConfig((uint32)PDM_CH_0, (PDMModeConfig_t *)&sModeConfigInfo);

    if(ret == SAL_RET_SUCCESS)
    {
        PDM_Enable((uint32)PDM_CH_0, PMM_ON);
    }

    PDM_TestSleepForSec(10UL);

    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);

    mcu_printf("\n== End of PDM Phase Mode2 Sample ==\n");

    return;
}

/*
***************************************************************************************************
*                                           PDM_TestRegisterMode1
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/

static void PDM_TestRegisterMode1
(
    void
)
{
    SALRetCode_t    ret             = SAL_RET_FAILED;
    PDMModeConfig_t sModeConfigInfo = {0,};

    mcu_printf("\n== Start PDM Register Mode1 Sample ==\n");

    (void)PDM_Init();

    PDM_TestSleepForSec(1UL);

    sModeConfigInfo.mcPortNumber        = GPIO_PERICH_CH0;
    sModeConfigInfo.mcOperationMode     = PDM_OUTPUT_MODE_REGISTER_1;
    sModeConfigInfo.mcClockDivide       = 1UL;

    sModeConfigInfo.mcOutPattern1       = 0x0000FFFFUL;
    sModeConfigInfo.mcOutPattern2       = 0UL;
    sModeConfigInfo.mcOutPattern3       = 0UL;
    sModeConfigInfo.mcOutPattern4       = 0UL;
    sModeConfigInfo.mcMaxCount          = 0UL;

    (void)PDM_Disable(PDM_CH_0, PMM_ON);

    PDM_TestSleepForSec(1UL);

    ret = PDM_SetConfig((uint32)PDM_CH_0, &sModeConfigInfo);

    if(ret == SAL_RET_SUCCESS)
    {
        PDM_Enable((uint32)PDM_CH_0, PMM_ON);
    }

    PDM_TestSleepForSec(10UL);

    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);

    mcu_printf("\n== End of PDM Register Mode1 Sample ==\n");

    return;
}

/*
***************************************************************************************************
*                                           PDM_TestPhaseMode1forRGBLED
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void PDM_TestPhaseMode1forRGBLED
(
    void
)
{
    uint32          uiCh            = 0;
    uint32          uiCnt           = 0;
    PDMModeConfig_t sModeConfigInfo = {0,};

    uiCnt = 0UL;

    mcu_printf("\n== Start PDM Phase Mode1 Sample for RGB LED ==\n");

    PDM_EnableLEDPower();

    PDM_Init();

    sModeConfigInfo.mcOperationMode = PDM_OUTPUT_MODE_PHASE_1;
    sModeConfigInfo.mcClockDivide = 0UL;
    sModeConfigInfo.mcLoopCount = 0UL;
    sModeConfigInfo.mcInversedSignal = 0UL;

    sModeConfigInfo.mcPosition1 = 0UL;
    sModeConfigInfo.mcPosition2 = 0UL;
    sModeConfigInfo.mcPosition3 = 0UL;
    sModeConfigInfo.mcPosition4 = 0UL;

    sModeConfigInfo.mcOutPattern1 = 0UL;
    sModeConfigInfo.mcOutPattern2 = 0UL;
    sModeConfigInfo.mcOutPattern3 = 0UL;
    sModeConfigInfo.mcOutPattern4 = 0UL;
    sModeConfigInfo.mcMaxCount = 0UL;

    /* GREEN */
    mcu_printf("\n== Blink GREEN LED ==\n");

    for(uiCnt = 1; uiCnt < 10; uiCnt+=8)
    {
        uiCh = 4UL; /* PDM1-A */
        sModeConfigInfo.mcPortNumber = GPIO_PERICH_CH0; /* GPIO_A14 */
        sModeConfigInfo.mcDutyNanoSec1 = uiCnt * 100UL * 1000UL * 1000UL;
        sModeConfigInfo.mcPeriodNanoSec1 = 1000UL * 1000UL * 1000UL;
        sModeConfigInfo.mcDutyNanoSec2 = 0UL;
        sModeConfigInfo.mcPeriodNanoSec2 = 0UL;

        (void)PDM_SetConfig(uiCh, (PDMModeConfig_t *)&sModeConfigInfo);

        (void)PDM_Enable(uiCh, PMM_ON);

        /* sleep 5sec for checking signal */
        PDM_TestSleepForSec(5UL);
        (void)PDM_Disable(uiCh, PMM_ON);
        PDM_TestSleepForSec(1UL);
    }

    /* BLUE */
    mcu_printf("\n== Blink BLUE LED ==\n");

    for(uiCnt = 1; uiCnt < 10; uiCnt+=8)
    {
        uiCh = 8UL; /* PDM2-A */
        sModeConfigInfo.mcPortNumber = GPIO_PERICH_CH0; /* GPIO_A18 */
        sModeConfigInfo.mcDutyNanoSec1 = uiCnt * 100UL * 1000UL * 1000UL;
        sModeConfigInfo.mcPeriodNanoSec1 = 1000UL * 1000UL * 1000UL;
        sModeConfigInfo.mcDutyNanoSec2 = 0UL;
        sModeConfigInfo.mcPeriodNanoSec2 = 0UL;

        (void)PDM_SetConfig(uiCh, (PDMModeConfig_t *)&sModeConfigInfo);

        (void)PDM_Enable(uiCh, PMM_ON);

        /* sleep 5sec for checking signal */
        PDM_TestSleepForSec(5UL);
        (void)PDM_Disable(uiCh, PMM_ON);
        PDM_TestSleepForSec(1UL);
    }

    /* RED */
    mcu_printf("\n== Blink RED LED ==\n");

    for(uiCnt = 1; uiCnt < 10; uiCnt+=8)
    {
        uiCh = 7UL; /* PDM1-D */
        sModeConfigInfo.mcPortNumber = GPIO_PERICH_CH1; /* GPIO_B27 */
        sModeConfigInfo.mcDutyNanoSec1 = uiCnt * 100UL * 1000UL * 1000UL;
        sModeConfigInfo.mcPeriodNanoSec1 = 1000UL * 1000UL * 1000UL;
        sModeConfigInfo.mcDutyNanoSec2 = 0UL;
        sModeConfigInfo.mcPeriodNanoSec2 = 0UL;

        (void)PDM_SetConfig(uiCh, (PDMModeConfig_t *)&sModeConfigInfo);

        (void)PDM_Enable(uiCh, PMM_ON);

        /* sleep 5sec for checking signal */
        PDM_TestSleepForSec(5UL);
        (void)PDM_Disable(uiCh, PMM_ON);
        PDM_TestSleepForSec(1UL);
    }

    mcu_printf("\n== End PDM Phase Mode1 Sample for RGB LED ==\n");

    return;
}

/*
***************************************************************************************************
*                                           PDM_SelectTestCase
*
* @param test case number
* @return
*
* Notes
*
***************************************************************************************************
*/
void PDM_SelectTestCase
(
    uint32                              uiTestCase
)
{
    switch(uiTestCase)
    {
        case    1:
        {
            PDM_TestPhaseMode1();
            break;
        }

        case    2:
        {
            PDM_TestPhaseMode2();
            break;
        }

        case    3:
        {
            PDM_TestRegisterMode1();
            break;
        }

        case    4:
        {
            PDM_TestPhaseMode1forRGBLED();
            break;
        }

        default :
        {
            mcu_printf("\n== Invaild Test Case ==\n");
            break;
        }
    }

    return;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )


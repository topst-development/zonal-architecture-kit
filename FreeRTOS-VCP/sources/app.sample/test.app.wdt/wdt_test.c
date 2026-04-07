// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wdt_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )

#include <wdt_test.h>

#include <gic.h>
#include <reg_phys.h>
#include <debug.h>

#include <wdt.h>
#include <wdt_pmu.h>
#include <timer.h>
#include <gpio.h>
/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define WDT_TESTING_REG_WDT             (4UL)
#define WDT_REG_TB_SIZE                 (WDT_TESTING_REG_WDT << 1UL)


static uint32 mWDTRegTb[WDT_REG_TB_SIZE] =
{
    0x00, 0x00000001UL,    // WDT_EN
    //0x04, 0x00000001UL,    // WDT_CLR
    0x08, 0xFFFFFFFFUL,    // WDT_IRQ_CNT
    0x0C, 0xFFFFFFFFUL,    // WDT_RSR_CNT
    0x10, 0x0000000FUL,    // WDT_SM_MODE
};

static uint32 mWDTPMURegTb[WDT_REG_TB_SIZE] =
{
    0x6C, 0x00000001UL,    // PMU_WDT_EN
    //0x70, 0x00000001UL,    // PMU_WDT_CLR
    0x74, 0xFFFFFFFFUL,    // PMU_WDT_IRQ_CNT
    0x78, 0xFFFFFFFFUL,    // PMU_WDT_RSR_CNT
    0x7C, 0x0000000FUL,    // PMU_WDT_SM_MODE
};


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void WDT_Operation1Test
(
    void
);

static void WDT_Operation2Test
(
    void
);

static void WDT_WriteProtectionTest
(
    void
);

static void WDT_WriteProtectionWithFaultTest
(
    void
);

static void WDT_DriverDevelTest
(
    void
);

static void WDT_DriverDevelResetTest
(
    void
);

static void WDT_SMTest
(
    void
);

static void WDTPMU_DriverDevelTest
(
    void
);

static void WDTPMU_DriverDevelResetTest
(
    void
);

static void WDTPMU_Operation1Test
(
    void
);

static void WDTPMU_Operation2Test
(
    void
);

static void WDTPMU_WriteProtectionTest
(
    void
);

static void WDTPMU_WriteProtectionWithFaultTest
(
    void
);

static void WDTPMU_SMTest
(
    void
);


/*
*********************************************************************************************************
*                                         FUNCTIONS
*********************************************************************************************************
*/

static sint32 WDT_KickTimerHandler(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mcu_printf("[SM-PV] WDT.01, Windowed Watchdog Timer, Kicking Action!!\n");
    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection Test, Password Write Protection, Kicking Action!!\n");

    WDT_KickPing();

    return (int32)SAL_RET_SUCCESS;
}

static sint32 WDT_SuccessCallback(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mcu_printf("[SM-PV] WDT.01, Windowed Watchdog Timer, Counter Clearing operation Test PASS!!\n");

    return (int32)SAL_RET_SUCCESS;
}

static sint32 WDT_FailCallback(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mcu_printf("[SM-PV] WDT.01, Windowed Watchdog Timer, Wrong Kick action operation Test FAIL!!\n");

    return (int32)SAL_RET_SUCCESS;
}

static void WDT_Operation1Test(void)
{
    SALRetCode_t ret;
    uint32 timerSec         = 12UL * 1000UL * 1000UL;     //12 sec
    uint32 successTimerSec  = 23UL * 1000UL * 1000UL;     //23 sec

    mcu_printf("\n[SM-PV] WDT.01, Windowed Watchdog Timer, Counter Clearing operation START!!\n");

    ret = WDT_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_Enable(TIMER_CH_5, timerSec, (TIMERHandler)&WDT_KickTimerHandler, NULL_PTR);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5 );

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   successTimerSec,
                                   TIMER_OP_FREERUN,
                                   (TIMERHandler)&WDT_SuccessCallback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static void WDT_Operation2Test(void)
{
    SALRetCode_t ret;
    uint32 timerSec     = 8UL * 1000UL * 1000UL;     //8 sec
    uint32 failTimerSec = 24UL * 1000UL * 1000UL;     //24 sec

    mcu_printf("\n[SM-PV] WDT.01, Windowed Watchdog Timer, Wrong Kick action operation START!!\n");

    ret = WDT_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_EnableWithMode(TIMER_CH_5,
                                   timerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDT_KickTimerHandler,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   failTimerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDT_FailCallback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static sint32 WDT_Succes02Callback(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection Test PASS!!\n");

    return (sint32)SAL_RET_SUCCESS;
}

static void WDT_WriteProtectionTest(void)
{
    SALRetCode_t isSuccess;
    SALRetCode_t ret;
    uint32 index;
    uint16 bitIndex;
    uint32 offset;
    uint32 mask;
    uint32 uiRegAddr;
    uint32 readWord;
    uint32 writingWord;
    uint32 testingBit;

    uint32 timerSec         = 15UL * 1000UL * 1000UL;     //15 sec
    uint32 successTimerSec  = 50UL * 1000UL * 1000UL;     //50 sec

    mcu_printf("\n[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection Test START!!\n");
    isSuccess = SAL_RET_SUCCESS;

    for (index = 0U; index < WDT_TESTING_REG_WDT; index++)
    {
        mask = 0x1U;
        offset = mWDTRegTb[(index << 1U)];
        uiRegAddr = (MCU_BSP_WDT_BASE + offset);
        readWord = SAL_ReadReg(uiRegAddr);

        for (bitIndex = 0U; bitIndex < 32U; bitIndex++)
        {
            if ((mWDTRegTb[(index << 1U) + 1U] & mask) == 0x0U)
            {
                mask <<= 1;
                continue;
            }

            testingBit = readWord & mask;
            writingWord = readWord ^ ((testingBit == 0x0U) ? (testingBit | mask) : (testingBit));
            uiRegAddr = MCU_BSP_WDT_BASE + 0x14U;
            SAL_WriteReg(0x5AFEACE5U, uiRegAddr);       //0x5AFEACE5U
            uiRegAddr = MCU_BSP_WDT_BASE + offset;
            SAL_WriteReg(writingWord, uiRegAddr);

            if (SAL_ReadReg(uiRegAddr) != writingWord)
            {
                mcu_printf("Fail to test. [ register : 0x%08x, bit offset : %d, Read : 0x%08x, Write : 0x%08x ]\n",
                       MCU_BSP_WDT_BASE + offset,
                       bitIndex,
                       readWord,
                       writingWord);
                isSuccess = SAL_RET_FAILED;

                break;
            }

            mask <<= 1;
        }

        if (isSuccess == SAL_RET_FAILED)
        {
            break;
        }
        else
        {
            mcu_printf("0x%08x Register is OK!\n", MCU_BSP_WDT_BASE + offset);
        }
    }

    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection Test COMPLETED!!\n");
    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection Test, WDT_CLR is verified NOW!!\n");

    ret = WDT_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_Enable(TIMER_CH_5, timerSec, (TIMERHandler)&WDT_KickTimerHandler, NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   successTimerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDT_Succes02Callback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static sint32 WDT_MissKickTimerHandler(TIMERChannel_t iChannel, void * pArgs)
{
    uint32 uiRegAddr;
    (void)iChannel;
    (void)pArgs;
    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection Test, Password Write Protection with Fault, Kicking Action ==> Missing (Without PW)!!\n");
    uiRegAddr = MCU_BSP_WDT_BASE + 0x4U;
    SAL_WriteReg(0x1U, uiRegAddr);

    return (sint32)SAL_RET_SUCCESS;
}

static sint32 WDT_Fail02Callback(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault Test FAIL!!\n");

    return (sint32)SAL_RET_SUCCESS;
}

static void WDT_WriteProtectionWithFaultTest(void)
{
    SALRetCode_t isSuccess;
    SALRetCode_t ret;
    uint32 index;
    uint16 bitIndex;
    uint32 offset;
    uint32 mask;
    uint32 readWord;
    uint32 writingWord;
    uint32 testingBit;
    uint32 uiRegAddr;


    uint32 timerSec     = 15UL * 1000UL * 1000UL;     //15 sec
    uint32 failTimerSec = 50UL * 1000UL * 1000UL;     //50 sec

    mcu_printf("\n[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault Test START!!\n");
    isSuccess = SAL_RET_SUCCESS;

    for (index = 0; index < WDT_TESTING_REG_WDT; index++)
    {
        mask = 0x1U;
        offset = mWDTRegTb[index << 1U];

        uiRegAddr = (MCU_BSP_WDT_BASE + offset);
        readWord = SAL_ReadReg(uiRegAddr);

        for (bitIndex = 0U; bitIndex < 32U; bitIndex++)
        {
            if ((mWDTRegTb[(index << 1U) + 1U] & mask) == 0x0U)
            {
                mask <<= 1;
                continue;
            }

            testingBit = readWord & mask;
            writingWord = readWord ^ ((testingBit == 0x0U) ? (testingBit | mask) : (testingBit));

            uiRegAddr = (MCU_BSP_WDT_BASE + offset);
            SAL_WriteReg(writingWord, uiRegAddr);

            if (SAL_ReadReg(uiRegAddr) == writingWord)
            {
                mcu_printf("Fail to test. [ register : 0x%08x, bit offset : %d, Read : 0x%08x, Write : 0x%08x ]\n",
                       MCU_BSP_WDT_BASE + offset,
                       bitIndex,
                       readWord,
                       writingWord);
                isSuccess = SAL_RET_FAILED;

                break;
            }
            mask <<= 1;
        }

        if (isSuccess == SAL_RET_FAILED)
        {
            break;
        }
        else
        {
            mcu_printf("0x%08x Register is OK!\n", MCU_BSP_WDT_BASE + offset);
        }
    }

    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault Test COMPLETED!!\n");
    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault Test, WDT_CLR is verified NOW!!\n");

    ret = WDT_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_Enable(TIMER_CH_5, timerSec, (TIMERHandler)&WDT_MissKickTimerHandler, NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   failTimerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDT_Fail02Callback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static sint32 WDT_ResetSystemCallback(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;
    /* [IP-PV] MC-18, Windowed Watchdog Timer, Resetting System */
    /* 18. Watchdog Driver Development And Verification, Resetting System, Step 1 */
    WDT_ResetSystem();

    return (sint32)SAL_RET_SUCCESS;
}

static void WDT_DriverDevelTest(void)
{
    mcu_printf("\n19. Watchdog Driver Development And Verification, Initializing Watchdog, Step 1\n");
    mcu_printf("19. Watchdog Driver Development And Verification, Enabling Watchdog, Step 1\n");
    (void)WDT_Init();

    mcu_printf("\n[IP-PV] 19, Windowed Watchdog Timer, Disabling Windowed Watchdog Timer\n");
    mcu_printf("19. Watchdog Driver Development And Verification, Disabling Watchdog, Step 1\n");
    mcu_printf("19. Watchdog Driver Development And Verification, Initializing Watchdog, Step 2\n");
    WDT_Stop();
    (void)WDT_Init();
}

static void WDT_DriverDevelResetTest(void)
{
    uint32 resetTimerSec = 120UL * 1000UL * 1000UL;     //120 sec

    mcu_printf("\n[IP-PV] 19, Windowed Watchdog Timer, Enabling Windowed Watchdog Timer\n");
    mcu_printf("\n19. Watchdog Driver Development And Verification, Kicking  Watchdog, Step 1\n");
    (void)WDT_Init();

    mcu_printf("\n19. Watchdog Driver Development And Verification, Resetting System, Step 1\n");
    (void)TIMER_Enable(TIMER_CH_5, resetTimerSec, (TIMERHandler)&WDT_ResetSystemCallback, NULL);
    (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);
}

static void WDT_SMTest(void)
{
    /** You Must set WDT_SM_FAULT_TEST as SALEnabled **/
    mcu_printf("\n[IP-PV] 19. Windowed Watchdog Timer, Safety Mechanism Test\n");
    (void)WDT_Init();
    (void)WDT_SmMode(TRUE, WDT_RST_FMU_HANDLER, WDT_2OO3_VOTE);
}

static uint32 wdtCounter = 0;
static uint32 wdtDuration = 0;

static sint32 WDT_AliveCallback(TIMERChannel_t iChannel, void * pArgs)
{
     uint32 duration = 0;

    (void)iChannel;
    (void)pArgs;

    wdtCounter++;

    duration = wdtDuration * wdtCounter;

    mcu_printf("This system is alive. %d hour %d min %d sec\n", duration/3600, (duration/60)%60, duration%60);

    return (sint32)SAL_RET_SUCCESS;
}

static void WDTPMU_DriverDevelTest(void)
{
    mcu_printf("\n19. PMU Watchdog Driver Development And Verification, Initializing Watchdog, Step 1\n");
    mcu_printf("19. PMU Watchdog Driver Development And Verification, Enabling Watchdog, Step 1\n");
    (void)WDTPMU_Init();

    mcu_printf("\n[IP-PV] 19 Windowed Watchdog Timer, Disabling Windowed Watchdog Timer\n");
    mcu_printf("19. PMU Watchdog Driver Development And Verification, Disabling Watchdog, Step 1\n");
    mcu_printf("19. PMU Watchdog Driver Development And Verification, Initializing Watchdog, Step 2\n");
    WDTPMU_Stop();
    (void)WDTPMU_Init();
}

static sint32 WDTPMU_ResetSystemCallback(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;
    /* PMU Windowed Watchdog Timer, Resetting System */
    /* PMU Watchdog Driver Development And Verification, Resetting System, Step 1 */
    WDTPMU_ResetSystem();

    return (sint32)SAL_RET_SUCCESS;
}


static void WDTPMU_DriverDevelResetTest(void)
{
    uint32 resetTimerSec = 120UL * 1000UL * 1000UL;     //120 sec

    mcu_printf("\n[IP-PV] 19. PMU Windowed Watchdog Timer, Enabling Windowed Watchdog Timer\n");
    mcu_printf("\n19. PMU Watchdog Driver Development And Verification, Kicking  Watchdog, Step 1\n");
    (void)WDTPMU_Init();

    mcu_printf("\n19. PMU Watchdog Driver Development And Verification, Resetting System, Step 1\n");
    (void)TIMER_Enable(TIMER_CH_5, resetTimerSec, (TIMERHandler)&WDTPMU_ResetSystemCallback, NULL);
    (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);
}

static sint32 WDTPMU_KickTimerHandler(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mcu_printf("19. PMU Windowed Watchdog Timer, Kicking Action!!\n");
    mcu_printf("19. PMU Register Write Lock with Password Write Protection Test, Password Write Protection, Kicking Action!!\n");

    WDTPMU_KickPing();

    return (int32)SAL_RET_SUCCESS;
}


static void WDTPMU_Operation1Test(void)
{
    SALRetCode_t ret;
    uint32 timerSec         = 12UL * 1000UL * 1000UL;     //12 sec
    uint32 successTimerSec  = 23UL * 1000UL * 1000UL;     //23 sec

    mcu_printf("\n[SM-PV] WDT.01, PMU Windowed Watchdog Timer, Counter Clearing operation START!!\n");

    ret = WDTPMU_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_Enable(TIMER_CH_5, timerSec, (TIMERHandler)&WDTPMU_KickTimerHandler, NULL_PTR);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5 );

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   successTimerSec,
                                   TIMER_OP_FREERUN,
                                   (TIMERHandler)&WDT_SuccessCallback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static void WDTPMU_Operation2Test(void)
{
    SALRetCode_t ret;
    uint32 timerSec     = 8UL * 1000UL * 1000UL;     //8 sec
    uint32 failTimerSec = 24UL * 1000UL * 1000UL;     //24 sec

    mcu_printf("\n[SM-PV] WDT.01, PMU Windowed Watchdog Timer, Wrong Kick action operation START!!\n");

    ret = WDTPMU_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_EnableWithMode(TIMER_CH_5,
                                   timerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDTPMU_KickTimerHandler,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   failTimerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDT_FailCallback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static void WDTPMU_WriteProtectionTest(void)
{
    SALRetCode_t isSuccess;
    SALRetCode_t ret;
    uint32 index;
    uint16 bitIndex;
    uint32 offset;
    uint32 mask;
    uint32 uiRegAddr;
    uint32 readWord;
    uint32 writingWord;
    uint32 testingBit;

    uint32 timerSec         = 15UL * 1000UL * 1000UL;     //15 sec
    uint32 successTimerSec  = 50UL * 1000UL * 1000UL;     //50 sec

    mcu_printf("\n[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection Test START!!\n");
    isSuccess = SAL_RET_SUCCESS;

    for (index = 0U; index < WDT_TESTING_REG_WDT; index++)
    {
        mask = 0x1U;
        offset = mWDTPMURegTb[(index << 1U)];
        uiRegAddr = (MCU_BSP_PMU_BASE + offset);
        readWord = SAL_ReadReg(uiRegAddr);

        for (bitIndex = 0U; bitIndex < 32U; bitIndex++)
        {
            if ((mWDTPMURegTb[(index << 1U) + 1U] & mask) == 0x0U)
            {
                mask <<= 1;
                continue;
            }

            testingBit = readWord & mask;
            writingWord = readWord ^ ((testingBit == 0x0U) ? (testingBit | mask) : (testingBit));
            uiRegAddr = MCU_BSP_PMU_BASE + 0x3FCU;
            SAL_WriteReg(0x5AFEACE5U, uiRegAddr);       //0x5AFEACE5U
            uiRegAddr = MCU_BSP_PMU_BASE + offset;
            SAL_WriteReg(writingWord, uiRegAddr);

            if (SAL_ReadReg(uiRegAddr) != writingWord)
            {
                mcu_printf("Fail to test. [ register : 0x%08x, bit offset : %d, Read : 0x%08x, Write : 0x%08x ]\n",
                       MCU_BSP_PMU_BASE + offset,
                       bitIndex,
                       readWord,
                       writingWord);
                isSuccess = SAL_RET_FAILED;

                break;
            }

            mask <<= 1;
        }

        if (isSuccess == SAL_RET_FAILED)
        {
            break;
        }
        else
        {
            mcu_printf("0x%08x Register is OK!\n", MCU_BSP_PMU_BASE + offset);
        }
    }

    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection Test COMPLETED!!\n");
    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection Test, WDT_CLR is verified NOW!!\n");

    ret = WDTPMU_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_Enable(TIMER_CH_5, timerSec, (TIMERHandler)&WDTPMU_KickTimerHandler, NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   successTimerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDT_Succes02Callback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static sint32 WDTPMU_MissKickTimerHandler(TIMERChannel_t iChannel, void * pArgs)
{
    uint32 uiRegAddr;
    (void)iChannel;
    (void)pArgs;
    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection Test, Password Write Protection with Fault, Kicking Action ==> Missing (Without PW)!!\n");
    uiRegAddr = MCU_BSP_PMU_BASE + 0x4U;
    SAL_WriteReg(0x1U, uiRegAddr);

    return (sint32)SAL_RET_SUCCESS;
}

static void WDTPMU_WriteProtectionWithFaultTest(void)
{
    SALRetCode_t isSuccess;
    SALRetCode_t ret;
    uint32 index;
    uint16 bitIndex;
    uint32 offset;
    uint32 mask;
    uint32 readWord;
    uint32 writingWord;
    uint32 testingBit;
    uint32 uiRegAddr;


    uint32 timerSec     = 15UL * 1000UL * 1000UL;     //15 sec
    uint32 failTimerSec = 50UL * 1000UL * 1000UL;     //50 sec

    mcu_printf("\n[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault Test START!!\n");
    isSuccess = SAL_RET_SUCCESS;

    for (index = 0; index < WDT_TESTING_REG_WDT; index++)
    {
        mask = 0x1U;
        offset = mWDTPMURegTb[index << 1U];

        uiRegAddr = (MCU_BSP_PMU_BASE + offset);
        readWord = SAL_ReadReg(uiRegAddr);

        for (bitIndex = 0U; bitIndex < 32U; bitIndex++)
        {
            if ((mWDTPMURegTb[(index << 1U) + 1U] & mask) == 0x0U)
            {
                mask <<= 1;
                continue;
            }

            testingBit = readWord & mask;
            writingWord = readWord ^ ((testingBit == 0x0U) ? (testingBit | mask) : (testingBit));

            uiRegAddr = (MCU_BSP_PMU_BASE + offset);
            SAL_WriteReg(writingWord, uiRegAddr);

            if (SAL_ReadReg(uiRegAddr) == writingWord)
            {
                mcu_printf("Fail to test. [ register : 0x%08x, bit offset : %d, Read : 0x%08x, Write : 0x%08x ]\n",
                       MCU_BSP_PMU_BASE + offset,
                       bitIndex,
                       readWord,
                       writingWord);
                isSuccess = SAL_RET_FAILED;

                break;
            }
            mask <<= 1;
        }

        if (isSuccess == SAL_RET_FAILED)
        {
            break;
        }
        else
        {
            mcu_printf("0x%08x Register is OK!\n", MCU_BSP_PMU_BASE + offset);
        }
    }

    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault Test COMPLETED!!\n");
    mcu_printf("[SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault Test, WDT_CLR is verified NOW!!\n");

    ret = WDTPMU_Init();

    if (ret == SAL_RET_SUCCESS)
    {
        (void)TIMER_Enable(TIMER_CH_5, timerSec, (TIMERHandler)&WDTPMU_MissKickTimerHandler, NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);

        (void)TIMER_EnableWithMode(TIMER_CH_4,
                                   failTimerSec,
                                   TIMER_OP_ONESHOT,
                                   (TIMERHandler)&WDT_Fail02Callback,
                                   NULL);
        (void)GIC_IntSrcEn((uint32)GIC_TIMER_4);
    }
}

static void WDTPMU_SMTest(void)
{
    /** You Must set WDT_SM_FAULT_TEST as SALEnabled **/
    mcu_printf("\n[IP-PV] 19. PMU Windowed Watchdog Timer, Safety Mechanism Test\n");
    (void)WDTPMU_Init();
    //(void)WDTPMU_SmMode(TRUE, WDT_RST_DIRECT, WDT_2OO3_VOTE);           //==> ES 에서 사용
    (void)WDTPMU_SmMode(TRUE, WDT_RST_FMU_HANDLER, WDT_2OO3_VOTE);      //==> CS 에서 사용
}

/*
***************************************************************************************************
*                                          Safety watchdog test
*
* This function is to verify the operation of the safety watchdog
*
* @param    uiFeedInterval [sec] Feeding Interval value,  ex) 10 means 10second
*
* Notes
*
* WDI (GPIO_K15) : Connect between GPIO_K15 and GPIO_C12
*                  GPIO_C, Ouput, toggle for WDI pulse,
*.WDO (GPIO_K17) : Low Active, if DISn is High and GPIO_K17 is LOW, Reset occur
*
* See VPU specification, chapter 31 : Safety watchdog
*
*.Check 3-item : Normal operation, Double edge fault, Timeout fault
***************************************************************************************************
*/
static void Safety_Wdt_Test
(
    uint32                              uiFeedInterval
)
{
    uint32 count;
    (void)GPIO_Config(GPIO_GPC(12), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    for (count = 0; count < 10; count++)
    {
        mcu_printf("\n Safety Watchdog Timer, WDI %d \n", uiFeedInterval);
        (void)GPIO_Set(GPIO_GPC(12), 1UL);
        (void)SAL_TaskSleep(1);
        (void)GPIO_Set(GPIO_GPC(12), 0UL);
        (void)SAL_TaskSleep(uiFeedInterval*1000);
    }
}

/*
***************************************************************************************************
*                                          WDT_StartTest
*
* This function is to verify the operation of WDT
*
* @param    ucMode [in] the verification of Watchdog driver development or Watchdog operation
* @param    uiDurationSec [in] Duration to check if this system is alive
*
* Notes
*
***************************************************************************************************
*/
void WDT_StartTest(int32 ucMode, uint32 uiDurationSec)
{
    switch (ucMode)
    {
        case 0 :
        {
            mcu_printf("\nReboot MCU in a few seconds\n");
            (void)WDT_ResetSystem();
            break;
        }

        /* [IP-PV] 19, Windowed Watchdog Timer, Watchdog Driver Development And Verification, Full test  */
        case 1 :
        {
            WDT_DriverDevelTest();
            break;
        }

        /* [IP-PV] 19, Windowed Watchdog Timer, Enabling Windowed Watchdog Timer */
        /* [IP-PV] 19, Windowed Watchdog Timer, Resetting System */
        case 2 :
        {
            WDT_DriverDevelResetTest();
            break;
        }

        /* [SM-PV] WDT.01, Windowed Watchdog Timer, Counter Clearing operation Test
           After the IRQ counter timeout, Kick the WDT. ==> (PASS) Not generating a system reset request */
        case 10:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDT_Operation1Test();
            break;
        }

        /* [SM-PV] WDT.01, Windowed Watchdog Timer, Wrong Kick action operation Test
           Before the IRQ counter timeout, Kick the WDT. ==> (PASS) Generating a system reset request */
        case 11:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDT_Operation2Test();
            break;
        }

        /* [SM-PV] SM-MC-WDT.02, Register Write Lock with Password Write Protection, Password Write Protection */
        case 12:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDT_WriteProtectionTest();
            break;
        }

        /* [SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault */
        case 13:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDT_WriteProtectionWithFaultTest();
            break;
        }

        /* [IP-PV] 19, Windowed Watchdog Timer, PMU WDT Driver Development And Verification, Full test  */
        case 21 :
        {
            WDTPMU_DriverDevelTest();
            break;
        }

        /* [IP-PV] 19 PMU Watchdog Driver Development And Verification, Resetting System test  */
        /* [IP-PV] 19 PMU Windowed Watchdog Timer, Enabling Windowed Watchdog Timer */
        /* [IP-PV] 19 PMU Windowed Watchdog Timer, Resetting System */
        case 22 :
        {
            WDTPMU_DriverDevelResetTest();
            break;
        }

        /* [SM-PV] WDT.01, PMU Windowed Watchdog Timer, Counter Clearing operation Test
           After the IRQ counter timeout, Kick the PMU-WDT. ==> (PASS) Not generating a system reset request */
        case 30:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDTPMU_Operation1Test();
            break;
        }

        /* [SM-PV] WDT.01, PMU Windowed Watchdog Timer, Wrong Kick action operation Test
           Before the IRQ counter timeout, Kick the PMU-WDT. ==> (PASS) Generating a system reset request */
        case 31:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDTPMU_Operation2Test();
            break;
        }

        /* [SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection */
        case 32:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDTPMU_WriteProtectionTest();
            break;
        }

        /* [SM-PV] WDT.02, Register Write Lock with Password Write Protection, Password Write Protection with Fault */
        case 33:
        {
            /*** SM_FAULT_TEST = 0x1 ***/
            WDTPMU_WriteProtectionWithFaultTest();
            break;
        }

        case 50:
        {
            /* WDT_SM_FAULT_TEST = 0x1 : The fact that FMU Handler is called means that it is successful.
               WDT_SM_FAULT_TEST = 0x0 : The system must reset from PMU although the IRQ/FIR status of FMU is high */
            WDT_SMTest();
            break;
        }

        case 60:
        {
            /* WDT_SM_FAULT_TEST = 0x1 : The fact that FMU Handler is called means that it is successful.
               WDT_SM_FAULT_TEST = 0x0 : The system must reset from PMU although the IRQ/FIR status of FMU is high */
            WDTPMU_SMTest();
            break;
        }

        case 100 :
        {
            uint32 timerSec     = uiDurationSec * 1000UL * 1000UL;     //15 sec

            wdtDuration = uiDurationSec;

            (void)WDT_Stop();
            (void)WDT_Init();

            (void)TIMER_Enable(TIMER_CH_5, timerSec, (TIMERHandler)&WDT_AliveCallback, NULL);
            (void)GIC_IntSrcEn((uint32)GIC_TIMER_5);
            break;
        }

        case 110:
        {
            mcu_printf("Safety_Wdt_Test, WDI Pulse\n");
            Safety_Wdt_Test(uiDurationSec);
            break;
        }

        default:
        {
            mcu_printf("Nothing to do\n");
            break;
        }
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )


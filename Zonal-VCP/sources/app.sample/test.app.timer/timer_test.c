// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : timer_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )

#include <timer_test.h>

#include <gic.h>
#include <wdt.h>
#include <wdt_reg.h>
#include <reg_phys.h>
#include <debug.h>

#include <timer.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define TIMER_TESTING_PRERIGHT_TIME     (2 * 1000 * 1000)        // 2 sec
#define TIMER_TESTING_RIGHT_TIME        (3 * 1000 * 1000)        // 3 sec
#define TIMER_TESTING_POSTRIGHT_TIME    (4 * 1000 * 1000)        // 4 sec
#define TIMER_TESTING_MAXSTART_TIME     (0xFFB43480U)

#define TIMER_PASS                      ("PASS")
#define TIMER_FAIL                      ("FAIL")

typedef struct TIMERTestState
{
    boolean                             tsIsStarted;
    boolean                             tsIsFinished;

} TIMERTestState_t;

typedef struct TIMERTestRepeatCount
{
    uint32                              trcCh1;
    uint32                              trcCh2;
    uint32                              trcCh3;
    uint32                              trcCh4;
    uint32                              trcCh5;
    uint32                              trcSection;

} TIMERTestRepeatCount_t;

typedef struct TIMERRunningChannel
{
    TIMERChannel_t                      rc16;
    TIMERChannel_t                      rc8;
    TIMERChannel_t                      rc4;
    TIMERChannel_t                      rc2;
    TIMERChannel_t                      rc1;

} TIMERTestRunningChannel_t;

static TIMERConfig_t                    mTimerCfg;
static TIMERTestState_t                 mTimerState;
static TIMERTestRepeatCount_t           mRepeatCount;
static TIMERTestRunningChannel_t        mRunningChannel = {0,};

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static boolean TIMER_RightTime
(
    void
);

static sint32 TIMER_PrestandardHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static sint32 TIMER_PoststandardHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static sint32 TIMER_MainHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static sint32 TIMER_Cmp0Handler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static sint32 TIMER_Cmp1Handler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static sint32 TIMER_EitherorHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static void TIMTER_CommonOperationTest
(
    void
);

static void TIMER_DriverDevelTest
(
    void
);

static sint32 TIMER_AccuracyHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
);

static void TIMER_StartWdt
(
    void
);

static void TIMER_AccuracyTest
(
    uint32                              uiMinDurationSec
);


/*
***************************************************************************************************
*                                          TIMER_MainOperationTest
*
* This function is to test the operation when the counter value is matched with 0xFFFFFFFF or
* MAIN_CNT_LDV
*
* Notes
*
***************************************************************************************************
*/
static void TIMER_MainOperationTest
(
    void
);

/*
***************************************************************************************************
*                                          TIMER_Cmp0OperationTest
*
* This function is to test the operation when the counter value is matched with CMP_VAL0
*
* Notes
*
***************************************************************************************************
*/
static void TIMER_Cmp0OperationTest
(
    void
);

/*
***************************************************************************************************
*                                          TIMER_Cmp1OperationTest
*
* This function is to test the operation when the counter value is matched with CMP_VAL1
*
* Notes
*
***************************************************************************************************
*/
static void TIMER_Cmp1OperationTest
(
    void
);

/*
***************************************************************************************************
*                                          TIMER_EitherorOperationTest
*
* This function is to test the operation when the counter value is matched with CMP_VAL0 or
* CMP_VAL1 (smaller one)
*
* Notes
*
***************************************************************************************************
*/
static void TIMER_EitherorOperationTest
(
    void
);

/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/
static boolean TIMER_RightTime(void)
{
    boolean ret = TRUE;

    if (mTimerCfg.ctStartMode == TIMER_START_ZERO)
    {
        if( (mTimerState.tsIsStarted != (boolean)TRUE) || (mTimerState.tsIsFinished != (boolean)FALSE))
        {
            ret = FALSE;
        }
    }

    return ret;
}

static sint32 TIMER_PrestandardHandler(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mTimerState.tsIsStarted = (boolean)TRUE;

    return (sint32)SAL_RET_SUCCESS;
}

static sint32 TIMER_PoststandardHandler(TIMERChannel_t iChannel, void * pArgs)
{
    (void)iChannel;
    (void)pArgs;

    mTimerState.tsIsFinished = (boolean)TRUE;

    return (sint32)SAL_RET_SUCCESS;
}

static sint32 TIMER_MainHandler(TIMERChannel_t iChannel, void * pArgs)
{
    uint32 retTick = 0;
    (void)pArgs;

    (void)SAL_GetTickCount(&retTick);

    mcu_printf("\n[IP-PV] MC-17, Channel : (%d) Timer, Operation Mode : (%s), (  Main  ) Counter Mode, LD_ZERO : (%d), ==> %s, TickCount : (%d)\n",
           iChannel,
           (mTimerCfg.ctOpMode == TIMER_OP_FREERUN) ? "FREE RUN" : "  STOP  ",
           mTimerCfg.ctStartMode,
           (TIMER_RightTime() == (boolean)TRUE) ? TIMER_PASS : TIMER_FAIL,
           retTick);

    (void)GIC_IntSrcDis((uint32)GIC_TIMER_0 + (uint32)mTimerCfg.ctChannel);
    (void)TIMER_Disable(mTimerCfg.ctChannel);
    TIMER_Cmp0OperationTest();

    return (sint32)SAL_RET_SUCCESS;
}

static sint32 TIMER_Cmp0Handler(TIMERChannel_t iChannel, void * pArgs)
{
    (void)pArgs;

    mcu_printf("\n[IP-PV] MC-17, Channel : (%d) Timer, Operation Mode : (%s), (  CMP0  ) Counter Mode, LD_ZERO : (%d), ==> %s\n",
           iChannel,
           (mTimerCfg.ctOpMode == TIMER_OP_FREERUN) ? "FREE RUN" : "  STOP  ",
           mTimerCfg.ctStartMode,
           (TIMER_RightTime() == (boolean)TRUE) ? TIMER_PASS : TIMER_FAIL);

    (void)GIC_IntSrcDis((uint32)GIC_TIMER_0 + (uint32)mTimerCfg.ctChannel);
    (void)TIMER_Disable(mTimerCfg.ctChannel);
    TIMER_Cmp1OperationTest();

    return (sint32)SAL_RET_SUCCESS;
}

static sint32 TIMER_Cmp1Handler(TIMERChannel_t iChannel, void * pArgs)
{
    (void)pArgs;

    mcu_printf("\n[IP-PV] MC-17, Channel : (%d) Timer, Operation Mode : (%s), (  CMP1  ) Counter Mode, LD_ZERO : (%d), ==> %s\n",
           iChannel,
           (mTimerCfg.ctOpMode == TIMER_OP_FREERUN) ? "FREE RUN" : "  STOP  ",
           mTimerCfg.ctStartMode,
           (TIMER_RightTime() == (boolean)TRUE) ? TIMER_PASS : TIMER_FAIL);

    (void)GIC_IntSrcDis((uint32)GIC_TIMER_0 + (uint32)mTimerCfg.ctChannel);
    (void)TIMER_Disable(mTimerCfg.ctChannel);
    TIMER_EitherorOperationTest();

    return (sint32)SAL_RET_SUCCESS;
}

static sint32 TIMER_EitherorHandler(TIMERChannel_t iChannel, void * pArgs)
{
    uint32 testChannel = 0;

    (void)pArgs;

    mcu_printf("\n[IP-PV] MC-17, Channel : (%d) Timer, Operation Mode : (%s), ( Smaller) Counter Mode, LD_ZERO : (%d), ==> %s\n",
           iChannel,
           (mTimerCfg.ctOpMode == TIMER_OP_FREERUN) ? "FREE RUN" : "  STOP  ",
           mTimerCfg.ctStartMode,
           (TIMER_RightTime() == (boolean)TRUE) ? TIMER_PASS : TIMER_FAIL);

    (void)GIC_IntSrcDis((uint32)GIC_TIMER_0 + (uint32)mTimerCfg.ctChannel);
    (void)TIMER_Disable(mTimerCfg.ctChannel);

    if (mTimerCfg.ctStartMode == TIMER_START_MAINCNT)
    {
        mTimerCfg.ctStartMode = TIMER_START_ZERO;

        TIMER_MainOperationTest();
    }
    else
    {
        if (mTimerCfg.ctOpMode == TIMER_OP_FREERUN)
        {
            mTimerCfg.ctStartMode = TIMER_START_MAINCNT;
            mTimerCfg.ctOpMode = TIMER_OP_ONESHOT;

            TIMER_MainOperationTest();
        }
        else
        {
            if (mTimerCfg.ctChannel < (TIMERChannel_t)TIMER_CH_9)
            {
                testChannel = (uint32)mTimerCfg.ctChannel + 1UL;
                mTimerCfg.ctChannel = (TIMERChannel_t)testChannel;
                mTimerCfg.ctStartMode = TIMER_START_MAINCNT;
                mTimerCfg.ctOpMode = TIMER_OP_FREERUN;

                TIMER_MainOperationTest();
            }
            else
            {
                mcu_printf("\n[IP-PV] MC-17 Test Completed!!\n");
            }
        }
    }

    return (sint32)SAL_RET_SUCCESS;
}

static void TIMTER_CommonOperationTest(void)
{
    uint32 testChannel = 0;

    if (mTimerCfg.ctStartMode == TIMER_START_ZERO)
    {
        mTimerCfg.ctMainValueUsec = TIMER_TESTING_RIGHT_TIME;

        mTimerState.tsIsStarted  = FALSE;
        mTimerState.tsIsFinished = FALSE;
        if (mTimerCfg.ctChannel == (TIMERChannel_t)TIMER_CH_9)
        {
            testChannel = (uint32)TIMER_CH_1;
        }
        else
        {
            testChannel = (uint32)((uint32)mTimerCfg.ctChannel + 1UL);
        }

        (void)TIMER_EnableWithMode((TIMERChannel_t)testChannel,
                                   TIMER_TESTING_PRERIGHT_TIME,
                                   TIMER_OP_ONESHOT,
                                   &TIMER_PrestandardHandler,
                                   NULL);

        GIC_IntSrcEn((uint32)GIC_TIMER_0 + testChannel);

        if (testChannel == (uint32)TIMER_CH_9)
        {
            testChannel = (uint32)TIMER_CH_1;
        }
        else
        {
            testChannel = (uint32)(testChannel + 1UL);
        }

        (void)TIMER_EnableWithMode((TIMERChannel_t)testChannel,
                                   TIMER_TESTING_POSTRIGHT_TIME,
                                   TIMER_OP_ONESHOT,
                                   &TIMER_PoststandardHandler,
                                   NULL);

        GIC_IntSrcEn((uint32)GIC_TIMER_0 + testChannel);
    }
    else
    {
        mTimerCfg.ctMainValueUsec = TIMER_TESTING_MAXSTART_TIME;
    }

    (void)TIMER_EnableWithCfg(&mTimerCfg);
    GIC_IntSrcEn((uint32)GIC_TIMER_0 + (uint32)mTimerCfg.ctChannel);
}

void TIMER_MainOperationTest(void)
{
    mTimerCfg.ctCounterMode   = (TIMERCounterMode_t)TIMER_COUNTER_MAIN;
    mTimerCfg.ctCmp0ValueUsec = 0;
    mTimerCfg.ctCmp1ValueUsec = 0;
    mTimerCfg.fnHandler = &TIMER_MainHandler;

    TIMTER_CommonOperationTest();
}

void TIMER_Cmp0OperationTest(void)
{
    mTimerCfg.ctCounterMode   = (TIMERCounterMode_t)TIMER_COUNTER_COMP0;
    mTimerCfg.ctCmp0ValueUsec = TIMER_TESTING_RIGHT_TIME;
    mTimerCfg.ctCmp1ValueUsec = 0;
    mTimerCfg.fnHandler = &TIMER_Cmp0Handler;

    TIMTER_CommonOperationTest();
}

void TIMER_Cmp1OperationTest(void)
{
    mTimerCfg.ctCounterMode   = (TIMERCounterMode_t)TIMER_COUNTER_COMP1;
    mTimerCfg.ctCmp0ValueUsec = 0;
    mTimerCfg.ctCmp1ValueUsec = TIMER_TESTING_RIGHT_TIME;
    mTimerCfg.fnHandler = &TIMER_Cmp1Handler;

    TIMTER_CommonOperationTest();
}

void TIMER_EitherorOperationTest(void)
{
    mTimerCfg.ctCounterMode   = (TIMERCounterMode_t)TIMER_COUNTER_SMALL_COMP;
    mTimerCfg.ctCmp0ValueUsec = TIMER_TESTING_RIGHT_TIME;
    mTimerCfg.ctCmp1ValueUsec = (TIMER_TESTING_RIGHT_TIME * 2);
    mTimerCfg.fnHandler = &TIMER_EitherorHandler;

    TIMTER_CommonOperationTest();
}

static void TIMER_DriverDevelTest(void)
{
    SALRetCode_t ret;
    TIMERConfig_t cfg;
    SALErrorInfo_t errorTable;

    mcu_printf("\n17. Timer Driver Development And Verification, Enabling Timer, Step 3\n");
    ret = TIMER_EnableWithCfg(NULL);

    if (ret == SAL_RET_FAILED)
    {
        (void)SAL_GetCurError(&errorTable);
    } else {;}

    mcu_printf("\n17. Timer Driver Development And Verification, Enabling Timer, Step 4\n");

    cfg.ctChannel = (TIMERChannel_t)TIMER_CH_MAX;
    ret = TIMER_EnableWithCfg(&cfg);
    if (ret == SAL_RET_FAILED)
    {
        (void)SAL_GetCurError(&errorTable);
    } else {;}

    mcu_printf("\n17. Timer Driver Development And Verification, Enabling Timer, Step 5\n");
    cfg.ctChannel       = (TIMERChannel_t)TIMER_CH_3;
    cfg.ctStartMode     = TIMER_START_MAINCNT;
    cfg.ctCounterMode   = (TIMERCounterMode_t)TIMER_COUNTER_COMP0;
    cfg.ctMainValueUsec = 0xFFFF0000U;
    cfg.ctCmp0ValueUsec = 0x00FFFFFFU;
    ret = TIMER_EnableWithCfg(&cfg);
    if (ret == SAL_RET_FAILED)
    {
        (void)SAL_GetCurError(&errorTable);
    } else {;}

    mcu_printf("\n17. Timer Driver Development And Verification, Enabling Timer, Step 6\n");
    cfg.ctCounterMode   = (TIMERCounterMode_t)TIMER_COUNTER_COMP1;
    cfg.ctCmp0ValueUsec = 0x0U;
    cfg.ctCmp1ValueUsec = 0x00FFFFFFU;
    ret = TIMER_EnableWithCfg(&cfg);
    if (ret == SAL_RET_FAILED)
    {
        (void)SAL_GetCurError(&errorTable);
    } else {;}

    mcu_printf("\n17. Timer Driver Development And Verification, Enabling Timer, Step 1\n");
    cfg.ctCounterMode   = (TIMERCounterMode_t)TIMER_COUNTER_MAIN;
    cfg.ctOpMode        = TIMER_OP_FREERUN;
    cfg.ctStartMode     = TIMER_START_ZERO;
    ret = TIMER_EnableWithCfg(&cfg);
    if (ret == SAL_RET_FAILED)
    {
        (void)SAL_GetCurError(&errorTable);
    } else {;}

    mcu_printf("\n17. Timer Driver Development And Verification, Disabling Timer, Step 1\n");
    ret = TIMER_Disable(cfg.ctChannel);
    if (ret == SAL_RET_FAILED)
    {
        (void)SAL_GetCurError(&errorTable);
    } else {;}

}

static uint32 mDurationSec = 0;

static sint32 TIMER_AccuracyHandler(TIMERChannel_t iChannel, void * pArgs)
{
    (void)pArgs;

    if (mRunningChannel.rc16 == iChannel)
    {
        // This channel is always updated the latest
        mRepeatCount.trcSection++;

        mcu_printf("TIMER_AccuracyHandler Channel : %d CH - [%3d] Times, Total [%d] Times\n", iChannel, mRepeatCount.trcCh1, mRepeatCount.trcSection);

        TIMER_Disable((TIMERChannel_t)mRunningChannel.rc16);

    }
    else if (mRunningChannel.rc8 == iChannel)
    {
        mRepeatCount.trcCh2++;

        mcu_printf("TIMER_AccuracyHandler Channel : %d CH - [%3d] Times, Total [%d] Times\n", iChannel, mRepeatCount.trcCh2, mRepeatCount.trcSection);

        if (mRepeatCount.trcCh2 == 2UL)
        {
            TIMER_Disable((TIMERChannel_t)mRunningChannel.rc8);
        }
    }
    else if (mRunningChannel.rc4 == iChannel)
    {
         mRepeatCount.trcCh3++;

        mcu_printf("TIMER_AccuracyHandler Channel : %d CH - [%3d] Times, Total [%d] Times\n", iChannel, mRepeatCount.trcCh3, mRepeatCount.trcSection);

        if (mRepeatCount.trcCh3 == 4UL)
        {
            TIMER_Disable((TIMERChannel_t)mRunningChannel.rc4);
        }
    }
    else if (mRunningChannel.rc2 == iChannel)
    {
        mRepeatCount.trcCh4++;

        mcu_printf("TIMER_AccuracyHandler Channel : %d CH - [%3d] Times, Total [%d] Times\n", iChannel, mRepeatCount.trcCh4, mRepeatCount.trcSection);

        if (mRepeatCount.trcCh4 == 8UL)
        {
            TIMER_Disable((TIMERChannel_t)mRunningChannel.rc2);
        }
    }
    else if (mRunningChannel.rc1 == iChannel)
    {
        mRepeatCount.trcCh5++;

        mcu_printf("TIMER_AccuracyHandler Channel : %d CH - [%3d] Times, Total [%d] Times\n", iChannel, mRepeatCount.trcCh5, mRepeatCount.trcSection);

        if (mRepeatCount.trcCh5 == 16UL)
        {
            TIMER_Disable((TIMERChannel_t)mRunningChannel.rc1);

            if (mRepeatCount.trcSection < 5UL)
            {
                // 5 Times is deadline
                TIMER_AccuracyTest(mRepeatCount.trcSection);
            }
            else
            {
                WDT_KickPing();
                mcu_printf("\nTimer AccuracyTest is Done! Success!\n");
                mcu_printf("System will be reboot in %d seconds \n", 16UL * mDurationSec);
            }
        }
    }

    return (sint32)SAL_RET_SUCCESS;
}

static void TIMER_StartWdt(void)
{
    uint32 reg;

    reg = MCU_BSP_WDT_BASE + WDT_WR_PW;

    WDT_Stop();
    (void)WDT_SmMode(TRUE, WDT_RST_DIRECT, WDT_2OO3_VOTE);

    SAL_WriteReg((uint32)WDT_PASSWORD, reg);
    SAL_WriteReg(((8UL * mDurationSec) * 24UL * 1000UL* 1000UL), MCU_BSP_WDT_BASE + WDT_IRQ_CNT);

    // Adding 1 second
    SAL_WriteReg((uint32)WDT_PASSWORD, reg);
    SAL_WriteReg(((8UL * mDurationSec + 1UL) * 24UL * 1000UL* 1000UL), MCU_BSP_WDT_BASE + WDT_RSR_CNT);

    (void)GIC_IntVectSet
    (
        GIC_WATCHDOG,
        GIC_PRIORITY_NO_MEAN,
        GIC_INT_TYPE_EDGE_RISING,
        NULL_PTR,
        NULL_PTR
    );

    (void)GIC_IntSrcEn(GIC_WATCHDOG);

    SAL_WriteReg((uint32)WDT_PASSWORD, reg);
    SAL_WriteReg(WDT_EN_ON, MCU_BSP_WDT_BASE + WDT_EN);
}

static void TIMER_AccuracyTest(uint32 uiSectionCount)
{
    TIMERChannel_t temp;
    mcu_printf("\nTimer AccuracyTest Duration [%d (sec)], %d/5 Started!\n", mDurationSec, uiSectionCount + 1UL);

    // Whenever this function is called, mRepeatCount is initialized without mRepeatCount.trcSection.
    mRepeatCount.trcCh1 = 0;
    mRepeatCount.trcCh2 = 0;
    mRepeatCount.trcCh3 = 0;
    mRepeatCount.trcCh4 = 0;
    mRepeatCount.trcCh5 = 0;

    // Change each channel into next channel
    temp = mRunningChannel.rc16;
    mRunningChannel.rc16 = mRunningChannel.rc8;
    mRunningChannel.rc8  = mRunningChannel.rc4;
    mRunningChannel.rc4  = mRunningChannel.rc2;
    mRunningChannel.rc2  = mRunningChannel.rc1;
    mRunningChannel.rc1  = temp;

    (void)TIMER_EnableWithMode(mRunningChannel.rc16,
                               16UL * (mDurationSec * 1000UL * 1000UL),
                               TIMER_OP_ONESHOT,
                               &TIMER_AccuracyHandler,
                               NULL);

    (void)TIMER_EnableWithMode(mRunningChannel.rc8,
                               8UL * (mDurationSec * 1000UL * 1000UL),
                               TIMER_OP_FREERUN,
                               &TIMER_AccuracyHandler,
                               NULL);

    (void)TIMER_EnableWithMode(mRunningChannel.rc4,
                               4UL * (mDurationSec * 1000UL * 1000UL),
                               TIMER_OP_FREERUN,
                               &TIMER_AccuracyHandler,
                               NULL);

    (void)TIMER_EnableWithMode(mRunningChannel.rc2,
                               2UL * (mDurationSec * 1000UL * 1000UL),
                               TIMER_OP_FREERUN,
                               &TIMER_AccuracyHandler,
                               NULL);

    (void)TIMER_EnableWithMode(mRunningChannel.rc1,
                               (mDurationSec * 1000UL * 1000UL),
                               TIMER_OP_FREERUN,
                               &TIMER_AccuracyHandler,
                               NULL);

    GIC_IntSrcEn((uint32)GIC_TIMER_1);
    GIC_IntSrcEn((uint32)GIC_TIMER_2);
    GIC_IntSrcEn((uint32)GIC_TIMER_3);
    GIC_IntSrcEn((uint32)GIC_TIMER_4);
    GIC_IntSrcEn((uint32)GIC_TIMER_5);

    TIMER_StartWdt();
}

/*
***************************************************************************************************
*                                          TIMER_StartTimerTest
*
* This function is to verify the operation of Timer
*
* @param    ucMode [in] the verification of Timer driver development or Timer operation
* @param    uiMinDurationSec [in] The minimum duration to be repeated
*
* Notes
*
***************************************************************************************************
*/
void TIMER_StartTimerTest(int32 ucMode, uint32 uiMinDurationSec)
{
    switch (ucMode)
    {
        /* 17. Timer Driver Development And Verification, Full test  */
        case 1:
        {
            TIMER_DriverDevelTest();
            break;
        }

        /* [IP-PV] MC-17, Timer, Timer operation, Check Free Running/Stop Mode, Count Mode, Pre-scaler */
        case 10 :
        {
            mTimerCfg.ctChannel   = (TIMERChannel_t)TIMER_CH_1;
            mTimerCfg.ctStartMode = TIMER_START_MAINCNT;
            mTimerCfg.ctOpMode    = TIMER_OP_FREERUN;
            mTimerCfg.pArgs       = NULL;

            TIMER_MainOperationTest();
            break;
        }

        case 100 :
        {
            mDurationSec = uiMinDurationSec;

            mRunningChannel.rc16 = TIMER_CH_5;
            mRunningChannel.rc8  = TIMER_CH_1;
            mRunningChannel.rc4  = TIMER_CH_2;
            mRunningChannel.rc2  = TIMER_CH_3;
            mRunningChannel.rc1  = TIMER_CH_4;

            TIMER_AccuracyTest(0);
            break;
        }

        default:
        {
            mcu_printf("Nothing To Do\n");
            break;
        }
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )


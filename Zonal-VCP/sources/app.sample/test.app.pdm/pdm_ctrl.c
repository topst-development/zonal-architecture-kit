#include "pdm.h"
#include "gpio.h"
#include "stdio.h"
#include "pdm_ctrl.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <vcp_types.h>

extern QueueHandle_t xQ_MotorSpeed;
extern QueueHandle_t xQ_MotorWheel;

#define ESC_SEL                         0U
#define ESC_PORT                        GPIO_PERICH_CH0
#define ESC_PERIOD_NS                   20000000UL
#define ESC_STOP_DUTY_NS                1000000UL
#define ESC_MAX_DUTY_NS                 2000000UL
#define ESC_MAX_ANGLE_FIX               (180U * 256U)
#define ESC_ARM_VALUE_FIX               ((uint32)0x40U << 8)
#define ESC_ARM_DELAY_MS                1000U

static PDMModeConfig_t esc_cfg;
static boolean esc_inited = FALSE;
static ESCState_t esc_state = ESC_STATE_OFF;

static SALRetCode_t ESC_SetAngle256(uint32 angle_fix);

SALRetCode_t ESC_PWM_Init(void)
{
    if (esc_inited != FALSE)
    {
        return SAL_RET_SUCCESS;
    }

    PDM_Init();
    PDM_CfgSetWrPw();
    PDM_CfgSetWrLock(0);

    SAL_MemSet(&esc_cfg, 0, sizeof(esc_cfg));
    esc_cfg.mcPortNumber = ESC_PORT;
    esc_cfg.mcOperationMode = PDM_OUTPUT_MODE_PHASE_1;
    esc_cfg.mcInversedSignal = 0;
    esc_cfg.mcOutSignalInIdle = 0;
    esc_cfg.mcLoopCount = 0;
    esc_cfg.mcOutputCtrl = 0;
    esc_cfg.mcPeriodNanoSec1 = ESC_PERIOD_NS;
    esc_cfg.mcDutyNanoSec1 = ESC_STOP_DUTY_NS;
    esc_cfg.mcPeriodNanoSec2 = 0;
    esc_cfg.mcDutyNanoSec2 = 0;

    if (PDM_SetConfig(ESC_SEL, &esc_cfg) != SAL_RET_SUCCESS)
    {
        esc_state = ESC_STATE_ERROR;
        mcu_printf("ESC SetConfig fail\n");
        return SAL_RET_FAILED;
    }

    if (PDM_Enable(ESC_SEL, PMM_OFF) != SAL_RET_SUCCESS)
    {
        esc_state = ESC_STATE_ERROR;
        mcu_printf("ESC Enable fail\n");
        return SAL_RET_FAILED;
    }

    esc_inited = TRUE;
    mcu_printf("ESC PWM Init Done\n");
    return SAL_RET_SUCCESS;
}

boolean ESC_IsReady(void)
{
    return (esc_state == ESC_STATE_ARMED) ? TRUE : FALSE;
}

static SALRetCode_t ESC_SetAngle256(uint32 angle_fix)
{
    uint32 duty_ns;
    uint32 wait;

    if (esc_inited == FALSE)
    {
        return SAL_RET_FAILED;
    }

    if (angle_fix > ESC_MAX_ANGLE_FIX)
    {
        angle_fix = ESC_MAX_ANGLE_FIX;
    }

    duty_ns = ESC_STOP_DUTY_NS
            + (uint32)((uint64)angle_fix * (ESC_MAX_DUTY_NS - ESC_STOP_DUTY_NS) / ESC_MAX_ANGLE_FIX);

    esc_cfg.mcDutyNanoSec1 = duty_ns;

    (void)PDM_Disable(ESC_SEL, PMM_OFF);

    wait = 0;
    while ((PDM_GetChannelStatus(ESC_SEL) != 0U) && (wait < 100U))
    {
        SAL_TaskSleep(1);
        wait++;
    }

    if (PDM_SetConfig(ESC_SEL, &esc_cfg) == SAL_RET_SUCCESS)
    {
        if (PDM_Enable(ESC_SEL, PMM_OFF) == SAL_RET_SUCCESS)
        {
            return SAL_RET_SUCCESS;
        }
    }

    return SAL_RET_FAILED;
}

void ConfigureServoPWM(uint32 channel, uint32 port, uint32 angle_deg)
{
    PDMModeConfig_t pwm_cfg;
    uint32 duty_ns = 500000 + (angle_deg * (2000000 / 180)); // 0~180도 → 0.5~2.5ms
    uint32 wait_cnt = 0;

    pwm_cfg.mcPortNumber      = port;
    pwm_cfg.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    pwm_cfg.mcInversedSignal  = 0;
    pwm_cfg.mcOutSignalInIdle = 0;
    pwm_cfg.mcLoopCount       = 0;
    pwm_cfg.mcOutputCtrl      = 0;

    pwm_cfg.mcPeriodNanoSec1  = 20000000; // 20ms (50Hz)
    pwm_cfg.mcDutyNanoSec1    = duty_ns;
    pwm_cfg.mcPeriodNanoSec2  = 0;
    pwm_cfg.mcDutyNanoSec2    = 0;

    PDM_Disable(channel, PMM_ON);
    while (PDM_GetChannelStatus(channel))
    {
        SAL_TaskSleep(1);
        if (++wait_cnt > 100)
        {
            mcu_printf("Timeout on channel %d\n", channel);
            return;
        }
    }

    if (PDM_SetConfig(channel, &pwm_cfg) != SAL_RET_SUCCESS)
    {
        mcu_printf("SetConfig fail (CH:%d)\n", channel);
        return;
    }

    if (PDM_Enable(channel, PMM_ON) != SAL_RET_SUCCESS)
    {
        mcu_printf("Enable fail (CH:%d)\n", channel);
        return;
    }

    mcu_printf("CH%d angle: %3d° → duty: %d ns\n", channel, angle_deg, duty_ns);
}

/* -------------------------- Task -------------------------- */

void MotorSpeedTask(void *pvParameters) 
{
    uint8 recvBuf[2];
    uint32 speed_fix = 0;

    (void)pvParameters;

    esc_state = ESC_STATE_INIT;
    mcu_printf("ESC Arming...\n");

    if (ESC_PWM_Init() != SAL_RET_SUCCESS)
    {
        esc_state = ESC_STATE_ERROR;
        mcu_printf("ESC Init Failed!\n");
    }
    else
    {
        esc_state = ESC_STATE_ARMING;
        if (ESC_SetAngle256(ESC_ARM_VALUE_FIX) != SAL_RET_SUCCESS)
        {
            esc_state = ESC_STATE_ERROR;
            mcu_printf("ESC Arming Failed!\n");
        }
        else
        {
            SAL_TaskSleep(ESC_ARM_DELAY_MS);
            esc_state = ESC_STATE_ARMED;
            mcu_printf("ESC Ready!\n");
        }
    }

    for (;;) 
    {
        if (xQueueReceive(xQ_MotorSpeed, recvBuf, portMAX_DELAY) == pdPASS) 
        {
            if (esc_state != ESC_STATE_ARMED)
            {
                continue;
            }

            speed_fix = ((uint32)recvBuf[0] << 8) | (uint32)recvBuf[1];

            if (ESC_SetAngle256(speed_fix) != SAL_RET_SUCCESS)
            {
                esc_state = ESC_STATE_ERROR;
                mcu_printf("ESC Update Failed!\n");
            }
        }
    }
}

void MotorWheelTask(void *pvParameters)
{
    uint8 recvBuf[2];
    int8_t input = 0;
    int angle = 0;

    (void)pvParameters;

    for (;;)
    {
        if (xQueueReceive(xQ_MotorWheel, recvBuf, portMAX_DELAY) == pdPASS)
        {
            input = (int8_t)recvBuf[0];

            if (input < 0) input = 0;
            if (input > 127) input = 127;

            angle = 90 + ((input - 65) * 45) / 65;

            ConfigureServoPWM(5, GPIO_PERICH_CH3, angle);
        }
    }
}

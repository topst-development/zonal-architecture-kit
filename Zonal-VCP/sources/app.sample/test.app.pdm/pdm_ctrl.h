#ifndef PDM_CTRL_HEADER
#define PDM_CTRL_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

typedef enum
{
    ESC_STATE_OFF = 0,
    ESC_STATE_INIT,
    ESC_STATE_ARMING,
    ESC_STATE_ARMED,
    ESC_STATE_ERROR
} ESCState_t;

SALRetCode_t ESC_PWM_Init(void);
boolean ESC_IsReady(void);
void ConfigureServoPWM(uint32 channel, uint32 port, uint32 angle_deg);

void MotorSpeedTask(void *pvParameters);
void MotorWheelTask(void *pvParameters);

#endif

#endif

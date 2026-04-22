#ifndef PDM_CTRL_HEADER
#define PDM_CTRL_HEADER

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

void ESC_PWM_Init(void);
boolean ESC_IsReady(void);
void ConfigureServoPWM(uint32 channel, uint32 port, uint32 angle_deg);

void MotorSpeedTask(void *pvParameters);
void MotorWheelTask(void *pvParameters);

#endif

#endif

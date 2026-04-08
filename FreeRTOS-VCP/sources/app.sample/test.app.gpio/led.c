#include "led.h"
#include <gpio.h>
#include <stdint.h>

static const uint32_t led_pins[4] = {
    GPIO_GPB(25),
    GPIO_GPB(28),
    GPIO_GPB(11),
    GPIO_GPB(27), // 4개 디지털 핀 GPIO 설정
};
void LED_Init(void)
{
    for (int i = 0; i < 4; ++i) {
        GPIO_Config(led_pins[i], (GPIO_FUNC(0) | GPIO_OUTPUT));
        GPIO_Set(led_pins[i], 0); } // GPIO 출력 설정, 초기 OFF
}
void LED_Run(void)
{
    LED_Init(); // 초기화
    while (1) {
        for (int i = 0; i < 4; i++) {
            GPIO_Set(led_pins[i], 1); // 순서대로 ON
            SAL_TaskSleep(500); } // 500ms 대기
        for (int i = 3; i >= 0; i--) {
            GPIO_Set(led_pins[i], 0); // 순서대로 OFF
            SAL_TaskSleep(500); } // 500ms 대기
    }
}

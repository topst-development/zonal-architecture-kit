// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : uart_example.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#include <gpio.h>
#include <uart.h>
#include <uart_example.h>

void UART_Test(void)
{
    uint8 tx[] = "[UART] Hello from UART!\r\n";
    uint8 rx[64] = {0};
    sint32 ret;

    GPIO_Config(UART_TX_PIN, GPIO_FUNC(UART_PORTCFG));
    GPIO_Config(UART_RX_PIN, GPIO_FUNC(UART_PORTCFG));

    UartParam_t param = {
        .sCh = UART_TEST_CH,
        .sBaudrate = UART_BAUDRATE,
        .sPriority = GIC_PRIORITY_NO_MEAN,
        .sMode = UART_POLLING_MODE,
        .sCtsRts = UART_CTSRTS_OFF,
        .sPortCfg = UART_PORTCFG,
        .sWordLength = WORD_LEN_8,
        .sFIFO = DISABLE_FIFO,
        .s2StopBit = TWO_STOP_BIT_OFF,
        .sParity = PARITY_SPACE,
        .sFnCallback = NULL_PTR
    };

    if (UART_Open(&param) != SAL_RET_SUCCESS) {
        mcu_printf("[UART] Open failed\n");
        return;
    }

    (void)UART_Write(UART_TEST_CH, tx, sizeof(tx));

    ret = UART_Read(UART_TEST_CH, rx, sizeof(rx));
    if (ret > 0) {
        mcu_printf("[UART] Received: %s\n", rx);
    } else {
        mcu_printf("[UART] No data received\n");
    }

    UART_Close(UART_TEST_CH);
}

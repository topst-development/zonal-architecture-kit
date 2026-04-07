// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : uart_example.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#ifndef UART_TEST_H
#define UART_TEST_H

#define UART_TEST_CH        UART_CH0
#define UART_TX_PIN         GPIO_GPA(28)
#define UART_RX_PIN         GPIO_GPA(29)
#define UART_BAUDRATE       115200
#define UART_PORTCFG        0

void UART_Test(void);

#endif

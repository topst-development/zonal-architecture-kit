// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : dot_matrix.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#include <stdint.h>
#include <gpsb.h>
#include <dot_matrix.h>

void MAX7219_Send(uint8_t addr, uint8_t data)
{
    uint8_t tx_buf[2] = { addr, data };
    uint8_t rx_buf[2] = { 0 };

    GPSB_CsActivate(MAX7219_CHANNEL, MAX7219_CS_GPIO, FALSE);
    GPSB_Xfer(MAX7219_CHANNEL, tx_buf, rx_buf, 2, GPSB_XFER_MODE_WITHOUT_INTERRUPT);
    GPSB_CsDeactivate(MAX7219_CHANNEL, MAX7219_CS_GPIO, FALSE);
    SAL_TaskSleep(1);
}

void MAX7219_Init(void)
{
    GPIO_Config(MAX7219_CS_GPIO, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Set(MAX7219_CS_GPIO, 1);

    GPIO_Config(MAX7219_SCLK_GPIO, GPIO_FUNC(MAX7219_GPIO_FUNC));
    GPIO_Config(MAX7219_MOSI_GPIO, GPIO_FUNC(MAX7219_GPIO_FUNC));

    GPSBOpenParam_t param = {
        .uiSdo = MAX7219_MOSI_GPIO,
        .uiSdi = MAX7219_MISO_GPIO,
        .uiSclk = MAX7219_SCLK_GPIO,
        .uiIsSlave = GPSB_MASTER_MODE,
        .uiDmaBufSize = 0,
        .pDmaAddrTx = NULL,
        .pDmaAddrRx = NULL,
        .fbCallback = NULL,
        .pArg = NULL
    };

    if (GPSB_Open(MAX7219_CHANNEL, param) != SAL_RET_SUCCESS) {
        mcu_printf("[MAX7219] GPSB open failed\n");
        return;
    }

    GPSB_SetBpw(MAX7219_CHANNEL, 16);
    GPSB_SetSpeed(MAX7219_CHANNEL, 1000000);
    GPSB_CsInit(MAX7219_CHANNEL, MAX7219_CS_GPIO, FALSE);

    MAX7219_Send(0x0F, 0x00); // Display test off
    MAX7219_Send(0x0C, 0x01); // Shutdown off
    MAX7219_Send(0x0B, 0x07); // Scan limit = 8 digits
    MAX7219_Send(0x0A, 0x07); // Intensity = max
    MAX7219_Send(0x09, 0x00); // No decode mode

    for (uint8_t i = 1; i <= 8; i++) {
        MAX7219_Send(i, 0x00);
    }
}

void MAX7219_XPattern(void)
{
    uint8_t pattern[8] = {
        0b10000001,
        0b01000010,
        0b00100100,
        0b00011000,
        0b00011000,
        0b00100100,
        0b01000010,
        0b10000001
    };
    for (uint8_t i = 0; i < 8; i++) {
        MAX7219_Send(i + 1, pattern[i]);
    }
}

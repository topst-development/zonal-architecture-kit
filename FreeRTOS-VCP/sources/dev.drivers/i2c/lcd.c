// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : lcd.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#include <i2c.h>
#include <lcd.h>

void lcd_send(uint8 mode, uint8 data) {
    uint8 high = data & 0xF0;
    uint8 low  = (data << 4) & 0xF0;
    uint8 buf[6];

    buf[0] = high | 0x08 | (mode ? 0x01 : 0x00);
    buf[1] = high | 0x0C | (mode ? 0x01 : 0x00);
    buf[2] = high | 0x08 | (mode ? 0x01 : 0x00);
    buf[3] = low  | 0x08 | (mode ? 0x01 : 0x00);
    buf[4] = low  | 0x0C | (mode ? 0x01 : 0x00);
    buf[5] = low  | 0x08 | (mode ? 0x01 : 0x00);

    I2CXfer_t xfer = {
        .xCmdLen = 0,
        .xOutLen = 6,
        .xOutBuf = buf,
        .xInLen = 0,
        .xInBuf = NULL,
        .xCmdBuf = NULL,
        .xOpt = 0
    };

    I2C_Xfer(I2C_CH, LCD_ADDR << 1, xfer, 0);
    SAL_TaskSleep(2);
}

void lcd_cmd(uint8 cmd)  { lcd_send(LCD_CMD, cmd); }
void lcd_data(uint8 dat) { lcd_send(LCD_DATA, dat); }

void lcd_init(void) {
    SAL_TaskSleep(50);
    lcd_cmd(0x33);
    lcd_cmd(0x32);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    SAL_TaskSleep(5);
}

void lcd_print(const char *str) {
    while (*str) lcd_data(*str++);
}

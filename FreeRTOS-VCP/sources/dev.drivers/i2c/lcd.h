// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : lcd.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#ifndef I2C_LCD_H_
#define I2C_LCD_H_
#define I2C_CH     0
#define I2C_PORT   0
#define LCD_ADDR   0x27
#define I2C_SPEED  100
#define LCD_CMD    0
#define LCD_DATA   1

void lcd_send(uint8 mode, uint8 data);
void lcd_cmd(uint8 cmd);
void lcd_data(uint8 data);
void lcd_init(void);
void lcd_print(const char *str);

#endif

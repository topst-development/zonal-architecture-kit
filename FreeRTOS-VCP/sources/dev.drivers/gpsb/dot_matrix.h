// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : dot_matrix.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#ifndef GPSB_MAX7219_H_
#define GPSB_MAX7219_H_

#define MAX7219_CHANNEL     0
#define MAX7219_CS_GPIO     GPIO_GPB(5)
#define MAX7219_SCLK_GPIO   GPIO_GPB(4)
#define MAX7219_MOSI_GPIO   GPIO_GPB(6)
#define MAX7219_MISO_GPIO   GPIO_GPB(7)
#define MAX7219_GPIO_FUNC   1

void MAX7219_Send(uint8 reg, uint8 data);
void MAX7219_Init(void);
void MAX7219_TestPattern(void);
void MAX7219_XPattern(void);

#endif

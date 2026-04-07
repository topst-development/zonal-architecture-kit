// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : audio_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_AUDIO_CHIP_VERI
#define MCU_BSP_AUDIO_CHIP_VERI

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

/**************************************************************************************************
*                                             INCLUDE FILES
**************************************************************************************************/


#include <i2s.h>
#include <gic_enum.h>
#include <gic.h>

#define TESTMODE                        (0x90UL)

void DR_Irq(  void * pArg);
void AUDIO_LRMode_Test(void);
void AUDIO_ErrorOcuur_TEST(void);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#endif  // MCU_BSP_AUDIO_CHIP_VERI


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : preload.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_PRELOAD == 1 )

#include <debug.h>
#include <stdlib.h>
#include <sal_internal.h>

#include <bsp.h>
#include <mpu.h>
#include "preload.h"

extern uint32 __PRECODE_START_LOAD;
extern uint32 __PRECODE_RAM_START__;
extern uint32 __PRECODE_SIZE__;

#ifdef PRELOAD_CLOCK_CONFIG_DEFAULT
    #define PRELOAD_CLK_CTRL_375_MHZ        ((2UL<<4UL)|(0UL<<0UL))
    #define PRELOAD_CLK_CTRL_200_MHZ        ((1UL<<4UL)|(0UL<<0UL))
    #define PRELOAD_CLK_CTRL_125_MHZ        ((2UL<<4UL)|(2UL<<0UL))
    #define PRELOAD_CLK_CTRL_100_MHZ        ((1UL<<4UL)|(1UL<<0UL))
    #define PRELOAD_CLK_CTRL_75_MHZ         ((2UL<<4UL)|(4UL<<0UL))
    #define PRELOAD_CLK_CTRL_50_MHZ         ((1UL<<4UL)|(3UL<<0UL))
    #define PRELOAD_CLK_CTRL_40_MHZ         ((1UL<<4UL)|(4UL<<0UL))
    #define PRELOAD_CLK_CTRL_25_MHZ         ((1UL<<4UL)|(7UL<<0UL))
    #define PRELOAD_CLK_CTRL_20_MHZ         ((1UL<<4UL)|(9UL<<0UL))

    #define PRELOAD_FREQ_CPU                (PRELOAD_CLK_CTRL_200_MHZ)
    #define PRELOAD_FREQ_BUS                (PRELOAD_CLK_CTRL_100_MHZ)
    #define PRELOAD_FREQ_EFL                (PRELOAD_CLK_CTRL_125_MHZ)
    #define PRELOAD_FREQ_HSM                (PRELOAD_CLK_CTRL_200_MHZ)

    // 400
    #define PRELOAD_PLL0_P                  (3UL)
    #define PRELOAD_PLL0_M                  (400UL)
    #define PRELOAD_PLL0_S                  (2UL)

    //750
    #define PRELOAD_PLL1_P                  (1UL)
    #define PRELOAD_PLL1_M                  (125UL)
    #define PRELOAD_PLL1_S                  (1UL)

    //80Mhz
    #define PRELOAD_PERI_CLK_SFMC		    ((0UL<<24UL)|(4UL<<0UL))
#endif

#ifdef PRELOAD_CLOCK_CONFIG_LEGACY_1
    #define PRELOAD_CLK_CTRL_375_MHZ        ((2UL<<4UL)|(0UL<<0UL))
    #define PRELOAD_CLK_CTRL_300_MHZ        ((1UL<<4UL)|(0UL<<0UL))
    #define PRELOAD_CLK_CTRL_187_5_MHZ      ((2UL<<4UL)|(1UL<<0UL))
    #define PRELOAD_CLK_CTRL_150_MHZ        ((1UL<<4UL)|(1UL<<0UL))
    #define PRELOAD_CLK_CTRL_125_MHZ        ((2UL<<4UL)|(2UL<<0UL))
    #define PRELOAD_CLK_CTRL_100_MHZ        ((1UL<<4UL)|(2UL<<0UL))
    #define PRELOAD_CLK_CTRL_93_75_MHZ      ((2UL<<4UL)|(3UL<<0UL))
    #define PRELOAD_CLK_CTRL_75_MHZ         ((1UL<<4UL)|(3UL<<0UL))
    #define PRELOAD_CLK_CTRL_60_MHZ         ((1UL<<4UL)|(4UL<<0UL))
    #define PRELOAD_CLK_CTRL_62_5_MHZ       ((2UL<<4UL)|(5UL<<0UL))
    #define PRELOAD_CLK_CTRL_50_MHZ         ((1UL<<4UL)|(5UL<<0UL))

    #define PRELOAD_FREQ_CPU                (PRELOAD_CLK_CTRL_300_MHZ)
    #define PRELOAD_FREQ_BUS                (PRELOAD_CLK_CTRL_187_5_MHZ)
    #define PRELOAD_FREQ_EFL                (PRELOAD_CLK_CTRL_150_MHZ)
    #define PRELOAD_FREQ_HSM                (PRELOAD_CLK_CTRL_187_5_MHZ)

    // 600
    #define PRELOAD_PLL0_P                  (1UL)
    #define PRELOAD_PLL0_M                  (100UL)
    #define PRELOAD_PLL0_S                  (1UL)

    //750
    #define PRELOAD_PLL1_P                  (1UL)
    #define PRELOAD_PLL1_M                  (125UL)
    #define PRELOAD_PLL1_S                  (1UL)

    //83.3Mhz
    #define PRELOAD_PERI_CLK_SFMC		    ((1UL<<24UL)|(8UL<<0UL));
#endif
void PRELOAD_loadOnRam(void)
{
    register uint32 uiPos;

    for(
            uiPos = 0UL;
            (((uint32)&__PRECODE_RAM_START__) + uiPos) < ((uint32)&__PRECODE_SIZE__) ;
            uiPos+=4UL
        )
    {
        SAL_WriteReg(
                        SAL_ReadReg(((uint32)&__PRECODE_START_LOAD + uiPos)),
                        (((uint32)&__PRECODE_RAM_START__) + uiPos)
                    );
    }

     return;
}

void PRELOAD_JOB
(
    void
)
{
    /*
       Change the PLL rate for low current
    */

    register uint32 uiRegDt;

    //HSM_CLK to XIN
    SAL_WriteReg(0x0UL, 0xA0F2401CUL);

    //CPU/BUS/EFL_CLK to XIN
    SAL_WriteReg(0x0UL, 0xA0F24020UL);

    //SFMC_CLK to XIN
    uiRegDt = (SAL_ReadReg(0xA0F24028UL) & 0x9FFFFFFFUL);
    SAL_WriteReg(uiRegDt, 0xA0F24028UL);

    uiRegDt = (SAL_ReadReg(0xA0F24028UL) & 0xE0FFF000UL);
    uiRegDt |= (5UL<<24UL);
    SAL_WriteReg(uiRegDt, 0xA0F24028UL);

    uiRegDt = (SAL_ReadReg(0xA0F24028UL) | (3UL<<29UL));
    SAL_WriteReg(uiRegDt, 0xA0F24028UL);



    //Disable SRC_CLK_DIV
    SAL_WriteReg(0x0UL, 0xA0F24018UL);

    // Drop  PLL0 : 1200 -> x  MHz
    uiRegDt = (SAL_ReadReg(0xA0F24000UL) & 0xFFFC0000UL);
    uiRegDt |= ((PRELOAD_PLL0_S<<15UL)|(PRELOAD_PLL0_M<<6UL)|(PRELOAD_PLL0_P<<0UL));
    SAL_WriteReg(uiRegDt, 0xA0F24000UL);

    //Check to start PLL0
    while( (SAL_ReadReg(0xA0F24000UL) & (0x1UL<<31UL)) == 0UL)
    {
       BSP_NOP_DELAY();
    }

    // Drop  PLL1 : 1500 -> x  MHz
    uiRegDt = (SAL_ReadReg(0xA0F2400CUL) & 0xFFFC0000UL);
    uiRegDt |= ((PRELOAD_PLL1_S<<15UL)|(PRELOAD_PLL1_M<<6UL)|(PRELOAD_PLL1_P<<0UL));
    SAL_WriteReg(uiRegDt, 0xA0F2400CUL);

    //Check to start PLL1
    while( (SAL_ReadReg(0xA0F2400CUL) & (0x1UL<<31UL)) == 0UL)
    {
       BSP_NOP_DELAY();
    }

    //Enable SRC_CLK_DIV
    SAL_WriteReg(0x81818100UL, 0xA0F24018UL);

    //Check to start PLL0/1 XIN Div
    while( (SAL_ReadReg(0xA0F24018UL) & ((0x1UL<<30UL)|(0x1UL<<22UL)|(0x1UL<<14UL))) != 0UL)
    {
       BSP_NOP_DELAY();
    }




    // Reset HSM rate
    uiRegDt = (PRELOAD_FREQ_HSM << 0UL);
    SAL_WriteReg(uiRegDt, 0xA0F2401CUL);

    //Check to start HSM
    while( (SAL_ReadReg(0xA0F2401CUL) & (0x1UL<<7UL)) != 0UL)
    {
       BSP_NOP_DELAY();
    }

    // Reset CPU/BUS/EFL rate (x  / x / x   MHz)
    uiRegDt  = (PRELOAD_FREQ_EFL << 24UL); //EFL
    uiRegDt |= (PRELOAD_FREQ_BUS << 8UL); //BUS
    uiRegDt |= (PRELOAD_FREQ_CPU << 0UL); //CPU
    SAL_WriteReg(uiRegDt, 0xA0F24020UL);

    //Check to start CPU/BUSS/EFL
    while( (SAL_ReadReg(0xA0F24020UL) & ((0x1UL<<31UL)|(0x1UL<<15UL)|(0x1UL<<7UL))) != 0UL)
    {
       BSP_NOP_DELAY();
    }

    // Reset SFMC rate
    uiRegDt = (SAL_ReadReg(0xA0F24028UL) & 0x9FFFFFFFUL);
    SAL_WriteReg(uiRegDt, 0xA0F24028UL); //disable

    uiRegDt = (SAL_ReadReg(0xA0F24028UL) & 0xE0FFF000UL);
    uiRegDt |= PRELOAD_PERI_CLK_SFMC;
    SAL_WriteReg(uiRegDt, 0xA0F24028UL); //src/div

    uiRegDt = (SAL_ReadReg(0xA0F24028UL) | (3UL<<29UL));
    SAL_WriteReg(uiRegDt, 0xA0F24028UL); //enable

    //Check to start SFMC
    while( (SAL_ReadReg(0xA0F24028UL) & (0x1UL<<31UL)) == 0UL)
    {
       BSP_NOP_DELAY();
    }
    return;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_PRELOAD == 1 )


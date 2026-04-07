// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : adc_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_ADC_TEST_HEADER
#define MCU_BSP_ADC_TEST_HEADER

#if MCU_BSP_SUPPORT_TEST_APP_ADC

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )
    #error MCU_BSP_SUPPORT_APP_KEY value must be 0.
#endif  // ( MCU_BSP_SUPPORT_APP_KEY ==1 )
/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <sal_internal.h>

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
#define SRAM0_BASE                      (0xc0000000UL)                      // 0xC0000000 ~ 0xC003FFFF
#define SRAM0_0_BASE                    (SRAM0_BASE+0x00000000UL)         // 0xC0000000 ~ 0xC000FFFF
#define SRAM0_1_BASE                    (SRAM0_BASE+0x00010000UL)         // 0xC0010000 ~ 0xC001FFFF
#define SRAM0_2_BASE                    (SRAM0_BASE+0x00020000UL)         // 0xC0020000 ~ 0xC002FFFF
#define SRAM0_3_BASE                    (SRAM0_BASE+0x00030000UL)         // 0xC0030000 ~ 0xC003FFFF
#define SRAM0_4_BASE                    (SRAM0_BASE+0x00040000UL)         // 0xC0040000 ~ 0xC004FFFF
#define SRAM0_5_BASE                    (SRAM0_BASE+0x00050000UL)         // 0xC0050000 ~ 0xC005FFFF
#define SRAM0_6_BASE                    (SRAM0_BASE+0x00060000UL)         // 0xC0060000 ~ 0xC006FFFF
#define SRAM0_7_BASE                    (SRAM0_BASE+0x00070000UL)         // 0xC0070000 ~ 0xC007FFFF
#define SRAM1_BASE                      (0xc1000000UL)                      // 0xC1000000 ~ 0xC000FFFF
#define ROM_BASE                        (0xc4000000UL)                      // 0xC4000000 ~ 0xC401FFFF
#define CPU_BASE                        (SRAM1_BASE)


#define SRAM0_ALIAS_BASE                (0x06000000UL)                      // 0x06000000 ~ 0x0603FFFF
#define SRAM0_0_ALIAS_BASE              (SRAM0_ALIAS_BASE+0x00000000UL)   // 0x06000000 ~ 0x0600FFFF
#define SRAM0_1_ALIAS_BASE              (SRAM0_ALIAS_BASE+0x00010000UL)   // 0x06010000 ~ 0x0601FFFF
#define SRAM0_2_ALIAS_BASE              (SRAM0_ALIAS_BASE+0x00020000UL)   // 0x06020000 ~ 0x0602FFFF
#define SRAM0_3_ALIAS_BASE              (SRAM0_ALIAS_BASE+0x00030000UL)   // 0x06030000 ~ 0x0603FFFF
#define SRAM1_ALIAS_BASE                (0x07000000UL)                      // 0x07000000 ~ 0x0700FFFF
#define SFMC_ALIAS_BASE                 (0x08000000UL)                      // 0x08000000 ~ 0x0FFFFFFF
#define SRAM0_HIV_BASE                  (0xffff0000UL)                      // 0xffff0000 ~ 0xFFFFFFFF

#define CPU_STACK_BASE                  (CPU_BASE+0x00010000UL)
#define CPU_STACK_ABT_SIZE              (0x1000UL)
#define CPU_STACK_UND_SIZE              (0x1000UL)
#define CPU_STACK_SVC_SIZE              (0x2000UL)
#define CPU_STACK_IRQ_SIZE              (0x1000UL)
#define CPU_STACK_FIQ_SIZE              (0x1000UL)
#define CPU_STACK_USR_SIZE              (0x1000UL)

#define MC_CMU_BASE_ADDR                (0x1b931000UL)

#define SMP_CMD_STS                     (0x000UL)
#define IRQ_DREQ_STS                    (0x004UL)
#define CTRL_CFG                        (0x008UL)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          ADC_StartAdcTest
*
* @param    ucMode [in]
*
* Notes
*
***************************************************************************************************
*/
void ADC_StartAdcTest
(
    int32                               ucMode
);

#endif  // MCU_BSP_SUPPORT_TEST_APP_ADC

#endif  // MCU_BSP_ADC_TEST_HEADER


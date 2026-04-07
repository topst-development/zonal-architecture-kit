// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wdt_reg.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_WDT_REG_HEADER
#define MCU_BSP_WDT_REG_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
/* Register Map */
#define WDT_EN                          (0x00UL)
#define WDT_CLR                         (0x04UL)
#define WDT_IRQ_CNT                     (0x08UL)
#define WDT_RSR_CNT                     (0x0CUL)
#define WDT_SM_MODE                     (0x10UL)
#define WDT_WR_PW                       (0x14UL)

#define WDTPMU_EN                       (0x6CUL)
#define WDTPMU_CLR                      (0x70UL)
#define WDTPMU_IRQ_CNT                  (0x74UL)
#define WDTPMU_RSR_CNT                  (0x78UL)
#define WDTPMU_SM_MODE                  (0x7CUL)

#define PMU_WDT_CTRL                    (0x0CUL)
#define PMU_WR_PW                       (0x3FCUL)       // PMU_WR_PW

/* Configuration Value */
#define WDT_EN_ON                       (0x1UL)
#define WDT_EN_OFF                      (0x0UL)
#define WDT_CLR_APPLY                   (0x1UL)
#define WDT_SM_MODE_CONT_EN             (0x0UL << 3UL)
#define WDT_SM_MODE_CONT_DIS            (0x1UL << 3UL)

#define PMU_WDT1_RSTEN                  (0x2UL)         // PMU-WDT
#define PMU_WDT0_RSTEN                  (0x1UL)         // WDT

/* Configuration Value */
#define WDT_PASSWORD                    (0x5AFEACE5UL)
#define PMU_PASSWORD                    (0x5AFEACE5UL)

#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#endif  // MCU_BSP_WDT_REG_HEADER


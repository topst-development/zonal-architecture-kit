// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gdma_dev.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_GDMA_DEV_HEADER
#define MCU_BSP_GDMA_DEV_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_GDMA == 1 )

#include "reg_phys.h"

#define GDMA_BASE_ADDRESS               (MCU_BSP_GDMA_BASE)

#define GDMA_INTSR                      (0x00UL)
#define GDMA_ITCSR                      (0x04UL)
#define GDMA_ITCCR                      (0x08UL)
#define GDMA_IESR                       (0x0CUL)
#define GDMA_IECR                       (0x10UL)
#define GDMA_RITCSR                     (0x14UL)
#define GDMA_REISR                      (0x18UL)
#define GDMA_ECR                        (0x1CUL)
#define GDMA_SBRR                       (0x20UL)
#define GDMA_SSRR                       (0x24UL)
#define GDMA_SLBRR                      (0x28UL)
#define GDMA_SLSRR                      (0x2CUL)
#define GDMA_CR                         (0x30UL)
#define GDMA_SR                         (0x34UL)

#define DMA_CH_SRC_ADDR(x)              ((0x100UL + ((x) * 0x20UL)))
#define DMA_CH_DST_ADDR(x)              ((0x104UL + ((x) * 0x20UL)))
#define DMA_CH_LLI(x)                   ((0x108UL + ((x) * 0x20UL)))
#define DMA_CH_CON(x)                   ((0x10CUL + ((x) * 0x20UL)))
#define DMA_CH_CONFIG(x)                ((0x110UL + ((x) * 0x20UL)))

#define DMA_MAX_XFER_SIZE               (0xFFFUL)

#endif  // ( MCU_BSP_SUPPORT_DRIVER_GDMA == 1 )

#endif  // MCU_BSP_GDMA_DEV_HEADER


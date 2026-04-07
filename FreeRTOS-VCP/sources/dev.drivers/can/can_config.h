// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_config.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef CAN_CONFIG_HEADER
#define CAN_CONFIG_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include "debug.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

/* Debug message control */
#if (DEBUG_ENABLE)
#define CAN_D(fmt, args...)             {LOGD(DBG_TAG_CAN, fmt, ## args)}
#define CAN_E(fmt, args...)             {LOGE(DBG_TAG_CAN, fmt, ## args)}
#else
#define CAN_D(fmt, args...)
#define CAN_E(fmt, args...)
#endif


#define CAN_CHANNEL_ADDR_OFFSET         (0x00010000UL)    /* 0xA0010000 */
#define CAN_CONFIG_ADDR                 (0xA0070000UL)
#define CAN_CONFIG_BASE_ADDR            (0xA0070004UL)
#define CAN_CONFIG_EXTS_CTRL0_ADDR      (0xA0070010UL)
#define CAN_CONFIG_EXTS_CTRL1_ADDR      (0xA0070014UL)
#define CAN_CONFIG_WR_PW_ADDR           (0xA0070020UL)
#define CAN_CONFIG_WR_LOCK_ADDR         (0xA0070024UL)

#define CAN_REG_PSR_PXE                 (14)
#define CAN_REG_PSR_RFDF                (13)
#define CAN_REG_PSR_RBRS                (12)
#define CAN_REG_PSR_RESI                (11)
#define CAN_REG_PSR_DLEC                (8)
#define CAN_REG_PSR_BO                  (7)
#define CAN_REG_PSR_EW                  (6)
#define CAN_REG_PSR_EP                  (5)
#define CAN_REG_PSR_ACT                 (3)
#define CAN_REG_PSR_LEC                 (0)

/* CAN non cache memory address is already calculated in linker script.
   Below definitions are used to check whether CAN non cache memory is out of SRAM address. */
#define CAN_NON_CACHE_MEMORY_START      (0x00000000UL) /* SRAM */
#define CAN_NON_CACHE_MEMORY_SIZE       (0x00080000UL) /* 512 Kbytes, It should be 0x00040000UL for TCC7022 */
#define CAN_NON_CACHE_MEMORY_END        (CAN_NON_CACHE_MEMORY_START + CAN_NON_CACHE_MEMORY_SIZE)

#define CAN_SW_RESET_REG_0              (*(SALReg32 *)0xA0F2000CUL)
#define CAN_BUS_CLK_MASK_REG_0          (*(SALReg32 *)0xA0F20000UL)

#define CAN_CFG_REG_FIELD_CAN_0         (14UL)
#define CAN_CFG_REG_FIELD_CAN_1         (15UL)
#define CAN_CFG_REG_FIELD_CAN_2         (16UL)
#define CAN_CFG_REG_FIELD_CAN_CFG       (17UL)

#define CAN_CHANNEL_0
#define CAN_CHANNEL_1
#define CAN_CHANNEL_2

/* can channels should be zero-based with consecutive unique values. */
typedef enum CANCh
{
#ifdef CAN_CHANNEL_0
    CAN_CH_0                            = 0U,
#endif
#ifdef CAN_CHANNEL_1
    CAN_CH_1,
#endif
#ifdef CAN_CHANNEL_2
    CAN_CH_2,
#endif
    CAN_CH_MAX
} CANCh_t;

#define CAN_CONTROLLER_NUMBER           (CAN_CH_MAX)
#define CAN_DATA_LENGTH_SIZE            (64U)

#define CAN_STANDARD_ID_FILTER_NUMBER_MAX   (6UL)
#define CAN_EXTENDED_ID_FILTER_NUMBER_MAX   (6UL)

#define CAN_STANDARD_ID_FILTER_NUMBER   (6U)
#define CAN_EXTENDED_ID_FILTER_NUMBER   (6U)

#define CAN_RX_FIFO_0_MAX               (16UL)
#define CAN_RX_FIFO_1_MAX               (16UL)
#define CAN_RX_BUFFER_MAX               (16UL)
#define CAN_TX_EVENT_FIFO_MAX           (16UL)
#define CAN_TX_BUFFER_MAX               (16UL)

#define CAN_STANDARD_ID_FILTER_SIZE     (4U)
#define CAN_EXTENDED_ID_FILTER_SIZE     (8U)
#define CAN_BUFFER_SIZE                 (8U+CAN_DATA_LENGTH_SIZE)
#define CAN_TX_EVENT_SIZE               (8U)

#define CAN_CONTROLLER_CLOCK            (40000000)    /* 40MHz */


#define CAN_RX_MSG_RING_BUFFER_MAX      (64UL)

/* TimeStamp */
#define CAN_TIMESTAMP_PRESCALER         (15)  /* Prescaler = MAX(15) +1 */
#define CAN_TIMESTAMP_TYPE              (1)   /* 0 : Internal Timestamp, 1: External Timestamp */
#define CAN_TIMESTAMP_RATIO             (CAN_TIMESTAMP_PRESCALER)
#define CAN_TIMESTAMP_COMP              (0xFFFF)

/* TimerCount */
#define CAN_TIMEOUT_VALUE               (0xFFFF)  /* reset = 0xFFFF(MAX) */
#define CAN_TIMEOUT_TYPE                (0x01) /* 0x00:Continuous, 0x01:TxEventFIFO, 0x02:RxFIFO0, 0x03:RxFIFO1 */

/* Interrupt */
#define CAN_INTERRUPT_LINE              (0U) /* 0:Line0, 1:Line1 */

#define CAN_INTERRUPT_ENABLE            (0x3FFEFFFFUL)

#if (CAN_INTERRUPT_LINE == 0U)
    #define CAN_INTERRUPT_LINE_SEL      (0x0)
    #define CAN_INTERRUPT_LINE_ENABLE   (0x1)
#elif (CAN_INTERRUPT_LINE == 1U)
    #define CAN_INTERRUPT_LINE_SEL      (0x3FFFFFFFUL)
    #define CAN_INTERRUPT_LINE_ENABLE   (0x2)
#else
##### ERROR - Select CAN Interrupt Line #####
#endif


/* WatchDog */
#define CAN_WATCHDOG_VALUE              (0xFF)


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

#endif  // CAN_CONFIG_HEADER


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spu_dev.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_SPU_DEV_HEADER
#define MCU_BSP_SPU_DEV_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_SPU == 1 )

/**************************************************************************************************
*                                       INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                       DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                       Sound Process Unit
**************************************************************************************************/
/*
 * SPU Registers pointer for internal control.
*/
typedef volatile unsigned int SPUREG32;
/**************************************************************************************************
*                                       Sound Process Unit
**************************************************************************************************/
#define SPU_BASE_ADDR                       (0xA0882000UL)

/**************************************************************************************************
*                                       Sound Source Load(SSL) Nth:0-4 Register
**************************************************************************************************/
#define SPU_SSL_CFG_REG                     (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF00UL))
#define SPU_SSL_STATUS_REG                  (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF04UL))
#define SPU_SSL_IRQ_STATUS_REG              (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF08UL))
#define SPU_SSL_IRQ_CTRL_REG                (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF0CUL))
#define SPU_SSL_FIFO_CFG_REG(N)             (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF10UL + (uint32)(0x10UL * (N))))
#define SPU_SSL_FIFO_ADDR_REG(N)            (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF14UL + (uint32)(0x10UL * (N))))
#define SPU_SSL_FIFO_SIZE_REG(N)            (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF18UL + (uint32)(0x10UL * (N))))
#define SPU_SSL_DUMMY_SIZE_REG(N)           (*(SPUREG32 *)(SPU_BASE_ADDR + 0xF1CUL + (uint32)(0x10UL * (N))))

/* SSL Global Enable */
#define SPU_SSL_GEN_BIT                     ((uint32)(0x1UL) << (uint32)(0UL))
/* SSL Status 0-4 */
#define SPU_SSL_STS_OFFSET(N)               (uint32)(4UL * (uint32)(N))
#define SPU_SSL_STS_MASK                    (0x3UL)//0b11
/* SSL IRQ Status Bit */
#define SPU_SSL_IRQ_STS_OFFSET(N)           (uint32)(1UL * (uint32)(N))
#define SPU_SSL_IRQ_STS_MASK                (0x1UL)
/* SSL IRQ CTRL Clear 0-4 */
#define SPU_SSL_IRQ_CLR_BIT(N)              ((uint32)(0x1UL) << (uint32)(0UL + (N)))
/* SSL IRQ CTRL Mask 0-4 */
#define SPU_SSL_IRQ_MASK_BIT(N)             ((uint32)(0x1UL) << (uint32)(16UL + (N)))
/* SSL FIFO Config 0-4 */
#define SPU_SSL_FIFO_CFG_EN_BIT             ((uint32)(0x1UL) << (0UL))
#define SPU_SSL_FIFO_CFG_STOP_BIT           ((uint32)(0x1UL) << (1UL))
#define SPU_SSL_FIFO_CFG_MODE_BIT           ((uint32)(0x1UL) << (4UL))
#define SPU_SSL_FIFO_CFG_REPEAT_OFFSET      (16UL)
#define SPU_SSL_FIFO_CFG_REPEAT_MASK        (0xFFFFUL)

/**************************************************************************************************
*                                       WaveGen(WG) Nth:0-4 Register
**************************************************************************************************/
#define SPU_WG_CFG0_REG(N)                  (*(SPUREG32 *)(SPU_BASE_ADDR + 0x200UL + (uint32)(0x100UL * (N))))
#define SPU_WG_CFG1_REG(N)                  (*(SPUREG32 *)(SPU_BASE_ADDR + 0x204UL + (uint32)(0x100UL * (N))))
#define SPU_WG_CFG2_REG(N)                  (*(SPUREG32 *)(SPU_BASE_ADDR + 0x208UL + (uint32)(0x100UL * (N))))
#define SPU_WG_CFG3_REG(N)                  (*(SPUREG32 *)(SPU_BASE_ADDR + 0x20CUL + (uint32)(0x100UL * (N))))
#define SPU_WG_CFG4_REG(N)                  (*(SPUREG32 *)(SPU_BASE_ADDR + 0x210UL + (uint32)(0x100UL * (N))))

/* WaveGen(WG) Start */
#define SPU_WG_START_BIT                    ((uint32)(0x1UL) << (0UL))
/* WaveGen(WG) Enable */
#define SPU_WG_ENABLE_BIT                   ((uint32)(0x1UL) << (4UL))
/* WaveGen(WG) Stop */
#define SPU_WG_STOP_BIT                     ((uint32)(0x1UL) << (16UL))
/* WaveGen(WG) Wave Type */
#define SPU_WG_WAVE_TYPE_OFFSET             (20UL)
#define SPU_WG_WAVE_TYPE_MASK               (0x7UL)//0b111
/* WaveGen(WG) Sample Frequency-Integer */
#define SPU_WG_SAMPLE_FREQ_OFFSET           (0UL)
#define SPU_WG_SAMPLE_FREQ_INTEGER_OFFSET   (3UL)
#define SPU_WG_SAMPLE_FREQ_MASK             (0x7FFFFUL)
/* WaveGen(WG) IDLE Time */
#define SPU_WG_IDLE_TIME_OFFSET             (0UL)
#define SPU_WG_IDLE_TIME_MASK               (0xFFFUL)//0b1111_1111_1111
/* WaveGen(WG) ATTACK Time */
#define SPU_WG_ATTACK_TIME_OFFSET           (16UL)
#define SPU_WG_ATTACK_TIME_MASK             (0xFFFUL)//0b1111_1111_1111
/* WaveGen(WG) STABLE Time */
#define SPU_WG_STABLE_TIME_OFFSET           (0UL)
#define SPU_WG_STABLE_TIME_MASK             (0x7FFUL)//0b111_1111_1111
/* WaveGen(WG) RELEASE Time */
#define SPU_WG_RELEASE_TIME_OFFSET          (16UL)
#define SPU_WG_RELEASE_TIME_MASK            (0x3FFUL)//0b11_1111_1111
/* WaveGen(WG) Filter Type */
#define SPU_WG_FILTER_TYPE_OFFSET           (0UL)
#define SPU_WG_FILTER_TYPE_MASK             (0x3UL)//0b11
/* WaveGen(WG) REPEAT */
#define SPU_WG_REPEAT_OFFSET                (16UL)
#define SPU_WG_REPEAT_MASK                  (0x3FUL)//0b11_1111

/**************************************************************************************************
*                                       Sample Rate Converter 0-5 Register
**************************************************************************************************/
#define SPU_SRC_CFG_REG(N)                  (*(SPUREG32 *)(SPU_BASE_ADDR + 0x700UL + (uint32)(0x100UL * (N))))
#define SPU_SRC_RATIO_REG(N)                (*(SPUREG32 *)(SPU_BASE_ADDR + 0x704UL + (uint32)(0x100UL * (N))))

/* SRC Enable */
#define SPU_SRC_ENABLE_BIT                  ((uint32)(0x1UL) << (0UL))
/* SRC Bypass */
#define SPU_SRC_BYPASS_BIT                  ((uint32)(0x1UL) << (4UL))
/* SRC Initialize */
#define SPU_SRC_INIT_BIT                    ((uint32)(0x1UL) << (8UL))
/* ZERO SIZE */
#define SPU_SRC_ZERO_SIZE_OFFSET            (16UL)
#define SPU_SRC_ZERO_SIZE_MASK              (0xFFUL)//0b1111_1111
/* FIFO SET POINT */
#define SPU_SRC_FIFO_POINT_OFFSET           (24UL)
#define SPU_SRC_FIFO_POINT_MASK             (0xFFUL)//0b1111_1111
/* RATIO Fraction */
#define SPU_SRC_RATIO_OFFSET                (0UL)
#define SPU_SRC_RATIO_INTEGER_OFFSET        (22UL)
#define SPU_SRC_RATIO_MASK                  (0x7FFFFFFUL)

/**************************************************************************************************
*                                       Mixer CH0-CH15 Register
**************************************************************************************************/
#define SPU_MIXER_CTL_REG                   (*(SPUREG32 *)(SPU_BASE_ADDR + 0xE00UL))
#define SPU_MIXER_SAT_REG                   (*(SPUREG32 *)(SPU_BASE_ADDR + 0xE04UL))

/* MIXER Saturation MAX */
#define SPU_MIXER_SAT_MAX_OFFSET            (16UL)
#define SPU_MIXER_SAT_MAX_MASK              (0xFFFFUL)

/* MIXER Saturation MIN */
#define SPU_MIXER_SAT_MIN_OFFSET            (0UL)
#define SPU_MIXER_SAT_MIN_MASK              (0xFFFFUL)

/**************************************************************************************************
*                                       Audio Volume Ctrolller 0-15 Register
**************************************************************************************************/
#define SPU_AVC_CFG0_REG(CH)                (*(SPUREG32 *)(SPU_BASE_ADDR + 0x000UL + (uint32)(0x20UL * (CH))))
#define SPU_AVC_CFG1_REG(CH)                (*(SPUREG32 *)(SPU_BASE_ADDR + 0x004UL + (uint32)(0x20UL * (CH))))
#define SPU_AVC_CFG2_REG(CH)                (*(SPUREG32 *)(SPU_BASE_ADDR + 0x008UL + (uint32)(0x20UL * (CH))))
#define SPU_AVC_CFG3_REG(CH)                (*(SPUREG32 *)(SPU_BASE_ADDR + 0x00CUL + (uint32)(0x20UL * (CH))))
#define SPU_AVC_STATUS_REG(CH)              (*(SPUREG32 *)(SPU_BASE_ADDR + 0x010UL + (uint32)(0x20UL * (CH))))

/* AVC Mode Control */
#define SPU_AVC_MODE_CTL_BIT               ((uint32)(0x1UL) << (0UL))
/* AVC Mute Control */
#define SPU_AVC_MUTE_CTL_BIT               ((uint32)(0x1UL) << (4UL))
/* AVC Volume Control Start */
#define SPU_AVC_VOL_CTL_START_BIT          ((uint32)(0x1UL) << (16UL))
/* AVC Interval */
#define SPU_AVC_INTERVAL_OFFSET             (16UL)
#define SPU_AVC_INTERVAL_MASK               (0x3FFFUL)
/* AVC Period */
#define SPU_AVC_PERIOD_OFFSET               (0UL)
#define SPU_AVC_PERIOD_MASK                 (0xFFFFUL)
/* AVC Gain */
#define SPU_AVC_GAIN_OFFSET                 (16UL)
#define SPU_AVC_GAIN_INTEGER_OFFSET         (4UL)
#define SPU_AVC_GAIN_MASK                   (0x7FFL)
/* AVC Wait */
#define SPU_AVC_WAIT_OFFSET                 (0UL)
#define SPU_AVC_WAIT_MASK                   (0xFFFFUL)
/* AVC Parameter Clear */
#define SPU_AVC_PARAM_CLR_BIT               ((uint32)(0x1UL) << (16UL))
/* AVC DB Clear */
#define SPU_AVC_DB_CLR_BIT                  ((uint32)(0x1UL) << (0UL))
/* AVC DB Status */
#define SPU_AVC_DB_STATUS_OFFSET            (0UL)
#define SPU_AVC_DB_STATUS_MASK              (0x1FFFUL)

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SPU == 1 )

#endif  // MCU_BSP_SPU_DEV_HEADER


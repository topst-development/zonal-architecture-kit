// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : ictc_dev.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_ICTC_DEV_HEADER
#define MCU_BSP_ICTC_DEV_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_ICTC == 1 )

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/******************************************************************************
 * ICTC channel
 ******************************************************************************/

#define ICTC_CHANNEL_OFFSET             (0x10000UL)

#define ICTC_CHC0_BASE                  (MCU_BSP_ICTC_BASE)
#define ICTC_CHC1_BASE                  (ICTC_CHC0_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_CHC2_BASE                  (ICTC_CHC1_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_CHC3_BASE                  (ICTC_CHC2_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_CHC4_BASE                  (ICTC_CHC3_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_CHC5_BASE                  (ICTC_CHC4_BASE + ICTC_CHANNEL_OFFSET)


/** The Redundancy ICTC0/1/2/4/5**/
#define ICTC_RED_CHC0_BASE              (MCU_BSP_ICTC_BASE + 0x80000UL)
#define ICTC_RED_CHC1_BASE              (ICTC_RED_CHC0_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_RED_CHC2_BASE              (ICTC_RED_CHC1_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_RED_CHC3_BASE              (ICTC_RED_CHC2_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_RED_CHC4_BASE              (ICTC_RED_CHC3_BASE + ICTC_CHANNEL_OFFSET)
#define ICTC_RED_CHC5_BASE              (ICTC_RED_CHC4_BASE + ICTC_CHANNEL_OFFSET)



/******************************************************************************
 * ICTC Register OFFSET
 ******************************************************************************/

#define ICTC_OP_EN_CTRL                 (0x00UL)
#define ICTC_OP_MODE_CTRL               (0x04UL)
#define ICTC_IRQ_CTRL                   (0x08UL)
#define ICTC_TIME_OUT                   (0x0cUL)
#define ICTC_R_EDGE                     (0x10UL)
#define ICTC_F_EDGE                     (0x14UL)
#define ICTC_PRD_CMP_RND                (0x18UL)
#define ICTC_DUTY_CMP_RND               (0x1cUL)
#define ICTC_EDGE_CNT_MAT               (0x20UL)
#define ICTC_CNT_PRE_PRD                (0x30UL)
#define ICTC_OLD_CNT_PRE_PRD            (0x34UL)
#define ICTC_CNT_PRE_DUTY               (0x38UL)
#define ICTC_OLD_CNT_PRE_DUTY           (0x3cUL)
#define ICTC_CUR_EDGE_CNT               (0x40UL)
#define ICTC_PRV_EDGE_CNT               (0x44UL)
#define ICTC_R_EDGE_TSTMP_CNT           (0x48UL)
#define ICTC_F_EDGE_TSTMP_CNT           (0x4cUL)

/******************************************************************************
 * Operation Enable Register (OP_EN_CTRL), ICTCn_BASE + 0x00
 ******************************************************************************/

 #define ICTC_OPEN_CTRL_RESET_VALUE     (0x00000000UL)

/* ICTC Enable*/
#define ICTC_OPEN_CTRL_EN               ((uint32)(1UL) << (31UL))

/*Prescaler counter Enable*/
#define ICTC_OPEN_CTRL_TCLK_EN          ((uint32)(1UL) << (21UL))

/*Timestamp counter Enable*/
#define ICTC_OPEN_CTRL_TSCNT_EN         ((uint32)(1UL) << (20UL))

/*Filter counter Enable*/
#define ICTC_OPEN_CTRL_FLTCNT_EN        ((uint32)(1UL) << (19UL))

/*Time-out counter Enable*/
#define ICTC_OPEN_CTRL_TOCNT_EN         ((uint32)(1UL) << (18UL))

/*Edge counter Enable*/
#define ICTC_OPEN_CTRL_EDGECNT_EN       ((uint32)(1UL) << (17UL))

/*Period and duty compare counter Enable*/
#define ICTC_OPEN_CTRL_PDCMPCNT_EN      ((uint32)(1UL) << (16UL))

/*Time stamp counter clear*/
#define ICTC_OPEN_CTRL_TSCNT_CLR        ((uint32)(1UL) << (4UL))

/*Filter counter clear*/
#define ICTC_OPEN_CTRL_FLTCNT_CLR       ((uint32)(1UL) << (3UL))

/*Timeout counter clear*/
#define ICTC_OPEN_CTRL_TOCNT_CLR        ((uint32)(1UL) << (2UL))

/*Edge counter clear*/
#define ICTC_OPEN_CTRL_EDEGCNT_CLR      ((uint32)(1UL) << (1UL))

/*period and duty compare counter clear*/
#define ICTC_OPEN_CTRL_PDCNT_CLR        ((uint32)(1UL) << (0UL))

/******************************************************************************
 * Operation Mode Control Register(OP_MODE_CTRL), ICTCn_BASE + 0x04
******************************************************************************/

#define ICTC_OPMODE_CTRL_RESET_VALUE    (0x00020000UL)

/******************************************************************************
 * Disable to Change external input for capture
 ******************************************************************************/

#define ICTC_OPMODE_CTRL_LOCK_EN_OFFSET     (31UL)
#define ICTC_OPMODE_CTRL_LOCK_EN_BIT_MASK   (0x1UL)//0b1
#define ICTC_OPMODE_CTRL_LOCK_EN            ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_LOCK_EN_OFFSET)

/******************************************************************************
 * Select whether to use absolute difference value for PRD/DT_ERR generation
 ******************************************************************************/

#define ICTC_OPMODE_CTRL_ABS_SEL_OFFSET     (29UL)
#define ICTC_OPMODE_CTRL_ABS_SEL_BIT_MASK   (0x1UL)
#define ICTC_OPMODE_CTRL_NOABS_SEL          ((uint32)(0x0UL) << ICTC_OPMODE_CTRL_ABS_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_ABS_SEL            ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_ABS_SEL_OFFSET)

/******************************************************************************
 * Select rising or falling edge for edge counter
 ******************************************************************************/

#define ICTC_OPMODE_CTRL_EDGE_SEL_OFFSET    (28UL)
#define ICTC_OPMODE_CTRL_EDGE_SEL_BIT_MASK  (0x1UL)
#define ICTC_OPMODE_CTRL_RISING_EDGE        ((uint32)(0x0UL) << ICTC_OPMODE_CTRL_EDGE_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_FALLING_EDGE       ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_EDGE_SEL_OFFSET)

/******************************************************************************
 * Pre-scaler output clock polarity change enable
 ******************************************************************************/

#define ICTC_OPMODE_CTRL_TCK_POL_OFFSET     (26UL)
#define ICTC_OPMODE_CTRL_TCK_POL_BIT_MASK   (0x1UL)
#define ICTC_OPMODE_CTRL_TCLK_POL           ((uint32)(0x1UL) <<ICTC_OPMODE_CTRL_TCK_POL_OFFSET)

/******************************************************************************
 * Pre-scaler output clock select
******************************************************************************/

#define ICTC_OPMODE_CTRL_TCK_SEL_OFFSET     (22UL)
#define ICTC_OPMODE_CTRL_TCK_SEL_BIT_MASK   (0xFUL)//0b1111
#define ICTC_OPMODE_CTRL_TCLK_DIV2          ((uint32)(0x0UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_DIV4          ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_DIV8          ((uint32)(0x2UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_DIV16         ((uint32)(0x3UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_DIV32         ((uint32)(0x4UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_DIV64         ((uint32)(0x5UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_DIV128        ((uint32)(0x6UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_DIV256        ((uint32)(0x7UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_TCLK_BYPASS        ((uint32)(0x8UL) << ICTC_OPMODE_CTRL_TCK_SEL_OFFSET)

/******************************************************************************
* Filter mode for Falling edge
******************************************************************************/

#define ICTC_OPMODE_CTRL_FLT_F_MODE_OFFSET      (20UL)
#define ICTC_OPMODE_CTRL_FLT_F_MODE_BIT_MASK    (0x3UL)//0b11
#define ICTC_OPMODE_CTRL_FLT_RST_F_MODE         ((uint32)(0x0UL) << ICTC_OPMODE_CTRL_FLT_F_MODE_OFFSET)
#define ICTC_OPMODE_CTRL_FLT_HD_F_MODE          ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_FLT_F_MODE_OFFSET)
#define ICTC_OPMODE_CTRL_FLT_UD_F_MODE          ((uint32)(0x2UL) << ICTC_OPMODE_CTRL_FLT_F_MODE_OFFSET)
#define ICTC_OPMODE_CTRL_FLT_IMM_F_MODE         ((uint32)(0x3UL) << ICTC_OPMODE_CTRL_FLT_F_MODE_OFFSET)

/******************************************************************************
* Filter mode for Rising edge
******************************************************************************/

#define ICTC_OPMODE_CTRL_FLT_R_MODE_OFFSET      (18UL)
#define ICTC_OPMODE_CTRL_FLT_R_MODE_BIT_MASK    (0x3UL)//0b11
#define ICTC_OPMODE_CTRL_FLT_RST_R_MODE         ((uint32)(0x0UL) << ICTC_OPMODE_CTRL_FLT_R_MODE_OFFSET)
#define ICTC_OPMODE_CTRL_FLT_HD_R_MODE          ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_FLT_R_MODE_OFFSET)
#define ICTC_OPMODE_CTRL_FLT_UD_R_MODE          ((uint32)(0x2UL) << ICTC_OPMODE_CTRL_FLT_R_MODE_OFFSET)
#define ICTC_OPMODE_CTRL_FLT_IMM_R_MODE         ((uint32)(0x3UL) << ICTC_OPMODE_CTRL_FLT_R_MODE_OFFSET)

/******************************************************************************
* Interrupt selection between period/duty compare result for counting
  falling edge
******************************************************************************/

#define ICTC_OPMODE_CTRL_CMP_ERR_SEL_OFFSET     (16UL)
#define ICTC_OPMODE_CTRL_CMP_ERR_SEL_BIT_MASK   (0x3UL)//0b11
#define ICTC_OPMODE_CTRL_PRD_CMP_ISR            ((uint32)(0x0UL) << ICTC_OPMODE_CTRL_CMP_ERR_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DUTY_CMP_ISR           ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_CMP_ERR_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_PRDDT_CMP_ISR          ((uint32)(0x2UL) << ICTC_OPMODE_CTRL_CMP_ERR_SEL_OFFSET)

/******************************************************************************
* DMA Request Enable
******************************************************************************/

#define ICTC_OPMODE_CTRL_DMA_EN_OFFSET      (15UL)
#define ICTC_OPMODE_CTRL_DMA_EN_BIT_MASK    (0x1UL)//0b1
#define ICTC_OPMODE_CTRL_DMA_EN             ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_DMA_EN_OFFSET)

/******************************************************************************
* DMA Request Select
******************************************************************************/

#define ICTC_OPMODE_CTRL_DMA_SEL_OFFSET     (12UL)
#define ICTC_OPMODE_CTRL_DMA_SEL_BIT_MASK   (0x7UL)//0b111
#define ICTC_OPMODE_CTRL_DMA_SEL_PPRD       ((uint32)(0x0UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DMA_SEL_PPPRD      ((uint32)(0x1UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DMA_SEL_PDT        ((uint32)(0x2UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DMA_SEL_PPDT       ((uint32)(0x3UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DMA_SEL_FEDGE      ((uint32)(0x4UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DMA_SEL_PFEDGE     ((uint32)(0x5UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DMA_SEL_RTSMP      ((uint32)(0x6UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)
#define ICTC_OPMODE_CTRL_DMA_SEL_FTSMP      ((uint32)(0x7UL) << ICTC_OPMODE_CTRL_DMA_SEL_OFFSET)

/******************************************************************************
* F_IN_SEL  Select F_IN
******************************************************************************/

#define ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET    (0UL)
#define ICTC_OPMODE_CTRL_F_IN_SEL_BIT_MASK  (0xFFUL)

/******************************************************************************
 * Interrupt Control and Status Register(IRQ_CTRL), ICTCn_BASE + 0x08
 ******************************************************************************/

/*Nosie Filter and edge detection counter full interrupt*/
#define ICTC_IRQ_CTRL_NOISEFLT_FULL_IRQ ((uint32)(1UL) << (0UL))

/*Time-out counter full interrupt*/
#define ICTC_IRQ_CTRL_TOCNT_FULL_IRQ    ((uint32)(1UL) << (1UL))

/*Edge counter full interrupt*/
#define ICTC_IRQ_CTRL_ECNT_FULL_IRQ     ((uint32)(1UL) << (2UL))

/*Duty change interrupt*/
#define ICTC_IRQ_CTRL_DT_ERR_IRQ        ((uint32)(1UL) << (3UL))

/*Frequency change interrupt*/
#define ICTC_IRQ_CTRL_PRD_ERR_IRQ       ((uint32)(1UL) << (4UL))

/*Duty and Frequency compareing counter full interrupt*/
#define ICTC_IRQ_CTRL_DTPRDCMP_FULL_IRQ ((uint32)(1UL) << (5UL))

/*Rising edge interrupt*/
#define ICTC_IRQ_CTRL_REDGE_NOTI_IRQ    ((uint32)(1UL) << (6UL))

/*Falling edge interrupt*/
#define ICTC_IRQ_CTRL_FEDGE_NOTI_IRQ    ((uint32)(1UL) << (7UL))

#define ICTC_IRQ_CTRL_STATUS_OFFSET     (0UL)
#define ICTC_IRQ_CTRL_IRQCLR_OFFSET     (8UL)
#define ICTC_IRQ_CTRL_FLAG_STS_OFFSET   (16UL)
#define ICTC_IRQ_CTRL_IRQEN_OFFSET      (24UL)

#endif  // ( MCU_BSP_SUPPORT_DRIVER_ICTC == 1 )

#endif  // MCU_BSP_ICTC_DEV_HEADER


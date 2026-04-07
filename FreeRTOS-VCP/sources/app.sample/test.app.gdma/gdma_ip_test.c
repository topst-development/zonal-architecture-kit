// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gdma_ip_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#include "gdma_ip_test.h"
#include "gdma.h"
#include "clock.h"
#include <gic.h>
#include <debug.h>
#include "gpio.h"
#include "gdma_test.h"
#include "bsp.h"
#ifdef GDMA_IP_TEST_CODE
#include "uart_test.h"
#if (DEBUG_ENABLE)
#define GDMA_IP_DBG(fmt, args...)          {LOGD(DBG_TAG_GDMA, fmt, ## args)}
#else
#define GDMA_IP_DBG(fmt, args...)
#endif


//#define TODO_CHECK
/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define PARAM_CHECK_EN
#define TCC8050_TEST

#define TOP_SIM
//#define POST
#define AP_SRAM0_BASE   (0xf0000000)                      // 0xF0000000 ~ 0xF003FFFF
#define AP_SRAM0_0_BASE (AP_SRAM0_BASE+0x00000000)      // 0xF0000000 ~ 0xF000FFFF
#define AP_SRAM0_1_BASE (AP_SRAM0_BASE+0x00010000)      // 0xF0010000 ~ 0xF001FFFF
#define AP_SRAM0_2_BASE (AP_SRAM0_BASE+0x00020000)      // 0xF0020000 ~ 0xF002FFFF
#define AP_SRAM0_3_BASE (AP_SRAM0_BASE+0x00030000)      // 0xF0030000 ~ 0xF003FFFF
#define AP_SRAM1_BASE   (0xf1000000)                      // 0xF1000000 ~ 0xF100FFFF

#define AP_INIT_DONE    (SRAM0_2_BASE + 0x0)            // 0xC0020000

#define AP_REMAP_BADDR  ((0x0<<28) |      0x0)

#define GBUS_3D_BADDR   ((0x1<<28) | (0x0<<24))
#define HSIOBUS_BADDR   ((0x1<<28) | (0x1<<24))
#define DDIBUS_BADDR    ((0x1<<28) | (0x2<<24))
#define MEMBUS_BADDR    ((0x1<<28) | (0x3<<24))
#define SMUBUS_BADDR    ((0x1<<28) | (0x4<<24))
#define VBUS_BADDR      ((0x1<<28) | (0x5<<24))
#define IOBUS_BADDR     ((0x1<<28) | (0x6<<24))
#define CBUSS_BADDR     ((0x1<<28) | (0x7<<24))
#define DAP_BADDR       ((0x1<<28) | (0x8<<24))
#define CMBUS_BADDR     ((0x1<<28) | (0x9<<24))
#define GBUS_2D_BADDR   ((0x1<<28) | (0xa<<24))
#define MICOM_BADDR     ((0x1<<28) | (0xb<<24))

#define PPU_BADDR       ((0x1<<28) | (0xc<<24))
#define STRBUS_BADDR    ((0x1<<28) | (0xd<<24))
#define HSMBUS_BADDR    ((0x1<<28) | (0xe<<24))
#define OIC_BADDR       ((0x1<<28) | (0xf<<24))

#define CAN0            (MICOM_BADDR | 0x00<<16)
#define CAN1            (MICOM_BADDR | 0x01<<16)
#define CAN2            (MICOM_BADDR | 0x02<<16)
#define CAN_CFG         (MICOM_BADDR | 0x03<<16)
#define CAN_DEF         (MICOM_BADDR | 0x04<<16)

#define GPSB0           (MICOM_BADDR | 0x10<<16)
#define GPSB1           (MICOM_BADDR | 0x11<<16)
#define GPSB2           (MICOM_BADDR | 0x12<<16)
#ifdef TCC8050_TEST
#define GPSB3           (MICOM_BADDR | 0x13<<16)
#define GPSB4           (MICOM_BADDR | 0x14<<16)
#define GPSB5           (MICOM_BADDR | 0x15<<16)
#define GPSB_CFG        (MICOM_BADDR | 0x16<<16)
#define GPSB_SM         (MICOM_BADDR | 0x17<<16)
#define GPSB_DEF        (MICOM_BADDR | 0x18<<16)
#else
#define GPSB_CFG        (MICOM_BADDR | 0x13<<16)
#define GPSB_SM         (MICOM_BADDR | 0x15<<16)
#define GPSB_DEF        (MICOM_BADDR | 0x18<<16)
#endif

#define UART0           (MICOM_BADDR | 0x20<<16)
#define UART1           (MICOM_BADDR | 0x21<<16)
#define UART2           (MICOM_BADDR | 0x22<<16)
#define UART3           (MICOM_BADDR | 0x23<<16)
#ifdef TCC8050_TEST
#define UART4           (MICOM_BADDR | 0x24<<16)
#define UART5           (MICOM_BADDR | 0x25<<16)
#define UART_DMA0       (MICOM_BADDR | 0x26<<16)
#define UART_DMA1       (MICOM_BADDR | 0x27<<16)
#define UART_DMA2       (MICOM_BADDR | 0x28<<16)
#define UART_DMA3       (MICOM_BADDR | 0x29<<16)
#define UART_DMA4       (MICOM_BADDR | 0x2a<<16)
#define UART_DMA5       (MICOM_BADDR | 0x2b<<16)
#define UART_CFG        (MICOM_BADDR | 0x2c<<16)
#define UART_DEF        (MICOM_BADDR | 0x2d<<16)
#else
#define UART_DMA0       (MICOM_BADDR | 0x25<<16)
#define UART_DMA1       (MICOM_BADDR | 0x26<<16)
#define UART_DMA2       (MICOM_BADDR | 0x27<<16)
#define UART_DMA3       (MICOM_BADDR | 0x28<<16)
#define UART_CFG        (MICOM_BADDR | 0x24<<16)
#define UART_DEF        (MICOM_BADDR | 0x2d<<16)
#endif

#define I2CM0           (MICOM_BADDR | 0x30<<16)
#define I2CM1           (MICOM_BADDR | 0x31<<16)
#define I2CM2           (MICOM_BADDR | 0x32<<16)
#define I2CM_CFG        (MICOM_BADDR | 0x33<<16)
#define I2CM0_SM        (MICOM_BADDR | 0x34<<16)
#define I2CM1_SM        (MICOM_BADDR | 0x35<<16)
#define I2CM2_SM        (MICOM_BADDR | 0x36<<16)
#define I2CM_DEF        (MICOM_BADDR | 0x37<<16)

#define PDM0            (MICOM_BADDR | 0x40<<16)

#define ICTC0_BASE          (MICOM_BADDR | 0x50<<16)
#define ICTC1           (MICOM_BADDR | 0x51<<16)
#define ICTC2           (MICOM_BADDR | 0x52<<16)
#define ICTC0_SM        (MICOM_BADDR | 0x53<<16)
#define ICTC1_SM        (MICOM_BADDR | 0x54<<16)
#define ICTC2_SM        (MICOM_BADDR | 0x55<<16)
#define ICTC_DEF        (MICOM_BADDR | 0x56<<16)

#define DMA             (MICOM_BADDR | 0x60<<16)

#define ADC_BASE            (MICOM_BADDR | 0x70<<16)
#define ADC_SM          (MICOM_BADDR | 0x71<<16)
#define ADC_DEF         (MICOM_BADDR | 0x72<<16)

#define MC_MBOX0_S      (MICOM_BADDR | 0x80<<16)
#define GIC             (MICOM_BADDR | 0x90<<16                    )

#define AP_MBOX0_S      (MICOM_BADDR | 0xa0<<16)
#define NIC400_GPV      (MICOM_BADDR | 0xb0<<16)
#define MIPI            (MICOM_BADDR | 0xc0<<16)
#define DP              (MICOM_BADDR | 0xd0<<16)
#define PERI_DEF        (MICOM_BADDR | 0xe0<<16)
#define TZ_CFG          (MICOM_BADDR | 0xff<<16 | 0x0<<14)


//----------------------------------------------------------------------------------------------------------------------------
// Global Variable
//----------------------------------------------------------------------------------------------------------------------------
#define SRAM0_BASE          (0xc0000000)                      // 0xC0000000 ~ 0xC003FFFF
#define SRAM0_0_BASE        (SRAM0_BASE+0x00000000)         // 0xC0000000 ~ 0xC000FFFF
#define SRAM0_1_BASE        (SRAM0_BASE+0x00010000)         // 0xC0010000 ~ 0xC001FFFF
#define SRAM0_2_BASE        (SRAM0_BASE+0x00020000)         // 0xC0020000 ~ 0xC002FFFF
#define SRAM0_3_BASE        (SRAM0_BASE+0x00030000)         // 0xC0030000 ~ 0xC003FFFF
#ifdef TCC8050_TEST
#define SRAM0_4_BASE        (SRAM0_BASE+0x00040000)         // 0xC0040000 ~ 0xC004FFFF
#define SRAM0_5_BASE        (SRAM0_BASE+0x00050000)         // 0xC0050000 ~ 0xC005FFFF
#define SRAM0_6_BASE        (SRAM0_BASE+0x00060000)         // 0xC0060000 ~ 0xC006FFFF
#define SRAM0_7_BASE        (SRAM0_BASE+0x00070000)         // 0xC0070000 ~ 0xC007FFFF
#endif
#define SRAM1_BASE          (0xc1000000)                      // 0xC1000000 ~ 0xC000FFFF
#define ROM_BASE            (0xc4000000)                      // 0xC4000000 ~ 0xC401FFFF
#define CPU_BASE            (SRAM1_BASE)


static uint32    gdma_irq_done;
static uint32    dsei_gdma_irq_done;
static uint32    tc_irq_sts;
static uint32    err_irq_sts;

static uint32    tc_irq_cnt;
static uint32    err_irq_cnt;
#ifdef TCC8050_TEST
static   uint32    ref_peri_data[40] = {   0x32150323,         // CAN0
                                    0x32150323,         // CAN1
                                    0x32150323,         // CAN2
                                    0x00000000,         // CAN_CFG
                                    0x0000000a,         // GPSB0
                                    0x0000000a,         // GPSB1
                                    0x0000000a,         // GPSB2
                                    0x0000000a,         // GPSB3
                                    0x0000000a,         // GPSB4
                                    0x0000000a,         // GPSB5
                                    0x3F3F3F3F,         // GPSB_CFG
                                    0x00000000,         // GPSB_SM
                                    0x00000500,         // UART0
                                    0x00000000,         // UART1
                                    0x00000000,         // UART2
                                    0x00000000,         // UART3
                                    0x00000000,         // UART4
                                    0x00000000,         // UART5
                                    0x00000000,         // UART_DMA0
                                    0x00000000,         // UART_DMA1
                                    0x00000000,         // UART_DMA2
                                    0x00000000,         // UART_DMA3
                                    0x00000000,         // UART_DMA4
                                    0x00000000,         // UART_DMA5
                                    0x00000000,         // UART_CFG
                                    0x0000ffff,         // I2CM0
                                    0x0000ffff,         // I2CM1
                                    0x0000ffff,         // I2CM2
                                    0x003F3F3F,         // I2CM_CFG
                                    0x00000000,         // I2CM0_SM
                                    0x00000000,         // I2CM1_SM
                                    0x00000000,         // I2CM2_SM
                                    0x00000000,         // ICTC0
                                    0x00000000,         // ICTC1
                                    0x00000000,         // ICTC2
                                    0x00000000,         // ICTC0_SM
                                    0x00000000,         // ICTC1_SM
                                    0x00000000,         // ICTC2_SM
                                    0x80000000,         // ADC
                                    0x80000000  };      // ADC_SM
#else // tcc8030
static uint32    ref_peri_data[34] = {   0x32150323,         // CAN0
                                    0x32150323,         // CAN1
                                    0x32150323,         // CAN2
                                    0x00000000,         // CAN_CFG
                                    0x00000000,         // GPSB0
                                    0x00000000,         // GPSB1
                                    0x00000000,         // GPSB2
                                    0x00000000,         // GPSB3
                                    0x03030100,         // GPSB_CFG
                                    0x00000000,         // GPSB_SM
                                    0x00000000,         // UART0
                                    0x00000000,         // UART1
                                    0x00000000,         // UART2
                                    0x00000000,         // UART3
                                    0x00000000,         // UART_DMA0
                                    0x00000000,         // UART_DMA1
                                    0x00000000,         // UART_DMA2
                                    0x00000000,         // UART_DMA3
                                    0x00000000,         // UART_CFG
                                    0x0000ffff,         // I2CM0
                                    0x0000ffff,         // I2CM1
                                    0x0000ffff,         // I2CM2
                                    0x00020100,         // I2CM_CFG
                                    0x00000000,         // I2CM0_SM
                                    0x00000000,         // I2CM1_SM
                                    0x00000000,         // I2CM2_SM
                                    0x00000000,         // ICTC0
                                    0x00000000,         // ICTC1
                                    0x00000000,         // ICTC2
                                    0x00000000,         // ICTC0_SM
                                    0x00000000,         // ICTC1_SM
                                    0x00000000,         // ICTC2_SM
                                    0x80000000,         // ADC
                                    0x80000000  };      // ADC_SM
#endif

/*
 Import ADC
*/
//----------------------------------------------------------------------------------------------------------------------------
// Define
//----------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------
// ADC controller register structure
//----------------------------------------------------------------------------------------------------------------------------
// MC_ADC_CMD_U
typedef struct {
    uint32                CMD             : 16;   // [15:00], R/W
    uint32                NOF_AIN         :  4;   // [19:16], RO
    uint32                CNV_CNT         :  5;   // [24:20], RO
    uint32                                :  6;   // [30:25], Reserved
    uint32                CNV_DONE        :  1;   // [   31], RO
} MC_ADC_CMD;

typedef union {
    uint32                nReg;
    MC_ADC_CMD          bReg;
} MC_ADC_CMD_U;

// MC_ADC_CLR_U
typedef struct {
    uint32                CLR_IRQ         :  1;   // [   00], R/W
    uint32                CLR_REQ         :  1;   // [   01], R/W
    uint32                DMA_ACK         :  1;   // [   02], RO
    uint32                                : 29;   // [31:03], Reserved
} MC_ADC_CLR;

typedef union {
    uint32                nReg;
    MC_ADC_CLR          bReg;
} MC_ADC_CLR_U;

// MC_ADC_CLK_U
typedef struct {
    uint32                DIV             :  8;   // [07:00], R/W
    uint32                IRQ_EN          :  1;   // [   08], R/W
    uint32                REQ_EN          :  1;   // [   09], R/W
    uint32                                : 22;   // [31:10], Reserved
} MC_ADC_CLK;

typedef union {
    uint32                nReg;
    MC_ADC_CLK          bReg;
} MC_ADC_CLK_U;

// MC_ADC_TIME_U
typedef struct {
    uint32                PRE_PWR_ON      :  4;   // [03:00], R/W
    uint32                SOC             :  4;   // [07:04], R/W
    uint32                PST_PWR_OFF     :  4;   // [11:08], R/W
    uint32                CVT             :  4;   // [15:12], R/W
    uint32                PRE_SEL         :  4;   // [19:16], R/W
    uint32                CAPTURE_MODE    :  1;   // [   20], R/W
    uint32                                : 11;   // [31:21], Reserved
} MC_ADC_TIME;

typedef union {
    uint32                nReg;
    MC_ADC_TIME         bReg;
} MC_ADC_TIME_U;

//----------------------------------------------------------------------------------------------------------------------------
// ADC controller register structure
//----------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MC_ADC_CMD_U        uADC_CMD;               // 0x000
    MC_ADC_CLR_U        uADC_CLR;               // 0x004
    MC_ADC_CLK_U        uADC_CLK;               // 0x008
    uint32                reserved_0x00c[13];     // 0x00c ~ 0x03c

    MC_ADC_TIME_U       uADC_TIME;              // 0x040
    uint32                reserved_0x044[15];     // 0x044 ~ 0x07c

    uint32                uADC_AIN_00;            // 0x080
    uint32                uADC_AIN_01;            // 0x084
    uint32                uADC_AIN_02;            // 0x088
    uint32                uADC_AIN_03;            // 0x08c
    uint32                uADC_AIN_04;            // 0x090
    uint32                uADC_AIN_05;            // 0x094
    uint32                uADC_AIN_06;            // 0x098
    uint32                uADC_AIN_07;            // 0x09c
    uint32                uADC_AIN_08;            // 0x0a0
    uint32                uADC_AIN_09;            // 0x0a4
    uint32                uADC_AIN_10;            // 0x0a8
    uint32                uADC_AIN_11;            // 0x0ac
    uint32                uADC_AIN_12;            // 0x0b0
    uint32                uADC_AIN_13;            // 0x0b4
    uint32                uADC_AIN_14;            // 0x0b8
    uint32                uADC_AIN_15;            // 0x0bc
} MC_ADC;

/*
  Import ICTC
*/
//----------------------------------------------------------------------------------------------------------------------------
// Define
//----------------------------------------------------------------------------------------------------------------------------
#define     DMA_REQ_PRD_PVL         (0)
#define     DMA_REQ_PRD_PPVL        (1)
#define     DMA_REQ_DT_PVL          (2)
#define     DMA_REQ_DT_PPVL         (3)
#define     DMA_REQ_EDGE_CNT_CVL    (4)
#define     DMA_REQ_EDGE_CNT_PVL    (5)
#define     DMA_REQ_TS_RVL          (6)
#define     DMA_REQ_TS_FVL          (7)

#define     DMA_ENABLE              (1)
#define     DMA_DISABLE             (0)

#define     CMP_ERR_PRD_ONLY        (0)
#define     CMP_ERR_DT_ONLY         (1)
#define     CMP_ERR_PRD_DT          (2)

#define     FLT_MODE_RST            (0)
#define     FLT_MODE_HD             (1)
#define     FLT_MODE_UD             (2)
#define     FLT_MODE_IMM            (3)

#define     USE_ABS_SEL             (1)
#define     NO_ABS_SEL              (0)

#define     FEDGE_CNT_SEL           (1)
#define     REDGE_CNT_SEL           (0)

//----------------------------------------------------------------------------------------------------------------------------
// ICTC register structure
//----------------------------------------------------------------------------------------------------------------------------
typedef struct {
    uint32    PD_CMP_CNT_CLEAR    :  1; //[00:00]
    uint32    EDGE_CNT_CLEAR      :  1; //[01:01]
    uint32    TO_CNT_CLEAR        :  1; //[02:02]
    uint32    FLT_CNT_CLEAR       :  1; //[03:03]
    uint32    TS_CNT_CLEAR        :  1; //[04:04]
    uint32                        : 11; //[15:05]
    uint32    PD_CMP_CNT_EN       :  1; //[16:16]
    uint32    EDGE_CNT_EN         :  1; //[17:17]
    uint32    TO_CNT_EN           :  1; //[18:18]
    uint32    FLT_CNT_EN          :  1; //[19:19]
    uint32    TS_CNT_EN           :  1; //[20:20]
    uint32    TCLK_EN             :  1; //[21:21]
    uint32                        :  9; //[30:22]
    uint32    ICTC_EN             :  1; //[31:31]
} MC_ICTC_EN;

typedef union {
    uint32        nICTC_EN;
    MC_ICTC_EN  bICTC_EN;
} MC_ICTC_EN_U;

typedef struct {
    uint32    F_IN_SEL        : 8; //[07:00]
    uint32                    : 4; //[11:08]
    uint32    DMA_SEL         : 3; //[14:12]
    uint32    DMA_EN          : 1; //[15:15]
    uint32    CMP_ERR_SEL     : 2; //[17:16]
    uint32    FLT_R_MODE      : 2; //[19:18]
    uint32    FLT_F_MODE      : 2; //[21:20]
    uint32    TCLK_SEL        : 4; //[25:22]
    uint32    TCLK_POL        : 1; //[26:26]
    uint32                    : 1; //[27:27]
    uint32    FALL_EDGE_SEL   : 1; //[28:28]
    uint32    ABS_SEL         : 1; //[29:29]
    uint32                    : 1; //[30:30]
    uint32                    : 1; //[31:31]
} MC_ICTC_CTRL;

typedef union {
    uint32            nICTC_CTRL;
    MC_ICTC_CTRL    bICTC_CTRL;
} MC_ICTC_CTRL_U;

typedef struct {
    uint32    IRQ0    : 1; //[00:00] //read-only
    uint32    IRQ1    : 1; //[01:01] //read-only
    uint32    IRQ2    : 1; //[02:02] //read-only
    uint32    IRQ3    : 1; //[03:03] //read-only
    uint32    IRQ4    : 1; //[04:04] //read-only
    uint32    IRQ5    : 1; //[05:05] //read-only
    uint32    IRQ6    : 1; //[06:06] //read-only
    uint32    IRQ7    : 1; //[07:07] //read-only
    uint32    ICLR0   : 1; //[08:08]
    uint32    ICLR1   : 1; //[09:09]
    uint32    ICLR2   : 1; //[10:10]
    uint32    ICLR3   : 1; //[11:11]
    uint32    ICLR4   : 1; //[12:12]
    uint32    ICLR5   : 1; //[13:13]
    uint32    ICLR6   : 1; //[14:14]
    uint32    ICLR7   : 1; //[15:15]
    uint32    FLG0    : 1; //[16:16] //read-only
    uint32    FLG1    : 1; //[17:18] //read-only
    uint32    FLG2    : 1; //[18:18] //read-only
    uint32    FLG3    : 1; //[19:19] //read-only
    uint32    FLG4    : 1; //[20:20] //read-only
    uint32    FLG5    : 1; //[21:21] //read-only
    uint32    FLG6    : 1; //[22:22] //read-only
    uint32    FLG7    : 1; //[23:23] //read-only
    uint32    IEN0    : 1; //[24:24]
    uint32    IEN1    : 1; //[25:25]
    uint32    IEN2    : 1; //[26:26]
    uint32    IEN3    : 1; //[27:27]
    uint32    IEN4    : 1; //[28:28]
    uint32    IEN5    : 1; //[29:29]
    uint32    IEN6    : 1; //[30:30]
    uint32    IEN7    : 1; //[31:31]
} MC_ICTC_IRQ;

typedef union {
    uint32            nICTC_IRQ;
    MC_ICTC_IRQ     bICTC_IRQ;
} MC_ICTC_IRQ_U;

typedef struct {
    uint32    TO_VL   : 28; //[27:00]
    uint32            :  4; //[31:28]
} MC_ICTC_TO_VL;

typedef union {
    uint32            nICTC_TO_VL;
    MC_ICTC_TO_VL   bICTC_TO_VL;
} MC_ICTC_TO_VL_U;

typedef struct {
    uint32    REDGE_VL    : 28; //[27:00]
    uint32                :  4; //[31:28]
} MC_ICTC_REDGE_VL;

typedef union {
    uint32                nICTC_REDGE_VL;
    MC_ICTC_REDGE_VL    bICTC_REDGE_VL;
} MC_ICTC_REDGE_VL_U;

typedef struct {
    uint32    FEDGE_VL    : 28; //[27:00]
    uint32                :  4; //[31:28]
} MC_ICTC_FEDGE_VL;

typedef union {
    uint32                nICTC_FEDGE_VL;
    MC_ICTC_FEDGE_VL    bICTC_FEDGE_VL;
} MC_ICTC_FEDGE_VL_U;

typedef struct {
    uint32    PRD_RND_VL  : 28; //[27:00]
    uint32                :  4; //[31:28]
} MC_ICTC_PRD_RND_VL;

typedef union {
    uint32                nICTC_PRD_RND_VL;
    MC_ICTC_PRD_RND_VL  bICTC_PRD_RND_VL;
} MC_ICTC_PRD_RND_VL_U;

typedef struct {
    uint32    DT_RND_VL   : 28; //[27:00]
    uint32                :  4; //[31:28]
} MC_ICTC_DT_RND_VL;

typedef union {
    uint32                nICTC_DT_RND_VL;
    MC_ICTC_DT_RND_VL   bICTC_DT_RND_VL;
} MC_ICTC_DT_RND_VL_U;

typedef struct {
    uint32    ECNT_VL     : 16; //[15:00]
    uint32                : 16; //[31:16]
} MC_ICTC_ECNT_VL;

typedef union {
    uint32                nICTC_ECNT_VL;
    MC_ICTC_ECNT_VL     bICTC_ECNT_VL;
} MC_ICTC_ECNT_VL_U;

typedef struct {
    uint32    PRD_CNT_PVL : 28; //[27:00] //read-only
    uint32                :  4; //[31:28]
} MC_ICTC_PRD_CNT_PVL;

typedef union {
    uint32                    nICTC_PRD_CNT_PVL;
    MC_ICTC_PRD_CNT_PVL     bICTC_PRD_CNT_PVL;
} MC_ICTC_PRD_CNT_PVL_U;

typedef struct {
    uint32    PRD_CNT_PPVL    : 28; //[27:00] //read-only
    uint32                    :  4; //[31:28]
} MC_ICTC_PRD_CNT_PPVL;

typedef union {
    uint32                    nICTC_PRD_CNT_PPVL;
    MC_ICTC_PRD_CNT_PPVL    bICTC_PRD_CNT_PPVL;
} MC_ICTC_PRD_CNT_PPVL_U;

typedef struct {
    uint32    DT_CNT_PVL      : 28; //[27:00] //read-only
    uint32                    :  4; //[31:28]
} MC_ICTC_DT_CNT_PVL;

typedef union {
    uint32                    nICTC_DT_CNT_PVL;
    MC_ICTC_DT_CNT_PVL      bICTC_DT_CNT_PVL;
} MC_ICTC_DT_CNT_PVL_U;

typedef struct {
    uint32    DT_CNT_PPVL     : 28; //[27:00] //read-only
    uint32                    :  4; //[31:28]
} MC_ICTC_DT_CNT_PPVL;
typedef union {
    uint32                    nICTC_DT_CNT_PPVL;
    MC_ICTC_DT_CNT_PPVL     bICTC_DT_CNT_PPVL;
} MC_ICTC_DT_CNT_PPVL_U;

typedef struct {
    uint32    EDGE_DET_CNT_CVL    : 16; //[15:00] //read-only
    uint32                        : 16; //[31:16]
} MC_ICTC_EDGE_DET_CNT_CVL;

typedef union {
    uint32                        nICTC_EDGE_DET_CNT_CVL;
    MC_ICTC_EDGE_DET_CNT_CVL    bICTC_EDGE_DET_CNT_CVL;
} MC_ICTC_EDGE_DET_CNT_CVL_U;

typedef struct {
    uint32    EDGE_DET_CNT_PVL    : 16; //[15:00] //read-only
    uint32                        : 16; //[31:16]
} MC_ICTC_EDGE_DET_CNT_PVL;

typedef union {
    uint32                        nICTC_EDGE_DET_CNT_PVL;
    MC_ICTC_EDGE_DET_CNT_PVL    bICTC_EDGE_DET_CNT_PVL;
} MC_ICTC_EDGE_DET_CNT_PVL_U;

typedef struct {
    uint32    REDGE_TSCNT_VL      : 16; //[15:00] //read-only
    uint32                        : 16; //[31:16]
} MC_ICTC_REDGE_TSCNT_VL;
typedef union {
    uint32                    nICTC_REDGE_TSCNT_VL;
    MC_ICTC_REDGE_TSCNT_VL  bICTC_REDGE_TSCNT_VL;
} MC_ICTC_REDGE_TSCNT_VL_U;

typedef struct {
    uint32    FEDGE_TSCNT_VL      : 16; //[15:00] //read-only
    uint32                        : 16; //[31:16]
} MC_ICTC_FEDGE_TSCNT_VL;

typedef union {
    uint32                    nICTC_FEDGE_TSCNT_VL;
    MC_ICTC_FEDGE_TSCNT_VL  bICTC_FEDGE_TSCNT_VL;
} MC_ICTC_FEDGE_TSCNT_VL_U;


//==================================================================================
//              ICTC REGISTER MAP
//==================================================================================
typedef struct {
    MC_ICTC_EN_U                uICTC_EN;               //0x00 //0
    MC_ICTC_CTRL_U              uICTC_CTRL;             //0x04 //1
    MC_ICTC_IRQ_U               uICTC_IRQ;              //0x08 //2
    MC_ICTC_TO_VL_U             uICTC_TO_VL;            //0x0c //3
    MC_ICTC_REDGE_VL_U          uICTC_REDGE_VL;         //0x10 //4
    MC_ICTC_FEDGE_VL_U          uICTC_FEDGE_VL;         //0x14 //5
    MC_ICTC_PRD_RND_VL_U        uICTC_PRD_RND_VL;       //0x18 //6
    MC_ICTC_DT_RND_VL_U         uICTC_DT_RND_VL;        //0x1c //7
    MC_ICTC_ECNT_VL_U           uICTC_ECNT_VL;          //0x20 //8
    uint32                        undef00[3];             //0x24 ~ 0x2C
    MC_ICTC_PRD_CNT_PVL_U       uICTC_PRD_CNT_PVL;      //0x30 //12    0x20 //8
    MC_ICTC_PRD_CNT_PPVL_U      uICTC_PRD_CNT_PPVL;     //0x34 //13    0x24 //9
    MC_ICTC_DT_CNT_PVL_U        uICTC_DT_CNT_PVL;       //0x38 //14    0x28 //10
    MC_ICTC_DT_CNT_PPVL_U       uICTC_DT_CNT_PPVL;      //0x3C //15    0x2c //11
    MC_ICTC_EDGE_DET_CNT_CVL_U  uICTC_EDGE_DET_CNT_CVL; //0x40 //16    0x30 //12
    MC_ICTC_EDGE_DET_CNT_PVL_U  uICTC_EDGE_DET_CNT_PVL; //0x44 //17    0x34 //13
    MC_ICTC_REDGE_TSCNT_VL_U    uICTC_REDGE_TSCNT_VL;   //0x48
    MC_ICTC_FEDGE_TSCNT_VL_U    uICTC_FEDGE_TSCNT_VL;   //0x4C
} MC_ICTC;

/*
  Import DSEI
*/
// Interrupt mask
typedef struct {
    uint32                MAIN_AHBM           :  1;   //  [   00], RW
    uint32                AP_AHBMUX           :  1;   //  [   01], RW
    uint32                SYS_AHBMUX          :  1;   //  [   02], RW
    uint32                UART_AHBM           :  1;   //  [   03], RW
    uint32                PDM_AHBMUX          :  1;   //  [   04], RW
    uint32                MBOX_AHBMUX         :  1;   //  [   05], RW
    uint32                CAN_AHBM            :  1;   //  [   06], RW
    uint32                GPSB_AHBM           :  1;   //  [   07], RW
    uint32                reserved            : 24;   //  [31:08]
} MC_DEF_SLV_IRQ_MASK;

typedef union {
    uint32                nReg;
    MC_DEF_SLV_IRQ_MASK bReg;
} MC_DEF_SLV_IRQ_MASK_U;

// Interrupt enable
typedef struct {
    uint32                IRQ_EN              :  1;   //  [   00], RW
    uint32                reserved            : 31;   //  [31:01]
} MC_DEF_SLV_IRQ_EN;

typedef union {
    uint32                nReg;
    MC_DEF_SLV_IRQ_EN   bReg;
} MC_DEF_SLV_IRQ_EN_U;

// Default slave error IRQ handler
typedef struct {
    uint32                MAIN_AHBM_RESP_EN   :  1;   //  [   00], RW
    uint32                AP_AHBMUX_RESP_EN   :  1;   //  [   01], RW
    uint32                SYS_AHBMUX_RESP_EN  :  1;   //  [   02], RW
    uint32                UART_AHBM_RESP_EN   :  1;   //  [   03], RW
    uint32                PDM_AHBMUX_RESP_EN  :  1;   //  [   04], RW
    uint32                MBOX_AHBMUX_RESP_EN :  1;   //  [   05], RW
    uint32                reserved0           : 10;   //  [15:06]
    uint32                MAIN_AHBM_IRQ_EN    :  1;   //  [   16], RW
    uint32                AP_AHBMUX_IRQ_EN    :  1;   //  [   17], RW
    uint32                SYS_AHBMUX_IRQ_EN   :  1;   //  [   18], RW
    uint32                UART_AHBM_IRQ_EN    :  1;   //  [   19], RW
    uint32                PDM_AHBMUX_IRQ_EN   :  1;   //  [   20], RW
    uint32                MBOX_AHBMUX_IRQ_EN  :  1;   //  [   21], RW
    uint32                CAN_AHBM_IRQ_EN     :  1;   //  [   22], RW
    uint32                GPSB_AHBM_IRQ_EN    :  1;   //  [   23], RW
    uint32                reserved1           :  8;   //  [31:24]
} MC_DEF_SLV_CFG;

typedef union {
    uint32                nReg;
    MC_DEF_SLV_CFG      bReg;
} MC_DEF_SLV_CFG_U;

// Default slave error IRQ handler soft fault status
typedef struct {
    uint32                reserved0           :  1;   // [   00]
    uint32                REG_IRQ_MASK        :  1;   // [   01], RW, 0x0_4, HCLK clock mask-1
    uint32                REG_IRQ_ENABLE      :  1;   // [   02], RW, 0x0_8, HCLK clock mask-2
    uint32                REG_DEF_SLV_CFG     :  1;   // [   03], RW, 0x0_c, SW HRESETn mask-0
    uint32                REG_CFG_WR_PW       :  1;   // [   04], RW, 0x1_0, SW HRESETn mask-1
    uint32                REG_WR_LOCK         :  1;   // [   05], RW, 0x1_0, SW HRESETn mask-1
    uint32                reserved1           : 26;   // [31:06], Reserved
} MC_DEF_SLV_SOFT_FAULT_EN;

typedef union {
    uint32                        nReg;
    MC_DEF_SLV_SOFT_FAULT_EN    bReg;
} MC_DEF_SLV_SOFT_FAULT_EN_U;

//----------------------------------------------------------------------------------------------------------------------------
// Default slave error IRQ handler register structure
//----------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MC_DEF_SLV_IRQ_MASK_U       uIRQ_STATUS;            // 0x00
    MC_DEF_SLV_IRQ_MASK_U       uIRQ_MASK;              // 0x04
    MC_DEF_SLV_IRQ_EN_U         uIRQ_ENABLE;            // 0x08
    MC_DEF_SLV_CFG_U            uDEF_SLV_CFG;           // 0x0c
    uint32                      uDEF_SLV_0;         // 0x10
    uint32                      uDEF_SLV_1;         // 0x14
    uint32                      uDEF_SLV_2;         // 0x18
    uint32                      uDEF_SLV_3;         // 0x1c
    uint32                      uDEF_SLV_4;         // 0x20
    uint32                      uDEF_SLV_5;         // 0x24
    uint32                      uDEF_SLV_6;         // 0x28
    uint32                      uDEF_SLV_7;         // 0x2c
    uint32                      uDEF_SLV_8;         // 0x30
    uint32                      reserved[3];            // 0x34-3c
    uint32                        CFG_WR_PW;              // 0x40
    uint32                        CFG_WR_LOCK;            // 0x44
    MC_DEF_SLV_SOFT_FAULT_EN_U  uSOFT_FAULT_EN;         // 0x48
    MC_DEF_SLV_SOFT_FAULT_EN_U  uSOFT_FAULT_STS;        // 0x4c
} MC_DEF_SLV;

#define CAPTURE_MODE_FSM    (0U)
#define CAPTURE_MODE_EOC    (1U)
#define DMA_CH_MAX  (2U)
#define DMA_CONTROL_MAX     (8U)

#define wr_data_32b(addr, data) (*(volatile uint32*)(addr)) = (uint32)(data)
#define rd_data_32b(addr)       (*(volatile uint32*)(addr))

//----------------------------------------------------------------------------------------------------------------------------
// Global Variable
//----------------------------------------------------------------------------------------------------------------------------
static uint32    dma_irq_check = 0;
//static uint32    dma_irq_check_cont_max= 0;

static uint32    ref_adc_data[16] = { 0x000, 0x08e, 0x11c, 0x1aa,
                             0x238, 0x2c7, 0x355, 0x3e3,
                             0x471, 0x4ff, 0x58e, 0x61c,
                             0x6aa, 0x738, 0x7c7, 0x855};

#define GDMA_TEST_MEM_BSE (0x3C000U)

static uint32 GDMA_TEST_MEM = (0x3C000U);//0xc1000000; //SRAM1 64KB
static uint32 dma_error;

static GDMAInformation_t gContorlData;
/*
***************************************************************************************************
*                                         LOCAL FUNCTION PROTOTYPES
***************************************************************************************************
*/
//static
static int GDMA_IPTEST_M2MTransfer(uint8 control, uint8 ch, uint8 * src, uint8 * dest, uint32 lengh, uint32 trans_size, uint32 enable);
static void GDMA_IPTEST_AdcConfig(uint8 ch);
static void GDMA_IPTEST_IctcConfig(uint8 dma_ch, uint8 ictc_ch);
static int32 GDMA_IPTEST_IctcDmaTest(uint8 dma_ch, uint8 ictc_ch);
static void GDMA_IPTEST_Enable(uint8 ch, uint32 enable);
static int32 GDMA_F0_ClockGatingAndSWResetTest(void);
static int32 GDMA_F1_IrqTest(void);
static int32 GDMA_F3_AHBMasterTest(void);
static int32 GDMA_F4_PriorityTest(void);
static int32 GDMA_F5_ScatterOrGatherTest(void);
static int32 GDAM_F6_SingleBurstRequestTest(void);
static int32 GDMA_F7_AcceptableMemoryTest(void);
static int32 GDMA_F8_P2MIctcTest(void);
static int32 GDMA_F9_P2M_AdcTest(void);
static int32 GDMA_F10_BusDefaultSlaveErrorTest(void);
static void GDMA_IPTEST_SWReset(void);
static uint32 check_dma_irq(void * p);
static void GDMA_IPTEST_WaitDmaIrq(void);
static void GDMA_IPTEST_AdcDmaTest(uint8 ch, uint32 cap_mode);

typedef enum
{
    GDMA_TEST_TYPE_NONE = -1,
    GDMA_TEST_TYPE_ADC = 0,
    GDMA_TEST_TYPE_ICTC = 1,
    GDMA_TEST_TYPE_PRIORITY = 2,
    GDMA_TEST_TYPE_LLI = 3,
    GDMA_TEST_TYPE_SINGLE_BURST = 4,
} GDMA_TEST_TYPE_T;

static int32 current_test = GDMA_TEST_TYPE_NONE;
static int32 irq_cnt = 0;
static int32 gdma_irq_status[8];
//----------------------------------------------------------------------------------------------------------------------------
// Sub Function
//----------------------------------------------------------------------------------------------------------------------------
// DMA enable get/set/clr
static void GDMA_IPTEST_DmaEnable(uint32 control);

// DMA request synchronize enable get/set/clr
static void GDMA_IPTEST_DmaSyncEnable(void);

// DMA channel address configuration
static void GDMA_IPTEST_SetAddress(uint32 control, uint32 ch, uint32 src_addr, uint32 dst_addr, uint32 lli_addr);

// DMA channel control configuration
static void GDMA_IPTEST_SetControlRegister(uint32 control, uint32 ch, uint32 trans_size, uint32 src_burst_size, uint32 dst_burst_size, uint32 src_width, uint32 dst_width, uint32 src_bus, uint32 dst_bus, uint32 src_incr, uint32 dst_incr, uint32 prot, uint32 irq_en);

// DMA channel configuration
static void GDMA_IPTEST_SetConfigureRegister(uint32 control,   uint32 ch, uint32 src_peri, uint32 dst_peri, uint32 flow_ctrl, uint32 err_irq_mask, uint32 tc_irq_mask, uint32 bus_lock);

// DMA channel enable get/set/clr
static void GDMA_IPTEST_SetChannelEnable(uint32 control, uint32 ch);

// Get/Clear DMA terminal counter interrupt ststua
static uint32 GDMA_IPTEST_GetDmaTerminalCounterIrqStatus(void);
static void GDMA_IPTEST_ClearDmaTerminalCounterIrqStatus(uint32 irq_ch);

// Get/Clear DMA error interrupt status
static uint32 GDMA_IPTEST_GetDmaErrorIrqStatus(void);
static void GDMA_IPTEST_ClearDmaErrorIrqStatus(uint32 irq_ch);

// DMA interrupt service routine
static void GDMA_IPTEST_IsrHandler(void* param);

// Wait DMA IRQ done
static void GDMA_IPTEST_WaitIrqDone(void);

// DMA memory to memory test normal function
static void GDMA_IPTEST_M2MNormalTransferTest(uint32 control,   uint32 ch, uint32 src_addr, uint32 src_incr, uint32 dst_addr, uint32 dst_incr, uint32 trans_size);

// DMA memory to memory test LLI function
static void GDMA_IPTEST_M2MLinkedListTest(uint32 control, uint32 ch, uint32 src_addr, uint32 src_incr, uint32 dst_addr, uint32 dst_incr, uint32 trans_size, uint32 lli_addr);

// DMA memory to memory test LLI function
static void GDMA_IPTEST_M2MLinkedListConfig(uint32 ch, uint32 src_addr, uint32 src_incr, uint32 dst_addr, uint32 dst_incr, uint32 trans_size, uint32 lli_addr, dma_sLLI_FORMAT *lli_cfg);

// Clear memory regiion
#define MICOM_BASE_ADDR                 (0xA0000000UL)
#define GDMA_BASE_ADDR                 (MICOM_BASE_ADDR + 0x800000)

#define GDMA_CONTROL_OFFSET (0x10000UL)
#define HwMC_DMA            ((volatile DMA_sPort        *)(MICOM_BASE_ADDR + 0x800000))
#define HwMC_ADC            ((volatile MC_ADC           *)(MICOM_BASE_ADDR + 0x700000))
#define HwMC_DEF_SLV        ((volatile MC_DEF_SLV       *)(MICOM_BASE_ADDR + 0xF27000))
#define HwMC_ICTC0          ((volatile MC_ICTC          *)(MICOM_BASE_ADDR + 0x500000))
#define HwMC_CCU            ((volatile MC_CCU           *)(MICOM_BASE_ADDR + 0x931000))
#define HwMC_CFG            ((volatile MC_CFG           *)(MICOM_BASE_ADDR + 0xF20000))
#define MC_CFG_BASE            (MICOM_BASE_ADDR + 0xF20000)
#define HwMC_CFG_CLL_MSK            (MC_CFG_BASE+ 0x00)
#define HwMC_CFG_SW_RESET            (MC_CFG_BASE+ 0x0C)

static void GDMA_IPTEST_SWReset(void)
{
    uint32 i;
    for ( i=0U; i < DMA_CONTROL_MAX; i++)
    {
        wr_data_32b((uint32)HwMC_CFG_SW_RESET, (uint32)(rd_data_32b((uint32)HwMC_CFG_SW_RESET) & ~(1UL<<(6UL+i))));
        wr_data_32b((uint32)HwMC_CFG_SW_RESET, (uint32)(rd_data_32b((uint32)HwMC_CFG_SW_RESET) |(uint32)(1U<<(6U+i))));
    }
}

static void GDMA_IPTEST_ClockGate(uint32 con, uint32 onoff)
{
    if(onoff == TRUE)
    {
        wr_data_32b(HwMC_CFG_CLL_MSK, rd_data_32b(HwMC_CFG_CLL_MSK) |(1UL<<(6+con)));
    }
    else
    {
        wr_data_32b(HwMC_CFG_CLL_MSK, rd_data_32b(HwMC_CFG_CLL_MSK) & ~(1UL<<(6+con)));

    }
}

// ADC interrupt handler
static uint32 check_dma_irq(void * p)
{
    uint32        index;
    uint32        nof_sample;
    uint32        adc_ch;
    uint32        ref_data;
    uint32        adc_data;
    GDMAInformation_t * pCtrl;
    DMA_sPort *pDMA;

    if ( p != NULL_PTR)
    {
        pCtrl = (GDMAInformation_t *)p;

           pDMA = (( DMA_sPort        *)(GDMA_BASE_ADDR + (GDMA_CONTROL_OFFSET*pCtrl->iCon)));

        // Clear interrupt
        if ( current_test == (int32)GDMA_TEST_TYPE_PRIORITY )
        {
            gdma_irq_status[irq_cnt] = pDMA->IRQ_ITC_STATUS.nReg;
            irq_cnt++;
        }

        pDMA->IRQ_ITC_CLEAR.nReg = pDMA->IRQ_ITC_STATUS.nReg;

        if ( current_test == (int32)GDMA_TEST_TYPE_ADC)
        {
            // Read number of sample
            nof_sample = HwMC_ADC->uADC_CMD.bReg.CNV_CNT;

            // ADC command display
            mcu_printf("ADC command : %x", HwMC_ADC->uADC_CMD.bReg.CMD);

            for(index=0; index<nof_sample; index++) {
                adc_data    = rd_data_32b(GDMA_TEST_MEM + (index<<2));
                adc_ch      = (adc_data >> 16);
                adc_data   &= 0xffff;
                ref_data    = ref_adc_data[adc_ch];

                if(ref_data != adc_data) {
                    mcu_printf("ADC data unmatched : ref = %x, adc = %x", ref_data, adc_data);
                }
                wr_data_32b((GDMA_TEST_MEM + (index<<2)), 0xffffffff);
            }
        }
        else if ( current_test == GDMA_TEST_TYPE_ICTC)
        {
            HwMC_ICTC0->uICTC_IRQ.nICTC_IRQ = 0xffUL;
            HwMC_ICTC0->uICTC_IRQ.nICTC_IRQ = 0x00UL;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR0 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR0 = 0U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR1 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR1 = 0U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR2 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR2 = 0U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR3 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR3 = 0U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR4 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR4 = 0U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR5 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR5 = 0U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR6 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR6 = 0U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR7 = 1U;
            //HwMC_ICTC0->uICTC_IRQ.bICTC_IRQ.ICLR7 = 0U;
        }
        dma_irq_check = 0;
    }

    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------
// Main Function
//----------------------------------------------------------------------------------------------------------------------------

extern void UartDmaRxTest(void);
sint32 GDMA_IPTest(uint32 test_case)
{
    sint32 ret;
    switch (test_case)
    {
        case 1:
            mcu_printf("GDMA TC 01 : GDMA_F0_ClockGatingAndSWResetTest \n");
            ret= GDMA_F0_ClockGatingAndSWResetTest();
            break;
        case 2:
            mcu_printf("GDMA TC 02 : GDMA_F1_IrqTest \n");
            ret= GDMA_F1_IrqTest();
            break;
        case 3:
            mcu_printf("GDMA TC 03 : GDMA_F3_AHBMasterTest \n");
            ret= GDMA_F3_AHBMasterTest();
            break;
        case 4:
            mcu_printf("GDMA TC 04: GDMA_F4_PriorityTest \n");
            ret= GDMA_F4_PriorityTest();
            break;
        case 5:
            mcu_printf("GDMA TC 05: GDMA_F5_ScatterOrGatherTest \n");
            ret= GDMA_F5_ScatterOrGatherTest();
            break;
        case 6:
            mcu_printf("GDMA TC 06: GDAM_F6_SingleBurstRequestTest \n");
            ret= GDAM_F6_SingleBurstRequestTest();
            break;
        case 7:
            mcu_printf("GDMA TC 07: GDMA_F7_AcceptableMemoryTest \n");
            ret= GDMA_F7_AcceptableMemoryTest();
            break;
        case 8:
            mcu_printf("GDMA TC 08: GDMA_F8_P2MIctcTest \n");
            ret= GDMA_F8_P2MIctcTest();
            break;
        case 9:
            mcu_printf("GDMA TC 09: GDMA_F9_P2M_AdcTest \n");
            ret= GDMA_F9_P2M_AdcTest();
            break;
        case 10:
            mcu_printf("GDMA TC 10: GDMA_F10_BusDefaultSlaveErrorTest \n");
            ret= GDMA_F10_BusDefaultSlaveErrorTest();
            break;
        case 11 :
            mcu_printf("GDMA TC 11: gdma_m2m_sample \n");
            GDMA_SampleForM2M();
            break;
#ifdef GDMA_IP_TEST_CODE
        case 12:
            UartDmaRxTest();
            break;
        case 13:
            UartDmaTxTest();
            break;
#endif
        default:
            break;
    }
       mcu_printf("GDMA_IPTest Test Case :  %d => result : %d \n", test_case, ret);
        return ret;
}

static int32 GDMA_F0_ClockGatingAndSWResetTest(void)
{
    int32 ret = 0;
    DMA_sPort *pDMA = (( DMA_sPort *)(HwMC_DMA));
    uint32 test_data;

    // clock gating test
    // test register set
    pDMA->DMA_CHANNEL[0].SRC_ADDR = 0xC1000000;
    test_data = pDMA->DMA_CHANNEL[0].SRC_ADDR;
    if ( test_data != 0xC1000000)
        dma_error = 0x00000001;

    // clock disable
       GDMA_IPTEST_ClockGate(0, 0);
    test_data = pDMA->DMA_CHANNEL[0].SRC_ADDR;
    if ( test_data == 0xC1000000)
        dma_error = 0x00000001;

    // clock enable
    GDMA_IPTEST_ClockGate(0, 1);
    test_data = pDMA->DMA_CHANNEL[0].SRC_ADDR;
    if ( test_data != 0xC1000000)
        dma_error = 0x00000001;

    // sw reset test
    // test register set
    pDMA->DMA_CHANNEL[0].SRC_ADDR = 0xC1000000;
    test_data = pDMA->DMA_CHANNEL[0].SRC_ADDR;
    if ( test_data != 0xC1000000)
        dma_error = 0x00000001;

    // dma sw reset
       GDMA_IPTEST_SWReset();

    // check reset value
    test_data = pDMA->DMA_CHANNEL[0].SRC_ADDR;
    if ( test_data == 0xC1000000)
        dma_error = 0x00000001;

    // re-setting and checking
    pDMA->DMA_CHANNEL[0].SRC_ADDR = 0xC1000000;
    test_data = pDMA->DMA_CHANNEL[0].SRC_ADDR;
    if ( test_data != 0xC1000000)
        dma_error = 0x00000001;

    if ( dma_error !=0)
        ret = -1;

    return ret;
}

#define DMA_MEM_SRC_LENGTH 32U
static int32 GDMA_F1_IrqTest(void)
{
    uint32 buf1[DMA_MEM_SRC_LENGTH];
    uint32* src = (uint32* )GDMA_TEST_MEM;
    uint32* dset = (uint32* )(GDMA_TEST_MEM + (DMA_MEM_SRC_LENGTH*(sizeof(int))));
    uint32 i;
    int32 result = 0;
    int32 ret = 0;
    uint32 test_ch = 0;
    uint32 test_control = 0;
    gContorlData.iCh = 0;
    gContorlData.iCon= 0;
    // alias to phy
    dset = (uint32* )((((uint32)&buf1[0]) & 0x00ffffff) );
    // ADC peripheral clock setting
    // - Source                = PLL0 fout
    // - Divider               = 1/6
    // - Peri. clock frequency = 1200MHz / 6 = 200MHz
    //peri_clk_cfg(MC_PCLK_ADC, MC_PCLK_PLL0_FOUT, (6-1));

    // prepare testing
    // bus reset for gdma
    // dma sw reset
       GDMA_IPTEST_SWReset();
    for (test_control = 0; test_control <DMA_CONTROL_MAX; test_control++)
    {
             // Initialize interrupt controller & Open interrupt handler
             //init_irq();
                gContorlData.iCon= test_control;

             (void)GIC_IntVectSet((uint32)(GIC_DMA0+test_control), (uint32)GIC_PRIORITY_NO_MEAN,
             (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&check_dma_irq, (void *)&gContorlData);
             (void)GIC_IntSrcEn((uint32)(GIC_DMA0+test_control));

             // M2M test
             for (test_ch = 0; test_ch <DMA_CH_MAX; test_ch++)
             {
                    gContorlData.iCh = test_ch;
                //fill src data & clear dest data
                for( i =0; i <DMA_MEM_SRC_LENGTH; i++)
                {
                    src[i] = i;
                    dset[i] = 0;
                }
                ret = GDMA_IPTEST_M2MTransfer(
                    (uint8)test_control,
                    (uint8)test_ch,
                    (uint8 *)&src[0],
                    (uint8 *)&dset[0],
                    DMA_MEM_SRC_LENGTH,
                    (uint32)dma_WIDTH_32_BIT,
                    (uint32)1U);
                if ( ret == 0)
                {
                    //verify mem copy
                    for( i =0; i <DMA_MEM_SRC_LENGTH; i++)
                    {
                        if (src[i] !=dset[i])
                        {
                            result = -1;
                            break;
                        }

                    }
                }
                else
                {
                    result = -1;
                }
             }
    }
    return result;
}

/*
1. Different transfer sizes per DMA
    DMA[0] : 4K
    DMA[max] : 3KB
2. Manipulate DMA Channel Enable time points
    Enable DMA[max] =>  DMA[0] sequentially
3. Check the Done Request sequence in DMA Interrupt
    DMA [0] => DMA[max] to confirm completion
*/
#define CH_HIGH_EN
#define CH_LOW_EN
static int32 GDMA_F4_PriorityTest(void)
{
    #define PRIORITY_MAX_TRANS_SIZE (1024)
    #define PRIORITY_LOW_TRANS_SIZE (800)
    //MC_CFG           * pMcCfg = ((MC_CFG           *)(HwMC_CFG));
    //DMA_sPort *pDMA = (( DMA_sPort        *)(HwMC_DMA));
    uint32 buf1[DMA_MEM_SRC_LENGTH];
    uint32* src = (uint32* )GDMA_TEST_MEM;
    uint32* dset = (uint32* )(GDMA_TEST_MEM + (DMA_MEM_SRC_LENGTH*(sizeof(int))));
    uint32 i;
    int32 result = 0;
    //int32 ret = 0;
    int32 test_ch = 0;
    int32 test_control = 0;
    // alias to phy
    dset = (uint32* )((((uint32)&buf1[0]) & 0x00ffffff) );
    // ADC peripheral clock setting
    // - Source                = PLL0 fout
    // - Divider               = 1/6
    // - Peri. clock frequency = 1200MHz / 6 = 200MHz
    //peri_clk_cfg(MC_PCLK_ADC, MC_PCLK_PLL0_FOUT, (6-1));

    // prepare testing
    // bus reset for gdma
    // dma sw reset
    GDMA_IPTEST_SWReset();
    gContorlData.iCon= 0;
    gContorlData.iCh = 0;

    // Initialize interrupt controller & Open interrupt handler
    //init_irq();
    current_test = GDMA_TEST_TYPE_PRIORITY;
    (void)GIC_IntVectSet((uint32)GIC_DMA0, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&check_dma_irq, &gContorlData);
    (void)GIC_IntSrcEn((uint32)GIC_DMA0);

    #ifdef CH_HIGH_EN
    // Different transfer sizes per DMA
    // Set DMA[0]
    test_ch = 0;
    gContorlData.iCon= 0;
    gContorlData.iCh = 0;

    src = (uint32* )GDMA_TEST_MEM;
    dset = (uint32* )(GDMA_TEST_MEM + (PRIORITY_MAX_TRANS_SIZE*4));
    //fill src data & clear dest data
    for( i =0; i <PRIORITY_MAX_TRANS_SIZE; i++)
    {
        src[i] = i;
        dset[i] = 0;
    }
    //PRIORITY_MAX_TRANS_SIZE x dma_WIDTH_32_BIT
        (void)GDMA_IPTEST_M2MTransfer(
            (uint8)test_control,
            (uint8)test_ch,
            (uint8 *)&src[0],
            (uint8 *)&dset[0],
            PRIORITY_MAX_TRANS_SIZE,
            (uint32)dma_WIDTH_32_BIT,
            (uint32)0U);

    #endif

    #ifdef CH_LOW_EN
    // Set DMA[max] //DMA_CH_MAX
    test_ch = (DMA_CH_MAX-1);
    src = (uint32* )(GDMA_TEST_MEM + (PRIORITY_MAX_TRANS_SIZE*4*2));
    dset = (uint32* )(GDMA_TEST_MEM + (PRIORITY_MAX_TRANS_SIZE*4*2)+ PRIORITY_LOW_TRANS_SIZE*4);

    for( i =0; i <PRIORITY_LOW_TRANS_SIZE; i++)
    {
        src[i] = i;
        dset[i] = 0;
    }
    //PRIORITY_LOW_TRANS_SIZE x dma_WIDTH_32_BIT
        (void)GDMA_IPTEST_M2MTransfer(
            (uint8)test_control,
            (uint8)test_ch,
            (uint8 *)&src[0],
            (uint8 *)&dset[0],
            PRIORITY_LOW_TRANS_SIZE,
            (uint32)dma_WIDTH_32_BIT,
            (uint32)0U);

    #endif

    // 2. Manipulate DMA Channel Enable time points
    // DMA[max] => DMA[mid] => DMA[0]
    test_ch = (DMA_CH_MAX -1);
    GDMA_IPTEST_Enable(test_ch, 1);
    test_ch = 0;
    GDMA_IPTEST_Enable(test_ch, 1);
//test_ch = (DMA_CH_MAX/2);
//  GDMA_IPTEST_Enable(test_ch, 1);
    //Check the Done Request sequence in DMA Interrupt
    while( irq_cnt !=2 )
    {
        BSP_NOP_DELAY();
    }

    if ( irq_cnt == 2)
    {
        if ( (gdma_irq_status[0] <  gdma_irq_status[1]) )
            result = 0;
        else
            result = -1;
    }
    else
    {
        result = -1;
    }
    return result;
}

//----------------------------------------------------------------------------------------------------------------------------
// Sub Function
//----------------------------------------------------------------------------------------------------------------------------
// Wait DMA intrrupt
static void GDMA_IPTEST_WaitDmaIrq(void)
{
#if 0
    dma_irq_check_cont_max =0;
    while(dma_irq_check_cont_max<=1000) {
    dma_irq_check_cont_max++;
        if(dma_irq_check == 0)
        {
            break;
        }
    }
#else
    while(1) {

        if(dma_irq_check == 0)
        {
            break;
        }
    }

#endif
}

// M2M Test
static void GDMA_IPTEST_Enable(uint8 ch, uint32 enable)
{
    DMA_sPort *pDMA = (( DMA_sPort        *)(HwMC_DMA));

    pDMA->DMA_CHANNEL[ch].CFG.nReg   = (pDMA->DMA_CHANNEL[ch].CFG.nReg | 0x1);
}

static int GDMA_IPTEST_M2MTransfer(uint8 control, uint8 ch, uint8 * src, uint8 * dest, uint32 lengh, uint32 trans_size, uint32 enable)
{
    DMA_sCH_CTRL    dma_ctrl;
    DMA_sCH_CFG     dma_cfg;
    //MC_CFG           * pMcCfg = ((MC_CFG           *)(HwMC_CFG));
    DMA_sPort *pDMA = (( DMA_sPort        *)(GDMA_BASE_ADDR + (GDMA_CONTROL_OFFSET*control)));
    int ret =0;

    // DMA burst request enable
    //pMcCfg->uDMA_REQ_CFG.bReg.ADC     = 0;

    // DMA controller enable
    // AHB master-1 endianness configuration : Little-endian mode
    // AHB master-2 endianness configuration : Little-endian mode
    pDMA->CONFIG.nReg = ((dma_LITTLE_ENDIAN<<2) | (dma_LITTLE_ENDIAN<<1) | (dma_ENABLE<<0));

    // DMA request synchronization enable
    //pDMA->SYNCH.nReg  = 0xffff;

    pDMA->DMA_CHANNEL[ch].SRC_ADDR   = (uint32)(src);
    pDMA->DMA_CHANNEL[ch].DEST_ADDR  = (uint32)dest;
    pDMA->DMA_CHANNEL[ch].LLI.nReg   = 0;

    dma_ctrl.TRANSFER_SIZE              = lengh;
    switch ( trans_size)
    {
        case dma_WIDTH_8_BIT:
            dma_ctrl.SRC_BURST_SIZE             = dma_BURST_1;
            dma_ctrl.DEST_BURST_SIZE            = dma_BURST_1;
            //dma_ctrl.SRC_BURST_SIZE             = dma_BURST_4;
            //dma_ctrl.DEST_BURST_SIZE            = dma_BURST_4;

            //dma_ctrl.SRC_BURST_SIZE             = dma_BURST_8;
            //dma_ctrl.DEST_BURST_SIZE            = dma_BURST_8;
            //dma_ctrl.SRC_BURST_SIZE             = dma_BURST_16;
            //dma_ctrl.DEST_BURST_SIZE            = dma_BURST_16;


            dma_ctrl.SRC_WIDTH                  = dma_WIDTH_8_BIT;
            dma_ctrl.DEST_WIDTH                 = dma_WIDTH_8_BIT;
            break;
        case dma_WIDTH_16_BIT:
            dma_ctrl.SRC_BURST_SIZE             = dma_BURST_1;
            dma_ctrl.DEST_BURST_SIZE            = dma_BURST_1;
            dma_ctrl.SRC_WIDTH                  = dma_WIDTH_16_BIT;
            dma_ctrl.DEST_WIDTH                 = dma_WIDTH_16_BIT;
            break;
        case dma_WIDTH_32_BIT:
            dma_ctrl.SRC_BURST_SIZE             = dma_BURST_1;
            dma_ctrl.DEST_BURST_SIZE            = dma_BURST_1;
            dma_ctrl.SRC_WIDTH                  = dma_WIDTH_32_BIT;
            dma_ctrl.DEST_WIDTH                 = dma_WIDTH_32_BIT;
            break;
        case dma_WIDTH_64_BIT:
            dma_ctrl.SRC_BURST_SIZE             = dma_BURST_1;
            dma_ctrl.DEST_BURST_SIZE            = dma_BURST_1;
            dma_ctrl.SRC_WIDTH                  = dma_WIDTH_64_BIT;
            dma_ctrl.DEST_WIDTH                 = dma_WIDTH_64_BIT;
            break;
        default:
            dma_ctrl.SRC_BURST_SIZE             = dma_BURST_1;
            dma_ctrl.DEST_BURST_SIZE            = dma_BURST_1;
            dma_ctrl.SRC_WIDTH                  = dma_WIDTH_32_BIT;
            dma_ctrl.DEST_WIDTH                 = dma_WIDTH_32_BIT;
            break;

    }
    dma_ctrl.SRC_BUS                    = dma_AHB_BUS_1;
    dma_ctrl.DEST_BUS                   = dma_AHB_BUS_1;
    dma_ctrl.SRC_INCREMENT              = dma_ENABLE;
    dma_ctrl.DEST_INCREMENT             = dma_ENABLE;
    dma_ctrl.PROTECTION                 = (dma_PROT_NON_CACHEABLE | dma_PROT_NON_BUFFERABLE | dma_PROT_USER);
    dma_ctrl.TC_INTERRUPT_ENABLE        = dma_ENABLE;

    if ( enable==1)
        dma_cfg.CHANNEL_ENABLED              = 1;
    else
        dma_cfg.CHANNEL_ENABLED              = 0;
    dma_cfg.SRC_PERIPHERAL              = 0;
    dma_cfg.DEST_PERIPHERAL             = 0;
    dma_cfg.FLOW_CONTROL                = dma_MEM_TO_MEM_DMA_CTRL;
    dma_cfg.ERROR_INTERRUPT_MASK        = dma_DISABLE;
    dma_cfg.TC_INTERRUPT_MASK           = dma_ENABLE;
    dma_cfg.BUS_LOCK                    = dma_DISABLE;
    dma_cfg.ACTIVE                      = 0;
    dma_cfg.HALT                        = 0;
    dma_cfg.reserved0                   = 0;

    pDMA->DMA_CHANNEL[ch].CTRL.bReg  = dma_ctrl;
    pDMA->DMA_CHANNEL[ch].CFG.bReg   = dma_cfg;
    if ( enable==1)
    {
        dma_irq_check = 1;
        GDMA_IPTEST_WaitDmaIrq();
        if ( dma_irq_check == 0)
            ret = 0;
        else
            ret = -1;
    }
    return ret;

}


// ADC test with DMA
static void GDMA_IPTEST_AdcConfig(uint8 ch)
{
    DMA_sCH_CTRL    dma_ctrl;
    DMA_sCH_CFG     dma_cfg;

    // DMA burst request enable
    HwMC_CFG->uDMA_REQ_CFG.bReg.ADC     = 0;

    // DMA controller enable
    // AHB master-1 endianness configuration : Little-endian mode
    // AHB master-2 endianness configuration : Little-endian mode
    HwMC_DMA->CONFIG.nReg = ((dma_LITTLE_ENDIAN<<2) | (dma_LITTLE_ENDIAN<<1) | (dma_ENABLE<<0));

    // DMA request synchronization enable
    //HwMC_DMA->SYNCH.nReg  = 0xffff;

    HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR   = (uint32)&HwMC_ADC->uADC_AIN_00;
    HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR  = (uint32)GDMA_TEST_MEM;
    HwMC_DMA->DMA_CHANNEL[ch].LLI.nReg   = 0;

    dma_ctrl.TRANSFER_SIZE              = 0;
    //dma_ctrl.SRC_BURST_SIZE             = dma_BURST_16;
    //dma_ctrl.DEST_BURST_SIZE            = dma_BURST_16;
    dma_ctrl.SRC_BURST_SIZE             = dma_BURST_32;
    dma_ctrl.DEST_BURST_SIZE            = dma_BURST_32;

    dma_ctrl.SRC_WIDTH                  = dma_WIDTH_32_BIT;
    dma_ctrl.DEST_WIDTH                 = dma_WIDTH_32_BIT;
    dma_ctrl.SRC_BUS                    = dma_AHB_BUS_1;
    dma_ctrl.DEST_BUS                   = dma_AHB_BUS_2;
    dma_ctrl.SRC_INCREMENT              = dma_ENABLE;
    dma_ctrl.DEST_INCREMENT             = dma_ENABLE;
    dma_ctrl.PROTECTION                 = (dma_PROT_NON_CACHEABLE | dma_PROT_NON_BUFFERABLE | dma_PROT_USER);
    dma_ctrl.TC_INTERRUPT_ENABLE        = dma_ENABLE;

    dma_cfg.CHANNEL_ENABLED              = 0;
#ifdef TCC8050_TEST
    dma_cfg.SRC_PERIPHERAL              = dma_REQUEST_INDEX_3;
#else
    dma_cfg.SRC_PERIPHERAL              = dma_REQUEST_INDEX_12;
#endif
    dma_cfg.DEST_PERIPHERAL             = 0;
    dma_cfg.FLOW_CONTROL                = dma_PERIPHERAL_TO_MEM_DMA_CTRL;
    dma_cfg.ERROR_INTERRUPT_MASK        = dma_DISABLE;
    dma_cfg.TC_INTERRUPT_MASK           = dma_ENABLE;
    dma_cfg.BUS_LOCK                    = dma_DISABLE;
    dma_cfg.ACTIVE                      = 0;
    dma_cfg.HALT                        = 0;
    dma_cfg.reserved0                   = 0;

    HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg  = dma_ctrl;
    HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg   = dma_cfg;
}

// ADC test with DMA
static void GDMA_IPTEST_AdcDmaTest(uint8 ch, uint32 cap_mode)
{
    int     i;
    uint32    index;

    // Set capture mode
    HwMC_ADC->uADC_TIME.nReg = ((cap_mode&0x1)<<20);

    //----------------------------------------------------------------------------------------------------------------------------
    // 1. Fixed sampling clock test
    //----------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------
    // ADC controller interrupt enable
    //--------------------------------------------------------------
    // - ADC clock divider = 4
    //   * Periclock         = 200MHz
    //   * Sampling clock    =  20MHz
    //   * ADC clock divider = (200MHz/20)/2 - 1 = 4
    // - Interrupt enable
    //--------------------------------------------------------------
    HwMC_ADC->uADC_CLK.bReg.DIV    = ((200/20)/2) - 1;
    HwMC_ADC->uADC_CLK.bReg.REQ_EN = 1;

    //--------------------------------------------------------------
    // 1.1. Sampling for each analog input
    //--------------------------------------------------------------
    for(index=0; index<16; index++) {
        dma_irq_check                                       = 1;
        HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR                   = (uint32)&HwMC_ADC->uADC_AIN_00;
        HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
        HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.TRANSFER_SIZE    = 1;
        HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED    = 1;
        HwMC_ADC->uADC_CMD.nReg = (1 << index);

        GDMA_IPTEST_WaitDmaIrq();
    }

    //--------------------------------------------------------------
    // 1.2. Sampling for all analog inputs
    //--------------------------------------------------------------
    dma_irq_check                                       = 1;
    HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR                   = (uint32)&HwMC_ADC->uADC_AIN_00;
    HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
    HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.TRANSFER_SIZE    = 16;
    //HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.SRC_BURST_SIZE    = dma_BURST_32;
    //HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.DEST_BURST_SIZE    = dma_BURST_32;
    HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED    = 1;
    HwMC_ADC->uADC_CMD.nReg                             = 0xffff;
    GDMA_IPTEST_WaitDmaIrq();

    //--------------------------------------------------------------
    // 1.3. Sampling for even analog inputs
    //--------------------------------------------------------------
    dma_irq_check                                       = 1;
    HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR                   = (uint32)&HwMC_ADC->uADC_AIN_00;
    HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
    HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.TRANSFER_SIZE    = 8;
    //pDMA->DMA_CHANNEL[ch].CTRL.bReg.SRC_BURST_SIZE    = dma_BURST_32;
    //pDMA->DMA_CHANNEL[ch].CTRL.bReg.DEST_BURST_SIZE    = dma_BURST_32;
    HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED    = 1;
    HwMC_ADC->uADC_CMD.nReg                             = 0x5555;
    GDMA_IPTEST_WaitDmaIrq();

    //--------------------------------------------------------------
    // 1.4. Sampling for odd analog inputs
    //--------------------------------------------------------------
    dma_irq_check                                       = 1;
    HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR                   = (uint32)&HwMC_ADC->uADC_AIN_00;
    HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
    HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.TRANSFER_SIZE    = 8;
    HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED    = 1;
    HwMC_ADC->uADC_CMD.nReg                             = 0xaaaa;
    GDMA_IPTEST_WaitDmaIrq();

    //----------------------------------------------------------------------------------------------------------------------------
    // 2. Variable sampling clock test
    //----------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------
    // ADC controller interrupt enable
    //--------------------------------------------------------------
    // - ADC clock divider = 4
    //   * Periclock                  = 200MHz
    //   * Sampling clock             =  10MHz, 5MHz, 2.5MHz
    //   * ADC clock divider(10.0MHz) = (200MHz/    10)/2 - 1 =  9
    //   * ADC clock divider( 5.0MHz) = (200MHz/     5)/2 - 1 = 19
    //   * ADC clock divider( 2.5MHz) = (200MHz/2.5Mhz)/2 - 1 = 39
    // - Interrupt enable
    //--------------------------------------------------------------
    for(i=2; i>=0; i--) {
        HwMC_ADC->uADC_CLK.bReg.DIV     = ((40 >> i)-1);

        //--------------------------------------------------------------
        // 2.1. Sampling for all analog inputs
        //--------------------------------------------------------------
        dma_irq_check                                       = 1;
        HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR                   = (uint32)&HwMC_ADC->uADC_AIN_00;
        HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
        HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.TRANSFER_SIZE    = 16;
        HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED    = 1;
        HwMC_ADC->uADC_CMD.nReg                             = 0xffff;
        GDMA_IPTEST_WaitDmaIrq();

        //--------------------------------------------------------------
        // 2.2. Sampling for even analog inputs
        //--------------------------------------------------------------
        dma_irq_check                                       = 1;
        HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR                   = (uint32)&HwMC_ADC->uADC_AIN_00;
        HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
        HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.TRANSFER_SIZE    = 8;
        HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED    = 1;
        HwMC_ADC->uADC_CMD.nReg                             = 0x5555;
        GDMA_IPTEST_WaitDmaIrq();

        //--------------------------------------------------------------
        // 2.3. Sampling for odd analog inputs
        //--------------------------------------------------------------
        dma_irq_check                                       = 1;
        HwMC_DMA->DMA_CHANNEL[ch].SRC_ADDR                   = (uint32)&HwMC_ADC->uADC_AIN_00;
        HwMC_DMA->DMA_CHANNEL[ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
        HwMC_DMA->DMA_CHANNEL[ch].CTRL.bReg.TRANSFER_SIZE    = 8;
        HwMC_DMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED    = 1;
        HwMC_ADC->uADC_CMD.nReg                             = 0xaaaa;
        GDMA_IPTEST_WaitDmaIrq();
    }
}

// ICTC test with DMA
static void GDMA_IPTEST_IctcConfig(uint8 dma_ch, uint8 ictc_ch)
{
    DMA_sCH_CTRL    dma_ctrl;
    DMA_sCH_CFG     dma_cfg;
    MC_CFG           * pMcCfg = ((MC_CFG           *)(HwMC_CFG));
    DMA_sPort *pDMA = (( DMA_sPort        *)(HwMC_DMA));
    MC_ICTC * pICTC  =  ((MC_ICTC          *)(MICOM_BASE_ADDR + 0x500000 + (ictc_ch*0x10000)));

    // DMA burst request enable
    //HwMC_CFG->uDMA_REQ_CFG.bReg.ICTC0     = 0;
    //HwMC_CFG->uDMA_REQ_CFG.bReg.ICTC1     = 0;
    //HwMC_CFG->uDMA_REQ_CFG.bReg.ICTC2     = 0;
    #ifdef TCC8050_TEST
    pMcCfg->uDMA_REQ_CFG.nReg = (pMcCfg->uDMA_REQ_CFG.nReg & ~((0x1<<2)|(0x1<<1)|(0x1<<0)));
    #else
    pMcCfg->uDMA_REQ_CFG.nReg = (pMcCfg->uDMA_REQ_CFG.nReg & ~((0x1<<9)|(0x1<<10)|(0x1<<11)));
    #endif
    // DMA controller enable
    // AHB master-1 endianness configuration : Little-endian mode
    // AHB master-2 endianness configuration : Little-endian mode
    pDMA->CONFIG.nReg = ((dma_LITTLE_ENDIAN<<2) | (dma_LITTLE_ENDIAN<<1) | (dma_ENABLE<<0));

    // DMA request synchronization enable
    //pDMA->SYNCH.nReg  = 0xffff;

    pDMA->DMA_CHANNEL[dma_ch].SRC_ADDR   = (uint32)&pICTC->uICTC_DT_CNT_PPVL.nICTC_DT_CNT_PPVL;
    pDMA->DMA_CHANNEL[dma_ch].DEST_ADDR  = (uint32)GDMA_TEST_MEM;
    pDMA->DMA_CHANNEL[dma_ch].LLI.nReg   = 0;

    dma_ctrl.TRANSFER_SIZE              = 0;
    //dma_ctrl.SRC_BURST_SIZE             = dma_BURST_1;
    //dma_ctrl.DEST_BURST_SIZE            = dma_BURST_1;

    dma_ctrl.SRC_BURST_SIZE             = dma_BURST_32;
    dma_ctrl.DEST_BURST_SIZE            = dma_BURST_32;
    dma_ctrl.SRC_WIDTH                  = dma_WIDTH_32_BIT;
    dma_ctrl.DEST_WIDTH                 = dma_WIDTH_32_BIT;
    dma_ctrl.SRC_BUS                    = dma_AHB_BUS_1;
    dma_ctrl.DEST_BUS                   = dma_AHB_BUS_2;
    dma_ctrl.SRC_INCREMENT              = dma_DISABLE;
    dma_ctrl.DEST_INCREMENT             = dma_ENABLE;
    dma_ctrl.PROTECTION                 = (dma_PROT_NON_CACHEABLE | dma_PROT_NON_BUFFERABLE | dma_PROT_USER);
    dma_ctrl.TC_INTERRUPT_ENABLE        = dma_ENABLE;

    dma_cfg.CHANNEL_ENABLED              = 0;
#ifdef TCC8050_TEST
    dma_cfg.SRC_PERIPHERAL              = (dma_REQUEST_INDEX_0 + ictc_ch);
#else
    dma_cfg.SRC_PERIPHERAL              = dma_REQUEST_INDEX_9;
#endif
    dma_cfg.DEST_PERIPHERAL             = 0;
    dma_cfg.FLOW_CONTROL                = dma_PERIPHERAL_TO_MEM_DMA_CTRL;
    dma_cfg.ERROR_INTERRUPT_MASK        = dma_DISABLE;
    dma_cfg.TC_INTERRUPT_MASK           = dma_ENABLE;
    dma_cfg.BUS_LOCK                    = dma_DISABLE;
    dma_cfg.ACTIVE                      = 0;
    dma_cfg.HALT                        = 0;
    dma_cfg.reserved0                   = 0;

    pDMA->DMA_CHANNEL[dma_ch].CTRL.bReg  = dma_ctrl;
    pDMA->DMA_CHANNEL[dma_ch].CFG.bReg   = dma_cfg;

}

static int32 GDMA_IPTEST_IctcDmaTest(uint8 dma_ch, uint8 ictc_ch)
{
    //uint32    index;
    MC_ICTC * pIctcReg;
    MC_CCU  * pClock;
    DMA_sPort *pDMA = (( DMA_sPort        *)(HwMC_DMA));

    volatile MC_ICTC_CTRL_U stIctcCtl;
    volatile MC_ICTC_EN_U stIctcEn;
    volatile MC_PCLK_CFG_U stPClkCtl;

    pClock = (MC_CCU  * )HwMC_CCU;
    pIctcReg = ((MC_ICTC          *)(MICOM_BASE_ADDR + 0x500000 + 0x10000*ictc_ch));
#ifdef TODO_CHECK
#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1)
    (void)GPIO_Config(GPIO_GPMA(20)/*SPEED_PWM_IPUT_GPIO*/, (GPIO_FUNC(10) | GPIO_INPUT|GPIO_INPUTBUF_EN));
#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1)
    (void)GPIO_Config(GPIO_GPMB(1)/*SPEED_PWM_IPUT_GPIO*/, (GPIO_FUNC(0) | GPIO_INPUT|GPIO_INPUTBUF_EN));
#elif (TCC_EVM_BD_VERSION == TCC803x_BD_VER_0_3)
    (void)GPIO_Config(GPIO_GPMA(20)/*SPEED_PWM_IPUT_GPIO*/, (GPIO_FUNC(10) | GPIO_INPUT|GPIO_INPUTBUF_EN));
#endif
#endif
    // Set Clock(target : src xin(24MHz), dividor out 12MHz)

    stPClkCtl.nReg = 0;
    stPClkCtl.bReg.CLK_SEL = 0x5; // xin direct
    stPClkCtl.bReg.CLK_DIV = 1;
    stPClkCtl.bReg.DIV_EN = 1;
    stPClkCtl.bReg.OUT_EN =1;

    if ( ictc_ch== 0)
    {
        pClock->uICTC0_PCLK_CFG.nReg = stPClkCtl.nReg;
    }
    else if ( ictc_ch== 1)
    {
        pClock->uICTC1_PCLK_CFG.nReg = stPClkCtl.nReg;
    }
    else if ( ictc_ch== 2)
    {
        pClock->uICTC2_PCLK_CFG.nReg = stPClkCtl.nReg;
    }

    // Set Ictc Ctl
    stIctcCtl.nICTC_CTRL = pIctcReg->uICTC_CTRL.nICTC_CTRL;

    stIctcCtl.bICTC_CTRL.CMP_ERR_SEL = 1;
    stIctcCtl.bICTC_CTRL.TCLK_SEL= 2;
    stIctcCtl.bICTC_CTRL.FLT_F_MODE = 3;
    stIctcCtl.bICTC_CTRL.FLT_R_MODE = 3;


    // ICTC DMA Set
    /*
0: Counted previous period value
1: Counted 1 cycle before the previous period value
2: Counted previous duty value
3: Counted 1 cycle before the previous duty value
4: Counted current falling edge count value
5: Counted previous falling edge count value
6: Counted rising edge timestamp count value
7: Counted falling edge timestamp count value
    */
    stIctcCtl.bICTC_CTRL.DMA_SEL= 3;
    stIctcCtl.bICTC_CTRL.DMA_EN = 1;

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1)
    stIctcCtl.bICTC_CTRL.F_IN_SEL = (0x86 + 0x14);/*MA20*/
#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1)
    stIctcCtl.bICTC_CTRL.F_IN_SEL = (164 + 1);/*MB01*/
#elif (TCC_EVM_BD_VERSION == TCC803x_BD_VER_0_3)
    stIctcCtl.bICTC_CTRL.F_IN_SEL = (0x99 + 0x14);/*MA20*/
#endif

    pIctcReg->uICTC_CTRL.nICTC_CTRL = stIctcCtl.nICTC_CTRL;

    // Set Timeout Value
    pIctcReg->uICTC_TO_VL.bICTC_TO_VL.TO_VL = 0x00ffffffUL;


    // Set Rising Edge Count
    pIctcReg->uICTC_REDGE_VL.bICTC_REDGE_VL.REDGE_VL = 0x000005ff;

    // Set Falling Edige Count
    pIctcReg->uICTC_FEDGE_VL.bICTC_FEDGE_VL.FEDGE_VL = 0x000005ff;

    // Filter Counter En
    stIctcEn.nICTC_EN = pIctcReg->uICTC_EN.nICTC_EN;

    stIctcEn.bICTC_EN.FLT_CNT_EN = 1;
    // Falling Edge Counter En
    stIctcEn.bICTC_EN.EDGE_CNT_EN = 1;
    //Period/Duty CMP Counter En
    stIctcEn.bICTC_EN.PD_CMP_CNT_EN= 1;
    // Timestamp Counter En
    stIctcEn.bICTC_EN.TS_CNT_EN = 1;

    // RND Value
    pIctcReg->uICTC_PRD_RND_VL.bICTC_PRD_RND_VL.PRD_RND_VL = 0xffffff0;
    pIctcReg->uICTC_DT_RND_VL.bICTC_DT_RND_VL.DT_RND_VL = 0xffffff0;

    // Set interrupt;

    // ICTC En Set
    stIctcEn.bICTC_EN.TCLK_EN = 1;
    //stIctcEn.bICTC_EN.ICTC_EN = 1;
    pIctcReg->uICTC_EN.nICTC_EN = stIctcEn.nICTC_EN;

    pDMA->DMA_CHANNEL[dma_ch].SRC_ADDR                   = (uint32)&pIctcReg->uICTC_DT_CNT_PPVL.nICTC_DT_CNT_PPVL;
    pDMA->DMA_CHANNEL[dma_ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
    pDMA->DMA_CHANNEL[dma_ch].CTRL.bReg.TRANSFER_SIZE    = 0xfffU;/*max*/
    pDMA->DMA_CHANNEL[dma_ch].CFG.bReg.CHANNEL_ENABLED    = 1U;

    pIctcReg->uICTC_EN.nICTC_EN = (pIctcReg->uICTC_EN.nICTC_EN|0x1<<31);
    dma_irq_check = 1;
    GDMA_IPTEST_WaitDmaIrq();
    return 0;
}


//----------------------------------------------------------------------------------------------------------------------------
// Main Function
//----------------------------------------------------------------------------------------------------------------------------
static int32 GDMA_F3_AHBMasterTest(void)
{
    uint32    ref_data;
    uint32 i =0;
    uint32    data;
    int32 test_control = 0;

    //MC_CFG           * pMcCfg = ((MC_CFG           *)(HwMC_CFG));
    (void)GIC_IntVectSet((uint32)GIC_DMA0, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&GDMA_IPTEST_IsrHandler, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DMA0);

    gdma_irq_done    = 0;

    tc_irq_sts  = 0;
    err_irq_sts = 0;

    tc_irq_cnt  = 0;
    err_irq_cnt = 0;

    //----------------------------------------------------------------------------------------------------------------------------
    // Interrupt controller initialization & DMA IRQ registeration
    //----------------------------------------------------------------------------------------------------------------------------
    HwMC_DEF_SLV->uIRQ_MASK.nReg                        = 0x00;             // Interrupt mask setting
    HwMC_DEF_SLV->uDEF_SLV_CFG.nReg                     = 0;                // Each default slave setting
    HwMC_DEF_SLV->uDEF_SLV_CFG.bReg.MAIN_AHBM_RESP_EN   = 0x1;              // - Main AHB BusMatrix error response enable
    HwMC_DEF_SLV->uDEF_SLV_CFG.bReg.UART_AHBM_RESP_EN   = 0x1;              // - UART AHB BusMatrix error response enable
    HwMC_DEF_SLV->uIRQ_ENABLE.nReg                      = 0x1;              // Interrupt enable setting
    // prepare testing
    // bus reset for gdma
    // dma sw reset
       GDMA_IPTEST_SWReset();
    //----------------------------------------------------------------------------------------------------------------------------
    // Reference read data generation
    //----------------------------------------------------------------------------------------------------------------------------
    #if 0
    for(i=0; i<32; i++) {
        wr_data_32b((   SRAM0_0_BASE + 0x4000 + (i<<2)), (  0 + i));
        wr_data_32b((   SRAM0_1_BASE + 0x4000 + (i<<2)), ( 32 + i));
        wr_data_32b((   SRAM0_2_BASE + 0x4000 + (i<<2)), ( 64 + i));
        wr_data_32b((   SRAM0_3_BASE + 0x4000 + (i<<2)), ( 96 + i));
        wr_data_32b((   SRAM1_BASE   + 0x4000 + (i<<2)), (128 + i));

        wr_data_32b((AP_SRAM0_0_BASE + 0x4000 + (i<<2)), ((  0 + i)<<16));
        wr_data_32b((AP_SRAM0_1_BASE + 0x4000 + (i<<2)), (( 32 + i)<<16));
        wr_data_32b((AP_SRAM0_2_BASE + 0x4000 + (i<<2)), (( 64 + i)<<16));
        wr_data_32b((AP_SRAM0_3_BASE + 0x4000 + (i<<2)), (( 96 + i)<<16));
        wr_data_32b((AP_SRAM1_BASE   + 0x4000 + (i<<2)), ((128 + i)<<16));
    }
    #endif
    //----------------------------------------------------------------------------------------------------------------------------
    // Global DMA configuration
    //----------------------------------------------------------------------------------------------------------------------------
    GDMA_IPTEST_DmaEnable(test_control);

    //----------------------------------------------------------------------------------------------------------------------------
    // Unacceptable slave access
    //----------------------------------------------------------------------------------------------------------------------------
#ifdef DEBUG_EN
    mcu_printf("Unacceptable slave access test");
#endif

    //                   ch,       src_addr, src_incr,              dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, AP_REMAP_BADDR,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // AP remap address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, GBUS_3D_BADDR ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // GBUS-3D
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, HSIOBUS_BADDR ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // HSIOBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, DDIBUS_BADDR  ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // DDIBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, MEMBUS_BADDR  ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // MEMBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, SMUBUS_BADDR  ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // SMUBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x6, VBUS_BADDR    ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // VBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x7, IOBUS_BADDR   ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // IOBUS

    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(err_irq_cnt < 8) {};

    //                   ch,       src_addr, src_incr,              dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, CBUSS_BADDR   ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // CBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, DAP_BADDR     ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // DAP
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, CMBUS_BADDR   ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // CMBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, GBUS_2D_BADDR ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // GBUS-2D
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, 0xc2000000    ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // Reserved micom memory address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, 0xe0000000    ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // OTPROM
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x6, CAN_DEF       ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // CAN                          reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x7, GPSB_DEF      ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // GPSB                         reserved address

    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(err_irq_cnt < 16) {};

    //                   ch,       src_addr, src_incr,              dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, UART_DEF      ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // UART                         reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, I2CM_DEF      ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // I2C master                   reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, PDM0          ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // PDM-0 address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, ICTC_DEF      ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // ICTC                         reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, DMA           ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // DMA slave address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, ADC_DEF       ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // ADC                          reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x6, MC_MBOX0_S    ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // Micom secure mail box-0
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x7, GIC           ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // GIC-400

    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(err_irq_cnt < 24) {};

    //                   ch,       src_addr, src_incr,              dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, AP_MBOX0_S    ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // AP secure mail box-0
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, NIC400_GPV    ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // NIC-400 GPV
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, MIPI          ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // Micom reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, DP            ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // Micom reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, PERI_DEF      ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // Micom reserved address
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, TZ_CFG        ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // TZ configuration

    for(i=0; i<6; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(err_irq_cnt < 30) {};

    // added sub-bus
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, PPU_BADDR     ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // CBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, STRBUS_BADDR  ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // DAP
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, HSMBUS_BADDR  ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // CMBUS
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, OIC_BADDR     ,      0x1, (SRAM0_BASE + 0x8000),      0x1,          1);    // GBUS-2D

    for(i=0; i<4; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(err_irq_cnt < 34) {};

    //----------------------------------------------------------------------------------------------------------------------------
    // Acceptable slave access
    //----------------------------------------------------------------------------------------------------------------------------
#ifdef DEBUG_EN
    mcu_printf("Acceptable slave access test");
#endif

    // CAN wrapper
    //                   ch,         src_addr, src_incr,              dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, CAN0            ,      0x0, (SRAM0_BASE + 0x8000),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, CAN1            ,      0x0, (SRAM0_BASE + 0x8004),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, CAN2            ,      0x0, (SRAM0_BASE + 0x8008),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, CAN_CFG         ,      0x0, (SRAM0_BASE + 0x800c),      0x0,         1);

    for(i=0; i<4; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 4) {};

    // GPSB wrapper
    //                   ch,         src_addr, src_incr,              dst_addr, dst_incr, trans_size
    #ifdef TCC8050_TEST
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, GPSB0 + 0x4     ,      0x0, (SRAM0_BASE + 0x8010),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, GPSB1 + 0x4     ,      0x0, (SRAM0_BASE + 0x8014),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, GPSB2 + 0x4     ,      0x0, (SRAM0_BASE + 0x8018),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, GPSB3 + 0x4     ,      0x0, (SRAM0_BASE + 0x801c),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, GPSB4 + 0x4     ,      0x0, (SRAM0_BASE + 0x8020),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, GPSB5 + 0x4     ,      0x0, (SRAM0_BASE + 0x8024),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x6, GPSB_CFG        ,      0x0, (SRAM0_BASE + 0x8028),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x7, GPSB_SM         ,      0x0, (SRAM0_BASE + 0x802c),      0x0,         1);
    #else
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, GPSB0 + 0x4     ,      0x0, (SRAM0_BASE + 0x8010),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, GPSB1 + 0x4     ,      0x0, (SRAM0_BASE + 0x8014),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, GPSB2 + 0x4     ,      0x0, (SRAM0_BASE + 0x8018),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, GPSB_CFG        ,      0x0, (SRAM0_BASE + 0x8028),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, GPSB_SM         ,      0x0, (SRAM0_BASE + 0x802c),      0x0,         1);
    #endif

    #ifdef TCC8050_TEST
    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 12) {};
    #else
    for(i=0; i<5; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 9) {};
    #endif
    // UART wrapper
    //                   ch,         src_addr, src_incr,              dst_addr, dst_incr, trans_size
    #ifdef TCC8050_TEST
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, UART0           ,      0x0, (SRAM0_BASE + 0x8030),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, UART1           ,      0x0, (SRAM0_BASE + 0x8034),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, UART2           ,      0x0, (SRAM0_BASE + 0x8038),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, UART3           ,      0x0, (SRAM0_BASE + 0x803c),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, UART4           ,      0x0, (SRAM0_BASE + 0x8040),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, UART5           ,      0x0, (SRAM0_BASE + 0x8044),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x6, UART_DMA0       ,      0x0, (SRAM0_BASE + 0x8048),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x7, UART_DMA1       ,      0x0, (SRAM0_BASE + 0x804c),      0x0,         1);
    #else
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, UART0           ,      0x0, (SRAM0_BASE + 0x8030),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, UART1           ,      0x0, (SRAM0_BASE + 0x8034),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, UART2           ,      0x0, (SRAM0_BASE + 0x8038),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, UART3           ,      0x0, (SRAM0_BASE + 0x803c),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, UART_DMA0       ,      0x0, (SRAM0_BASE + 0x8048),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, UART_DMA1       ,      0x0, (SRAM0_BASE + 0x804c),      0x0,         1);
    #endif

    #ifdef TCC8050_TEST
    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 20) {};
    #else
    for(i=0; i<6; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 15) {};
    #endif

    // I2C master wrapper
    //                   ch,         src_addr, src_incr,              dst_addr, dst_incr, trans_size
    #ifdef TCC8050_TEST
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, UART_DMA2       ,      0x0, (SRAM0_BASE + 0x8050),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, UART_DMA3       ,      0x0, (SRAM0_BASE + 0x8054),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, UART_DMA4       ,      0x0, (SRAM0_BASE + 0x8058),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, UART_DMA5       ,      0x0, (SRAM0_BASE + 0x805c),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, UART_CFG        ,      0x0, (SRAM0_BASE + 0x8060),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, I2CM0           ,      0x0, (SRAM0_BASE + 0x8064),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x6, I2CM1           ,      0x0, (SRAM0_BASE + 0x8068),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x7, I2CM2           ,      0x0, (SRAM0_BASE + 0x806c),      0x0,         1);
    #else
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, UART_DMA2       ,      0x0, (SRAM0_BASE + 0x8050),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, UART_DMA3       ,      0x0, (SRAM0_BASE + 0x8054),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, UART_CFG        ,      0x0, (SRAM0_BASE + 0x8060),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, I2CM0           ,      0x0, (SRAM0_BASE + 0x8064),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, I2CM1           ,      0x0, (SRAM0_BASE + 0x8068),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, I2CM2           ,      0x0, (SRAM0_BASE + 0x806c),      0x0,         1);
    #endif

    #ifdef TCC8050_TEST
    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 28) {};
    #else
    for(i=0; i<6; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 21) {};
    #endif

    // ICTC wrapper
    //                   ch,         src_addr, src_incr,              dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, I2CM_CFG        ,      0x0, (SRAM0_BASE + 0x8070),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, I2CM0_SM        ,      0x0, (SRAM0_BASE + 0x8074),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, I2CM1_SM        ,      0x0, (SRAM0_BASE + 0x8078),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, I2CM2_SM        ,      0x0, (SRAM0_BASE + 0x807c),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, ICTC0_BASE           ,      0x0, (SRAM0_BASE + 0x8080),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x5, ICTC1           ,      0x0, (SRAM0_BASE + 0x8084),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x6, ICTC2           ,      0x0, (SRAM0_BASE + 0x8088),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x7, ICTC0_SM        ,      0x0, (SRAM0_BASE + 0x808c),      0x0,         1);

    #ifdef TCC8050_TEST
    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 36) {};
    #else
    for(i=0; i<8; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 29) {};
    #endif

    // ADC wrapper
    //                   ch,         src_addr, src_incr,              dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, ICTC1_SM        ,      0x0, (SRAM0_BASE + 0x8090),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, ICTC2_SM        ,      0x0, (SRAM0_BASE + 0x8094),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, ADC_BASE             ,      0x0, (SRAM0_BASE + 0x8098),      0x0,         1);
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, ADC_SM          ,      0x0, (SRAM0_BASE + 0x809C),      0x0,         1);

    #ifdef TCC8050_TEST
    for(i=0; i<4; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 40) {};
    #else
    for(i=0; i<4; i++)
        GDMA_IPTEST_SetChannelEnable(test_control, i);

    while(tc_irq_cnt < 33) {};
    #endif
    for(i=0; i<40; i++) {
        ref_data    = ref_peri_data[i];
        data        = rd_data_32b(SRAM0_BASE + 0x8000 + (i<<2));

        if(ref_data != data) {
            mcu_printf ("%d : Ref. peri data(%x) != DMA rd data(%x)", i, ref_data, data);
        }
    }

    return 0;
}

// total transfer data size = TRANSFER_SIZE * dma_eWIDTH
static int32 GDMA_F5_ScatterOrGatherTest(void)
{
    #define GDMA_LLI_TEST_CH (0)
    #define OFFSET_OF_ALIAS_MEM (0x00000000U)
    #define LLI_MEM_SIZE (1024)
    #define LLI_0_SRC (GDMA_TEST_MEM_BSE)
    #define LLI_0_DST (LLI_0_SRC+LLI_MEM_SIZE)

    #define LLI_1_SRC (LLI_0_DST+LLI_MEM_SIZE)
    #define LLI_1_DST (LLI_1_SRC+LLI_MEM_SIZE)

    #define LLI_2_SRC (LLI_1_DST+LLI_MEM_SIZE)
    #define LLI_2_DST (LLI_2_SRC+LLI_MEM_SIZE)
    uint32* src ;
    uint32* dset ;
    int32 i;
    int32 ret;
    uint32 test_ch = 0;
    uint32 test_control = 0;

    //DMA_sPort *pDMA = (( DMA_sPort        *)(HwMC_DMA));

    // lli items
    dma_sLLI_FORMAT test_lli[3];

    gContorlData.iCh = 0;
    gContorlData.iCon= 0;

    GDMA_IPTEST_SWReset();

    for (test_control = 0; test_control <DMA_CONTROL_MAX; test_control++)
    {
            GDMA_IPTEST_DmaEnable(test_control);

            // prepare testing
            // set up lli items
            current_test = GDMA_TEST_TYPE_LLI;
        gContorlData.iCon= test_control;
             (void)GIC_IntVectSet((uint32)(GIC_DMA0 + test_control), (uint32)GIC_PRIORITY_NO_MEAN,
                                  (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&check_dma_irq, (void *)&gContorlData);
             (void)GIC_IntSrcEn((uint32)(GIC_DMA0+test_control));
             for (test_ch = 0; test_ch <DMA_CH_MAX; test_ch++)
             {
                    gContorlData.iCh = test_ch;
                // setup lli0
                GDMA_IPTEST_M2MLinkedListConfig(test_ch,  //ch
                                LLI_0_SRC, //src_adr
                                1,  //src incr
                                LLI_0_DST, //dest_addr
                                1, // dest_incr
                                LLI_MEM_SIZE/4, // trans_size
                                ((uint32)&test_lli[1] +OFFSET_OF_ALIAS_MEM), //next lli addr
                                ((dma_sLLI_FORMAT *)&test_lli[0] +OFFSET_OF_ALIAS_MEM)// this lli addr
                                );
                //fill src data & clear dest data
                src = (uint32* )LLI_0_SRC;
                dset = (uint32* )(LLI_0_DST);

                for( i =0; i <LLI_MEM_SIZE/4; i++)
                {
                    src[i] = 0x11111111;
                    dset[i] = 0;
                }


                // setup lli1
                GDMA_IPTEST_M2MLinkedListConfig(test_ch,  //ch
                                LLI_1_SRC, //src_adr
                                1,  //src incr
                                LLI_1_DST, //dest_addr
                                1, // dest_incr
                                LLI_MEM_SIZE/4, // trans_size
                                ((uint32)&test_lli[2] +OFFSET_OF_ALIAS_MEM), //next lli addr
                                ((dma_sLLI_FORMAT *)&test_lli[1] +OFFSET_OF_ALIAS_MEM) // this lli addr
                                );
                src = (uint32* )LLI_1_SRC;
                dset = (uint32* )(LLI_1_DST);

                for( i =0; i <LLI_MEM_SIZE/4; i++)
                {
                    src[i] = 0x22222222;
                    dset[i] = 0;
                }

                // setup lli2
                GDMA_IPTEST_M2MLinkedListConfig(test_ch,  //ch
                                LLI_2_SRC, //src_adr
                                1,  //src incr
                                LLI_2_DST, //dest_addr
                                1, // dest_incr
                                LLI_MEM_SIZE/4, // trans_size
                                0, //last. end of item
                                ((dma_sLLI_FORMAT *)&test_lli[2] +OFFSET_OF_ALIAS_MEM) // this lli addr
                                );
                src = (uint32* )LLI_2_SRC;
                dset = (uint32* )(LLI_2_DST);

                for( i =0; i <LLI_MEM_SIZE/4; i++)
                {
                    src[i] = 0x33333333;
                    dset[i] = 0;
                }

                GDMA_IPTEST_M2MLinkedListTest(test_control,
                                  test_ch, //dma ch
                                0, // src
                                0, // src incr
                                0, // dest
                                0, // dest incr
                                LLI_MEM_SIZE, // trans size
                                ((uint32)&test_lli[0]+OFFSET_OF_ALIAS_MEM) // first lli
                                );

                GDMA_IPTEST_SetChannelEnable(test_control, test_ch);
                dma_irq_check = 1;
                GDMA_IPTEST_WaitDmaIrq();
                // m2m check
                src = (uint32* )LLI_0_SRC;
                dset = (uint32* )(LLI_0_DST);
                ret= 0;
                for( i =0; i <LLI_MEM_SIZE/4; i++)
                {
                    if(src[i] != dset[i])
                    {
                        ret = -1;
                        break;
                    }
                }

                if ( ret == 0)
                {
                    src = (uint32* )LLI_1_SRC;
                    dset = (uint32* )(LLI_1_DST);

                    for( i =0; i <LLI_MEM_SIZE/4; i++)
                    {
                        if(src[i] != dset[i])
                        {
                            ret = -1;
                            break;
                        }
                    }
                }

                if ( ret == 0)
                {
                    src = (uint32* )LLI_2_SRC;
                    dset = (uint32* )(LLI_2_DST);

                    for( i =0; i <LLI_MEM_SIZE/4; i++)
                    {
                        if(src[i] != dset[i])
                        {
                            ret = -1;
                            break;
                        }
                    }
                }
                }
    }

    return ret;

}


static int32 GDAM_F6_SingleBurstRequestTest(void)
{
    uint32    i;
    //uint32    ref_data;
    //uint32    data;
    uint32    index;
    int32 test_ch = 0;
    int32 ret =0;
    DMA_sPort *pDMA = (( DMA_sPort        *)(HwMC_DMA));
    MC_ADC *pADC = ((MC_ADC           *)(HwMC_ADC));

    (void)GIC_IntVectSet((uint32)GIC_DMA0, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&check_dma_irq, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DMA0);
    // prepare testing
    // bus reset for gdma
    // dma sw reset
    GDMA_IPTEST_SWReset();

    //dma_REQUEST_INDEX_12

    //M2P test : ADC
    // prepare testing
    // bus reset for gdma
    current_test = GDMA_TEST_TYPE_SINGLE_BURST;
    // dma sw reset
    GDMA_IPTEST_SWReset();

    // DMA configuration
    test_ch = 0;
    GDMA_IPTEST_AdcConfig(test_ch);

    // ADC with DMA
    pADC->uADC_TIME.nReg = ((CAPTURE_MODE_FSM&0x1)<<20);

    //----------------------------------------------------------------------------------------------------------------------------
    // 1. Fixed sampling clock test
    //----------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------
    // ADC controller interrupt enable
    //--------------------------------------------------------------
    // - ADC clock divider = 4
    //   * Periclock         = 200MHz
    //   * Sampling clock    =  20MHz
    //   * ADC clock divider = (200MHz/20)/2 - 1 = 4
    // - Interrupt enable
    //--------------------------------------------------------------
    pADC->uADC_CLK.bReg.DIV    = ((200/20)/2) - 1;
    pADC->uADC_CLK.bReg.REQ_EN = 1;

    //--------------------------------------------------------------
    // 1.1. Sampling for each analog input
    //--------------------------------------------------------------
    for(index=0; index<16; index++)
    {
        uint32 dest_val;
        uint32 *dest_ptr;
        dma_irq_check                                       = 1;
        pDMA->DMA_CHANNEL[test_ch].SRC_ADDR                   = (uint32)&pADC->uADC_AIN_00;
        pDMA->DMA_CHANNEL[test_ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
        pDMA->DMA_CHANNEL[test_ch].CTRL.bReg.TRANSFER_SIZE    = 12;
        pDMA->DMA_CHANNEL[test_ch].CFG.bReg.CHANNEL_ENABLED    = 1;
        //pADC->uADC_CMD.nReg = (1 << index); // peripheral request
        // 1. SRC/DEST increment condition
        // 2. adc 12 ch conversion
        // single request
        // clear dest mem
        for ( i = 0; i < 12; i++)
        {
            dest_ptr = (uint32 *)(pDMA->DMA_CHANNEL[test_ch].DEST_ADDR);

            *dest_ptr = 0xabababab;
            dest_val = *(dest_ptr);
            #ifdef TCC8050_TEST
            pDMA->SW_SINGLE_REQ.bReg.REQ3 = 1; //SW request
            #else
            pDMA->SW_SINGLE_REQ.bReg.REQ12 = 1; //SW request
            #endif
            if (dest_val == *(dest_ptr))
            {
                ret = -1;
                break;
            }
        }
        GDMA_IPTEST_WaitDmaIrq();
    }

    // burst request
    for(index=0; index<16; index++)
    {
        //uint32 dest_val;
        uint32 *dest_ptr;

        dma_irq_check                                       = 1;
        pDMA->DMA_CHANNEL[test_ch].SRC_ADDR                   = (uint32)&pADC->uADC_AIN_00;
        pDMA->DMA_CHANNEL[test_ch].DEST_ADDR                  = (uint32)GDMA_TEST_MEM;
        pDMA->DMA_CHANNEL[test_ch].CTRL.bReg.TRANSFER_SIZE    = 12;
        pDMA->DMA_CHANNEL[test_ch].CFG.bReg.CHANNEL_ENABLED    = 1;
        //pADC->uADC_CMD.nReg = (1 << index); // peripheral request
        // 1. SRC/DEST increment condition
        // 2. adc 12 ch conversion
        // single request
        // clear dest mem
        dest_ptr = (uint32 *)(pDMA->DMA_CHANNEL[test_ch].DEST_ADDR);
        for ( i = 0; i < 12; i++)
        {
            *(dest_ptr+i) = 0xabababab;
        }
        // request burst request
        #ifdef TCC8050_TEST
        pDMA->SW_BURST_REQ.bReg.REQ3 = 1; //SW request
        #else
        pDMA->SW_BURST_REQ.bReg.REQ12 = 1; //SW request
        #endif

        GDMA_IPTEST_WaitDmaIrq();
        // compare the result
        for ( i = 0; i < 12; i++)
        {
            if (*(dest_ptr+i) == 0xabababab)
            {
                ret = -1;
                break;
            }
        }
    }

    return ret;
}

const uint32 gdma_non_accessible_dse_addr[] =
{
    0xA0030000, //CAN
    0xA00C0000, //ADC
    0x1B2D0000, //UART
    0xA0150000, //GPSB
    0xA0300000, //UART
    0xA0460000, //I2C M
    0xA0560000, //I2C V
    0xA06F0000, //ICTC
    0xA0730000, //PDM
    0xA0890000, //I2S
};

static void GDMA_IPTEST_DefaultSlaveErrorIsr (void)
{
    uint32    i;
    uint32    def_slv_irq_status;

    // Interrupt status check
    def_slv_irq_status = HwMC_DEF_SLV->uIRQ_STATUS.nReg;

    for(i=0; i<8; i++) {
        if ((def_slv_irq_status & (0x1<<i)) != 0) {
            switch (i) {
                case    0   : mcu_printf("Warning : Illegal access interrupt @ Peripheral AHB Interconnector's Main AHB BusMatrix");  break;
                case    1   : mcu_printf("Warning : Illegal access interrupt @ AP Mail-box Terminal AHB Multiplexer's AP side AHB Mux."  ); break;
                case    2   : mcu_printf("Warning : Illegal access interrupt @ System Peripheral AHB Multiplexer's System AHB Mux."   );  break;
                case    3   : mcu_printf("Warning : Illegal access interrupt @ UART AHB BusMatrix");  break;
                case    4   : mcu_printf("Warning : Illegal access interrupt @ PDM AHB Mux."      );  break;
                case    5   : mcu_printf("Warning : Illegal access interrupt @ MICOM Mail-box AHB Mux." );  break;
                case    6   : mcu_printf("Warning : Illegal access interrupt @ CAN AHB BusMatrix" );  break;
                default     : mcu_printf("Warning : Illegal access interrupt @ GPSB AHB BusMatrix");  break;
            }
        }
    }

    // Interrupt status clear
    HwMC_DEF_SLV->uIRQ_STATUS.nReg = def_slv_irq_status;
    dsei_gdma_irq_done = 1;
}

static void GDMA_IPTEST_WaitDefaultSlaveErrorIrqDone(void) {
    //gdma_irq_done = 0;

    while(1) {
        if(dsei_gdma_irq_done == 1)
            break;
    }
}


static int32 GDMA_F10_BusDefaultSlaveErrorTest(void)
{
    uint32 i;
    uint32 area = sizeof(gdma_non_accessible_dse_addr)/sizeof(gdma_non_accessible_dse_addr[0]);
    //int32 test_ch = 0;
    int32 test_control = 0;

    (void)GIC_IntVectSet((uint32)GIC_DEFAULT_SLV_ERR, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&GDMA_IPTEST_DefaultSlaveErrorIsr, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DEFAULT_SLV_ERR);
    GDMA_IPTEST_DmaEnable(test_control);

    //----------------------------------------------------------------------------------------------------------------------------
    // Default slave error interupt handler setting
    //----------------------------------------------------------------------------------------------------------------------------
    HwMC_DEF_SLV->uIRQ_MASK.nReg    = 0x00;                     // Interrupt mask setting
    HwMC_DEF_SLV->uIRQ_ENABLE.nReg  = 0x1;                      // Interrupt enable setting
    HwMC_DEF_SLV->uDEF_SLV_CFG.nReg = ((0xff<<16) | (0x3f));    // Each default slave setting

    for( i =0; i < area; i++)
    {
        uint32 src = gdma_non_accessible_dse_addr[i];
        dsei_gdma_irq_done = 0;
        GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, src, 0x1, (SRAM0_0_BASE + 0x4000),      0x1,         32);
        GDMA_IPTEST_SetChannelEnable(test_control, 0x0);
        GDMA_IPTEST_WaitDefaultSlaveErrorIrqDone();
        mcu_printf("Defalut Slave Error : GDMA to %x", src);
    }
    if ( i == area)
        return 0;
    else
        return -1;

}

static int32 GDMA_F7_AcceptableMemoryTest(void)
{
    uint32    i;//, j;
    uint32    ref_data;
    uint32    data;
    //uint32    index;
    //DMA_sPort *pDMA = (( DMA_sPort        *)(HwMC_DMA));
    //int32 test_ch = 0;
    int32 test_control = 0;



    (void)GIC_IntVectSet((uint32)GIC_DMA0, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&GDMA_IPTEST_IsrHandler, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DMA0);

    gdma_irq_done    = 0;

    tc_irq_sts  = 0;
    err_irq_sts = 0;

    tc_irq_cnt  = 0;
    err_irq_cnt = 0;

    // prepare testing
    // bus reset for gdma
    // dma sw reset
    GDMA_IPTEST_SWReset();

    //----------------------------------------------------------------------------------------------------------------------------
    // Interrupt controller initialization & DMA IRQ registeration
    //----------------------------------------------------------------------------------------------------------------------------
    HwMC_DEF_SLV->uIRQ_MASK.nReg                        = 0x00;             // Interrupt mask setting
    HwMC_DEF_SLV->uDEF_SLV_CFG.nReg                     = 0;                // Each default slave setting
    HwMC_DEF_SLV->uDEF_SLV_CFG.bReg.MAIN_AHBM_RESP_EN   = 0x1;              // - Main AHB BusMatrix error response enable
    HwMC_DEF_SLV->uDEF_SLV_CFG.bReg.UART_AHBM_RESP_EN   = 0x1;              // - UART AHB BusMatrix error response enable
    HwMC_DEF_SLV->uIRQ_ENABLE.nReg                      = 0x1;              // Interrupt enable setting

    //----------------------------------------------------------------------------------------------------------------------------
    // Reference read data generation
    //----------------------------------------------------------------------------------------------------------------------------
    for(i=0; i<32; i++) {
        wr_data_32b((   SRAM0_0_BASE + 0x4000 + (i<<2)), (  0 + i));
        wr_data_32b((   SRAM0_1_BASE + 0x4000 + (i<<2)), ( 32 + i));
        wr_data_32b((   SRAM0_2_BASE + 0x4000 + (i<<2)), ( 64 + i));
        wr_data_32b((   SRAM0_3_BASE + 0x4000 + (i<<2)), ( 96 + i));
        wr_data_32b((   SRAM1_BASE   + 0x4000 + (i<<2)), (128 + i));

        wr_data_32b((AP_SRAM0_0_BASE + 0x4000 + (i<<2)), ((  0 + i)<<16));
        wr_data_32b((AP_SRAM0_1_BASE + 0x4000 + (i<<2)), (( 32 + i)<<16));
        wr_data_32b((AP_SRAM0_2_BASE + 0x4000 + (i<<2)), (( 64 + i)<<16));
        wr_data_32b((AP_SRAM0_3_BASE + 0x4000 + (i<<2)), (( 96 + i)<<16));
        wr_data_32b((AP_SRAM1_BASE   + 0x4000 + (i<<2)), ((128 + i)<<16));
    }

    //----------------------------------------------------------------------------------------------------------------------------
    // Global DMA configuration
    //----------------------------------------------------------------------------------------------------------------------------
    GDMA_IPTEST_DmaEnable(test_control);

    //----------------------------------------------------------------------------------------------------------------------------
    // Memory to Memory for micom IMC
    //----------------------------------------------------------------------------------------------------------------------------
#ifdef DEBUG_EN
    mcu_printf("Micom memory to memory(IMC) test");
#endif
    // Clear write memory
    SAL_MemSet((void *)(SRAM0_BASE + 0x8000), 0x0, (32*5));
    //                   ch,                src_addr, src_incr,                            dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, (SRAM0_0_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x8000 + ((32*0)<<2)),      0x1,         32);
    gdma_irq_done = 0;
    GDMA_IPTEST_SetChannelEnable(test_control, 0x0);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                            dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, (SRAM0_1_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x8000 + ((32*1)<<2)),      0x1,         32);
    GDMA_IPTEST_SetChannelEnable(test_control, 0x1);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                            dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, (SRAM0_2_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x8000 + ((32*2)<<2)),      0x1,         32);
    GDMA_IPTEST_SetChannelEnable(test_control, 0x2);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                            dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, (SRAM0_3_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x8000 + ((32*3)<<2)),      0x1,         32);
    GDMA_IPTEST_SetChannelEnable(test_control, 0x3);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                            dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, (SRAM1_BASE   + 0x4000),      0x1, (SRAM0_BASE + 0x8000 + ((32*4)<<2)),      0x1,         32);
    GDMA_IPTEST_SetChannelEnable(test_control, 0x4);
    GDMA_IPTEST_WaitIrqDone();

    for(i=0; i<(32*5); i++) {
        ref_data    = i;
        data        = rd_data_32b(SRAM0_BASE + 0x8000 + (i<<2));

        if(ref_data != data) {
            mcu_printf ("%d : Ref. memory data(%x) != Read memory data(%x)", i, ref_data, data);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------
    // Memory to Memory for micom SFMC
    //----------------------------------------------------------------------------------------------------------------------------
#ifdef DEBUG_EN
    mcu_printf("Micom memory to memory(SFMC) test");
#endif

    // Clear write memory
    SAL_MemSet((void *)(SRAM0_BASE + 0x8000), 0x0, 8);

    //                   ch,   src_addr, src_incr,                            dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, 0xd0000000,      0x1, (SRAM0_BASE + 0x8000 + ((32*0)<<2)),      0x1,          8);
    gdma_irq_done = 0;
    GDMA_IPTEST_SetChannelEnable(test_control, 0x0);
    GDMA_IPTEST_WaitIrqDone();

    #ifdef ONLY_SIM
    for(i=0; i<8; i++) {
        ref_data    = 0xdeadc0de;
        data        = rd_data_32b(SRAM0_BASE + 0x8000 + (i<<2));

        if(ref_data != data) {
            mcu_printf ("%d : Ref. memory data(%x) != Read memory data(%x)", i, ref_data, data);
        }
    }
    #endif
#ifdef TOP_SIM
#ifdef ONLY_SIM
    //----------------------------------------------------------------------------------------------------------------------------
    // Memory to Memory for micom AP IMC
    //----------------------------------------------------------------------------------------------------------------------------
#ifdef DEBUG_EN
    mcu_printf("Micom memory to memory(AP IMC) test");
#endif
    // Clear write memory
    SAL_MemSet((void *)(SRAM0_BASE + 0x9000), 0x0, (32*5));

    //                   ch,                src_addr, src_incr,                               dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x0, (AP_SRAM0_0_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x9000 + ((32*0)<<2)),      0x1,         32);
    gdma_irq_done = 0;
    GDMA_IPTEST_SetChannelEnable(test_control, 0x0);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                               dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x1, (AP_SRAM0_1_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x9000 + ((32*1)<<2)),      0x1,         32);
    gdma_irq_done = 0;
    GDMA_IPTEST_SetChannelEnable(test_control, 0x1);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                               dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x2, (AP_SRAM0_2_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x9000 + ((32*2)<<2)),      0x1,         32);
    gdma_irq_done = 0;
    GDMA_IPTEST_SetChannelEnable(test_control, 0x2);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                               dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x3, (AP_SRAM0_3_BASE + 0x4000),      0x1, (SRAM0_BASE + 0x9000 + ((32*3)<<2)),      0x1,         32);
    gdma_irq_done = 0;
    GDMA_IPTEST_SetChannelEnable(test_control, 0x3);
    GDMA_IPTEST_WaitIrqDone();

    //                   ch,                src_addr, src_incr,                               dst_addr, dst_incr, trans_size
    GDMA_IPTEST_M2MNormalTransferTest   (test_control, 0x4, (AP_SRAM1_BASE   + 0x4000),      0x1, (SRAM0_BASE + 0x9000 + ((32*4)<<2)),      0x1,         32);
    gdma_irq_done = 0;
    GDMA_IPTEST_SetChannelEnable(test_control, 0x4);
    GDMA_IPTEST_WaitIrqDone();

    for(i=0; i<(32*5); i++) {
        ref_data    = (i<<16);
        data        = rd_data_32b(SRAM0_BASE + 0x9000 + (i<<2));

        if(ref_data != data) {
            mcu_printf ("%d : Ref. memory data(%x) != Read memory data(%x)", i, ref_data, data);
        }
    }
#endif
    //----------------------------------------------------------------------------------------------------------------------------
    // Memory to Memory for DRAM
    //----------------------------------------------------------------------------------------------------------------------------
    // Wait DRAM initialization(by A ommand)
    // Use virtual DRAM, TODO need to change real DRAM
    //    do {
    //        data = rd_data_32b(AP_INIT_DONE);
    //    } while (data != 0x80309090);

    mcu_printf ("AP command wait done");

#ifdef DEBUG_EN
    mcu_printf("Micom memory to memory(DRAM) test");
#endif

#endif
    return 0;
}

static int32 GDMA_F9_P2M_AdcTest(void)
{
    int32 test_ch = 0;

    // prepare testing
    (void)GIC_IntVectSet((uint32)GIC_DMA0, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&check_dma_irq, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DMA0);

    //M2P test : ADC
    // prepare testing
    // bus reset for gdma
    current_test = GDMA_TEST_TYPE_ADC;
    // dma sw reset
    GDMA_IPTEST_SWReset();

    // DMA configuration
    test_ch = 0;
    GDMA_IPTEST_AdcConfig(test_ch);

    // ADC with DMA
    GDMA_IPTEST_AdcDmaTest(test_ch, CAPTURE_MODE_FSM);
    GDMA_IPTEST_AdcDmaTest(test_ch, CAPTURE_MODE_EOC);

    return 0;
}

static int32 GDMA_F8_P2MIctcTest(void)
{
    int32 dma_test_ch = 0;
    int32 ictc_test_ch = 0;
    int32 ret;

    // prepare testing
    (void)GIC_IntVectSet((uint32)GIC_DMA0, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&check_dma_irq, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DMA0);

    //M2P test : ICTC
    // prepare testing
    // bus reset for gdma
    current_test = GDMA_TEST_TYPE_ICTC;
    // dma sw reset
    GDMA_IPTEST_SWReset();

    for ( ictc_test_ch = 0; ictc_test_ch < 3; ictc_test_ch++)
    {
        GDMA_IPTEST_IctcConfig(dma_test_ch, ictc_test_ch);
        ret = GDMA_IPTEST_IctcDmaTest(dma_test_ch, ictc_test_ch);
        if ( ret != 0)
            break;
    }
    // tested all ictc channel(3CH)
    if ( ictc_test_ch != 3)
        return -1;
    else
        return 0;
}

//----------------------------------------------------------------------------------------------------------------------------
// Sub Function
//----------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------
// DMA enable get/set/clr
//--------------------------------------------------------------
static void GDMA_IPTEST_DmaEnable(uint32 control)
{
    DMA_sPort *pDMA = (( DMA_sPort        *)(GDMA_BASE_ADDR + (GDMA_CONTROL_OFFSET*control)));
    pDMA->CONFIG.bReg.CTRL_ENABLE = 0x1;
}

//--------------------------------------------------------------
// DMA request synchronize enable get/set/clr
//--------------------------------------------------------------
static void GDMA_IPTEST_DmaSyncEnable(void)
{
    //HwMC_DMA->SYNCH.nReg = 0xff;
}

//--------------------------------------------------------------
// DMA channel address configuration
//--------------------------------------------------------------
static void GDMA_IPTEST_SetAddress   (    uint32 control,
            uint32    ch                  ,
                        uint32    src_addr            ,
                        uint32    dst_addr            ,
                        uint32    lli_addr            ) {
    DMA_sPort *pDMA = (( DMA_sPort        *)(GDMA_BASE_ADDR + (GDMA_CONTROL_OFFSET*control)));

#ifdef PARAM_CHECK_EN
    if (ch >= DMA_MAX_CHANNELS) {
        mcu_printf("Undefined DMA channel : %d", ch);
    }
#endif

    pDMA->DMA_CHANNEL[ch].SRC_ADDR  = src_addr;
    pDMA->DMA_CHANNEL[ch].DEST_ADDR = dst_addr;
    pDMA->DMA_CHANNEL[ch].LLI.nReg  = lli_addr;
}

//--------------------------------------------------------------
// DMA channel control configuration
//--------------------------------------------------------------
static void GDMA_IPTEST_SetControlRegister(    uint32 control,
                        uint32    ch                  ,
                        uint32    trans_size          ,
                        uint32    src_burst_size      ,
                        uint32    dst_burst_size      ,
                        uint32    src_width           ,
                        uint32    dst_width           ,
                        uint32    src_bus             ,
                        uint32    dst_bus             ,
                        uint32    src_incr            ,
                        uint32    dst_incr            ,
                        uint32    prot                ,
                        uint32    irq_en              )
{
    DMA_sPort *pDMA = (( DMA_sPort        *)(GDMA_BASE_ADDR + (GDMA_CONTROL_OFFSET*control)));

#ifdef PARAM_CHECK_EN
    if (ch              >= DMA_MAX_CHANNELS     ) { mcu_printf("Undefined DMA channel                 : %d", ch               ); }
    if (trans_size      >= 4096                 ) { mcu_printf("DMA transfer size exceeded the limit  : %d", trans_size       );  }
    if (src_burst_size  >  dma_BURST_128        ) { mcu_printf("Undefined DMA source      burst_size  : %d", src_burst_size   );  }
    if (dst_burst_size  >  dma_BURST_128        ) { mcu_printf("Undefined DMA destination burst_size  : %d", dst_burst_size   );  }
    if (src_width       >  dma_WIDTH_1024_BIT   ) { mcu_printf("Undefined DMA source      width       : %d", src_width        );  }
    if (dst_width       >  dma_WIDTH_1024_BIT   ) { mcu_printf("Undefined DMA destination width       : %d", dst_width        );  }
    if (src_bus         >= 2                    ) { mcu_printf("Undefined DMA source      bus         : %d", src_bus          );  }
    if (dst_bus         >= 2                    ) { mcu_printf("Undefined DMA destination bus         : %d", dst_bus          );  }
    if (src_incr        >= 2                    ) { mcu_printf("Undefined DMA source      increment   : %d", src_incr         );  }
    if (dst_incr        >= 2                    ) { mcu_printf("Undefined DMA destination increment   : %d", dst_incr         );  }
    if (prot            >= 8                    ) { mcu_printf("Undefined protection                  : %d", prot             );  }
    if (irq_en          >= 2                    ) { mcu_printf("Undefined interrupt enable            : %d", irq_en           );  }
#endif

    pDMA->DMA_CHANNEL[ch].CTRL.nReg = ( (irq_en         << 31) |
                                            (prot           << 28) |
                                            (dst_incr       << 27) |
                                            (src_incr       << 26) |
                                            (dst_bus        << 25) |
                                            (src_bus        << 24) |
                                            (dst_width      << 21) |
                                            (src_width      << 18) |
                                            (dst_burst_size << 15) |
                                            (src_burst_size << 12) |
                                            (trans_size     <<  0) );
}

//--------------------------------------------------------------
// DMA channel configuration
//--------------------------------------------------------------
void GDMA_IPTEST_SetConfigureRegister     ( uint32 control,
                        uint32    ch                  ,
                        uint32    src_peri            ,
                        uint32    dst_peri            ,
                        uint32    flow_ctrl           ,
                        uint32    err_irq_mask        ,
                        uint32    tc_irq_mask         ,
                        uint32    bus_lock            )
{
    DMA_sPort *pDMA = (( DMA_sPort        *)(GDMA_BASE_ADDR + (GDMA_CONTROL_OFFSET*control)));
#ifdef PARAM_CHECK_EN
    if (ch              >= DMA_MAX_CHANNELS                                 ) { mcu_printf("Undefined DMA channel                 : %d", ch           );  }
    if (src_peri        >= 16                                               ) { mcu_printf("Undefined DMA source      peripheral  : %d", src_peri     );  }
    if (dst_peri        >= 16                                               ) { mcu_printf("Undefined DMA destination peripheral  : %d", dst_peri     );  }
    if (flow_ctrl       >  dma_PERIPHERAL_TO_PERIPHERAL_SRC_PERIPHERAL_CTRL ) { mcu_printf("Undefined DMA flow control            : %d", flow_ctrl    );  }
    if (err_irq_mask    >= 2                                                ) { mcu_printf("Undefined DMA error interrupt mask    : %d", err_irq_mask );  }
    if (tc_irq_mask     >= 2                                                ) { mcu_printf("Undefined DMA counter interrupt mask  : %d", tc_irq_mask  );  }
    if (bus_lock        >= 2                                                ) { mcu_printf("Undefined DMA bus lock                : %d", bus_lock     );  }
#endif

    pDMA->DMA_CHANNEL[ch].CFG.nReg  = ( (bus_lock       << 16) |
                                            (tc_irq_mask    << 15) |
                                            (err_irq_mask   << 14) |
                                            (flow_ctrl      << 11) |
                                            (dst_peri       <<  6) |
                                            (src_peri       <<  1) );
}


static void GDMA_IPTEST_SetChannelEnable  (uint32 control, uint32 ch)
{
    DMA_sPort *pDMA;
#ifdef PARAM_CHECK_EN
    if (ch >= DMA_MAX_CHANNELS) {
        mcu_printf("Undefined DMA channel : %d", ch);
    }
#endif
    pDMA = (( DMA_sPort        *)(GDMA_BASE_ADDR + (GDMA_CONTROL_OFFSET*control)));
    pDMA->DMA_CHANNEL[ch].CFG.bReg.CHANNEL_ENABLED = 0x1;
}

//--------------------------------------------------------------
// Get/Clear DMA terminal counter interrupt ststua
//--------------------------------------------------------------
static uint32 GDMA_IPTEST_GetDmaTerminalCounterIrqStatus (void) {
    return HwMC_DMA->IRQ_ITC_STATUS.nReg;
}

static void GDMA_IPTEST_ClearDmaTerminalCounterIrqStatus (uint32 irq_ch) {
#ifdef PARAM_CHECK_EN
    if (irq_ch >= 0xff) {
        mcu_printf("Undefined DMA terminal counter IRQ channel : %x", irq_ch);
    }
#endif

    HwMC_DMA->IRQ_ITC_CLEAR.nReg  = irq_ch;
}

//--------------------------------------------------------------
// Get/Clear DMA error interrupt status
//--------------------------------------------------------------
static uint32 GDMA_IPTEST_GetDmaErrorIrqStatus (void) {
    return HwMC_DMA->IRQ_ERR_STATUS.nReg;
}

static void GDMA_IPTEST_ClearDmaErrorIrqStatus(uint32 irq_ch) {
#ifdef PARAM_CHECK_EN
    if (irq_ch >= 0xff) {
        mcu_printf("Undefined DMA error IRQ channel : %x", irq_ch);
    }
#endif

    HwMC_DMA->IRQ_ERR_CLEAR.nReg  = irq_ch;
}

//--------------------------------------------------------------
// DMA interrupt service routine
//--------------------------------------------------------------
static void GDMA_IPTEST_IsrHandler (void * param) {
    uint32    i;

    tc_irq_sts  = GDMA_IPTEST_GetDmaTerminalCounterIrqStatus();
    err_irq_sts = GDMA_IPTEST_GetDmaErrorIrqStatus();

    if (err_irq_sts != 0) {
        for (i=0; i<8; i++) {
            if (err_irq_sts & (0x1<<i)) {
#ifdef DEBUG_EN
                mcu_printf("DMA channel-%d error response received", i);
#endif
                GDMA_IPTEST_ClearDmaErrorIrqStatus(0x1<<i);
                err_irq_cnt++;
            }
        }
    }

    if (tc_irq_sts != 0) {
        for (i=0; i<8; i++) {
            if (tc_irq_sts & (0x1<<i)) {
#ifdef DEBUG_EN
                mcu_printf("DMA channel-%d transmission done", i);
#endif
                GDMA_IPTEST_ClearDmaTerminalCounterIrqStatus(0x1<<i);
                tc_irq_cnt++;
            }
        }
    }

    gdma_irq_done = 1;
}

//--------------------------------------------------------------
// Wait DMA IRQ done
//--------------------------------------------------------------
static void GDMA_IPTEST_WaitIrqDone(void) {
    //gdma_irq_done = 0;

    while(1) {
        if(gdma_irq_done == 1)
            break;
    }
}

//--------------------------------------------------------------
// DMA test function
//--------------------------------------------------------------

// DMA memory to memory test normal function
static void GDMA_IPTEST_M2MNormalTransferTest   (   uint32 control,
                uint32    ch          ,
                            uint32    src_addr    ,
                            uint32    src_incr    ,
                            uint32    dst_addr    ,
                            uint32    dst_incr    ,
                            uint32    trans_size  ) {

    GDMA_IPTEST_SetAddress (   control,
            ch                      ,   //  uint32    ch
                    src_addr                ,   //  uint32    src_addr
                    dst_addr                ,   //  uint32    dst_addr
                    0x00000000              );  //  uint32    lli_addr        : LLI disable

    GDMA_IPTEST_SetControlRegister (   control,
            ch                      ,   //  uint32    ch
                    trans_size              ,   //  uint32    trans_size
                    dma_BURST_4             ,   //  uint32    src_burst_size  : Burst-4
                    dma_BURST_4             ,   //  uint32    dst_burst_size  : Burst-4
                    dma_WIDTH_32_BIT        ,   //  uint32    src_width       : 32-bit
                    dma_WIDTH_32_BIT        ,   //  uint32    dst_width       : 32-bit
                    dma_AHB_BUS_1           ,   //  uint32    src_bus         : AHB-1
                    dma_AHB_BUS_2           ,   //  uint32    dst_bus         : AHB-2
                    src_incr                ,   //  uint32    src_incr
                    dst_incr                ,   //  uint32    dst_incr
                    0x3                     ,   //  uint32    prot            : Privileged mode, Bufferable, Cacheable,
                    0x1                     );  //  uint32    irq_en          : Interrupt enable

    GDMA_IPTEST_SetConfigureRegister  (   control,
            ch                      ,   //  uint32    ch
                    0x0                     ,   //  uint32    src_peri        : Ignored for meomry to memory
                    0x0                     ,   //  uint32    dst_peri        : Ignored for meomry to memory
                    dma_MEM_TO_MEM_DMA_CTRL ,   //  uint32    flow_ctrl       : Memory to memory
                    0x1                     ,   //  uint32    err_irq_mask    : Error          IRQ mask out
                    0x1                     ,   //  uint32    tc_irq_mask     : Terminal count IRQ mask out
                    0x0                     );  //  uint32    bus_lock        : Non-locked transfer
}

// DMA memory to memory test LLI function
/*

1. LLI seting
 - SRAM / DRAM 등에 LLI의 contents를 write 해놓습니다.
  예를 들어 0x2000_8000 번지부터 시작이라면
   0x2000_8000 : src addr
   0x2000_8004 : dst addr
   0x2000_8008 : next point (0x2000_8010)
   0x2000_800C : control

   0x2000_8010 : src addr
   0x2000_8014 : dst addr
   0x2000_8018 : next point (0x2000_8020)
   0x2000_801C : control

   0x2000_8020 : src addr
   0x2000_8024 : dst addr
   0x2000_8028 : next point (0x0000_0000) <- last linked list
   0x2000_802C : control
2. DMA를 하나 선택후 (0번 DMA라 가정) CnLLI 에 LLI의 맨 처음 address를 입력
  0x1B600108 : 0x2000_8000

3. DMA0의 CnConfiguration 레지스터를 통해 channel enable 하면 3번의 DMA 동작이 연속적으로 수행

4. 한번의 동작마다 interruprt를 띄우고 싶다면 "Terminal count interrupt enable bit"를 1로

*/
static void GDMA_IPTEST_M2MLinkedListTest   (   uint32 control,
                uint32    ch          ,
                            uint32    src_addr    ,
                            uint32    src_incr    ,
                            uint32    dst_addr    ,
                            uint32    dst_incr    ,
                            uint32    trans_size  ,
                            uint32    lli_addr    ) {

    GDMA_IPTEST_SetAddress (   control, // control
              ch                      ,   //  uint32    ch
                    src_addr                ,   //  uint32    src_addr
                    dst_addr                ,   //  uint32    dst_addr
                    lli_addr                );  //  uint32    lli_addr        : LLI disable

    GDMA_IPTEST_SetControlRegister (   control, // control
                    ch                      ,   //  uint32    ch
                    trans_size              ,   //  uint32    trans_size
                    dma_BURST_4             ,   //  uint32    src_burst_size  : Burst-4
                    dma_BURST_4             ,   //  uint32    dst_burst_size  : Burst-4
                    dma_WIDTH_32_BIT        ,   //  uint32    src_width       : 32-bit
                    dma_WIDTH_32_BIT        ,   //  uint32    dst_width       : 32-bit
                    dma_AHB_BUS_1           ,   //  uint32    src_bus         : AHB-1
                    dma_AHB_BUS_2           ,   //  uint32    dst_bus         : AHB-2
                    src_incr                ,   //  uint32    src_incr
                    dst_incr                ,   //  uint32    dst_incr
                    0x7                     ,   //  uint32    prot            : Privileged mode, Bufferable, Cacheable,
                    0x1                     );  //  uint32    irq_en          : Interrupt enable

    GDMA_IPTEST_SetConfigureRegister  (   control, // control
                    ch                      ,   //  uint32    ch
                    0x0                     ,   //  uint32    src_peri        : Ignored for meomry to memory
                    0x0                     ,   //  uint32    dst_peri        : Ignored for meomry to memory
                    dma_MEM_TO_MEM_DMA_CTRL ,   //  uint32    flow_ctrl       : Memory to memory
                    0x1                     ,   //  uint32    err_irq_mask    : Error          IRQ mask out
                    0x1                     ,   //  uint32    tc_irq_mask     : Terminal count IRQ mask out
                    0x0                     );  //  uint32    bus_lock        : Non-locked transfer
}

// DMA memory to memory test LLI function
static void GDMA_IPTEST_M2MLinkedListConfig(   uint32            ch          ,
                            uint32            src_addr    ,
                            uint32            src_incr    ,
                            uint32            dst_addr    ,
                            uint32            dst_incr    ,
                            uint32            trans_size  ,
                            uint32            lli_addr    ,
                            dma_sLLI_FORMAT *lli_cfg    ) {

    dma_sLLI_FORMAT *target_lli_cfg;
    target_lli_cfg = lli_cfg;

#ifdef PARAM_CHECK_EN
    if (ch              >= DMA_MAX_CHANNELS     ) { mcu_printf("Undefined DMA channel                 : %d", ch               );  }
    if (trans_size      >= 4096                 ) { mcu_printf("DMA transfer size exceeded the limit  : %d", trans_size       );  }
    if (src_incr        >= 2                    ) { mcu_printf("Undefined DMA source      increment   : %d", src_incr         );  }
    if (dst_incr        >= 2                    ) { mcu_printf("Undefined DMA destination increment   : %d", dst_incr         );  }
#endif

    target_lli_cfg->SRC_ADDR    = src_addr;
    target_lli_cfg->DEST_ADDR   = dst_addr;
    target_lli_cfg->LLI.nReg    = lli_addr;


    target_lli_cfg->CTRL.nReg   = ( (0x1                << 31) |    //  irq_en          : Interrupt enable
                                    (0x7                << 28) |    //  prot            : Privileged mode, Bufferable, Cacheable,
                                    (dst_incr           << 27) |    //  dst_incr
                                    (src_incr           << 26) |    //  src_incr
                                    (dma_AHB_BUS_2      << 25) |    //  dst_bus         : AHB-2
                                    (dma_AHB_BUS_1      << 24) |    //  src_bus         : AHB-1
                                    (dma_WIDTH_32_BIT   << 21) |    //  dst_width       : 32-bit
                                    (dma_WIDTH_32_BIT   << 18) |    //  src_width       : 32-bit
                                    (dma_BURST_4        << 15) |    //  dst_burst_size  : Burst-4
                                    (dma_BURST_4        << 12) |    //  src_burst_size  : Burst-4
                                    (trans_size         <<  0) );   //  trans_size
}
#endif

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )


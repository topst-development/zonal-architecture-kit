// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gpio_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )

#include <gpio_test.h>
#include <debug.h>
#include <gpio.h>
#include <gic.h>
#include <fmu.h>
#include <reg_phys.h>
#include <bsp.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

static uint32 nof_gpio[4] =         {   31,         /* orig:32, GPIO-A  bit-width */
                                        29,                  // GPIO-B  bit-width
                                        28,                  // GPIO-C  bit-width
                                        18};                 // GPIO-K  bit-width

static uint32 gpio_port[4] =        {   0,          /* orig:32, GPIO-A  bit-width */
                                        31,                  // GPIO-B  bit-width
                                        60,                  // GPIO-C  bit-width
                                        88};                 // GPIO-K  bit-width

static uint32 reset_bit[4] =        {   0x7FFFFFFFUL,           // GPIO-A
                                        0x1FFFFFFFUL,           // GPIO-B
                                        0x007FFFFFUL,           // GPIO-C
                                        0x0001FFFFUL};          // GPIO-K

static uint32 addr_gpio[4] =        {   0xA0F22000UL, /* orig:0xA0F22000, GPIO-A  base address */
                                        0xA0F22040UL,                  // GPIO-B  base address
                                        0xA0F22080UL,                  // GPIO-C  base address
                                        0xA0F220C0UL};                 // GPIO-K  base address

static uint32 user_mode_addr[27] =  {   0xA0F20000,     // MC_CFG                  0
                                        0xA0F21000,     // CR5_CFG                 1
                                        0xA0F22000,     // GPIO                    2
                                        0xA0F23000,     // WDT                     3
                                        0xA0F24000,     // CCU_PLL                 4
                                        0xA0F2401c,     // CCU_HSM                 5
                                        0xA0F24020,     // CCU                     6
                                        0xA0F25000,     // CMU                     7
                                        0xA0F26000,     // SYS_SM_CTRL             8
                                        0xA0F27000,     // DEF_SLV_IRQ             9
                                        0xA0F28000,     // PMU                    10
                                        0xA0F28018,     // PMU(HSM reset related) 11
                                        0xA0F28020,     // PMU(HSM only related)  12
                                        0xA0F28024,     // PMU others             13
                                        0xA0F28400,     // FMU                    14
                                        0xA0F28800,     // PMIO                   15
                                        0xA0F28C00,     // RTC                    16
                                        0xA0F2A000,     // TIMER0                 17
                                        0xA0F2A100,     // TIMER1                 18
                                        0xA0F2A200,     // TIMER2                 19
                                        0xA0F2A300,     // TIMER3                 20
                                        0xA0F2A400,     // TIMER4                 21
                                        0xA0F2A500,     // TIMER5                 22
                                        0xA0F2A600,     // TIMER6                 23
                                        0xA0F2A700,     // TIMER7                 24
                                        0xA0F2A800,     // TIMER8                 25
                                        0xA0F2A900,     // TIMER9                 26
                                    };

static uint32 reset_reg[224] =      {   0, 0x00000000UL,
                                        1, 0x00000000UL,
                                        5, 0x00000000UL,
                                        6, 0x00000000UL,
                                        7, 0x00000000UL,
                                        8, 0x00000000UL,
                                        9, 0x00000000UL,
                                        12, 0x00000000UL,
                                        13, 0x00000000UL,
                                        14, 0x00000000UL,
                                        15, 0x00000000UL,
                                        16, 0x00000000UL,
                                        17, 0x00000000UL,
                                        21, 0x00000000UL,
                                        22, 0x00000000UL,
                                        23, 0x00000000UL,
                                        24, 0x00000000UL,
                                        25, 0x00000000UL,
                                        28, 0x00000000UL,
                                        29, 0x00000000UL,
                                        30, 0x00000000UL,
                                        31, 0x00000000UL,
                                        32, 0x00000000UL,
                                        33, 0x00210000UL,
                                        37, 0x00000000UL,
                                        38, 0x00000000UL,
                                        39, 0x001E0000UL,
                                        40, 0x000E0000UL,
                                        41, 0x001E0000UL,
                                        44, 0x00000000UL,
                                        45, 0x00000000UL,
                                        46, 0x00000000UL,
                                        48, 0x00000000UL,
                                        49, 0x00000000UL,
                                        60, 0x00000000UL,
                                        61, 0x00000000UL,
                                        62, 0x00000000UL,
                                        172, 0x00000000UL,
                                        173, 0x00000000UL,
                                        174, 0x00000000UL,
                                        192, 0x00000000UL,
                                        193, 0x00000000UL,
                                        194, 0x00000000UL,
                                        195, 0x00000000UL,
                                        224, 0x7FFFFFFFUL,
                                        225, 0x1FFFFFFFUL,
                                        226, 0x007FFFFFUL,
                                        227, 0x0001FFFFUL,
                                        256, 0x00000000UL,
                                        257, 0x00000000UL,
                                        258, 0x00000000UL,
                                        259, 0x00000000UL,
                                        264, 0x00000000UL,
                                        265, 0x00000000UL,
                                        266, 0x00000000UL,
                                        267, 0x00000000UL,
                                        272, 0x00000000UL,
                                        273, 0x00000000UL,
                                        274, 0x00000000UL,
                                        275, 0x00000000UL,
                                        280, 0x00000000UL,
                                        281, 0x00000000UL,
                                        282, 0x00000000UL,
                                        283, 0x00000000UL,
                                        512, 0x00000000UL, //0x800
                                        513, 0x00000000UL,
                                        514, 0x00000000UL,
                                        515, 0x00000000UL,
                                        516, 0x00000000UL,
                                        517, 0x00000000UL,
                                        518, 0x00000000UL,
                                        519, 0x00000000UL,
                                        520, 0x00000000UL,
                                        521, 0x00000000UL,
                                        560, 0x00000000UL, //0x8C0
                                        561, 0x00000000UL,
                                        832, 0x00000000UL, //0xD00
                                        833, 0x00000000UL,
                                        834, 0x00000000UL,
                                        835, 0x00000000UL,
                                        837, 0x00000000UL,
                                        838, 0x00000000UL,
                                        839, 0x00000000UL,
                                        840, 0x00000000UL,
                                        841, 0x00000000UL,
                                        842, 0x00000000UL,
                                        843, 0x00000000UL,
                                        844, 0x00000000UL,
                                        845, 0x00000000UL,
                                        846, 0x00000000UL,
                                        847, 0x00000000UL,
                                        848, 0x00000000UL,
                                        849, 0x00000000UL, //0xD44
                                        896, 0x00000000UL, //0xE00
                                        897, 0x00000000UL,
                                        898, 0x00000000UL,
                                        899, 0x00000000UL,
                                        901, 0x00000000UL,
                                        902, 0x00000000UL,
                                        903, 0x00000000UL,
                                        904, 0x00000000UL,
                                        905, 0x00000000UL,
                                        906, 0x00000000UL,
                                        907, 0x00000000UL,
                                        908, 0x00000000UL,
                                        909, 0x00000000UL,
                                        910, 0x00000000UL,
                                        911, 0x00000000UL,
                                        912, 0x00000000UL,
                                        913, 0x00000000UL,
                                        960, 0x00070000UL,
                                        961, 0x00000000UL
                                    };

uint32 wr_reg[92*2] =               {   0, 0x7fffffff,    // 0x00_0         REG_GPA_DAT           000
                                        1, 0x7fffffff,    // 0x00_4         REG_GPA_OEN           001
                                        5, 0xffffffff,    // 0x01_4         REG_GPA_DS0           002
                                        6, 0x3fffffff,    // 0x01_8         REG_GPA_DS1           003
                                        7, 0x7fffffff,    // 0x01_c         REG_GPA_PE            004
                                        8, 0x7fffffff,    // 0x02_0         REG_GPA_PS            005
                                        9, 0x7fffffff,    // 0x02_4         REG_GPA_IEN           006

                                        12, 0xffffffff,    // 0x03_0         REG_GPA_FNC0          009
                                        13, 0xffffffff,    // 0x03_4         REG_GPA_FNC1          010
                                        14, 0xffffffff,    // 0x03_8         REG_GPA_FNC2          011
                                        15, 0x0fffffff,    // 0x03_c         REG_GPA_FNC3          012
                                        16, 0x1fffffff,    // 0x04_0         REG_GPB_DAT           013
                                        17, 0x1fffffff,    // 0x04_4         REG_GPB_OEN           014

                                        21, 0xffffffff,    // 0x05_4         REG_GPB_DS0           015
                                        22, 0x03ffffff,    // 0x05_8         REG_GPB_DS1           016
                                        23, 0x1fffffff,    // 0x05_c         REG_GPB_PE            017
                                        24, 0x1fffffff,    // 0x06_0         REG_GPB_PS            018
                                        25, 0x1fffffff,    // 0x06_4         REG_GPB_IEN           019

                                        28, 0xffffffff,    // 0x07_0         REG_GPB_FNC0          022
                                        29, 0xffffffff,    // 0x07_4         REG_GPB_FNC1          023
                                        30, 0xffffffff,    // 0x07_8         REG_GPB_FNC2          024
                                        31, 0x000fffff,    // 0x07_c         REG_GPB_FNC3          025
                                        32, 0x0fe1ffff,    // 0x08_0         REG_GPC_DAT           026
                                        33, 0x0fd0ffff,    // 0x08_4         REG_GPC_OEN           027

                                        37, 0xffffffff,    // 0x09_4         REG_GPC_DS0           028
                                        38, 0x00fffffc,    // 0x09_8         REG_GPC_DS1           029
                                        39, 0x0fdeffff,    // 0x09_c         REG_GPC_PE            030
                                        40, 0x0fceffff,    // 0x0a_0         REG_GPC_PS            031
                                        41, 0x0fdeffff,    // 0x0a_4         REG_GPC_IEN           032

                                        44, 0xffffffff,    // 0x0b_0         REG_GPC_FNC0          035
                                        45, 0xffffffff,    // 0x0b_4         REG_GPC_FNC1          036
                                        46, 0xff000000,    // 0x0b_8         REG_GPC_FNC2          037
                                        47, 0x0000ffff,    // 0x0b_c         REG_GPC_FNC3
                                        48, 0x0003ffff,    // 0x0c_0         REG_GPK_DAT           038
                                        49, 0x0003ffff,    // 0x0c_4         REG_GPK_OEN           039
                                        //53, 0xffffffff,    // 0x0d_4         REG_GPK_DS0
                                        //54, 0x000000ff,    // 0x0d_8         REG_GPK_DS1
                                        //55, 0x000fffff,    // 0x0d_c         REG_GPK_PE
                                        //56, 0x000fffff,    // 0x0e_0         REG_GPK_PS
                                        //57, 0x000fffff,    // 0x0e_4         REG_GPK_IEN
                                        //58, 0x000fffff,    // 0x0e_8         REG_GPK_IS
                                        //59, 0x000fffff,    // 0x0e_c         REG_GPK_SR
                                        60, 0xffffffff,    // 0x0f_0         REG_GPK_FNC0          040
                                        61, 0xffffffff,    // 0x0f_4         REG_GPK_FNC1          041
                                        62, 0x000000ff,    // 0x0f_8         REG_GPK_FNC2

                                        160, 0xffffffff,   // 0x28_0         REG_EINT_CFG0         042
                                        161, 0xffffffff,   // 0x28_4         REG_EINT_CFG1         043
                                        162, 0x0000ffff,   // 0x28_8         REG_EINT_CFG2         044

                                        172, 0xffffffff,   // 0x2B_0         REG_EXTCLK_CFG        045
                                        173, 0x00333333,   // 0x2B_4         REG_FLEXIO_CFG        046
                                        174, 0x0fffffff,   // 0x2B_8         REG_PERICH_CFG        047

                                        192, 0x7fffffff,   // 0x30_0         REG_GPA_FNC_LOCK      048
                                        193, 0x1fffffff,   // 0x30_4         REG_GPB_FNC_LOCK      049
                                        194, 0x003fffff,   // 0x30_8         REG_GPC_FNC_LOCK      050
                                        195, 0x0000ffff,   // 0x30_C         REG_GPK_FNC_LOCK      051

                                        224, 0x7fffffff,   // 0x38_0         REG_GPA_OEN_LOCKn     052
                                        225, 0x1fffffff,   // 0x38_4         REG_GPB_OEN_LOCKn     053
                                        226, 0x003fffff,   // 0x38_8         REG_GPC_OEN_LOCKn     054
                                        227, 0x0000ffff,   // 0x38_C         REG_GPK_OEN_LOCKn     055

                                        256, 0x7fffffff,   // 0x40_0         REG_GPA_VI2O_OEN      056
                                        257, 0x7fffffff,   // 0x40_4         REG_GPA_VI2I_DAT      057
                                        258, 0x7fffffff,   // 0x40_8         REG_GPA_VI2O_EN       058
                                        259, 0x7fffffff,   // 0x40_c         REG_GPA_VI2I_EN       059
                                        260, 0x7fffffff,   // 0x41_0         REG_GPA_MON_DO        060
                                        261, 0x7fffffff,   // 0x41_4         REG_GPA_MON_OEN       061

                                        264, 0x1fffffff,   // 0x42_0         REG_GPB_VI2O_OEN      062
                                        265, 0x1fffffff,   // 0x42_4         REG_GPB_VI2I_DAT      063
                                        266, 0x1fffffff,   // 0x42_8         REG_GPB_VI2O_EN       064
                                        267, 0x1fffffff,   // 0x42_c         REG_GPB_VI2I_EN       065
                                        268, 0x1fffffff,   // 0x43_0         REG_GPB_MON_DO        066
                                        269, 0x1fffffff,   // 0x43_4         REG_GPB_MON_OEN       067

                                        272, 0x003fffff,    // 0x44_0         REG_GPC_VI2O_OEN      068
                                        273, 0x003fffff,    // 0x44_4         REG_GPC_VI2I_DAT      069
                                        274, 0x003fffff,    // 0x44_8         REG_GPC_VI2O_EN       070
                                        275, 0x003fffff,    // 0x44_c         REG_GPC_VI2I_EN       071
                                        276, 0x003fffff,    // 0x45_0         REG_GPC_MON_DO        072
                                        277, 0x003fffff,    // 0x45_4         REG_GPC_MON_OEN       073

                                        280, 0x0001ffff,    // 0x46_0         REG_GPK_VI2O_OEN      074
                                        281, 0x0001ffff,    // 0x46_4         REG_GPK_VI2I_DAT      075
                                        282, 0x0001ffff,    // 0x46_8         REG_GPK_VI2O_EN       076
                                        283, 0x0001ffff,    // 0x46_c         REG_GPK_VI2I_EN       077
                                        284, 0x0001ffff,    // 0x47_0         REG_GPK_MON_DO        078
                                        285, 0x0001ffff,    // 0x47_4         REG_GPK_MON_OEN       079

                                        512, 0x07ffffff,    // 0x80_0         REG_EXT_IRQ0_FIL      080
                                        513, 0x07ffffff,    // 0x80_4         REG_EXT_IRQ1_FIL      081
                                        514, 0x07ffffff,    // 0x80_8         REG_EXT_IRQ2_FIL      082
                                        515, 0x07ffffff,    // 0x80_c         REG_EXT_IRQ3_FIL      083
                                        516, 0x07ffffff,    // 0x81_0         REG_EXT_IRQ4_FIL      084
                                        517, 0x07ffffff,    // 0x81_4         REG_EXT_IRQ5_FIL      085
                                        518, 0x07ffffff,    // 0x81_8         REG_EXT_IRQ6_FIL      086
                                        519, 0x07ffffff,    // 0x81_c         REG_EXT_IRQ7_FIL      087
                                        520, 0x07ffffff,    // 0x82_0         REG_EXT_IRQ8_FIL      088
                                        521, 0x07ffffff
                                    };   // 0x82_4         REG_EXT_IRQ9_FIL      089

static uint32 fault_index;

static uint32 gpio_irq_done;

static uint32 tpg_reg[GPIO_MAX_NUM] = { 0xA0F22400UL,       //GPIO_A
                                        0xA0F22420UL,       //GPIO_B
                                        0xA0F22440UL,       //GPIO_C
                                        0xA0F22460UL};      //GPIO_K

static uint32 din_reg[GPIO_MAX_NUM] = { 0xA0F22840UL,       //GPIO_A
                                        0xA0F22844UL,       //GPIO_B
                                        0xA0F22848UL,       //GPIO_C
                                        0xA0F2284CUL};      //GPIO_K

static uint32 wait_irq_p[10]            = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static uint32 wait_irq_n[10]            = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void delay1us
(
    uint32                              uiUs
) {
    uint32                              i;
    uint32                              sec;

    i = 0xffUL;
    sec = 0;

    sec = uiUs * (uint32)500UL;

    for (i = 0; i < sec; i++)
    {
        BSP_NOP_DELAY();
    }

    return;
}

static void GpioTestCommandList
(
    uint8                               ucArgc,
    const void * const                  pArgv[]
);


static void ext_irq_p0_isr
(
    void *                              arg
);

static void ext_irq_p1_isr
(
    void *                              arg
);

static void ext_irq_p2_isr
(
    void *                              arg
);

static void ext_irq_p3_isr
(
    void *                              arg
);

static void ext_irq_p4_isr
(
    void *                              arg
);

static void ext_irq_p5_isr
(
    void *                              arg
);

static void ext_irq_p6_isr
(
    void *                              arg
);

static void ext_irq_p7_isr
(
    void *                              arg
);

static void ext_irq_p8_isr
(
    void *                              arg
);

static void ext_irq_p9_isr
(
    void *                              arg
);

static void ext_irq_n0_isr
(
    void *                              arg
);

static void ext_irq_n1_isr
(
    void *                              arg
);

static void ext_irq_n2_isr
(
    void *                              arg
);

static void ext_irq_n3_isr
(
    void *                              arg
);

static void ext_irq_n4_isr
(
    void *                              arg
);

static void ext_irq_n5_isr
(
    void *                              arg
);

static void ext_irq_n6_isr
(
    void *                              arg
);

static void ext_irq_n7_isr
(
    void *                              arg
);

static void ext_irq_n8_isr
(
    void *                              arg
);

static void ext_irq_n9_isr
(
    void *                              arg
);

static void wait_irq_done
(
    void
);
static void gpio_fault_isr
(
   void *           arg
);

static FMUErrTypes_t open_irq_p
(
    uint32                              nth_irq,
    uint32                              port,
    uint32                              i
);

static FMUErrTypes_t open_irq_n
(
    uint32                              uiNnthIrq,
    uint32                              uiPort,
    uint32                              iIndex
);

static void close_irq_p
(
    uint32                              uiNnthIrq
);

static void close_irq_n
(
    uint32                              uiNnthIrq
);

static void ext_irq_filter_cfg
(
    uint32                              uiNth,
    uint32                              uiPolarity,
    uint32                              uiDepth
);

static void ext_irq_filter_en
(
    uint32                              uiNth
);

static void ext_irq_filter_clr
(
    uint32                              uiNth
);

static void ext_irq_sel_p
(
    uint32              eint_sel,
    uint32                              uiPort
);


static void ext_irq_sel_n
(
    uint32              eint_sel,
    uint32                              uiPort
);

/*
***************************************************************************************************
*                                         FUNCTIONS - SUB
***************************************************************************************************
*/

void wait_irq_done(void)
{
    while (1)
    {
        if (gpio_irq_done == 1UL)
        {
            break;
        }
    }
}

void gpio_fault_isr(void *arg)
{

    (void)arg;
    mcu_printf(" -> GPIO FMU ISR\n");
    gpio_irq_done = 1;
}

void ext_irq_p0_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P0");
    wait_irq_p[0] = 0;
}

void ext_irq_p1_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P1");
    wait_irq_p[1] = 0;
}

void ext_irq_p2_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P2");
    wait_irq_p[2] = 0;
}

void ext_irq_p3_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P3");
    wait_irq_p[3] = 0;
}

void ext_irq_p4_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P4");
    wait_irq_p[4] = 0;
}

void ext_irq_p5_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P5");
    wait_irq_p[5] = 0;
}

void ext_irq_p6_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P6");
    wait_irq_p[6] = 0;
}

void ext_irq_p7_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P7");
    wait_irq_p[7] = 0;
}

void ext_irq_p8_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P8");
    wait_irq_p[8] = 0;
}

void ext_irq_p9_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P9");
    wait_irq_p[9] = 0;
}

void ext_irq_n0_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N0");
    wait_irq_n[0] = 0;
}

void ext_irq_n1_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N1");
    wait_irq_n[1] = 0;
}

void ext_irq_n2_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N2");
    wait_irq_n[2] = 0;
}

void ext_irq_n3_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N3");
    wait_irq_n[3] = 0;
}

void ext_irq_n4_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N4");
    wait_irq_n[4] = 0;
}

void ext_irq_n5_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N5");
    wait_irq_n[5] = 0;
}

void ext_irq_n6_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N6");
    wait_irq_n[6] = 0;
}

void ext_irq_n7_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N7");
    wait_irq_n[7] = 0;
}

void ext_irq_n8_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N8");
    wait_irq_n[8] = 0;
}

void ext_irq_n9_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N9");
    wait_irq_n[9] = 0;
}

FMUErrTypes_t open_irq_p (uint32 nth_irq, uint32 port, uint32 i)
{

    FMUErrTypes_t ret;
    ret = FMU_OK;
#if 0
         if(nth_irq == 0) GIC_IntVectSet(INT_EXT0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p0_isr, NULL);
    else if(nth_irq == 1) GIC_IntVectSet(INT_EXT1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p1_isr, NULL);
    else if(nth_irq == 2) GIC_IntVectSet(INT_EXT2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p2_isr, NULL);
    else if(nth_irq == 3) GIC_IntVectSet(INT_EXT3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p3_isr, NULL);
    else if(nth_irq == 4) GIC_IntVectSet(INT_EXT4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p4_isr, NULL);
    else if(nth_irq == 5) GIC_IntVectSet(INT_EXT5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p5_isr, NULL);
    else if(nth_irq == 6) GIC_IntVectSet(INT_EXT6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p6_isr, NULL);
    else if(nth_irq == 7) GIC_IntVectSet(INT_EXT7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p7_isr, NULL);
    else if(nth_irq == 8) GIC_IntVectSet(INT_EXT8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p8_isr, NULL);
    else                  GIC_IntVectSet(INT_EXT9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p9_isr, NULL);
#endif
    if(nth_irq == 0UL)
    {
        (void)GIC_IntVectSet(GIC_EXT0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p0_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT0);
    }
    else if(nth_irq == 1UL)
    {
        (void)GIC_IntVectSet(GIC_EXT1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p1_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT1);
    }
    else if(nth_irq == 2UL)
    {
        (void)GIC_IntVectSet(GIC_EXT2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p2_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT2);
    }
    else if(nth_irq == 3UL)
    {
        (void)GIC_IntVectSet(GIC_EXT3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p3_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT3);
    }
    else if(nth_irq == 4UL)
    {
        (void)GIC_IntVectSet(GIC_EXT4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p4_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT4);
    }
    else if(nth_irq == 5UL)
    {
        (void)GIC_IntVectSet(GIC_EXT5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p5_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT5);
    }
    else if(nth_irq == 6UL)
    {
        (void)GIC_IntVectSet(GIC_EXT6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p6_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT6);
    }
    else if(nth_irq == 7UL)
    {
        (void)GIC_IntVectSet(GIC_EXT7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p7_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT7);
    }
    else if(nth_irq == 8UL)
    {
        (void)GIC_IntVectSet(GIC_EXT8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p8_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT8);
    }
    else
    {
        (void)GIC_IntVectSet(GIC_EXT9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p9_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT9);
    }
    return ret;

}

FMUErrTypes_t open_irq_n (uint32 uiNnthIrq,uint32 uiPort,uint32 iIndex)
{
    FMUErrTypes_t ret = FMU_OK;

#if 0
          if(uiNnthIrq == 0) GIC_IntVectSet(GIC_EXTn0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n0_isr, NULL);
     else if(uiNnthIrq == 1) GIC_IntVectSet(GIC_EXTn1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n1_isr, NULL);
     else if(uiNnthIrq == 2) GIC_IntVectSet(GIC_EXTn2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n2_isr, NULL);
     else if(uiNnthIrq == 3) GIC_IntVectSet(GIC_EXTn3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n3_isr, NULL);
     else if(uiNnthIrq == 4) GIC_IntVectSet(GIC_EXTn4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n4_isr, NULL);
     else if(uiNnthIrq == 5) GIC_IntVectSet(GIC_EXTn5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n5_isr, NULL);
     else if(uiNnthIrq == 6) GIC_IntVectSet(GIC_EXTn6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n6_isr, NULL);
     else if(uiNnthIrq == 7) GIC_IntVectSet(GIC_EXTn7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n7_isr, NULL);
     else if(uiNnthIrq == 8) GIC_IntVectSet(GIC_EXTn8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n8_isr, NULL);
     else                  GIC_IntVectSet(GIC_EXTn9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n9_isr, NULL);
#endif

    if(uiNnthIrq == 0UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n0_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn0);
    }
    else if(uiNnthIrq == 1UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n1_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn1);
    }
    else if(uiNnthIrq == 2UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n2_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn2);
    }
    else if(uiNnthIrq == 3UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n3_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn3);

    }
    else if(uiNnthIrq == 4UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n4_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn4);
    }
    else if(uiNnthIrq == 5UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n5_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn5);
    }
    else if(uiNnthIrq == 6UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n6_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn6);
    }
    else if(uiNnthIrq == 7UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n7_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn7);
    }
    else if(uiNnthIrq == 8UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n8_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn8);
    }
    else
    {
        (void)GIC_IntVectSet(GIC_EXTn9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n9_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn9);
    }

    //GPIO_WriteReg((addr_gpio[iIndex]),GPIO_ReadReg(addr_gpio[iIndex]) | (0x1<<uiPort));  // GPIO-MD TODO Confirm
    GPIO_WriteReg((addr_gpio[iIndex]),GPIO_ReadReg(addr_gpio[iIndex]) & ~((uint32)0x1<<uiPort));   // GPIO-MD TODO Confirm

    return ret;
}


void close_irq_p (uint32 uiNnthIrq)
{

    if(uiNnthIrq == 0UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT0 );
    }
    else if(uiNnthIrq == 1UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT1 );
    }
    else if(uiNnthIrq == 2UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT2 );
    }
    else if(uiNnthIrq == 3UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT3 );
    }
    else if(uiNnthIrq == 4UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT4 );
    }
    else if(uiNnthIrq == 5UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT5 );
    }
    else if(uiNnthIrq == 6UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT6 );
    }
    else if(uiNnthIrq == 7UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT7 );
    }
    else if(uiNnthIrq == 8UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT8 );
    }
    else
    {
        (void)GIC_IntSrcDis(GIC_EXT9 );
    }
}

void close_irq_n (uint32 uiNnthIrq)
{

    if(uiNnthIrq == 0UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn0);
    }
    else if(uiNnthIrq == 1UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn1);
    }
    else if(uiNnthIrq == 2UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn2);
    }
    else if(uiNnthIrq == 3UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn3);
    }
    else if(uiNnthIrq == 4UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn4);
    }
    else if(uiNnthIrq == 5UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn5);
    }
    else if(uiNnthIrq == 6UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn6);
    }
    else if(uiNnthIrq == 7UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn7);
    }
    else if(uiNnthIrq == 8UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn8);
    }
    else
    {
        (void)GIC_IntSrcDis(GIC_EXTn9);
    }
}




void ext_irq_filter_cfg (uint32 uiNth, uint32 uiPolarity, uint32 uiDepth)
{
    uint32 wr_addr;
    uint32 wr_data;

    wr_data = ((uiPolarity<<(uint32)26) | uiDepth);
    wr_addr = (MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2));
    GPIO_WriteReg(wr_addr, wr_data);
}

void ext_irq_filter_en (uint32 uiNth)
{
    uint32 wr_addr;
    uint32 rd_addr;
    uint32 rd_data;

    rd_addr = MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2);
    rd_data = GPIO_ReadReg(rd_addr);

    wr_addr = (MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2));

    GPIO_WriteReg(wr_addr, (rd_data | ((uint32)0x1<<(uint32)24)));
}

void ext_irq_filter_clr(uint32 uiNth)
{
    uint32 wr_addr;
    uint32 rd_addr;
    uint32 rd_data;

    rd_addr = MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2);
    rd_data = GPIO_ReadReg (rd_addr);

    wr_addr = (MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2));

    GPIO_WriteReg(wr_addr, (rd_data & (0xffffffffUL - ((uint32)0x1<<(uint32)24))));
    // GPIO_WriteReg((MCU_BSP_GPIO_BASE + 0x800 + (uiNth << 2)),0x0);
}


void ext_irq_sel_p(uint32 eint_sel, uint32 uiPort)
{
    uint32 sel0 =0;
//    uint32 sel1 =0;
//    uint32 sel2 =0;
    uint32 base_addr;

#if 1
    switch (eint_sel) {
    case 0 :
        sel0 |= (uiPort << (uint32)0);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 1 :
        sel0 |= (uiPort << (uint32)8);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 2 :
        sel0 |= (uiPort << (uint32)16);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 3 :
        sel0 |= (uiPort << (uint32)24);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 4 :
        sel0 |= (uiPort << (uint32)0);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 5 :
        sel0 |= (uiPort << (uint32)8);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 6 :
        sel0 |= (uiPort << (uint32)16);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 7 :
        sel0 |= (uiPort << (uint32)24);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 8 :
        sel0 |= (uiPort << (uint32)0);
        base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 9 :
        sel0 |= (uiPort << (uint32)8);
        base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    }
    GPIO_WriteReg(base_addr,sel0);
    if (uiPort < 31) {
        sel0 = 0;
        sel0 |= (uiPort << (uint32)0);
        base_addr = (MCU_BSP_GPIO_BASE + 0x2B0UL);
        GPIO_WriteReg(base_addr,sel0);
    } else if ((uiPort >= 31) || (uiPort < 60)) {
        sel0 = 0;
        sel0 |= (uiPort << (uint32)8);
        base_addr = (MCU_BSP_GPIO_BASE + 0x2B0UL);
        GPIO_WriteReg(base_addr,sel0);
    } else if ((uiPort >= 60) || (uiPort < 88)) {
        sel0 = 0;
        sel0 |= (uiPort << (uint32)16);
        base_addr = (MCU_BSP_GPIO_BASE + 0x2B0UL);
        GPIO_WriteReg(base_addr,sel0);
    } else {
        sel0 = 0;
        sel0 |= (uiPort << (uint32)24);
        base_addr = (MCU_BSP_GPIO_BASE + 0x2B0UL);
        GPIO_WriteReg(base_addr,sel0);
    }
#else
    sel0 = 0;
    sel0 |= (uiPort << (uint32)0);
    sel0 |= (uiPort << (uint32)8);
    sel0 |= (uiPort << (uint32)16);
    sel0 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x2B0UL);
    GPIO_WriteReg(base_addr,sel0);

    sel1 |= (uiPort << (uint32)0);
    sel1 |= (uiPort << (uint32)8);
    sel1 |= (uiPort << (uint32)16);
    sel1 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(base_addr,sel1);

    sel2 |= (uiPort << (uint32)0);
    sel2 |= (uiPort << (uint32)8);
    base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(base_addr,sel2);

    sel0 = 0;
    sel0 |= (uiPort << (uint32)0);
    sel0 |= (uiPort << (uint32)8);
    sel0 |= (uiPort << (uint32)16);
    sel0 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x2B0UL);
    GPIO_WriteReg(base_addr,sel0);
#endif
#if 0
    HwMC_GPIO->pEINTCFG.bReg.EINT0  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT1  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT2  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT3  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT4  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT5  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT6  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT7  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT8  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT9  = uiPort;
#endif
}

static void ext_irq_sel_n(uint32 eint_sel, uint32 uiPort)
{
    uint32 sel0 =0;
//    uint32 sel1 =0;
//    uint32 sel2 =0;
    uint32 base_addr;
#if 1
    switch (eint_sel) {
    case 0 :
        sel0 |= (uiPort << (uint32)0);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 1 :
        sel0 |= (uiPort << (uint32)8);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 2 :
        sel0 |= (uiPort << (uint32)16);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 3 :
        sel0 |= (uiPort << (uint32)24);
        base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 4 :
        sel0 |= (uiPort << (uint32)0);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    break;
    case 5 :
        sel0 |= (uiPort << (uint32)8);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 6 :
        sel0 |= (uiPort << (uint32)16);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 7 :
        sel0 |= (uiPort << (uint32)24);
        base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x288UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 8 :
        sel0 |= (uiPort << (uint32)0);
        base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    case 9 :
        sel0 |= (uiPort << (uint32)8);
        base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x284UL,0);
    GPIO_WriteReg(MCU_BSP_GPIO_BASE + 0x280UL,0);
    break;
    }
    GPIO_WriteReg(base_addr,sel0);
#else
    sel0 |= (uiPort << (uint32)0);
    sel0 |= (uiPort << (uint32)8);
    sel0 |= (uiPort << (uint32)16);
    sel0 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(base_addr,sel0);

    sel1 |= (uiPort << (uint32)0);
    sel1 |= (uiPort << (uint32)8);
    sel1 |= (uiPort << (uint32)16);
    sel1 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(base_addr,sel1);

    sel2 |= (uiPort << (uint32)0);
    sel2 |= (uiPort << (uint32)8);

    base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(base_addr,sel2);

    sel0 = 0;
    sel0 |= (uiPort << (uint32)0);
    sel0 |= (uiPort << (uint32)8);
    sel0 |= (uiPort << (uint32)16);
    sel0 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x2B0UL);
    GPIO_WriteReg(base_addr,sel0);
#endif
#if 0
    HwMC_GPIO->pEINTCFG.bReg.EINT0  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT1  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT2  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT3  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT4  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT5  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT6  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT7  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT8  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT9  = uiPort;
#endif
}



static void GPIO_InoutControl(void)
{
    uint32 i;

    //PMGPIO_FS
    GPIO_WriteReg(0xA0F28808, 0xFFFFFFFFUL);

    for(i=0 ; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));

        (void)GPIO_Set(GPIO_GPA(i), 1UL);
        (void)GPIO_Set(GPIO_GPB(i), 1UL);
        (void)GPIO_Set(GPIO_GPC(i), 1UL);
        (void)GPIO_Set(GPIO_GPK(i), 1UL);
    }
}



static void GPIO_controlatfunction0(void)
{
    uint32    i;
    uint32    check_data;
    uint32    base_addr;
    uint32    comp_addr;

    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }

    check_data = 0;
    for(i=0; i<(uint32)GPIO_MAX_NUM; i++)
    {
        comp_addr = addr_gpio[i] + GPIO_OFFSET_DAT;

        //GP_#_DAT
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("DAT write 0x00000000 fail!! [0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT write 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(base_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("DAT read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }


        //GP_#_SET
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("DAT write 0x00000000 fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT write 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_SET;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("SET write 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("SET write 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        //GP_#_CLR
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(base_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("DAT read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_CLR;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("CLR read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("CLR read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        //GP_#_XOR
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("DAT read 0x00000000 fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT read 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_XOR;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("XOR read 0x00000000 fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("XOR read 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_XOR;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("XOR read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("XOR read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_XOR;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("XOR read 0xffffffff fail!![0x%08X]\n\n",base_addr);
        }
        else
        {
            mcu_printf("XOR read 0xffffffff sucess!![0x%08X]\n\n",base_addr);
        }
    }
}


static void GPIO_lockfunctionmultiplexer(void)
 {

     uint32    base_addr;



     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x55555555UL) & 0xffffffffUL;
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x55555555UL) & 0xffffffffUL;
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));

     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x000fffffUL));

     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00ffffffUL));

     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x0000ffffUL));


     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x4UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x8UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0xCUL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);



     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));

     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x000fffffUL));

     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00ffffffUL));

     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x0000ffffUL));




     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x4UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x1fffffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x8UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x3fffffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0xCUL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x000fffff);



}


static void GPIO_Tpg(void)
{
    uint32    i;
    uint32    base_addr;

    for(i=0; i<(uint32)GPIO_MAX_NUM; i++)
    {
        base_addr = tpg_reg[i] + GPIO_VI2O_EN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_VI2O_OEN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_MON_OEN;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("Normal OEN Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("Normal OEN Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_VI2I_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr =tpg_reg[i] + GPIO_MON_DO;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("Normal DO Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("Normal DO Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        base_addr = tpg_reg[i] + GPIO_VI2O_EN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_VI2O_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr= tpg_reg[i] + GPIO_MON_OEN;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("TPG OEN Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("TPG OEN Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_VI2I_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_MON_DO;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("TPG DO Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("TPG DO Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }

        base_addr = tpg_reg[i] + GPIO_VI2O_EN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_VI2I_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_MON_DO;
        if(GPIO_ReadReg(base_addr) != 0x00000000UL)
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("Normal OEM / TPG Do Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("Normal OEM / TPG Do Sucess [%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        base_addr = tpg_reg[i] + GPIO_VI2I_EN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        if(GPIO_ReadReg(din_reg[i]) != reset_bit[i])
        {
            mcu_printf("TPG LoopBack Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, din_reg[i], GPIO_ReadReg(din_reg[i]));
        }
        else
        {
            mcu_printf("TPG LoopBack Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, din_reg[i], GPIO_ReadReg(din_reg[i]));
        }

    }
}


static void GPIO_SmWriteProtection(void)
{
    uint32    i;
    uint32    check_data;
    uint32    rd_data;
    uint32    base_addr;

    check_data = 0;
    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    if ((i < 18) && (i > 11)) {
    } else if ((i == 25) || (i == 26)){
    } else {
            (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }
    if ((i < 22) && (i > 15)) {
    } else {
            (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }
    if (i < 7) {
            (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }
    }

    for(i=0; i<(uint32)GPIO_WR_REG_MAX; i++)
    {
        fault_index = reset_reg[i<<1UL];

    //if((fault_index < (uint32)192) || (fault_index > (uint32)240))
    if(fault_index < 15)
        {        // *_FNC_LOCK, *_OEN_LOCKn skip
            base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL);

      /*  if (fault_index == 33) {
                GPIO_WriteReg(base_addr, 0x21ffffUL);
        } else if (fault_index == 32) {
                GPIO_WriteReg(base_addr, 0x10ffffUL);
        } else if (fault_index == 38) {
                GPIO_WriteReg(base_addr, 0x3UL);
        } else if (fault_index == 39) {
                GPIO_WriteReg(base_addr, 0x1EFFFFUL);
        } else if (fault_index == 40) {
                GPIO_WriteReg(base_addr, 0x1FFFFUL);
        } else if (fault_index == 41) {
                GPIO_WriteReg(base_addr, 0x1EFFFFUL);
        } else if (fault_index == 44) {
                GPIO_WriteReg(base_addr, 0xFUL);
        } else {*/
                GPIO_WriteReg(base_addr, 0xffffffffUL);
                check_data |= (GPIO_ReadReg(base_addr) ^ reset_reg[(i << 1UL) + 1UL]);
       // }

            if(check_data != 0UL)
            {
                check_data = 0;
            }
        }
    }

    //CFG_WR_PW
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_PW;
    GPIO_WriteReg(base_addr, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_LOCK;
    GPIO_WriteReg(base_addr, 0x00000001UL);

    for(i=0; i<(uint32)GPIO_WR_REG_MAX ;i++)
    {
        fault_index = reset_reg[i<<1UL];

        base_addr = MCU_BSP_GPIO_BASE + (fault_index<< 2UL);
        rd_data = GPIO_ReadReg(base_addr);
        GPIO_WriteReg(base_addr, 0x55555555UL);

        if(GPIO_ReadReg(base_addr) != rd_data)
        {
            mcu_printf("CFG_WR_LOCK fail   [%d]ori[0x%08X]write[0x%08X]\n",
                    fault_index,
                    rd_data,
                    GPIO_ReadReg(base_addr));
        }
        else
        {
            mcu_printf("CFG_WR_LOCK sucess [%d]ori[0x%08X]write[0x%08X]\n",
                    fault_index,
                    rd_data,
                    GPIO_ReadReg(base_addr));
        }
    }

    //CFG_WR_PW
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_PW;
    GPIO_WriteReg(base_addr, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_LOCK;
    GPIO_WriteReg(base_addr, 0x00000000UL);

    for(i=0; i<(uint32)GPIO_WR_REG_MAX ;i++)
    {
        fault_index = reset_reg[i<<1UL];
        if ((fault_index != 30) &&
        (fault_index != 31) &&
        (fault_index < 560)) {
            base_addr = MCU_BSP_GPIO_BASE + (fault_index<< 2UL);
            rd_data = GPIO_ReadReg(base_addr);
            if (fault_index == 174)
                GPIO_WriteReg(base_addr, 0x55555550UL);
            else
                GPIO_WriteReg(base_addr, 0x55555555UL);

            if(GPIO_ReadReg(base_addr) != rd_data)
            {
                mcu_printf("CFG_WR_Release sucess [%d]ori[0x%08X]write[0x%08X]\n",
                        fault_index,
                        rd_data,
                        GPIO_ReadReg(base_addr));
            }
            else
            {
                mcu_printf("CFG_WR_Release fail   [%d]ori[0x%08X]write[0x%08X]\n",
                        fault_index,
                        rd_data,
                        GPIO_ReadReg(base_addr));
            }
        }
    }
}

#define DEPTH 0x0UL
#define TOGGLE_DLY 1
static void GPIO_SmExtglitchintFilter(void)
{
    uint32 i;
    uint32 nth_irq;
    uint32 port;
    uint32 base_addr;
    uint32 uiK1;
    uint32 uiK2;
    const uint32  uiDelaycnt = (5000000UL);

    mcu_printf("\n glitch test DO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    // Setup PAD mux
    /*    base_addr =  (0x1B936000 + 0x148);
          GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-A
          base_addr = (0x1B936000 + 0x14c);
          GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-B
          base_addr= (0x1B936000 + 0x150);
          GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-C
          base_addr = (0x1B936000 + 0x154);
          GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-E
          base_addr = (0x1B936000 + 0x158);
          GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-G
    //GPIO_WriteReg((0x1B936000 + 0x15c), 0x00000000UL);    // GPIO-H
    //GPIO_WriteReg((0x1B936000 + 0x160), 0x00000000UL);    // GPIO-MA
    base_addr = (0x1B936000 + 0x164);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-MB TODO Confirm
    base_addr = (0x1B936000 + 0x168);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-MC TODO Confirm
    base_addr = (0x1B936000 + 0x16c);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-MD TODO Confirm
    GPIO_WriteReg((0x1B935044UL), 0xffffffffUL);  // GPIO A Input
    GPIO_WriteReg((0x1B935040UL), 0xffffffffUL);  // GPIO A Input*/
    //GPIO_WriteReg((0x1B935000), 0x00000000UL);    // GPIO A Input
#if 1
    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
//      if (i == 14) {
//      } else {
            (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
//      }
        if ((i < 18) && (i > 11)) {
        } else if ((i == 25) || (i == 26)){
        } else {
            (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
        }
        if ((i < 22) && (i > 15)) {
        } else {
            (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
        }

        (void)GPIO_Set(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
//      if (i == 14) {
//      } else {
            (void)GPIO_Set(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
//      }
        if ((i < 18) && (i > 11)) {
        } else if ((i == 25) && (i == 26)){
        } else {
            (void)GPIO_Set(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
        }
        if ((i < 22) && (i > 15)) {
        } else {
            (void)GPIO_Set(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_INPUTBUF_EN | GPIO_PULLUP));
        }
    }
#else
    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        if ((i < 18) && (i > 11)) {
        } else if ((i == 25) || (i == 26)){
        } else {
            (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        }
        if ((i < 22) && (i > 15)) {
        } else {
            (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        }

        (void)GPIO_Set(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Set(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        if ((i < 18) && (i > 11)) {
        } else if ((i == 25) && (i == 26)){
        } else {
            (void)GPIO_Set(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        }
        if ((i < 22) && (i > 15)) {
        } else {
            (void)GPIO_Set(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        }
    }
#endif

    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS + 4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS + 8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS + 12UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr= MCU_BSP_GPIO_BASE + GPIO_OFFSET_ECS;
    GPIO_WriteReg(base_addr, 0xffffffffUL);

    for(nth_irq=0; nth_irq<(uint32)GPIO_MAX_NTH_IRQ; nth_irq++)
    {
        for(i =0;i <(uint32)4; i++)
        {
            for(port=0; port<nof_gpio[i]; port++)
            {
                if ((i == 1) &&
                    (((port > 11) &&(port < 18)) ||
                    ((port == 25) || (port == 26)))) {
                } else if ((i == 2) &&
                    ((port > 15) &&(port < 22))){
//              } else if ((i == 3) && (port == 14)) {
                } else {
                    if (i < 3) {
                        GPIO_WriteReg(addr_gpio[i]+0x20,
                            (GPIO_ReadReg(addr_gpio[i]+0x20) & ~(1 << port)));
                    } else {
                        GPIO_WriteReg(0xA0F28814,
                            (GPIO_ReadReg(0xA0F28814) & ~(1 << port)));
                    }
                    //ext_irq_sel_p(port);                // For GPIO-A
                    ext_irq_sel_p(nth_irq, port + gpio_port[i]);               // For GPIO-B
                    //ext_irq_sel_p(130);              // For GPIO-A
                    //for(type=0; type<GPIO_MAX_TYPE; type++) {
                    mcu_printf(" \n filter-%d, 0x%08X[%d] ", nth_irq, addr_gpio[i], port);

                    //ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_HIGH, ((0x1 << ((type + 8) +1)) -1)); // Active High
                    //ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_HIGH, 0xFFFFF); // Active High
                    //ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_LOW, 0x0); // Active Low
                    ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_HIGH, DEPTH); //  GPIO_ACTIV_HIGH
                    ext_irq_filter_en (nth_irq);

                    open_irq_p(nth_irq, port, i);
                    //mc_sim_ext_irq_test(SET_IRQ_PORT | IRQ_ENABLE | (port<<4) | type);
                    delay1us(TOGGLE_DLY);
                    // Wait positive edge interrupt
//                  GPIO_WriteReg(addr_gpio[i], (GPIO_ReadReg(addr_gpio[i]) & ~(1 << port)));
//                  delay1us(1);
                    if (i < 3) {
                        GPIO_WriteReg(addr_gpio[i]+0x20,
                            (GPIO_ReadReg(addr_gpio[i]+0x20) | (1 << port)));
                    } else {
                        GPIO_WriteReg(0xA0F28814,
                            (GPIO_ReadReg(0xA0F28814) | (1 << port)));
                    }
                    delay1us(TOGGLE_DLY);

                    for (uiK1 = 0 ; (uiK1 < uiDelaycnt) ; uiK1++)
                    {
                        if(wait_irq_p[nth_irq] == 0UL)
                        {
                            break;
                        }
                        BSP_NOP_DELAY();
                    }
                    open_irq_n(nth_irq, port, i);
                    delay1us(TOGGLE_DLY);

                    //GPIO_WriteReg(addr_gpio[i], (GPIO_ReadReg(addr_gpio[i]) & ~(1 << port)));
                    if (i < 3) {
                        GPIO_WriteReg(addr_gpio[i]+0x20,
                            (GPIO_ReadReg(addr_gpio[i]+0x20) & ~(1 << port)));
                    } else {
                        GPIO_WriteReg(0xA0F28814,
                            (GPIO_ReadReg(0xA0F28814) & ~(1 << port)));
                    }
                    delay1us(TOGGLE_DLY);

                    close_irq_p(nth_irq);

                    // Wait negative edge interrupt

                    for (uiK2 = 0 ; (uiK2 < uiDelaycnt) ; uiK2++)
                    {
                        if(wait_irq_n[nth_irq] == 0UL)
                        {
                            break;
                        }
                        BSP_NOP_DELAY();
                    }
                    close_irq_n(nth_irq);

                    ext_irq_filter_clr(nth_irq);
                    //mc_sim_ext_irq_test(SET_IRQ_PORT | IRQ_DISABLE | (port<<4) | type);
                }
            }
        }
    }

    mcu_printf("\n glitch test DONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

}


static void GPIO_SmSwFaultInjection(void)
{
    uint32    i;
    uint32    base_addr_pw;
    uint32    base_addr;
    uint32    read_stat;
    //uint32 uiK1;
    //const uint32  uiDelaycnt = (5000000UL);

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
    mcu_printf("\nFault Injection test start\n");
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

    (void)FMU_IsrHandler(FMU_ID_GPIO_CFG, FMU_SVL_LOW, (FMUIntFnctPtr)&gpio_fault_isr, NULL);
    (void)FMU_Set(FMU_ID_GPIO_CFG);
    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    if ((i < 18) && (i > 11)) {
    } else if ((i == 25) || (i == 26)){
    } else {
            (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }
    if ((i < 22) && (i > 15)) {
    } else {
            (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }

    }

    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS + 0x4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS + 0x8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);

    // Fault inection test enable
    base_addr_pw = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_PW;
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0xCUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x14UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x18UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x1CUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x20UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x24UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x28UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x2CUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr= MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x30UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr= MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x34UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x38UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x3cUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x40UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x44UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);

#if 0
    for(i=0; i<18; i++) {
        HwMC_GPIO->CFG_WR_PW                        = GPIO_PASSWORD;
        HwMC_GPIO->SOFT_FAULT_EN[i]                 = 0xffffffff;
    }

    HwMC_GPIO->CFG_WR_PW                                = GPIO_PASSWORD;//;//GPIO_PASSWORD;
    HwMC_GPIO->uSOFT_FAULT_CTRL.bReg.SOFT_FAULT_TEST_EN = 1;

    HwMC_GPIO->CFG_WR_PW                            = GPIO_PASSWORD;
    HwMC_GPIO->uSOFT_FAULT_CTRL.bReg.SOFT_FAULT_EN  = 1;
#endif

    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SF_CTRL_CFG;
    GPIO_WriteReg(base_addr, GPIO_ReadReg(base_addr) | 0x00000011UL);
    //GPIO_WriteReg(base_addr, GPIO_ReadReg(base_addr) | 0x0FFF0011UL);
    GPIO_WriteReg(0xA0F20048, GPIO_PASSWORD);
    GPIO_WriteReg(0xA0F20070, GPIO_ReadReg(0xA0F20070) | 0x00000011UL);
    //GPIO_WriteReg(0xA0F20070, GPIO_ReadReg(0xA0F20070) | 0x0FFF0011UL);

    for(i=0; i<(uint32)91; i++)
    {
        fault_index = wr_reg[i<<1];

    if ((fault_index != 30) &&
        (fault_index != 31) &&
        (fault_index < 560)) {
               // *_FNC_LOCK, *_OEN_LOCKn skip
            base_addr = MCU_BSP_GPIO_BASE + (fault_index<< 2);
            GPIO_WriteReg(base_addr,
            GPIO_ReadReg(base_addr) ^ 0x11111111UL);
            wait_irq_done();

            mcu_printf("GPIO Inturrupt done [%d][0x%08X]",fault_index,base_addr);
            (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_GPIO_CFG);
#if 0
        read_stat = (GPIO_ReadReg(0xA0F28450));
        while(read_stat != 0) {
            read_stat = (GPIO_ReadReg(0xA0F28450));
             (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_GPIO_CFG);
        }
#endif
            GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
            base_addr = MCU_BSP_GPIO_BASE + GPIO_SF_CTRL_STS;
        read_stat = (GPIO_ReadReg(base_addr) & ~0x1);
        GPIO_WriteReg(base_addr, (read_stat | 0x1));
            gpio_irq_done = 0;
        }
    }

    // Fault inection test disable
    //HwMC_GPIO->CFG_WR_PW                                 = GPIO_PASSWORD;//GPIO_PASSWORD;
    //HwMC_GPIO->uSOFT_FAULT_CTRL.bReg.SOFT_FAULT_TEST_EN  = 0;
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SF_CTRL_CFG;
    GPIO_WriteReg(base_addr,
    GPIO_ReadReg(base_addr) ^ 0x00000011UL);
}

static void GPIO_RegisterBank(void) /* based on SoC test bench */
{
    uint32    i;
    uint32    rw_error = 0;
    uint32    base_addr;

    mcu_printf("---- GPIO REG RESET TEST START \n");

    for(i=0; i<(uint32)GPIO_WR_REG_MAX; i++)
    {
        fault_index = reset_reg[i<<1UL];
        base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL);
        rw_error |= (GPIO_ReadReg(base_addr) ^ wr_reg[(i<<1UL) + 1UL]);

        if(rw_error != 0UL)
        {
            //mcu_printf("reset value 0x%08X , actual reg 0x%08X  fault_id %d \n", wr_reg[(i<<1UL) + 1UL], GPIO_ReadReg(base_addr), fault_index);
            rw_error = 0;
        }
    }
    mcu_printf("++++ GPIO REG RESET TEST START \n");
#if 1
    //micom select
    for(i=0; i< (uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
    }
#endif

    mcu_printf("---- GPIO REG READ/WRITE TEST START \n");

    for(i=0; i<(uint32)GPIO_WR_REG_MAX; i++)
    {
        fault_index = reset_reg[i<<1UL];
        base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL);

        GPIO_WriteReg(base_addr, 0xFFFFFFFFUL);
        //rw_error |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
        mcu_printf("0xff table 0x%08X , actual reg 0x%08X  fault_id %d \n", wr_reg[(i<<1UL) + 1UL], GPIO_ReadReg(base_addr), fault_index);

        GPIO_WriteReg(base_addr, 0x00000000UL);
        //rw_error |= (GPIO_ReadReg(base_addr) ^ wr_reg[(i << 1UL) + 1UL]);
        mcu_printf("0x00 table 0x%08X , actual reg 0x%08X  fault_id %d \n", wr_reg[(i<<1UL) + 1UL], GPIO_ReadReg(base_addr), fault_index);

    }

    mcu_printf("++++ GPIO REG READ/WRITE TEST END \n");
}


static void GPIO_lockoutputconfigure(void)
{
    uint32    i;
    uint32    check_data;
    uint32    base_addr;

    check_data =0;

    mcu_printf("[ip_test_GPIO_lockoutputconfigure start!!]\n");

    mcu_printf("EN register set 0x1 \n");

    for(i=0; i<(uint32)GPIO_MAX_NUM; i++)
    {
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    }

    mcu_printf("EN register set 0x0 \n");

    for(i=0; i<(uint32)GPIO_MAX_NUM; i++)
    {
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    }

    mcu_printf("EN_LOCK_ENn regitse 0x0 \n");
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x4UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x8UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x10UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));


    mcu_printf("EN register set 0x1 \n");
    for(i=0; i<(uint32)GPIO_MAX_NUM; i++)
    {
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("EN register set fail [0x%08X][0x%08X]\n",base_addr, GPIO_ReadReg(base_addr) );
        }
        else
        {
            mcu_printf("EN register set sucess [0x%08X][0x%08X]\n", base_addr, GPIO_ReadReg(base_addr) );
        }
    }

    mcu_printf("EN_LOCK_ENn regitser write 0x1 and read\n");
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x10UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));


    mcu_printf("[ip_test_GPIO_lockoutputconfigure End!!]\n");
}

static void SYS_Dis(void)
{
    uint32 i, dat;
    uint32    base_addr;

    GPIO_WriteReg(0xA0F2003C, 0x10);
    GPIO_WriteReg(0xA0F2003C, 0xffffffff);

    i = GPIO_ReadReg(0xA0F2003C);
    if (i != 0xffffffff) {
        mcu_printf("error!! SPER_USR_WR_DIS0 : 0x%08x\n", i);
        while(1);
    }

    asm("mov r9,r13");
    asm("mov r10,r14");
    asm("mrs r0, cpsr");
    asm("bic r1, r0, #0x1f");
    asm("orr r1, r1, #0x10");
    asm("msr cpsr,r1");               // For Supervisor mode, IRQ, FIQ enable
    asm("mov r13,r9");
    asm("mov r14,r14");

    for (i = 0; i < 27; i++) {
        base_addr = user_mode_addr[i];
        GPIO_WriteReg(base_addr, 0x1111);
        dat = GPIO_ReadReg(base_addr);
        if (dat == 0x1111) {
            mcu_printf("error!! user mode addr (0x%08x)\n", base_addr);
        } else {
            mcu_printf("user mode addr (0x%08x) is OK\n", base_addr);
        }
    }
    mcu_printf("Done!!!\n", i);
}

void GPIO_StartGpioTest(int32 ucMode)
{
    switch (ucMode)
    {

        case 1 :
        {
            // Trace32
            // GPIO_RegisterBank();
            break;
        }
        case 2 :
        {
            // Trace32
            // GPIO_InoutControl();
            break;
        }
        case 3:
        {
            // Trace32
            // GPIO_controlatfunction0();
            break;
        }
        case 4:
        {
            // Trace32
            // GPIO_lockfunctionmultiplexer();
            break;
        }
        case 5:
        {
            // Trace32
            // GPIO_lockoutputconfigure();
            break;
        }
        case 9:
        {
            // ?
            // GPIO_Tpg();
            break;
        }
        case 10:
        {
            GPIO_SmWriteProtection();
            break;
        }
        case 11:
        {
            GPIO_SmSwFaultInjection();
            break;
        }
        case 12:
        {
            GPIO_SmExtglitchintFilter();
            break;
        }
        case 13:
        {
            // ?
            // SYS_Dis();
            break;
        }
        default:
        {
            mcu_printf("invalid test mode param\n");
            break;
        }
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )


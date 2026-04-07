// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : cpu_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

/*
 // Test code
 Disble_Cache();
 Calculate_CacheTime();
 Enable_Cache();
 Calculate_CacheTime();
 */

#if ( MCU_BSP_SUPPORT_TEST_APP_CPU == 1 )

#include <math.h>
#include <float.h>
#include "debug.h"
#include "reg_phys.h"
#include "wdt.h"
#include "cpu_test.h"

static uint64 my_pow
(
    uint64                              base,
    int                                 exp
);


void Disble_Cache
(
    void
)
{
    /* Disable I-Cache , D-Cache */

    __asm volatile (
            "mrc    p15, 0, r0, c1, c0, 0 \n" // Read CP15 SCTLR
            "bic    r0, r0, #(0x1<<2)     \n" // Disable D-Cache
            "bic    r0, r0, #(0x1<<12)    \n" // Disable I-Cache
            "DSB                          \n"
            "mcr    p15, 0, r0, c1, c0, 0 \n" // Write CP15 SCTLR
            "ISB                          \n"
    );
}

void Enable_Cache
(
    void
)
{
    /* Enable I-Cache , D-Cache */

    __asm volatile (
            "mrc    p15, 0, r0, c1, c0, 0 \n" // Read CP15 SCTLR
            "orr    r0, r0, #(0x1<<2)     \n" // Enable D-Cache
            "orr    r0, r0, #(0x1<<12)    \n" // Enable I-Cache
            "DSB                          \n"
            "mcr    p15, 0, r0, c1, c0, 0 \n" // Write CP15 SCTLR
            "ISB                          \n"
    );
}

static uint64 my_pow
(
    uint64                              base,
    int                                 exp
)
{
    uint64  res = 1;

    while(exp)
    {
        if (exp & 1)
        {
            res *= base;
        }

        exp >>= 1;
        base *= base;
    }

    return res;
}

/**********************************/
/*          runtime.init          */
/*          runtime.view          */
/**********************************/
void Calculate_CacheTime
(
    void
)
{
    uint32  i, j;
    uint32  columns = 100;
    uint32  rows = 100;
    uint32  arr[100][100];

    for (i = 0; i < columns; i += 1)
    {
        for (j = 0; j < rows; j += 1)
        {
            arr[i][j] = my_pow(i, j);
        }
    }
    mcu_printf("CacheTime Test!!, %d\n", arr[columns][rows]);
}

/***********************************************************************/
/*  CFLAGS += -mfpu=vfpv3-d16 -mfloat-abi=softfp -DVFP_D16 -DNEON_USE  */
/***********************************************************************/
void Calculate_FPU
(
    void
)
{
    float   test_float1 = 0.0f;
    float   test_float2 = 0.0f;
    double  add_float = 0.0;
    double  sub_float = 0.0;
    double  mul_float = 0.0;
    double  div_float = 0.0;

    test_float1 = 2.3f;
    test_float2 = 1.15f;

    // TEST ADD
    add_float = test_float1 + test_float2;
    if (fabsf(add_float - 3.45f) <= FLT_EPSILON)
        mcu_printf("FPU ADD SUCCESS!! (%d)\n", (int)(add_float*1000));

    // TEST SUBTRACT
    sub_float = test_float1 - test_float2;
    if (fabsf(sub_float - 1.15f) <= FLT_EPSILON)
        mcu_printf("FPU SUBTRACT SUCCESS!! (%d)\n", (int)(sub_float*1000));

    // TEST MULTIPLY
    mul_float = test_float1 * test_float2;
    if (fabsf(mul_float - 2.645f) <= FLT_EPSILON)
        mcu_printf("FPU MULTIPLY SUCCESS!! (%d)\n", (int)(mul_float*1000));

    // TEST DIVIDE
    div_float = test_float1 / test_float2;
    if (fabsf(div_float - 2.0f) <= FLT_EPSILON)
        mcu_printf("FPU DIVIDE SUCCESS!! (%d)\n", (int)(div_float*1000));

    /****************/

    add_float = 0.0;
    sub_float = 0.0;
    mul_float = 0.0;
    div_float = 0.0;
    test_float1 = 2.4f;
    test_float2 = 1.12f;

    // TEST ADD
    add_float = test_float1 + test_float2;
    if (fabsf(add_float - 3.52f) <= FLT_EPSILON)
        mcu_printf("FPU ADD SUCCESS!! (%d)\n", (int)(add_float*1000));

    // TEST SUBTRACT
    sub_float = test_float1 - test_float2;
    if (fabsf(sub_float - 1.28f) <= FLT_EPSILON)
        mcu_printf("FPU SUBTRACT SUCCESS!! (%d)\n", (int)(sub_float*1000));

    // TEST MULTIPLY
    mul_float = test_float1 * test_float2;
    if (fabsf(mul_float - 2.688f) <= FLT_EPSILON)
        mcu_printf("FPU MULTIPLY SUCCESS!! (%d)\n", (int)(mul_float*1000));

    // TEST DIVIDE
    div_float = test_float1 / test_float2;
    //if (fabsf(div_float - 2.142857142857143d) <= FLT_EPSILON)
    //    printf("FPU DIVIDE SUCCESS!! (%d)\n", (int)(div_float*1000));
}

void RemapMicomTest
(
    void
)
{
    uint32  choice = 0;

    // Disable Internal sram initialize
    SAL_WriteReg(0x06, 0x1B936188);

    switch(choice)
    {
        case 0: //SRAM0
            //Copy assemble code (b .) to SRAM0
            SAL_WriteReg(0xEAFFFFFE, 0x06000000);
            // Remap MICOM (Internal SRAM)
            SAL_WriteReg(0x12200003, 0x1B936014);
            break;
        case 1: //SRAM1
            //Copy assemble code (b .) to SRAM1
            SAL_WriteReg(0xEAFFFFFE, 0x07000000);
            // Remap MICOM (Secure Internal SRAM)
            SAL_WriteReg(0x12400003, 0x1B936014);
            break;
        default:
            break;
    }

    WDT_Init();
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_CPU == 1 )


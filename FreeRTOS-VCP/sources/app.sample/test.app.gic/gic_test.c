// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gic_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )

#include <gic_test.h>
#include <gic.h>
#include <bsp.h>

#include <debug.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/




/***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/


static uint32                           guiWaitSmIrq = 0;
static uint32                           guiIsrArg  = 0u;
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void GIC_Test_Isr(void *pArg)
{
    const uint32* puiIrqIndex;

    puiIrqIndex = NULL_PTR;

    if(pArg != NULL_PTR)
    {
        puiIrqIndex = (const uint32 *)pArg;
        // Clear SW interrupt
        (void)GIC_SmIrqSetHigh(*puiIrqIndex, (boolean)0u);

        (void)GIC_SmIrqEn(*puiIrqIndex, (boolean)0u);
        // Clear corresponding interrupt enable
        (void)GIC_IntSrcDis(*puiIrqIndex);

        // Clear interrupt handler for irq_index
        (void)GIC_IntVectSet(*puiIrqIndex, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, NULL, NULL);

        mcu_printf(" Get IRQ-%d  Ok\n", *puiIrqIndex);
        if(guiWaitSmIrq > 0u)
        {
            guiWaitSmIrq--;// = 0;
        }
    }

}

/*
***************************************************************************************************
*                                       GIC_Test_SmInt
*
* GIC interrupt test code(SPI)
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void GIC_Test_SmInt(void)
{
    uint32        uiIrqIndex;
    uint32        uiK;
    const uint32  uiDelaycnt = (5000000UL);

    uiIrqIndex = 0;
    guiIsrArg   = 0;
    uiK        = 0UL;

    //---------------------------------------------------------------------------------------------
    // SW test using HW GIC test block
    //---------------------------------------------------------------------------------------------

    mcu_printf("================== SM SPI interrupt test ==================\n");

    // Disable all interrupts for test
    for(uiIrqIndex = GIC_SPI_START; uiIrqIndex < GIC_INT_SRC_CNT; uiIrqIndex++)
    {
        (void)GIC_IntSrcDis(uiIrqIndex);
    }


    for(uiIrqIndex = GIC_SPI_START; uiIrqIndex < GIC_INT_SRC_CNT; uiIrqIndex++)
    {
        (void)GIC_SmIrqEn(uiIrqIndex, (boolean)0u);
    }

    // GIC SW interrupt generation

    guiWaitSmIrq = (GIC_INT_SRC_CNT - GIC_SPI_START);

    for(uiIrqIndex = GIC_SPI_START; uiIrqIndex < GIC_INT_SRC_CNT; uiIrqIndex++)
    {
        guiIsrArg = uiIrqIndex;
        (void)GIC_IntVectSet(uiIrqIndex,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&GIC_Test_Isr,
                             &guiIsrArg);

        (void)GIC_SmIrqEn(uiIrqIndex, (boolean)1u);

        (void)GIC_IntSrcEn(uiIrqIndex);

        (void)GIC_SmIrqSetHigh(uiIrqIndex, (boolean)1u);

        for ( ; (uiK < uiDelaycnt) ; uiK++)
        {
            if(guiWaitSmIrq < (GIC_INT_SRC_CNT - uiIrqIndex))
            {
                break;
            }

            BSP_NOP_DELAY();
        }

        if(uiK == uiDelaycnt)
        {
            mcu_printf("Interrupt not occured\n");
        }

    }

    mcu_printf("guiWaitSmIrq %d\n",guiWaitSmIrq);

    mcu_printf("====== GIC Test Done            ============================\n");
    mcu_printf("====== All of the interrupt has been disabled. =============\n");
    mcu_printf("====== Please. reset the board for normal operation. =======\n");
    return;
}

/*
***************************************************************************************************
*                                       GIC_Test_Sgi
*
* GIC interrupt test code(SGI)
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void GIC_Test_Sgi(void)
{

    uint32        uiIrqIndex;
    uint32        uiK;
    const uint32  uiDelaycnt = (5000000UL);

    uiIrqIndex = 0;
    guiIsrArg   = 0;
    uiK        = 0UL;

    //---------------------------------------------------------------------------------------------
    // SW test using HW GIC test block
    //---------------------------------------------------------------------------------------------

    mcu_printf("================== SGI interrupt test ==================\n");

    // Disable all interrupts for test
    for( ; uiIrqIndex < GIC_PPI_START; uiIrqIndex++)
    {
        (void)GIC_IntSrcDis(uiIrqIndex);
    }

    // GIC SW interrupt generation
    guiWaitSmIrq = (GIC_PPI_START);

    for(uiIrqIndex = 0; uiIrqIndex < GIC_PPI_START; uiIrqIndex++)
    {
        guiIsrArg = uiIrqIndex;
        (void)GIC_IntVectSet(uiIrqIndex,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&GIC_Test_Isr,
                             &guiIsrArg);


        (void)GIC_IntSrcEn(uiIrqIndex);

        (void)GIC_IntSGI(uiIrqIndex);

        for ( ; (uiK < uiDelaycnt) ; uiK++)
        {
            if(guiWaitSmIrq < (GIC_PPI_START- uiIrqIndex))
            {
                break;
            }

            BSP_NOP_DELAY();
        }

        if(uiK == uiDelaycnt)
        {
            mcu_printf("Interrupt not occured\n");
        }

    }

    mcu_printf("guiWaitSmIrq %d\n",guiWaitSmIrq);

    mcu_printf("====== GIC SGI Test Done        ============================\n");
    mcu_printf("====== All of the SGI interrupt has been disabled. =========\n");
    mcu_printf("====== Please. reset the board for normal operation. =======\n");
    return;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )


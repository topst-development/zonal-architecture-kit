// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : sfmc_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <sfmc_test.h>
#include <bsp.h>
#include <fmu.h>
#include <mpu.h>
#include <debug.h>
#include <snor_mio.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*************************************************************************************************/
/*                                             LOCAL VARIABLES                                   */
/*************************************************************************************************/

#ifdef NEVER
/*
***************************************************************************************************
*                                          delay1us
*
* Function to set delay with 'nop'.
*
* @param    uiUs
* @return
* Notes
*
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
#endif /* NEVER */

/*
***************************************************************************************************
*                                          SFMC_FmuIsr
*
* Function to process FMU ISR.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void SFMC_FmuIsr
(
    void                                *pArg
) {
    FMUErrTypes_t                       err;

    // unused
    (void)pArg;

    err = FMU_OK;

    mcu_printf("***************************\n");
    mcu_printf("SFMC_FmuIsr\n");
    mcu_printf("***************************\n");

    err = FMU_IsrClr(FMU_ID_SFMC_ECC);

    if(err != FMU_OK)
    {
        mcu_printf("FMU ISR ERROR\n");
    }

    return;
}

/*
***************************************************************************************************
*                                          SFMC_EnableFmu
*
* Function to register FMU ISR and enable FMU SFMC interrupt.
*
* @return
* Notes
*
***************************************************************************************************
*/

static void SFMC_EnableFmu
(
    void
) {
//    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SFMC_ECC, (FMUSeverityLevelType_t)FMU_SVL_LOW,
//            (FMUIntFnctPtr)&SFMC_FmuIsr, NULL_PTR);
//    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SFMC_ECC, (FMUSeverityLevelType_t)FMU_SVL_MID,
//            (FMUIntFnctPtr)&SFMC_FmuIsr, NULL_PTR);
    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SFMC_ECC, (FMUSeverityLevelType_t)FMU_SVL_HIGH,
            (FMUIntFnctPtr)&SFMC_FmuIsr, NULL_PTR);

    (void)FMU_Set((FMUFaultid_t)FMU_ID_SFMC_ECC);

    return;
}

/*
***************************************************************************************************
*                                          SFMC_Test
*
* Functions to call all of test functions.
*
* @return
* Notes
*
***************************************************************************************************
*/

void SFMC_Test
(
    void
) {
    int32 ret;

    mcu_printf("--------------------------\n");
    mcu_printf("Start SFMC Test\n");
    mcu_printf("--------------------------\n");
//    SFMC_EnableFmu();

    ret = SNOR_MIO_Init(SFMC1);
    if (ret == 0)
    {
        mcu_printf("SFMC Init Done.\n");
    }

#if 0
    mcu_printf("Program test...");
    ret = SNOR_MIO_FWDN_Write(0x00000000, 646400,(void *)0x20000000);
    if (ret == 0)
        mcu_printf("Good.\n");
    else
        mcu_printf("fail.\n");
#endif
    mcu_printf("\n");
    mcu_printf("========================================================\n");
    mcu_printf("\n");

    return;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )


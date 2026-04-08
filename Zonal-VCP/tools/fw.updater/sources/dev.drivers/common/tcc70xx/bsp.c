// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : bsp.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#include "bsp.h"

#include <reg_phys.h>
//#include <debug.h>

#include <clock.h>
#include <gic.h>
#include <uart.h>
#include <mbox.h>

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
    #include "fmu.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_GPSB == 1 )
    #include "gpsb.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_GPSB == 1 )

//#include <rtc.h>

/*
***************************************************************************************************
                                         STATIC VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/

void BSP_PreInit
(
    void
)
{
    CLOCK_Init();
    GIC_DeInit(); /* Disable all interrupts */
    GIC_Init();
    //PMU_Init();
}

void BSP_Init
(
    void
)
{

    (void)UART_Init();

#ifdef FMU_DRIVER
    (void)FMU_Init();
#endif

#if ( MCU_BSP_SUPPORT_DRIVER_GPSB == 1 )
    GPSB_Init();
#endif

    MBOX_Init();

    //mcu_printf("\nInitialize System done\n");
    //mcu_printf("Welcome to Telechips MCU BSP\n");
}

/* R0 : ARM Exception ID, R1 : Dummy , R2 : Link Register(Return PC)*/
void BSP_UndefAbortExceptionHook
(
    uint32                              uiExceptId,
    uint32                              uiDummy,
    uint32                              uiLinkReg
)
{
    (void)uiExceptId;
    (void)uiDummy;
    (void)uiLinkReg;

}

/*
Warning !!!  After calliing this function. it will never return from exception except H/W reseting.
This function is designed for very restricted use case such as firmware upgrading.
*/
void BSP_SystemHalt
(
    void
)
{
    //(void)SAL_CoreCriticalEnter();  // Interrupt disable(I/F)
    ARM_Reserved();                 // move ARM p.c on the sram.
}


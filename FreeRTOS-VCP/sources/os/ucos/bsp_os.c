// SPDX-License-Identifier: Apache-2.0

/**************************************************************************************************

*   FileName : bps_os.c

*   Copyright (c) Telechips Inc.

*   Description : System interface for arm exception handling of uC-OS-III


**************************************************************************************************/
/*-------------------------------------------------------------------------------------------------
Revision History

Author      Date             Version      Description of Changes
---------------------------------------------------------------------------------------------------
hskim       2020/07/16       0.1          Draft
JMP         2020/08/10       0.2          Adding OS Timer tick for uCOS
-------------------------------------------------------------------------------------------------*/

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <bsp.h>
#include <gic.h>
#include <bsp_os.h>
#include <timer.h>
#include <os.h>
#include <os_cfg_app.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/* ARM exception IDs                                      */
#define  BSP_OS_ARM_EXCEPT_RESET                      0x00u
#define  BSP_OS_ARM_EXCEPT_UNDEF_INSTR                0x01u
#define  BSP_OS_ARM_EXCEPT_SWI                        0x02u
#define  BSP_OS_ARM_EXCEPT_PREFETCH_ABORT             0x03u
#define  BSP_OS_ARM_EXCEPT_DATA_ABORT                 0x04u
#define  BSP_OS_ARM_EXCEPT_ADDR_ABORT                 0x05u
#define  BSP_OS_ARM_EXCEPT_IRQ                        0x06u
#define  BSP_OS_ARM_EXCEPT_FIQ                        0x07u
#define  BSP_OS_ARM_EXCEPT_NBR                        0x08u

/*
*********************************************************************************************************
*                                       LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void OS_TickHandler
(
    void *                              pArg
);

/*
*********************************************************************************************************
*********************************************************************************************************
**                                      GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

 /*
***************************************************************************************************
*                                       OS_CPU_ExceptHndlr
*
* Set increment option for source and destination.
*
* @param	except_id [in] ARM exception type:
 *                                  OS_CPU_ARM_EXCEPT_RESET             0x00
 *                                  OS_CPU_ARM_EXCEPT_UNDEF_INSTR       0x01
 *                                  OS_CPU_ARM_EXCEPT_SWI               0x02
 *                                  OS_CPU_ARM_EXCEPT_PREFETCH_ABORT    0x03
 *                                  OS_CPU_ARM_EXCEPT_DATA_ABORT        0x04
 *                                  OS_CPU_ARM_EXCEPT_ADDR_ABORT        0x05
 *                                  OS_CPU_ARM_EXCEPT_IRQ               0x06
 *                                  OS_CPU_ARM_EXCEPT_FIQ               0x07
* @return	none.
*
* Notes		(1) Only OS_CPU_ARM_EXCEPT_FIQ and OS_CPU_ARM_EXCEPT_IRQ exceptions handler are implemented.
*                   For the rest of the exception a infinite loop is implemented for debuging pruposes. This behavior
*                   should be replaced with another behavior (reboot, etc).
*
***************************************************************************************************
*/

void OS_CPU_ExceptHndlr
(
    CPU_INT32U except_id
) {
    switch (except_id)
    {
        case BSP_OS_ARM_EXCEPT_FIQ:
            (void)GIC_IntHandler();
            break;

        case BSP_OS_ARM_EXCEPT_IRQ:
            (void)GIC_IntHandler();
            break;

        case BSP_OS_ARM_EXCEPT_RESET:
        /* $$$$ Insert code to handle a Reset exception               */

        case BSP_OS_ARM_EXCEPT_UNDEF_INSTR:
        /* $$$$ Insert code to handle a Undefine Instruction exception */

        case BSP_OS_ARM_EXCEPT_SWI:
        /* $$$$ Insert code to handle a Software exception             */

        case BSP_OS_ARM_EXCEPT_PREFETCH_ABORT:
        /* $$$$ Insert code to handle a Prefetch Abort exception       */

        case BSP_OS_ARM_EXCEPT_DATA_ABORT:
        /* $$$$ Insert code to handle a Data Abort exception           */

        case BSP_OS_ARM_EXCEPT_ADDR_ABORT:
        /* $$$$ Insert code to handle a Address Abort exception        */

        default:
            while (1)
            { /* Infinite loop on other exceptions. (see note #1)          */
                SAL_CoreWaitForEvent();
            }
    }
}

void OS_TickHandler
(
    void * pArg
) {
    OSTimeTick();

    (void)TIMER_InterruptClear(SAL_OS_TIMER_ID);

    SAL_IncreaseSystemTick(NULL);
}

void OS_TmrTickInit(void)
{
    uint32 uiTickToUSec;

    (void)GIC_IntVectSet(GIC_TIMER_0 + SAL_OS_TIMER_ID,
        	            0xau,
        	            GIC_INT_TYPE_LEVEL_HIGH,
        	            (GICIsrFunc)OS_TickHandler,
        	            NULL);

	(void)GIC_IntSrcEn(GIC_TIMER_0 + SAL_OS_TIMER_ID);
	uiTickToUSec = (uint32)(((uint32)1000U*(uint32)1000U)/OS_CFG_TICK_RATE_HZ);
	(void)TIMER_Enable(SAL_OS_TIMER_ID, uiTickToUSec, 0, 0);
	SAL_CoreMB();
}


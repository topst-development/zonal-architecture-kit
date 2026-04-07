# SPDX-License-Identifier: Apache-2.0

 /*******************************************************************************

*   Copyright (c) Telechips Inc.


*
*******************************************************************************/

/*****************************************************
 *
 * osal.S
 * Telechips OS abstraction Layer
 *
 * History
 * -------------
 * Created by : Bong Sik, Kim  2017/10/31
 *
 *****************************************************/

    .global  ARM_ExceptUndefInstrHndlr
    .global  ARM_ExceptSwiHndlr
    .global  ARM_ExceptPrefetchAbortHndlr
    .global  ARM_ExceptDataAbortHndlr
    .global  ARM_ExceptIrqHndlr
    .global  ARM_ExceptFiqHndlr


/*********************************************************************************************************
*                               UNDEFINED INSTRUCTION EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptUndefInstrHndlr:
    B            .
/*
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptUndefInstrHndlr
#else
    B            OS_CPU_ARM_ExceptUndefInstrHndlr
#endif
*/


/*********************************************************************************************************
*                                SOFTWARE INTERRUPT EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptSwiHndlr:
    B            .
/*
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptSwiHndlr
#else
    B            OS_CPU_ARM_ExceptSwiHndlr
#endif
*/


/*********************************************************************************************************
*                                  PREFETCH ABORT EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptPrefetchAbortHndlr:
    B            .
/*
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptPrefetchAbortHndlr
#else
    B            OS_CPU_ARM_ExceptPrefetchAbortHndlr
#endif
*/


/*********************************************************************************************************
*                                    DATA ABORT EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptDataAbortHndlr:
    B            .
/*
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptDataAbortHndlr
#else
    B            OS_CPU_ARM_ExceptDataAbortHndlr
#endif
*/


/*********************************************************************************************************
*                                 INTERRUPT REQUEST EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptIrqHndlr:
    B            arm_irq
/*
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptIrqHndlr
#else
    B            OS_CPU_ARM_ExceptIrqHndlr
#endif
*/


/*********************************************************************************************************
*                              FAST INTERRUPT REQUEST EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptFiqHndlr:
    B            .
/*
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptFiqHndlr
#else
    B            OS_CPU_ARM_ExceptFiqHndlr
#endif
*/


	.end

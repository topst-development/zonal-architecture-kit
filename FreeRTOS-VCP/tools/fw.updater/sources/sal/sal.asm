# SPDX-License-Identifier: Apache-2.0

;/*******************************************************************************
  
;*   Copyright (c) Telechips Inc.
 
 
;*
;*******************************************************************************/
;*****************************************************
;
; tcsal.S
; Telechips OS abstraction Layer
;
; History
; -------------
; Created by : Bong Sik, Kim  2018/03/05
;
;*****************************************************
 
	GLOBAL	ARM_ExceptUndefInstrHndlr
	GLOBAL	ARM_ExceptSwiHndlr
	GLOBAL	ARM_ExceptPrefetchAbortHndlr
	GLOBAL	ARM_ExceptDataAbortHndlr
	GLOBAL	ARM_ExceptIrqHndlr
	GLOBAL	ARM_ExceptFiqHndlr


;*********************************************************************************************************
;                               UNDEFINED INSTRUCTION EXCEPTION HANDLER
;*********************************************************************************************************

ARM_ExceptUndefInstrHndlr
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptUndefInstrHndlr
#else
    B            OS_CPU_ARM_ExceptUndefInstrHndlr
#endif


;*********************************************************************************************************
;                                SOFTWARE INTERRUPT EXCEPTION HANDLER
;*********************************************************************************************************

ARM_ExceptSwiHndlr
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptSwiHndlr
#else
    B            OS_CPU_ARM_ExceptSwiHndlr
#endif


;*********************************************************************************************************
;*                                  PREFETCH ABORT EXCEPTION HANDLER
;*********************************************************************************************************

ARM_ExceptPrefetchAbortHndlr
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptPrefetchAbortHndlr
#else
    B            OS_CPU_ARM_ExceptPrefetchAbortHndlr
#endif


;*********************************************************************************************************
;*                                    DATA ABORT EXCEPTION HANDLER
;*********************************************************************************************************

ARM_ExceptDataAbortHndlr
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptDataAbortHndlr
#else
    B            OS_CPU_ARM_ExceptDataAbortHndlr
#endif


;*********************************************************************************************************
;*                                 INTERRUPT REQUEST EXCEPTION HANDLER
;*********************************************************************************************************

ARM_ExceptIrqHndlr
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptIrqHndlr
#else
    B            OS_CPU_ARM_ExceptIrqHndlr
#endif


;*********************************************************************************************************
;*                              FAST INTERRUPT REQUEST EXCEPTION HANDLER
;*********************************************************************************************************

ARM_ExceptFiqHndlr
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptFiqHndlr
#else
    B            OS_CPU_ARM_ExceptFiqHndlr
#endif


	END

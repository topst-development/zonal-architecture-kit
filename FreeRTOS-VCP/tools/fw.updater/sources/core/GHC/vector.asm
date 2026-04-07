# SPDX-License-Identifier: Apache-2.0

;/*******************************************************************************
  
;*   Copyright (c) Telechips Inc.
 
 
;*
;*******************************************************************************/
;********************************************************************************************************
;
; vector.asm
; Telechips TCC70xx SoCs Cortex-R5 Single Processor
;
; History
; -------------
; Created by : SangWon, Lee  2016/8/19
;
;********************************************************************************************************


;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

;	RSEG CODE:CODE:NOROOT(2)
	CODE32

	GLOBAL	_vector
	GLOBAL	ARM_Reserved
	EXTERN	reset

_vector
	ldr pc, =reset_handler
	ldr pc, =ARM_ExceptUndefInstrHndlr
	ldr pc, =ARM_ExceptSwiHndlr
	ldr pc, =ARM_ExceptPrefetchAbortHndlr
	ldr pc, =ARM_ExceptDataAbortHndlr
	ldr pc, =ARM_Reserved
	ldr pc, =ARM_ExceptIrqHndlr
	ldr pc, =ARM_ExceptFiqHndlr

	
reset_handler
	ldr pc, = reset

ARM_Reserved
	wfi
	b ARM_Reserved

	END

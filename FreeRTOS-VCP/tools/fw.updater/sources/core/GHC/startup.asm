# SPDX-License-Identifier: Apache-2.0

;/*******************************************************************************

;*   Copyright (c) Telechips Inc.


;*
;*******************************************************************************/
;********************************************************************************************************
;
; startup.asm
; Telechips TCC70xx SoCs Cortex-R5 Single Processor
;
; History
; -------------
; Created by : SangWon, Lee  2016/8/19
;
;********************************************************************************************************

#include <pll_config.h>
#include <version.h>
;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

/* Init Stack Pointer */
USERMODE		EQU	0x10
FIQMODE			EQU	0x11
IRQMODE			EQU	0x12
SVCMODE			EQU	0x13
ABORTMODE		EQU	0x17
UNDEFMODE		EQU	0x1b
SYS_MODE		EQU	0x1f
MODEMASK		EQU	0x1f
NOINT			EQU	0xc0

SRAM_OFFSET		EQU    (0x00000000)

STACK_BASE_ADDR 	EQU	(SRAM_OFFSET + SRAM_TOTAL_SIZE - DMA_NC_SIZE)
StackEnd			EQU 	(STACK_BASE_ADDR - 0x680)
SvcStackSt		EQU	(STACK_BASE_ADDR - 0x380)
UserStackSt		EQU	(STACK_BASE_ADDR - 0x370)
UndefStackSt		EQU	(STACK_BASE_ADDR - 0x368)
AbortStackSt		EQU	(STACK_BASE_ADDR - 0x360)
IrqStackSt		EQU	(STACK_BASE_ADDR - 0x60)
FIQStackSt		EQU	(STACK_BASE_ADDR - 0x0)

;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

;	RSEG CODE:CODE:NOROOT(2)
	CODE32

	EXTERN	reset
	EXTERN	_vector
	EXTERN	vfp_init

;********************************************************************************************************
;                                             reset()
;
; Description :
;********************************************************************************************************

reset
;/****** Don't touch below 8 words !!!! ******/
	b init				/* 0x00 : initial branch instruction */
.word 0x44555746		/* 0x04 : FWUD */
.word __TEXT_START__ 	/* 0x08 : updater f/w load base */

; Don't delete below 3 words.
; This is to apply the 2oo3 voting method.
.word (0x56000000|(SDK_MAJOR_VERSION<<16)|(SDK_MINOR_VERSION<<8)|SDK_PATCH_VERSION) 	/* 0x0C : SDK VERSION INFO. Format : "V(0x56)xxx". x is 8bit digit number */
.word (0x56000000|(SDK_MAJOR_VERSION<<16)|(SDK_MINOR_VERSION<<8)|SDK_PATCH_VERSION)
.word (0x56000000|(SDK_MAJOR_VERSION<<16)|(SDK_MINOR_VERSION<<8)|SDK_PATCH_VERSION)

.word 0x00000000 		/* 0x10 : reserved */
.word 0x00000000 		/* 0x14 : reserved */
.word 0x00000000 		/* 0x18 : reserved */
.word 0x00000000 		/* 0x1c : reserved */
;/****** Don't touch end !!!! ******/

init
	; check R5 Core Enter to Serial Flash
	mrc p15, 0, r0, c0, c0, 0
	ldr r1, =0xFFFF
	and r0, r0, r1
	ldr r1, =0xC153
	cmp r0, r1
	bne wfi_loop

	; set the cpu to SVC32 mode and IRQ & FIQ disable
	mrs	r0, cpsr
	bic	r0, r0, #0x3f
	orr	r0, r0, #0xd3
	msr	cpsr, r0
; Stack Setup for each ARM Excution Mode
stack_setup
	mrs	r0, cpsr
	bic	r0, r0, #MODEMASK

	orr	r1, r0, #FIQMODE|NOINT
	msr	cpsr, r1
	ldr	sp, =FIQStackSt

	orr	r1, r0, #UNDEFMODE|NOINT
	msr	cpsr, r1
	ldr	sp, =UndefStackSt

	orr	r1, r0, #ABORTMODE|NOINT
	msr	cpsr, r1
	ldr	sp, =AbortStackSt

	orr	r1, r0, #IRQMODE|NOINT
	msr	cpsr_cxsf, r1
	ldr	sp, =IrqStackSt

	orr	r1, r0, #SYS_MODE|NOINT
	msr	cpsr_cxsf, r1
	ldr	sp, =UserStackSt

	orr	r1, r0, #SVCMODE|NOINT
	msr	cpsr_cxsf, r1
	ldr sp, =SvcStackSt

	; Cache, and MPU Setup */

	; Disable MPU, I-Cache, D-CAche */
	mrc p15, 0, r0, c1, c0, 0       ; Read CP15 SCTLR
	bic r0, r0, #(0x1<<0)           ; Disable MPU
	bic r0, r0, #(0x1<<2)           ; Disable D-Cache
	bic r0, r0, #(0x1<<12)          ; Disable I-Cache
	dsb
	mcr p15, 0, r0, c1, c0, 0       ; Write CP15 SCTLR
	isb

	mrc p15, 0, r0, c1, c0, 1       ; Read CP15, ACTLR
	orr r0, r0, #(0x1 << 17)        ; Clear RSDIS bit 17 to enable return stack
	orr r0, r0, #(0x1 << 16)        ; Clear BP bit 15 and set BP bit 16:
	bic r0, r0, #(0x1 << 15)        ; Branch always not taken and history table updates disabled
	bic r0, r0, #(0x1 << 5)         ; Generate abort on parity errors, with [5:3]=b 000
	bic r0, r0, #(0x1 << 4)
	bic r0, r0, #(0x1 << 3)
	mcr p15, 0, r0, c1, c0, 1       ; Write ACTLR
	dsb                             ; Complete all outstanding explicit memory operations

	; Invalidate Caches */
	mov r0, #0
	dsb
	mcr p15, 0, r0, c7, c5, 0       ; Invalidate I-Cache All
	mcr p15, 0, r0, c15, c5, 0      ; Incalidate D-Cache All
	isb

	; The default mpu table is not working in SRAM */
	bl MPU_Init                     ; MPU Initialize */

branch_prefiction
	; Enable Branch Predictions */
	mrc p15, 0, r0, c1, c0, 1       ; Read ACTLR
	bic r0, r0, #(0x1<<17)          ; Clear RSDIS bit 17 to enable returen stack
	bic r0, r0, #(0x1<<16)          ; Clear BP bit 15 and BP bit 16
	bic r0, r0, #(0x1<<15)          ; Normal operation BP is taken from the global history table
	mcr p15, 0, r0, c1, c0, 1

	; Enable I-Cache , D-Cache */
	mrc p15, 0, r1, c1, c0, 0
	orr r1, r1, #(0x1<<0)           ; Enable MPU
	orr r1, r1, #(0x1<<2)           ; Enable D-Cache
	orr r1, r1, #(0x1<<12)          ; Enable I-Cache
	DSB
	mcr p15, 0, r1, c1, c0, 0
	ISB

        bl InitCVar

; Set Vector Table to high Vector */
	mrc p15, 0, r0, c1, c0, 0
	bic r0, r0, #(0x1 << 13)
	mcr p15, 0, r0, c1, c0, 0

#ifdef PFLASH_STANDALONE
; Copy Vector Table to SRAM_0 */
vector_copy
	ldr r0, =_vector
	ldr r1, =0x00000000
	ldr r2, =0x00000020
vcopy_loop
	cmp r1, r2
	ldrlt r3, [r0], #4
	strlt r3, [r1], #4
	blt vcopy_loop
#endif
; move before copying vector table.

	bl vfp_init


	msr	cpsr_c,	0x53			// enable irq
	b	cmain
	b	.




;********************************************************************************************************
;                                            InitCVar()
;
; Description : Initialize RW/ZI Area
;********************************************************************************************************

InitCVar
	ldr	r0, =__ghs_romend
	ldr	r1, =__ghs_ramstart
	ldr	r2, =__ghs_bssstart

	cmp	r0, r1
	beq	5f
4:						; Copy RW Region
	ldm	r0!, {r4-r11}
	stm	r1!, {r4-r11}
	cmp	r1, r2
	bne	4b
5:						; Clear BSS Region
	ldr	r0, =__ghs_bssstart
	ldr	r1, =__ghs_ramend
	add	r1, r1, #0x1F
	bic	r1, r1, #0x1F
	cmp	r0, r1
	moveq	pc, lr

	mov	r4, #0
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
	mov	r9, #0
	mov	r10, #0
	mov	r11, #0
6:
	stm	r0!, {r4-r11}
	cmp	r0, r1
	bne	6b

	mov	pc, lr

wfi_loop
    wfi
    b wfi_loop

.bss
.align 2
stack_limit
	.skip (0x20)
mon_stack_top

	END

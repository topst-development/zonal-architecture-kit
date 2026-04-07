# SPDX-License-Identifier: Apache-2.0

;/*******************************************************************************
  
;*   Copyright (c) Telechips Inc.
 
 
;*
;*******************************************************************************/
;*****************************************************
;
;  arm_a.asm
;  Telechips TCC802x SoCs Cortex-A7 Single Processor
;
;  History
;  -------------
;  Created by : SangWon, Lee  2017/1/31
; 
;*****************************************************/

;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

	GLOBAL	ARM_TIMER_Get_Rate
	GLOBAL	ARM_TIMER_Get_CNTV_CTL
	GLOBAL	ARM_TIMER_Set_CNTV_CTL
	GLOBAL	ARM_TIMER_Set_CNTV_CVAL
	GLOBAL	ARM_TIMER_Get_CNTVCT
	GLOBAL	ARM_Set_VBAR
	GLOBAL	ARM_DSB


;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

;	RSEG CODE:CODE:NOROOT(2)
	CODE32


;********************************************************************************************************
;
;********************************************************************************************************

ARM_Set_VBAR
        MCR     p15, 0, r0, c12, c0, 0
        BX      LR


ARM_TIMER_Get_Rate
        MRC     p15, 0, r0, c14, c0, 0
        BX      LR


ARM_TIMER_Get_CNTV_CTL
        MRC     p15, 0, r0, c14, c3, 1
        BX      LR


ARM_TIMER_Set_CNTV_CTL
        MCR     p15, 0, r0, c14, c3, 1
        BX      LR


ARM_TIMER_Set_CNTV_CVAL
        MCRR    p15, 3, r0, r1, c14
        BX      LR


ARM_TIMER_Get_CNTVCT
        MRRC    p15, 1, r0, r1, c14
        BX      LR

ARM_DSB
        DSB
        ISB
        BX      LR
        

	END

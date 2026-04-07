// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : pmio_dev_extn_pw_ctl.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************/
#include <pmio_dev_extn_pw_ctl.h>
#include <pmio_dev.h>
#include <gpio.h>
#include <gic.h>
#include <debug.h>

/*************************************************************************************************
 *                                             DEFINITIONS
 *************************************************************************************************/

static PMIOEXTNExtPwrPins_t gtExtPwrPins;

static uint8 gucTccRstIrqStatus = 0U;
static uint32 guiGpk32 = 0UL;

/************************************************************************************************/
/*                                             STATIC FUNCTION                                  */
/************************************************************************************************/

static boolean PMIO_EXTN_IsGpk
(
    uint32 uiVa
);

static void PMIO_EXTN_PmicResetInterruptHandler
(
    void *                              pArg
);
/*************************************************************************************************/
/*                                             Implementation                                    */
/* ***********************************************************************************************/
static boolean PMIO_EXTN_IsGpk
(
    uint32 uiVa
)
{
    boolean bRet = FALSE;

    if(GPIO_PORT_K == (uiVa & GPIO_PORT_K))
    {
        bRet = TRUE;
    }

    return bRet;
}

static uint32 PMIO_EXTN_GetGpkNum
(
    uint32 uiVa
)
{
    uint32 uiGpkNum = (uiVa & 0x1FUL);

    //Store using gpk
    guiGpk32 |= (1UL << uiGpkNum);

    return uiGpkNum;
}

static void PMIO_EXTN_PmicResetInterruptHandler
(
    void *                              pArg
)
{
    (void)pArg;

    if(gucTccRstIrqStatus == 1U)
    {
        gucTccRstIrqStatus = 0U;
        (void)GIC_IntSrcDis(PMIO_EXTN_VA_INTERRUPT_SRC_PMIC_RST);
    }
    (void)GPIO_Set(gtExtPwrPins.uiTccRstPin, 1UL);
}

void PMIO_EXTN_Init
(
    PMIOEXTNExtPwrPins_t tExtPwrPins
)
{
    uint32 uiGpkNum = 0UL;

    gtExtPwrPins.uiPwrEnPin          = tExtPwrPins.uiPwrEnPin      ;
    gtExtPwrPins.uiAlivePwrConPin    = tExtPwrPins.uiAlivePwrConPin;
    gtExtPwrPins.uiStrModePin        = tExtPwrPins.uiStrModePin    ;
    gtExtPwrPins.uiPmicRstPin        = tExtPwrPins.uiPmicRstPin    ;
    gtExtPwrPins.uiTccRstPin         = tExtPwrPins.uiTccRstPin     ;

    /*STR_MODE*/
    if(PMIO_EXTN_IsGpk(gtExtPwrPins.uiStrModePin) == TRUE)
    {
        uiGpkNum = PMIO_EXTN_GetGpkNum(gtExtPwrPins.uiStrModePin);
        PMIO_REG_PMGPIO_FS  &=  (~(1UL<< uiGpkNum)); //0: pin controlled by PMGPIO
        PMIO_REG_PMGPIO_EN  |=    (1UL<< uiGpkNum);  //1: out
    }
    else
    {
        mcu_printf("Warnning! STR pin must be GPIO_K port.\n");
    }

    /*PWR_EN*/
    if(PMIO_EXTN_IsGpk(gtExtPwrPins.uiPwrEnPin) == TRUE)
    {
        uiGpkNum = PMIO_EXTN_GetGpkNum(gtExtPwrPins.uiPwrEnPin);
        PMIO_REG_PMGPIO_FS  |=  (1UL<< uiGpkNum ); //1: pin controlled by GPIO
        PMIO_REG_PMGPIO_EN  |=  (1UL<< uiGpkNum );  //1: out
    }
    ( void ) GPIO_Config(
            gtExtPwrPins.uiPwrEnPin,
            ( uint32 ) ( GPIO_FUNC( 0U ) | GPIO_OUTPUT )
            );

#if 0
    /*ALIVE_PWR_CON*/
    if(PMIO_EXTN_IsGpk(gtExtPwrPins.uiAlivePwrConPin) == TRUE)
    {
        uiGpkNum = PMIO_EXTN_GetGpkNum(gtExtPwrPins.uiAlivePwrConPin);
        PMIO_REG_PMGPIO_FS  |=  (1UL<< uiGpkNum ); //1: pin controlled by GPIO
        PMIO_REG_PMGPIO_EN  |=  (1UL<< uiGpkNum );  //1: out
    }
    ( void ) GPIO_Config(
            gtExtPwrPins.uiAlivePwrConPin,
            ( uint32 ) ( GPIO_FUNC( 0U ) | GPIO_OUTPUT )
            );
#endif

    /*PMIC_RESET_OUT*/
    if(PMIO_EXTN_IsGpk(gtExtPwrPins.uiPmicRstPin) == TRUE)
    {
        uiGpkNum = PMIO_EXTN_GetGpkNum(gtExtPwrPins.uiPmicRstPin);
        PMIO_REG_PMGPIO_FS  |=    (1UL<< uiGpkNum ); //1: pin controlled by GPIO
        PMIO_REG_PMGPIO_EN  &=  (~(1UL<< uiGpkNum ));  //0: in
    }
    ( void ) GPIO_Config(
            gtExtPwrPins.uiPmicRstPin,
            ( uint32 ) ( GPIO_FUNC( 0U ) | GPIO_INPUT|GPIO_INPUTBUF_EN )
            );

    /*TCC_RESET*/
    if(PMIO_EXTN_IsGpk(gtExtPwrPins.uiTccRstPin) == TRUE)
    {
        uiGpkNum = PMIO_EXTN_GetGpkNum(gtExtPwrPins.uiTccRstPin);
        PMIO_REG_PMGPIO_FS  |=  (1UL<< uiGpkNum ); //1: pin controlled by GPIO
        PMIO_REG_PMGPIO_EN  |=  (1UL<< uiGpkNum );  //1: out
    }
    ( void ) GPIO_Config(
            gtExtPwrPins.uiTccRstPin,
            ( uint32 ) ( GPIO_FUNC( 0U ) | GPIO_OUTPUT )
            );
    (void)GPIO_Set(gtExtPwrPins.uiTccRstPin, 0UL);


    /*check PMIC_RESET_OUT*/
    (void)GPIO_IntExtSet(PMIO_EXTN_VA_INTERRUPT_SRC_PMIC_RST, gtExtPwrPins.uiPmicRstPin);
    (void)GIC_IntVectSet( \
            PMIO_EXTN_VA_INTERRUPT_SRC_PMIC_RST, \
            PMIO_EXTN_VA_INTERRUPT_PRIO_PMIC_RST, \
            GIC_INT_TYPE_LEVEL_HIGH, \
            &PMIO_EXTN_PmicResetInterruptHandler, \
            NULL_PTR );

    //(void)GIC_IntSrcEn(PMIO_EXTN_VA_INTERRUPT_SRC_PMIC_RST);
}

void PMIO_EXTN_SetStr
(
    uint8 ucEn
)
{
    uint32 uiGpkNum = 0UL;

    if(PMIO_EXTN_IsGpk(gtExtPwrPins.uiStrModePin) == TRUE)
    {
        uiGpkNum = PMIO_EXTN_GetGpkNum(gtExtPwrPins.uiStrModePin);

        if(ucEn == 0U)
        {
            PMIO_REG_PMGPIO_DAT &= (~(1UL<< uiGpkNum )); //0: Low
        }
        else
        {
            PMIO_REG_PMGPIO_DAT |=   (1UL<< uiGpkNum ); //1: High
        }
    }
    else
    {
        mcu_printf("Warnning! STR pin must be GPIO_K port.\n");
    }
}

void PMIO_EXTN_SetPower
(
    uint8 ucEn
)
{
    uint32 uiGpkNum = 0UL;

    if(ucEn == 0U)
    {
        uiGpkNum = PMIO_EXTN_GetGpkNum(gtExtPwrPins.uiStrModePin);
#if 1
        if((PMIO_REG_PMGPIO_DAT & (1UL<< uiGpkNum )) == 0x0UL)
        {
            (void)GPIO_Set(gtExtPwrPins.uiAlivePwrConPin, 0UL);
        }
        else
        {
            (void)GPIO_Set(gtExtPwrPins.uiAlivePwrConPin, 1UL);
        }
 #endif       
        (void)GPIO_Set(gtExtPwrPins.uiPwrEnPin, 0UL);

        if(gucTccRstIrqStatus == 1U)
        {
            gucTccRstIrqStatus = 0U;
            (void)GIC_IntSrcDis(PMIO_EXTN_VA_INTERRUPT_SRC_PMIC_RST);
        }
    }
    else
    {
//        (void)GPIO_Set(gtExtPwrPins.uiAlivePwrConPin, 1UL);
        (void)GPIO_Set(gtExtPwrPins.uiPwrEnPin, 1UL);

        if(gucTccRstIrqStatus == 0U)
        {
            gucTccRstIrqStatus = 1U;
            (void)GIC_IntSrcEn(PMIO_EXTN_VA_INTERRUPT_SRC_PMIC_RST);
        }
    }
}

uint32 PMIO_EXTN_GetUsingGpk32
(
    void
)
{
    return guiGpk32;
}


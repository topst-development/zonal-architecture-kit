// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gic.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_GIC_HEADER
#define MCU_BSP_GIC_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_GIC == 1 )

#include <sal_com.h>
#include <gic_enum.h>
/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

typedef void (*GICIsrFunc)
(
    void *                              pArg
);
/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                       GIC_Init
*
* Init GIC400
*
* @param    none
*
* @return   none
*
* Notes
*               Caller(s) : GIC_Init().
***************************************************************************************************
*/

void GIC_Init
(
    void
);

/*
***************************************************************************************************
*                                       GIC_DeInit
*
* DeInit GIC400
*
* @param    none
*
* @return   none
*
* Notes
*               Caller(s) : GIC_DeInit().
***************************************************************************************************
*/

void GIC_DeInit
(
    void
);

/*
***************************************************************************************************
*                                       GIC_IntSrcEn
*
* Enable interrupt source id.
*
* @param    uiIntId  [in] Index of Interrupt Source id.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt enable succeed.
*           SAL_RET_FAILED   Error enabling interrupt.
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GIC_IntSrcEn
(
    uint32                              uiIntId
);

/*
***************************************************************************************************
*                                       GIC_IntSrcDis
*
* Disable interrupt source id.
*
* @param    uiIntId  [in] Index of Interrupt Source id.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt disable succeed.
*           SAL_RET_FAILED   Error disabling interrupt.
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GIC_IntSrcDis
(
    uint32                              uiIntId

);


/*
***************************************************************************************************
*                                       GIC_IntPrioSet
*
* Set interrupt priority.
*
* @param    uiIntId [in] Index of Interrupt Source id.
* @param    uiPrio  [in] Interrupt priority.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt priority set succeed.
*           SAL_RET_FAILED   Error setting interrupt priority.
* Notes     internal function
*
***************************************************************************************************
*/

SALRetCode_t GIC_IntPrioSet
(
    uint32                              uiIntId,
    uint32                              uiPrio
);

/*
***************************************************************************************************
*                                       GIC_IntVectSet
*
* Configure interrupt vector.
*
* @param    uiIntId   [in] Index of Interrupt Source id.
* @param    uiPrio    [in] Interrupt priority. (0~15)
* @param    ucIntType [in] Interrupt trigger type.
* @param    fnIntFunc [in] ISR function pointer.
* @param    pIntArg   [in] ISR function Argument
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt successfully set.
*           SAL_RET_FAILED   Error setting interrupt.
*
* Notes
*
***************************************************************************************************
*/


SALRetCode_t GIC_IntVectSet
(
    uint32                              uiIntId,
    uint32                              uiPrio,
    uint8                               ucIntType,
    GICIsrFunc                          fnIntFunc,
    void*                               pIntArg
);

/*
***************************************************************************************************
*                                       GIC_IntHandlerWithParam
*
*  Call the ISR function registered in the GICIsrTable that matches the interrupt source id.
*
* @param    uiGICC_IAR [in] uiGICC_IAR
*
* @return   none
*
* Notes
*               This function used in freeRTOS
***************************************************************************************************
*/



void GIC_IntHandlerWithParam
(
    uint32                              uiGICC_IAR
);

/*
***************************************************************************************************
*                                       GIC_IntHandler
*
*  Call the ISR function registered in the GICIsrTable that matches the interrupt source id.
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/



void GIC_IntHandler
(
    void
);


/*
***************************************************************************************************
*                                       GIC_IntSGI
*
* Controls the generation of SGIs
*
* @param    uiIntId   [in] Index of Interrupt Source id(SGI).
*
* @return
*           SAL_RET_SUCCESS  register setting succeed
*           SAL_RET_FAILED   register setting fail(out of range)
*
* Notes
*           This function can be used to test the operation of GIC.
***************************************************************************************************
*/

SALRetCode_t GIC_IntSGI
(
    uint32                              uiIntId
);

/*
***************************************************************************************************
*                                       GIC_SmIrqEn
*
* System Safety Mechanism (SM)'s SW_IRQn(GIC SPI) Enable or Disable
*
* @param    uiIntId   [in] Index of Interrupt Source id.
* @param    bEnable   [in] SM SW interrupt 1 : Enable, 0 : Disable
*
* @return   The result of SM SW Interrupt setting
*           SAL_RET_SUCCESS  register setting succeed
*           SAL_RET_FAILED   register setting fail
*
* Notes
*           This function can be used to test the operation of GIC.
***************************************************************************************************
*/


SALRetCode_t GIC_SmIrqEn
(
    uint32                              uiIntId,
    boolean                             bEnable
);

/*
***************************************************************************************************
*                                       GIC_SmIrqSetHigh
*
* System Safety Mechanism (SM)'s SW_IRQn(GIC SPI) input to GIC or not
*
* @param    uiIntId   [in] Index of Interrupt Source id.
* @param    bEnable   [in] SM SW interrupt If bEnable is 1, SW_IRQn(GIC SPI) input to GIC, 0 :
*
* @return   The result of SM SW Interrupt setting
*           SAL_RET_SUCCESS  register setting succeed
*           SAL_RET_FAILED   register setting fail
*
* Notes
*           This function can be used to test the operation of GIC.
***************************************************************************************************
*/

SALRetCode_t GIC_SmIrqSetHigh
(
    uint32                              uiIntId,
    boolean                             bEnable
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_GIC == 1 )

#endif  // MCU_BSP_GIC_HEADER


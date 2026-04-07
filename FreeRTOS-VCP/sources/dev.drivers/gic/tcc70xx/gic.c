// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gic.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_GIC == 1 )

#include  <gic.h>

#include  <reg_phys.h>
#include  <gpio.h>


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/


#define GIC_DIST_BASE                   ((MCU_BSP_GIC_BASE) + (0x1000UL))
#define GIC_CPU_BASE                    ((MCU_BSP_GIC_BASE) + (0x2000UL))

#define GICD_CTRL                       (GIC_DIST_BASE)
#define GICD_ISENABLER_BASE             ((GIC_DIST_BASE) + (0x100u))
#define GICD_ICENABLER_BASE             ((GIC_DIST_BASE) + (0x180u))
#define GICD_IPRIORITYR_BASE            ((GIC_DIST_BASE) + (0x400u))
#define GICD_ICFGR_BASE                 ((GIC_DIST_BASE) + (0xC00u))
#define GICD_SGIR                       ((GIC_DIST_BASE) + (0xF00u))

#define GICC_CTRL                       (GIC_CPU_BASE)
#define GICC_PMR                        ((GIC_CPU_BASE) + (0x4u))
#define GICC_IAR                        ((GIC_CPU_BASE) + (0xCu))
#define GICC_EOIR                       ((GIC_CPU_BASE) + (0x10u))


#define GICD_IPRIORITYR(IDX)            ((GICD_IPRIORITYR_BASE) + ((IDX) >> (2u) << (2u)))
#define GICD_ICFGR(IDX)                 ((GICD_ICFGR_BASE) + ((IDX) >> (4u) << (2u)))
#define GICD_ISENABLER(IDX)             ((GICD_ISENABLER_BASE) + ((IDX) >> (5u) << (2u)))
#define GICD_ICENABLER(IDX)             ((GICD_ICENABLER_BASE) + ((IDX) >> (5u) << (2u)))

                                                            /* ----------- DISTRIBUTOR CONTROL REGISTER -----------                  */
#define  ARM_BIT_GIC_DIST_ICDDCR_EN     (0x00000001UL)      /* Global GIC enable.                                                    */

                                                            /* ---------- CPU_INTERFACE_CONTROL_REGISTER ----------                  */
#define  GIC_CPUIF_CTRL_ENABLEGRP0      (0x00000001UL)      /* Enable secure interrupts.                                             */
#define  GIC_CPUIF_CTRL_ENABLEGRP1      (0x00000002UL)      /* Enable non-secure interrupts.                                         */
#define  GIC_CPUIF_CTRL_ACKCTL          (0x00000004UL)      /* Secure ack of NS interrupts.                                          */


#define  GIC_SGI_TO_TARGETLIST          (0UL)

#define  GIC_SM_WRITEPROTECT            (MCU_BSP_SYSSM_BASE + 0x0A0UL)      /* SW Interrupt Configuration Write Password Register (SW_IRQ_CFG_WR_PW) */
#define  GIC_SM_PASSWORD                (0x5AFEACE5UL)

#define  GIC_SM_IRQ_EN                  (MCU_BSP_SYSSM_BASE + 0x080UL)      /* SM (SW_IRQ_ENn, n =0~14) 0x1B9210B0 + (0x4 x n)                       */
#define  GIC_SM_IRQ_ASSERT              (MCU_BSP_SYSSM_BASE + 0x090UL)      /* SM Interrupt-n Register (SW_IRQn, n= 0~14) 0x1B9210EC + (0x4 x n)     */


/**************************************************************************************************/
/*                                             LOCAL VARIABLES                                    */
/***************************************************************************************************/


typedef struct GICIntFuncPtr
{
    GICIsrFunc                          ifpFunc;
    uint8                               ifpIsBothEdge;
    void *                              ifpArg;
}GICIntFuncPtr_t;

static GICIntFuncPtr_t                  GICIsrTable[GIC_INT_SRC_CNT];        /* Interrupt vector table.                              */


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static SALRetCode_t GIC_IntPrioSet_internal
(
    uint32                              uiIntId,
    uint32                              uiPrio
);

static SALRetCode_t GIC_IntConfigSet
(
    uint32                              uiIntId,
    uint8                               ucIntType
);

static uint32 GIC_IntAck
(
    void
);

static void GIC_IntEOI
(
    uint32                              uiIntId
);

static inline uint32 GIC_READREG
(
    uint32 uiAddr
);

static inline void GIC_WRITEREG
(
    uint32                              uiValue,
    uint32                              uiAddr
);

/*
***************************************************************************************************
*                                       GIC_READREG
*
* Set interrupt priority.
*
* @param    uiAddr  [in] Register Address
*
* @return   Register Value
*
* Notes     internal function, To avoid frequent CERT INT36-C
*
***************************************************************************************************
*/

static inline uint32 GIC_READREG
(
    uint32 uiAddr
)
{
    uint32 uiRet = 0u;
    uiRet = SAL_ReadReg(uiAddr);
    return uiRet;
}

/*
***************************************************************************************************
*                                       GIC_WRITEREG
*
* Set interrupt priority.
*
* @param    uiValue [in] Register Value
* @param    uiAddr  [in] Register Address
*
* @return   None
*
* Notes     internal function, To avoid frequent CERT INT36-C
*
***************************************************************************************************
*/

static inline void GIC_WRITEREG
(
    uint32                              uiValue,
    uint32                              uiAddr
)
{
    SAL_WriteReg(uiValue, uiAddr);
    return;
}


/*
***************************************************************************************************
*                                       GIC_IntPrioSet_internal
*
* Set interrupt priority.
*
* @param    uiIntId [in] Index of Interrupt Source id.
* @param    uiPrio  [in] Interrupt priority.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt priority set succeed.
*           SAL_RET_FAILED   Error setting interrupt priority.
*
* Notes     internal function
*
***************************************************************************************************
*/

static SALRetCode_t GIC_IntPrioSet_internal
(
    uint32                              uiIntId,
    uint32                              uiPrio
)
{
    uint32        uiTargetAddr   = 0u;
    uint32        uiTargetValue  = 0u;
    uint32        uiRegBitField  = 0u;
    SALRetCode_t  ucRet          = (SALRetCode_t)SAL_RET_FAILED;

    if ( (uiPrio < GIC_PRIORITY_NO_MEAN) && (uiIntId < GIC_INT_SRC_CNT) )
    {
        uiTargetAddr  = GICD_IPRIORITYR(uiIntId);
        uiTargetValue = GIC_READREG(uiTargetAddr);
        uiRegBitField = ((uiIntId & 0x03u) * 8u);
        uiTargetValue = (uiTargetValue & ~((uint32)0xFFu << (uiRegBitField) ) );
        uiTargetValue = (uiTargetValue | (((uiPrio<<4) & 0xF0u) << (uiRegBitField) ) );
        GIC_WRITEREG(uiTargetValue, uiTargetAddr);

        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}


/*
***************************************************************************************************
*                                       GIC_IntConfigSet
*
* Set interrupt trigger type.
*
* @param    uiIntId  [in] Index of Interrupt Source id.
* @param    ucIntType[in] Interrupt trigger type.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt trigger type set succeed.
*           SAL_RET_FAILED   Error setting interrupt trigger type.
*
* Notes     internal function
*
***************************************************************************************************
*/

static SALRetCode_t GIC_IntConfigSet
(
    uint32                          uiIntId,
    uint8                           ucIntType
)
{
    uint32        uiTargetAddr   = 0u;
    uint32        uiTargetValue  = 0u;
    uint32        uiRegBitField  = 0u;
    uint32        uiRegMask      = 0u;

    SALRetCode_t  ucRet          = (SALRetCode_t)SAL_RET_FAILED;

    if ( uiIntId < GIC_INT_SRC_CNT)
    {
        uiTargetAddr  = GICD_ICFGR(uiIntId);
        uiTargetValue = GIC_READREG(uiTargetAddr);
        uiRegBitField = ((uiIntId & 0xfu) * 2u);
        uiRegMask     = ((uint32)0x2u << (uiRegBitField));

        if (((ucIntType & (uint8)GIC_INT_TYPE_LEVEL_HIGH)== (uint8)GIC_INT_TYPE_LEVEL_HIGH)  || \
            ((ucIntType & (uint8)GIC_INT_TYPE_LEVEL_LOW)== (uint8)GIC_INT_TYPE_LEVEL_LOW))
        {
            uiTargetValue = (uint32)(uiTargetValue & ~uiRegMask);
        }
        else
        {
            uiTargetValue = (uint32)(uiTargetValue | uiRegMask);
        }

        GIC_WRITEREG(uiTargetValue, uiTargetAddr);
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}



/*
***************************************************************************************************
*                                       GIC_IntAck
*
* obtain the interrupt ID
*
* @param    none
*
* @return   return GICC_IAR
*
* Notes     internal function
*           Caller(s)   : GIC_IntHandler().
***************************************************************************************************
*/

static uint32  GIC_IntAck
(
    void
)
{
    uint32 uiIntID;

    uiIntID = GIC_READREG(GICC_IAR);

    return uiIntID;
}

/*
***************************************************************************************************
*                                       GIC_IntEOI
*
* inform End of interrupt to CPU Interface of GIC
*
* @param    uiIntId  [in] Index of Interrupt Source id.
*
* @return   none
*
* Notes     internal function
*               Caller(s) : GIC_IntHandler().
***************************************************************************************************
*/

static void GIC_IntEOI
(
    uint32                              uiIntId
)
{
    GIC_WRITEREG(uiIntId, GICC_EOIR);

    return;
}


/**************************************************************************************************
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
*               set default priority value to 0xA0
**************************************************************************************************/

void GIC_Init
(
    void
)
{
    uint32        uiRegOffset    = 0u;
    uint32        uiTargetAddr   = 0u;
    uint32        uiTargetValue  = 0u;

    uiTargetValue = GIC_READREG(GICD_CTRL);
    uiTargetValue = (uiTargetValue & (uint32)(~ARM_BIT_GIC_DIST_ICDDCR_EN));
    GIC_WRITEREG(uiTargetValue, GICD_CTRL);

    uiTargetValue = GIC_READREG(GICD_CTRL);
    uiTargetValue = (uiTargetValue | (uint32)ARM_BIT_GIC_DIST_ICDDCR_EN);
    GIC_WRITEREG(uiTargetValue, GICD_CTRL);

    for (; uiRegOffset <= ((uint32)(GIC_INT_SRC_CNT - 1UL) / 4UL); uiRegOffset++)
    {
        uiTargetAddr = (GICD_IPRIORITYR_BASE + (uiRegOffset * 4u));
        GIC_WRITEREG(0xA0A0A0A0u, uiTargetAddr);
    }

    GIC_WRITEREG(0xFFu, GICC_PMR);

    uiTargetValue = GIC_READREG(GICC_CTRL);
    uiTargetValue = (uiTargetValue | (uint32)GIC_CPUIF_CTRL_ENABLEGRP0);
    GIC_WRITEREG(uiTargetValue, GICC_CTRL);

    (void)SAL_CoreMB();

    return;
}

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
)
{
    uint32        uiTargetAddr   = 0u;
    uint32        uiTargetValue  = 0u;
    SALRetCode_t  ucRet          = (SALRetCode_t)SAL_RET_FAILED;;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        uiTargetAddr = GICD_ISENABLER(uiIntId);
        uiTargetValue = ((uint32)1UL << (uiIntId & 0x1Fu));
        GIC_WRITEREG(uiTargetValue, uiTargetAddr);

        if (GICIsrTable[uiIntId].ifpIsBothEdge == (1UL))
        {
            uiTargetAddr = GICD_ISENABLER((uiIntId + 10UL));
            uiTargetValue = ((uint32)1UL << ((uiIntId + 10UL) & 0x1Fu));
            GIC_WRITEREG(uiTargetValue, uiTargetAddr);

        }

        (void)SAL_CoreMB();
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}



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
)
{
    uint32        uiTargetAddr   = 0u;
    uint32        uiTargetValue  = 0u;
    SALRetCode_t  ucRet          = (SALRetCode_t)SAL_RET_FAILED;;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        uiTargetAddr = GICD_ICENABLER(uiIntId);
        uiTargetValue = ((uint32)1UL << (uiIntId & 0x1Fu));
        GIC_WRITEREG(uiTargetValue, uiTargetAddr);

        if (GICIsrTable[uiIntId].ifpIsBothEdge == (1UL))
        {
            uiTargetAddr = GICD_ICENABLER((uiIntId + 10UL));
            uiTargetValue = ((uint32)1UL << ((uiIntId + 10UL) & 0x1Fu));
            GIC_WRITEREG(uiTargetValue, uiTargetAddr);
        }

        (void)SAL_CoreMB();
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}



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
)
{
    SALRetCode_t ucRet;

    (void)SAL_CoreCriticalEnter();
    ucRet = GIC_IntPrioSet_internal(uiIntId, uiPrio);
    (void)SAL_CoreCriticalExit();
    return ucRet;
}



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
    void *                              pIntArg
)
{
    uint32        uiRevIntId     = 0u;
    SALRetCode_t  ucRet          = (SALRetCode_t)SAL_RET_SUCCESS;

    if((uiPrio > GIC_PRIORITY_NO_MEAN)||(uiIntId >= GIC_INT_SRC_CNT))
    {
        ucRet = (SALRetCode_t)SAL_RET_FAILED;
    }
    else
    {
        (void)SAL_CoreCriticalEnter();                                      /* Prevent partially configured interrupts.              */
        (void)GIC_IntPrioSet_internal(uiIntId, uiPrio);
        (void)GIC_IntConfigSet(uiIntId, ucIntType);

        GICIsrTable[uiIntId].ifpFunc = fnIntFunc;
        GICIsrTable[uiIntId].ifpArg = pIntArg;
        GICIsrTable[uiIntId].ifpIsBothEdge = 0;

        if ((uiIntId >= GIC_EINT_START_INT) && (uiIntId <= GIC_EINT_END_INT)  /* Set reversed external interrupt     */
            && (ucIntType == GIC_INT_TYPE_EDGE_BOTH))
        {                                                                   /* for supporting both edge.                             */

            uiRevIntId = (uiIntId + GIC_EINT_NUM);                          /* add offset of IRQn                                    */

            (void)GIC_IntPrioSet_internal(uiRevIntId, uiPrio);
            (void)GIC_IntConfigSet(uiRevIntId, ucIntType);

            GICIsrTable[uiRevIntId].ifpFunc = fnIntFunc;
            GICIsrTable[uiRevIntId].ifpArg = pIntArg;
            GICIsrTable[uiIntId].ifpIsBothEdge = (1U);
        }

        (void)SAL_CoreCriticalExit();
    }

    return ucRet;
}



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
)
{
    uint32        uiIAR          = uiGICC_IAR;
    uint32        uiIntId        = (uiIAR & 0x3FFU);         /* Mask away the CPUID.                                                  */
    GICIsrFunc    fnIsr          = (GICIsrFunc)NULL;
    void          *pIntArg       = NULL_PTR;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        fnIsr   = GICIsrTable[uiIntId].ifpFunc;             /* Fetch ISR handler.                                                    */
        pIntArg = GICIsrTable[uiIntId].ifpArg;

        if(fnIsr != (GICIsrFunc)NULL)
        {
            (*fnIsr)(pIntArg);                              /* Call ISR handler.                                                     */
        }

        (void)SAL_CoreMB();                                 /* Memory barrier before ending the interrupt.                           */
    }

    return;
}


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
)
{
    uint32        uiIAR          = 0u;
    uint32        uiIntId        = 0u;
    GICIsrFunc    fnIsr          = (GICIsrFunc)NULL;
    void          *pIntArg       = NULL_PTR;

    uiIAR   = GIC_IntAck();
    uiIntId = (uiIAR & 0x3FFu);                               /* Mask away the CPUID.                                                  */

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        fnIsr   = GICIsrTable[uiIntId].ifpFunc;             /* Fetch ISR handler.                                                    */
        pIntArg = GICIsrTable[uiIntId].ifpArg;

        if(fnIsr != (GICIsrFunc)NULL)
        {
            (*fnIsr)(pIntArg);                              /* Call ISR handler.                                                     */
        }

        (void)SAL_CoreMB();                                 /* Memory barrier before ending the interrupt.                           */
        GIC_IntEOI(uiIAR);
    }

    return;
}

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
)
{
    SALRetCode_t ucRet              = (SALRetCode_t)SAL_RET_FAILED;
    uint32       uiTargetValue      = 0u;
    uint32       uiTargetListFilter = GIC_SGI_TO_TARGETLIST;
    uint32       uiCPUTargetList    = 0x1UL;                /* bitfiled 0 : cpu #0, bitfield n : cpu #n, n : 0 ~ 7          */
    uint32       uiNASTT            = 0UL;                  /* 0 : group 0 , 1: group 1                                     */

    if(uiIntId <= 15UL)
    {
        uiTargetValue = (uint32)((uiTargetListFilter & 0x3UL) << 24) | ((uiCPUTargetList & 0xffUL)<<16)\
                          | ((uiNASTT & 0x1UL)<<15) | (uiIntId & 0xfUL);
        GIC_WRITEREG(uiTargetValue, GICD_SGIR);

        ucRet           = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}

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
)
{
    uint32        uiSmIrqEn      = 0;
    uint32        uiRegOffset    = 0;
    uint32        uiBit          = 0;
    uint32        uiIntIdForSm   = 0;
    SALRetCode_t  ucRet          = (SALRetCode_t)SAL_RET_FAILED;;

    if((uiIntId >= GIC_SPI_START) && (uiIntId < GIC_INT_SRC_CNT))
    {
        uiIntIdForSm = (uiIntId - GIC_SPI_START);
        uiRegOffset = (uint32)(((uiIntIdForSm & ~0x1FUL)>>5UL) << 2UL);

        uiBit = ((uint32)0x1UL << (uiIntIdForSm & 0x1FUL));

        uiSmIrqEn = GIC_READREG((uint32)(GIC_SM_IRQ_EN + uiRegOffset));

        if(bEnable == (boolean)FALSE)
        {
            uiSmIrqEn = (uiSmIrqEn & ~uiBit);
        }
        else
        {
            uiSmIrqEn = (uiSmIrqEn | uiBit);
        }

        GIC_WRITEREG(GIC_SM_PASSWORD, GIC_SM_WRITEPROTECT);
        GIC_WRITEREG(uiSmIrqEn, (uint32)(GIC_SM_IRQ_EN + uiRegOffset));
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}

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
)
{
    uint32        uiSmIrqAssert  = 0;
    uint32        uiRegOffset    = 0;
    uint32        uiBit          = 0;
    uint32        uiIntIdForSm   = 0;
    SALRetCode_t  ucRet          = (SALRetCode_t)SAL_RET_FAILED;

    if((uiIntId >= GIC_SPI_START) && (uiIntId < GIC_INT_SRC_CNT))
    {
        uiIntIdForSm = (uiIntId - GIC_SPI_START);
        uiRegOffset  = (uint32)(((uiIntIdForSm & ~0x1FUL)>>5UL) << 2UL);
        uiBit        = ((uint32)0x1UL << (uiIntIdForSm & 0x1FUL));

        uiSmIrqAssert = GIC_READREG((uint32)(GIC_SM_IRQ_ASSERT + uiRegOffset));

        if(bEnable == (boolean)FALSE)
        {
            uiSmIrqAssert = (uint32)(uiSmIrqAssert & ~uiBit);
        }
        else
        {
            uiSmIrqAssert = (uint32)(uiSmIrqAssert | uiBit);
        }


        GIC_WRITEREG(GIC_SM_PASSWORD, GIC_SM_WRITEPROTECT);
        GIC_WRITEREG(uiSmIrqAssert,   (uint32)(GIC_SM_IRQ_ASSERT + uiRegOffset));
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_GIC == 1 )


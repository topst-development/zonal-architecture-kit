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
#include <sal_impl.h>


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

                                                            /* ---------------- GIC DISTRIBUTOR INTERFACE -------------------------- */
typedef  struct  GICDistributor
{
    uint32                              dCTRL;              /* GICD_CTLR        Distributor Control Register.                        */
    uint32                              dTYPER;             /* GICD_TYPER       Interrupt Controller Type Register.                  */
    uint32                              dIIDR;              /* GICD_IIDR        Distributor Implementer Identification Register.     */
    uint32                              dRSVD1[29];         /* Reserved.                                                             */
    uint32                              dIGROUPRn[32];      /* GICD_IGROUPRn    Interrupt Security Registers.                        */
    uint32                              dISENABLERn[32];    /* GICD_ISENABLERn  Interrupt Set-Enable Registers.                      */
    uint32                              dICENABLERn[32];    /* GICD_ICENABLERn  Interrupt Clear-Enable Registers.                    */
    uint32                              dISPENDRn[32];      /* GICD_ISPENDRn    Interrupt Set-Pending Registers.                     */
    uint32                              dICPENDRn[32];      /* GICD_ICPENDRn    Interrupt Clear-Pending Registers.                   */
    uint32                              dISACTIVERn[32];    /* GICD_ISACTIVERn  Interrupt Set-Active Registers.                      */
    uint32                              dICACTIVERn[32];    /* GICD_ICACTIVERn  Interrupt Clear-Active Registers.                    */
    uint32                              dIPRIORITYRn[255];  /* GICD_IPRIORITYRn Interrupt Priority Registers.                        */
    uint32                              dRSVD3[1];          /* Reserved.                                                             */
    uint32                              dITARGETSRn[255];   /* GICD_ITARGETSRn  Interrupt Processor Target Registers.                */
    uint32                              dRSVD4[1];          /* Reserved.                                                             */
    uint32                              dICFGRn[64];        /* GICD_ICFGRn      Interrupt Configuration Registers.                   */
    uint32                              dPPISR[1];          /* GICD_PPISR.                                                           */
    uint32                              dSPISRn[15];        /* GICD_SPISRn                                                           */
    uint32                              dRSVD5[112];        /* Reserved.                                                             */
    uint32                              dSGIR;              /* GICD_SGIR        Software Generate Interrupt Register.                */
    uint32                              dRSVD6[3];          /* Reserved.                                                             */
    uint32                              dCPENDSGIRn[4];     /* GICD_CPENDSGIRn  SGInterrupt Clear-Active Registers.                  */
    uint32                              dSPENDSGIRn[4];     /* GICD_SPENDSGIRn  SGInterrupt Set-Active Registers.                    */
} GICDistributor_t;


                                                            /* ---------------- GIC CPU INTERFACE ---------------------------------- */
typedef  struct  GICCpuInterface
{
    uint32                              cCTLR;              /* GICC_CTLR        CPU Interface Control Register.                      */
    uint32                              cPMR;               /* GICC_PMR         Interrupt Priority Mask Register.                    */
    uint32                              cBPR;               /* GICC_BPR         Binary Point Register.                               */
    uint32                              cIAR;               /* GICC_IAR         Interrupt Acknowledge Register.                      */
    uint32                              cEOIR;              /* GICC_EOIR        End Interrupt Register.                              */
    uint32                              cRPR;               /* GICC_RPR         Running Priority Register.                           */
    uint32                              cHPPIR;             /* GICC_HPPIR       Highest Pending Interrupt Register.                  */
    uint32                              cABPR;              /* GICC_ABPR        Aliased Binary Point Register.                       */
    uint32                              cAIAR;              /* GICC_AIAR        Aliased Interrupt Acknowledge Register               */
    uint32                              cAEOIR;             /* GICC_AEOIR       Aliased End Interrupt Register.                      */
    uint32                              cAHPPIR;            /* GICC_AHPPIR      Aliased Highest Pending Interrupt Register.          */
    uint32                              cRSVD[52];          /* Reserved.                                                             */
    uint32                              cIIDR;              /* GICC_IIDR        CPU Interface Identification Register.               */
} GICCpuInterface_t;

/* 0x1b903000u GICC_DIR  */
#define  GIC_DIST_BASE                  (MCU_BSP_GIC_BASE + 0x1000UL)
#define  GIC_CPU_BASE                   (MCU_BSP_GIC_BASE + 0x2000UL)


#define  GIC_DIST                       ((volatile GICDistributor_t  *)(GIC_DIST_BASE))
#define  GIC_CPU                        ((volatile GICCpuInterface_t *)(GIC_CPU_BASE))


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
    uint32        uiRegOffset;
    uint32        uiRegBitField;
    uint32        uiGICD_IPRIORITYRn;
    SALRetCode_t  ucRet;

    uiRegOffset        = 0;
    uiRegBitField      = 0;
    uiGICD_IPRIORITYRn = 0;
    ucRet              = (SALRetCode_t)SAL_RET_FAILED;

    if ( (uiPrio < GIC_PRIORITY_NO_MEAN) && (uiIntId < GIC_INT_SRC_CNT) )
    {
        uiRegOffset = (uiIntId >> 2u);
        uiRegBitField = (uiIntId & 0x03u);

        uiGICD_IPRIORITYRn = GIC_DIST->dIPRIORITYRn[uiRegOffset];
        uiGICD_IPRIORITYRn = (uint32)(uiGICD_IPRIORITYRn & ~((uint32)0xFFu << (uiRegBitField * 8u)));
        uiGICD_IPRIORITYRn = (uint32)(uiGICD_IPRIORITYRn | (((uiPrio<<4) & 0xF0u) << (uiRegBitField * 8u)));

        GIC_DIST->dIPRIORITYRn[uiRegOffset] = uiGICD_IPRIORITYRn;
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
    uint32         uiRegOffset;
    uint32         uiRegMask;
    uint32         uiGICD_ICFGRn;
    SALRetCode_t   ucRet;

    uiRegOffset   = 0;
    uiRegMask     = 0;
    uiGICD_ICFGRn = 0;
    ucRet         = (SALRetCode_t)SAL_RET_FAILED;

    if ( uiIntId < GIC_INT_SRC_CNT)
    {
        uiRegOffset   = (uiIntId >> 4u);
        uiRegMask     = (uint32)((uint32)0x2u << ((uiIntId & 0xfu) * 2u));
        uiGICD_ICFGRn = GIC_DIST->dICFGRn[uiRegOffset];

        if (((ucIntType & (uint8)GIC_INT_TYPE_LEVEL_HIGH)== (uint8)GIC_INT_TYPE_LEVEL_HIGH)  || \
            ((ucIntType & (uint8)GIC_INT_TYPE_LEVEL_LOW)== (uint8)GIC_INT_TYPE_LEVEL_LOW))
        {
            uiGICD_ICFGRn = (uint32)(uiGICD_ICFGRn & ~uiRegMask);
        }
        else
        {
            uiGICD_ICFGRn = (uint32)(uiGICD_ICFGRn | uiRegMask);
        }

        GIC_DIST->dICFGRn[uiRegOffset] = uiGICD_ICFGRn;
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
    uint32 int_id;

    int_id = GIC_CPU->cIAR;

    return (int_id);
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
    GIC_CPU->cEOIR = uiIntId;
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
    uint32 uiRegOffset;

    uiRegOffset = 0;

    GIC_DIST->dCTRL    &= (uint32)(~ARM_BIT_GIC_DIST_ICDDCR_EN);
    GIC_DIST->dCTRL    |= (uint32)ARM_BIT_GIC_DIST_ICDDCR_EN;

    for (; uiRegOffset <= ((uint32)(GIC_INT_SRC_CNT - 1UL) / 4UL); uiRegOffset++)
    {
        GIC_DIST->dIPRIORITYRn[uiRegOffset] = 0xA0A0A0A0UL;
    }

    GIC_CPU->cPMR  = 0xFFUL;
    GIC_CPU->cCTLR |= GIC_CPUIF_CTRL_ENABLEGRP0;
    (void)SAL_CoreMB();

    return;
}


/**************************************************************************************************
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
*               Disable all interrupts
**************************************************************************************************/

void GIC_DeInit
(
    void
)
{
    uint32 uiIntId;

    for( uiIntId = GIC_CAN0_0; uiIntId < GIC_MAX; uiIntId++ )
    {
        GIC_IntSrcDis( uiIntId );
    }
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
    SALRetCode_t ucRet;
    uint32       uiRegOffset;
    uint32       uiBit;

    ucRet       = (SALRetCode_t)SAL_RET_FAILED;
    uiRegOffset = 0;
    uiBit       = 0;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        uiRegOffset = (uiIntId >> 5u);                      /* Calculate the register offset.                                        */
        uiBit       = (uiIntId & 0x1Fu);                    /* Mask bit ID.                                                          */

        GIC_DIST->dISENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);

        if (GICIsrTable[uiIntId].ifpIsBothEdge == (1UL))
        {
            uiRegOffset = ((uiIntId + 10UL) >> 5UL);        /* Calculate the register offset.                                        */
            uiBit       = ((uiIntId + 10UL) & 0x1FUL);      /* Mask bit ID.                                                          */

            GIC_DIST->dISENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);
        }

        (void)SAL_CoreMB();
        ucRet           = (SALRetCode_t)SAL_RET_SUCCESS;
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
    SALRetCode_t   ucRet;
    uint32         uiRegOffset;
    uint32         uiBit;

    ucRet       = (SALRetCode_t)SAL_RET_FAILED;
    uiRegOffset = 0;
    uiBit       = 0;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        uiRegOffset = (uiIntId >> 5UL);                       /* Calculate the register offset.                                      */
        uiBit       = (uiIntId & 0x1FUL);                     /* Mask bit ID.                                                        */

        GIC_DIST->dICENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);

        if (GICIsrTable[uiIntId].ifpIsBothEdge == (1UL))
        {
            uiRegOffset = ((uiIntId + 10UL) >> 5UL);          /* Calculate the register offset.                                      */
            uiBit       = ((uiIntId + 10UL) & 0x1FUL);        /* Mask bit ID.                                                        */

            GIC_DIST->dICENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);
        }
        ucRet           = (SALRetCode_t)SAL_RET_SUCCESS;
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

    ucRet = (SALRetCode_t)SAL_RET_SUCCESS;

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
    uint32        uiRevIntId;
    SALRetCode_t  ucRet;

    uiRevIntId = 0;
    ucRet      = (SALRetCode_t)SAL_RET_SUCCESS;

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

        if ((uiIntId >= (uint32)GIC_EINT_START_INT) && (uiIntId <= (uint32)GIC_EINT_END_INT)  /* Set reversed external interrupt     */
            && (ucIntType == (uint8)GIC_INT_TYPE_EDGE_BOTH))
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
    uint32        uiIAR;
    uint32        uiIntId;
    GICIsrFunc    fnIsr;
    void          *pIntArg;

    uiIAR   = uiGICC_IAR;
    uiIntId = uiIAR & 0x3FFU;                               /* Mask away the CPUID.                                                  */
    fnIsr   = (GICIsrFunc)NULL;
    pIntArg = NULL_PTR;

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
    uint32        uiIAR;
    uint32        uiIntId;
    GICIsrFunc    fnIsr;
    void          *pIntArg;

    uiIAR   = GIC_IntAck();
    uiIntId = uiIAR & 0x3FFU;                               /* Mask away the CPUID.                                                  */
    fnIsr   = (GICIsrFunc)NULL;
    pIntArg = NULL_PTR;

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
    SALRetCode_t  ucRet;
    uint32        uiTargetListFilter;
    uint32        uiCPUTargetList;
    uint32        uiNASTT;

    ucRet              = (SALRetCode_t)SAL_RET_FAILED;
    uiTargetListFilter = GIC_SGI_TO_TARGETLIST;
    uiCPUTargetList    = 0x1UL;                             /* bitfiled 0 : cpu #0, bitfield n : cpu #n, n : 0 ~ 7          */
    uiNASTT            = 0UL;                               /* 0 : group 0 , 1: group 1                                     */

    if(uiIntId <= 15UL)
    {
        GIC_DIST->dSGIR = (uint32)((uiTargetListFilter & 0x3UL) << 24) | ((uiCPUTargetList & 0xffUL)<<16)\
                          | ((uiNASTT & 0x1UL)<<15) | (uiIntId & 0xfUL);

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
    uint32        uiSmIrqEn;
    uint32        uiRegOffset;
    uint32        uiBit;
    uint32        uiIntIdForSm;
    SALRetCode_t  ucRet;

    uiSmIrqEn    = 0;
    uiRegOffset  = 0;
    uiBit        = 0;
    uiIntIdForSm = 0;
    ucRet        = (SALRetCode_t)SAL_RET_FAILED;

    if(((uiIntId >= GIC_SPI_START) && (uiIntId < GIC_INT_SRC_CNT)) && (bEnable < (uint8)2))
    {
        uiIntIdForSm = (uiIntId - (uint32)GIC_SPI_START);
        uiRegOffset = (uint32)(((uiIntIdForSm & ~0x1FUL)>>5UL) << 2UL);

        uiBit = ((uint32)0x1UL << (uiIntIdForSm & 0x1FUL));

        uiSmIrqEn = SAL_ReadReg((uint32)(GIC_SM_IRQ_EN + uiRegOffset));

        if(bEnable == (uint8)0)
        {
            uiSmIrqEn = (uiSmIrqEn & ~uiBit);
        }
        else
        {
            uiSmIrqEn = (uiSmIrqEn | uiBit);
        }

        SAL_WriteReg(GIC_SM_PASSWORD, GIC_SM_WRITEPROTECT);
        SAL_WriteReg(uiSmIrqEn, (uint32)(GIC_SM_IRQ_EN + uiRegOffset));
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
    uint32        uiSmIrqAssert;
    uint32        uiRegOffset;
    uint32        uiBit;
    uint32        uiIntIdForSm;
    SALRetCode_t  ucRet;

    uiSmIrqAssert = 0;
    uiRegOffset   = 0;
    uiBit         = 0;
    uiIntIdForSm  = 0;
    ucRet         = (SALRetCode_t)SAL_RET_FAILED;

    if(((uiIntId >= GIC_SPI_START) && (uiIntId < GIC_INT_SRC_CNT)) && (bEnable < (uint8)2))
    {
        uiIntIdForSm = (uiIntId - (uint32)GIC_SPI_START);
        uiRegOffset  = (uint32)(((uiIntIdForSm & ~0x1FUL)>>5UL) << 2UL);
        uiBit        = ((uint32)0x1UL << (uiIntIdForSm & 0x1FUL));

        uiSmIrqAssert = SAL_ReadReg((uint32)(GIC_SM_IRQ_ASSERT + uiRegOffset));

        if(bEnable == (uint8)0)
        {
            uiSmIrqAssert = (uint32)(uiSmIrqAssert & ~uiBit);
        }
        else
        {
            uiSmIrqAssert = (uint32)(uiSmIrqAssert | uiBit);
        }


        SAL_WriteReg(GIC_SM_PASSWORD, GIC_SM_WRITEPROTECT);
        SAL_WriteReg(uiSmIrqAssert,   (uint32)(GIC_SM_IRQ_ASSERT + uiRegOffset));
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_GIC == 1 )


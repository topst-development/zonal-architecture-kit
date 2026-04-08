// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : dse.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_DSE == 1 )

#include <dse.h>
#include <gic.h>
#include <bsp.h>

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define DSE_D(fmt,args...)          {LOGD(DBG_TAG_DSE, fmt, ## args)}
    #define DSE_E(fmt,args...)          {LOGE(DBG_TAG_DSE, fmt, ## args)}
#else
    #define DSE_D(fmt,args...)
    #define DSE_E(fmt,args...)
#endif

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/


#define DSE_PW                          (0x5AFEACE5UL)


#define DSE_IRQ_STATUS                  (MCU_BSP_DSE_BASE + 0x000u)
#define DSE_IRQ_MASK                    (MCU_BSP_DSE_BASE + 0x004u)
#define DSE_IRQ_EN                      (MCU_BSP_DSE_BASE + 0x008u)
#define DEF_SLV_CFG                     (MCU_BSP_DSE_BASE + 0x00Cu)
#define DEF_SLV0_ADDR                   (MCU_BSP_DSE_BASE + 0x010u)
#define DSE_IH_CFG_WR_PW                (MCU_BSP_DSE_BASE + 0x040u)
#define DSE_IH_CFG_WR_LOCK              (MCU_BSP_DSE_BASE + 0x044u)
#define SF_CHK_GRP_EN                   (MCU_BSP_DSE_BASE + 0x048u)
#define SF_CHK_GRP_STS                  (MCU_BSP_DSE_BASE + 0x04Cu)









/**************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/

uint32                                  gWait_irq   = 0UL;    /* using by unittest                */

static SALAddr                          gDseErrAddr = 0UL;    /* the address that caused the error*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void DSE_IRQ_Isr
(
    void *                               pArg
);

static inline uint32 DSE_READREG(uint32 uiAddr)
{
    uint32 uiRet = 0u;
    uiRet = SAL_ReadReg(uiAddr);
    return uiRet;
}

static inline void DSE_WRITEREG(uint32 uiValue, uint32 uiAddr)
{
    SAL_WriteReg(uiValue, uiAddr);
    return;
}

/*
***************************************************************************************************
*                                       DSE_WriteLock
*
* Default Slave Error Register Write Protection Enable/Disable
*
* @param    uiLock [in] Write Protection 1 : Enable, 0 : Disable
*
* @return   The result of Write Protection Function setting
*           SAL_RET_SUCCESS  Write Protection register setting succeed
*           SAL_RET_FAILED   Write Protection register setting fail
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_WriteLock
(
    uint32                              uiLock
)
{
    uint32       uiConfPw;
    SALRetCode_t ucRet;

    ucRet                       = (SALRetCode_t)SAL_RET_SUCCESS;
    DSE_WRITEREG(DSE_PW, DSE_IH_CFG_WR_PW);
    uiConfPw                    = DSE_READREG(DSE_IH_CFG_WR_PW);

    if(uiConfPw == 1u)
    {
        if(uiLock == 1u)                        /* lock mode */
        {
            DSE_WRITEREG(1u, DSE_IH_CFG_WR_LOCK);
        }
        else
        {
            DSE_WRITEREG(0u, DSE_IH_CFG_WR_LOCK);
        }
    }
    else
    {
        ucRet = (SALRetCode_t)SAL_RET_FAILED;
        DSE_E("  Fail set password\n");
    }

    return ucRet;
}

/*
***************************************************************************************************
*                                       DSE_IRQ_Isr
*
* Default Slave Error Interrupt Service Routine.
*
* @param    pArg [in] none
*
* @return   none
*
* Notes     update error address to gDseErrAddr
*
***************************************************************************************************
*/

static void DSE_IRQ_Isr
(
    void *                              pArg
)
{
    uint32 uiDseSts;

    (void)pArg;
    uiDseSts = DSE_READREG(DSE_IRQ_STATUS);

    if(uiDseSts == DSE_INTERCON)
    {
        gDseErrAddr = DSE_READREG(DEF_SLV0_ADDR);
    }
    else
    {
        DSE_E("unexpected code flow\n");
    }

    DSE_D("##### Default Slave Error occurred(IRQ 0x%x), addr x%x\n", uiDseSts, gDseErrAddr);
    (void)DSE_WriteLock(0UL);

    DSE_WRITEREG(uiDseSts, DSE_IRQ_STATUS);
    (void)DSE_WriteLock(1UL);
    gWait_irq = 0UL;

    return;
}

/*
***************************************************************************************************
*                                       DSE_Deinit
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void DSE_Deinit
(
    void
)
{
    (void)GIC_IntVectSet
    (
        GIC_DEFAULT_SLV_ERR,
        GIC_PRIORITY_NO_MEAN,
        GIC_INT_TYPE_LEVEL_HIGH,
        NULL,
        NULL
    );
    (void)GIC_IntSrcDis(GIC_DEFAULT_SLV_ERR);
    DSE_WRITEREG(0u, DSE_IRQ_MASK);
    DSE_WRITEREG(0u, DSE_IRQ_EN);

    return;
}

/*
***************************************************************************************************
*                                       DSE_Init
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_Init
(
    uint32                              uiModeFlag
)
{
    uint32       uiSlvCfg = 0u;
    uint32       uiMode   = 0u;
    SALRetCode_t ucRet;

    (void)GIC_IntVectSet
    (
        GIC_DEFAULT_SLV_ERR,
        GIC_PRIORITY_NO_MEAN,
        GIC_INT_TYPE_EDGE_RISING,
        (GICIsrFunc)&DSE_IRQ_Isr,
        NULL
    );
    (void)GIC_IntSrcEn(GIC_DEFAULT_SLV_ERR);
    ucRet   = DSE_WriteLock(0UL);

    if(ucRet == SAL_RET_SUCCESS)
    {
        DSE_WRITEREG(0u, DSE_IRQ_MASK);
        DSE_WRITEREG(1u, DSE_IRQ_EN);

        uiMode = (uiModeFlag & DSE_MODE_MASK);

        if(uiMode == DSE_RES_MODE)
        {
            uiSlvCfg = (uiModeFlag & DES_SEL_MASK);
        }
        else if(uiMode == DSE_INT_MODE)
        {
            uiSlvCfg = ((uiModeFlag & DES_SEL_MASK)<<16UL);
        }
        else if(uiMode == DSE_RES_INT_MODE)
        {
            uiSlvCfg = ((uiModeFlag & DES_SEL_MASK) | ((uiModeFlag & DES_SEL_MASK)<<16UL));
        }
        else
        {
            uiSlvCfg = 0UL;
        }

        DSE_WRITEREG(uiSlvCfg, DEF_SLV_CFG);
        DSE_WRITEREG(DSE_PW ,DSE_IH_CFG_WR_PW);
        DSE_WRITEREG(DSE_SM_SF_CHK_GRP_SEL, SF_CHK_GRP_EN);

        ucRet = DSE_WriteLock(1UL);
    }



    return ucRet;
}


/*
***************************************************************************************************
*                                       DSE_GetGrpSts
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   rSF_CHK_GRP_STS : Soft Fault Check Group Status Register value
*
* Notes
*
***************************************************************************************************
*/
uint32 DSE_GetGrpSts
(
    void
)
{
    uint32 uiRet;

    uiRet = DSE_READREG(SF_CHK_GRP_STS);

    return uiRet;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_DSE == 1 )


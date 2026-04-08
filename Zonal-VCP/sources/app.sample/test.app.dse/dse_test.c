// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : dse_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )

#include <dse_test.h>
#include <bsp.h>
#include <dse.h>
#include <reg_phys.h>
#include <fmu.h>
#include <debug.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define DSETEST_SM_PW                   (0x5AFEACE5UL)
#define DSETEST_SM_SF_CTRL_PW           (MCU_BSP_SYSSM_BASE + 0x1C0UL)
#define DSETEST_SM_SF_CTRL_CFG          (MCU_BSP_SYSSM_BASE + 0x1F0UL)
#define DSETEST_SM_SF_CTRL_STS          (MCU_BSP_SYSSM_BASE + 0x1F4UL)

#define DSETEST_DSE_IRQ_MASK            (MCU_BSP_DSE_BASE + 0x04UL)
#define DSETEST_DSE_IRQ_EN              (MCU_BSP_DSE_BASE + 0x08UL)
#define DSETEST_DEF_SLV_CFG             (MCU_BSP_DSE_BASE + 0x0CUL)



/***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/



static uint32                           guiDseWaitIrq = 0UL;
static uint32                           guiFmuArg[2] = {0};
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void DSE_TEST_Fmu_IRQ_ISR
(
    void *                              pArg
);

static inline uint32 DSE_TEST_READREG(uint32 uiAddr)
{
    uint32 uiRet = 0u;
    uiRet = SAL_ReadReg(uiAddr);
    return uiRet;
}

static inline void DSE_TEST_WRITEREG(uint32 uiValue, uint32 uiAddr)
{
    SAL_WriteReg(uiValue, uiAddr);
    return;
}

/*
***************************************************************************************************
*                                       DSE_Test_R5AccessFault
*
* Default Slave Error Sample test code
*
* @param    none
*
* @return   none
*
* Notes     Access the inaccessible address and check the dse error interrupt.
*
***************************************************************************************************
*/

void DSE_Test_R5AccessFault(void)
{
    uint32       uiI = 0;

    uint32       uiTargetAddr;
    uint32       uiTestEndAddr;

    const uint32 uiDelaycnt   = (50UL);

    DSE_Deinit();
    (void)DSE_Init(DES_SEL_ALL);

    mcu_printf("=========== DSE_Test_R5AccessFault ===========\n");

    uiTargetAddr  = 0xA0F00000u;
    uiTestEndAddr = 0xA0F2AB00u;

    for(uiTargetAddr  = 0xA0F00000u; uiTargetAddr < uiTestEndAddr; uiTargetAddr = (uiTargetAddr + 200u))
    {
        mcu_printf("Try to access @0x%x\n", uiTargetAddr);
        gWait_irq = 1UL;
        (void)DSE_TEST_READREG(uiTargetAddr);


        for (uiI = 0 ; (uiI < uiDelaycnt) ; uiI++)
        {
            if(gWait_irq == 0UL)
            {
                break;
            }

            BSP_NOP_DELAY();
        }

        if(uiI == uiDelaycnt)
        {
            mcu_printf("Interrupt not occured\n");
        }

    }


    mcu_printf("=========== Done ===========\n");
    DSE_Deinit();
    return;
}


/*
***************************************************************************************************
*                                       DSE_TEST_Fmu_IRQ_ISR
*
* DSE Soft fault check , fmu callback isr function
*
* @param    pArg : guiFmuArg at the DSE_Test_SoftFaultCheck
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

static void DSE_TEST_Fmu_IRQ_ISR(void *pArg)
{
    uint32 uiRdata;
    uint32 uiReadSts;
    const uint32 uiTargetValue = guiFmuArg[0];
    const uint32 uiTargetAddr  = guiFmuArg[1];

    uiRdata          = 0UL;
    uiReadSts        = 0UL;

    if(uiTargetAddr != 0u)
    {

        uiReadSts = DSE_TEST_READREG(DSETEST_SM_SF_CTRL_STS);
        mcu_printf("FMU interrupt ... SF_CTRL_STS x%x\n", uiReadSts);

        if(uiReadSts == 0x4UL)
        {
            uiRdata = DSE_GetGrpSts();

            if((uiRdata & DSE_SM_IRQ_MASK) == DSE_SM_IRQ_MASK)
            {
                mcu_printf("DSE_IRQ_MASK error\n");
            }

            if((uiRdata & DSE_SM_IRQ_EN) == DSE_SM_IRQ_EN)
            {
                mcu_printf("DSE_SM_IRQ_EN error\n");
            }

            if((uiRdata & DSE_SM_SLV_CFG) == DSE_SM_SLV_CFG)
            {
                mcu_printf("DSE_SM_SLV_CFG error\n");
            }

            if((uiRdata & DSE_SM_CFG_WR_PW) == DSE_SM_CFG_WR_PW)
            {
                mcu_printf("DSE_SM_CFG_WR_PW error\n");
            }

            if((uiRdata & DSE_SM_CFG_WR_LOCK) == DSE_SM_CFG_WR_LOCK)
            {
                mcu_printf("DSE_SM_CFG_WR_LOCK error\n");
            }

        }

        DSE_TEST_WRITEREG(DSETEST_SM_PW, DSETEST_SM_SF_CTRL_PW);
        uiRdata = DSE_TEST_READREG(DSETEST_SM_SF_CTRL_PW);

        if(uiRdata == 1UL)
        {
            mcu_printf("restore previous reg x%x, val x%x.. \n", uiTargetAddr, uiTargetValue);
            DSE_TEST_WRITEREG(uiTargetValue, uiTargetAddr);
            DSE_TEST_WRITEREG(uiReadSts, DSETEST_SM_SF_CTRL_STS);
            uiReadSts = DSE_TEST_READREG(DSETEST_SM_SF_CTRL_STS);

            if(uiReadSts != 0UL)
            {
                mcu_printf("Fail.. DSETEST_SM_SF_CTRL_STS not 0..maybe..exist another Fault\n");
            }

        }
        else
        {
            mcu_printf("Fail.. SM_SF_CTRL_PW set\n");
        }

        (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_SYS_SM_CFG);

        if(guiDseWaitIrq > 0u)
        {
            guiDseWaitIrq--;
        }
    }

    return;
}




/*
***************************************************************************************************
*                                       DSE_Test_SoftFaultCheck
*
* DSE Soft fault check test code
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void DSE_Test_SoftFaultCheck(void)
{
#if (MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
    uint32 uiRdata;
    uint32 uiErr;

    uint32 uiI;

    uiRdata     = 0UL;
    uiErr       = 0UL;
    uiI         = 0UL;
    guiFmuArg[0] = 0UL;
    guiFmuArg[1] = 0UL;

    mcu_printf("\n Fault Injection Test \n");
    (void)DSE_Init(DES_SEL_ALL);
    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SYS_SM_CFG, (FMUSeverityLevelType_t)FMU_SVL_LOW, (FMUIntFnctPtr)&DSE_TEST_Fmu_IRQ_ISR, NULL_PTR);//(void *)guiFmuArg);
    (void)FMU_Set((FMUFaultid_t)FMU_ID_SYS_SM_CFG);
    (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_SYS_SM_CFG);

    DSE_TEST_WRITEREG(DSETEST_SM_PW, DSETEST_SM_SF_CTRL_PW);
    uiRdata = DSE_TEST_READREG(DSETEST_SM_SF_CTRL_PW);

    if(DSE_WriteLock(0UL) == SAL_RET_SUCCESS)
    {

        if(uiRdata != 1UL)
        {
            mcu_printf("  Fail.. DSETEST_SM_SF_CTRL_PW set\n");
            uiErr |= (0x10UL);
        }
        else
        {
            uiRdata = DSE_TEST_READREG(DSETEST_SM_SF_CTRL_CFG);
            /* for write timeout to 0xfff and dse fault en, test mod */
            DSE_TEST_WRITEREG(uiRdata | 0xFFF0044UL, DSETEST_SM_SF_CTRL_CFG);
            mcu_printf("  SF_CTRL_CFG set\n");
        }

        guiDseWaitIrq = 1UL;
        mcu_printf("  Fault injection>> DSE_IRQ_MASK\n");
        guiFmuArg[0] = DSE_TEST_READREG(DSETEST_DSE_IRQ_MASK);
        guiFmuArg[1] = DSETEST_DSE_IRQ_MASK;
        DSE_TEST_WRITEREG( ~guiFmuArg[0], DSETEST_DSE_IRQ_MASK);

        guiDseWaitIrq++;
        mcu_printf("  Fault injection>> DSE_IRQ_EN\n");
        guiFmuArg[0] = DSE_TEST_READREG(DSETEST_DSE_IRQ_EN);
        guiFmuArg[1] = DSETEST_DSE_IRQ_EN;
        DSE_TEST_WRITEREG( ~guiFmuArg[0], DSETEST_DSE_IRQ_EN);

        guiDseWaitIrq++;
        mcu_printf("  Fault injection>> DEF_SLV_CFG\n");
        guiFmuArg[0] = DSE_TEST_READREG(DSETEST_DEF_SLV_CFG);
        guiFmuArg[1] = DSETEST_DEF_SLV_CFG;
        DSE_TEST_WRITEREG( ~guiFmuArg[0], DSETEST_DEF_SLV_CFG);


        for(uiI = 0 ; uiI < 50000UL ; uiI++)
        {
            BSP_NOP_DELAY();
            if(guiDseWaitIrq == 0UL)
            {
                break;
            }
        }
    }


    DSE_TEST_WRITEREG(DSETEST_SM_PW, DSETEST_SM_SF_CTRL_PW);
    uiRdata = DSE_TEST_READREG(DSETEST_SM_SF_CTRL_PW);

    if(uiRdata != 1UL)
    {
        mcu_printf("  Fail.. DSETEST_SM_SF_CTRL_PW set\n");
        uiErr |= (0x40UL);
    }
    else
    {
        uiRdata = DSE_TEST_READREG(DSETEST_SM_SF_CTRL_CFG);
        DSE_TEST_WRITEREG(uiRdata & (~0x44UL), DSETEST_SM_SF_CTRL_CFG);
    }

    mcu_printf("  Fault Injection Test Done uiErr x%x\n",uiErr);

    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SYS_SM_CFG, (FMUSeverityLevelType_t)FMU_SVL_LOW, NULL, NULL );
    DSE_Deinit();

#else
    mcu_printf("MCU_BSP_SUPPORT_DRIVER_FMU flag is not set to 1. Please check the build flag.\n");
#endif  // defined( MCU_BSP_SUPPORT_DRIVER_FMU ) && ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

    return;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )


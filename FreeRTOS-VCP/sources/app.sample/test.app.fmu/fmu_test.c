// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : fmu_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )

#include <fmu_test.h>
#include <debug.h>
#include <fmu.h>
#include <bsp.h>

#define FMU_DEBUG

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

static uint32 fault_id                  = 0UL;
static uint32 fmu_irq_done              = 0UL;


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static FMUSts_t get_fmu_irq_status
(
    void
);

static void fmu_soft_fault_test_en
(
    uint32 uiValue
);

static void fmu_soft_fault_irq_isr
(
    void * pArg
);

static void fmu_soft_fault_en
(
    uint32 uiValue
);

static void FMU_SoftFaultTest
(
    void
);

static void FMU_PasswordWriteProtection
(
    void
);

/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/
static FMUSts_t get_fmu_irq_status
(
    void
)
{

    uint8   ucIdCnt;
    uint32  uiReg0;
    uint32  uiReg1;
    uint32  uiIdStatus;
    uint32  check_out;
    FMUSts_t sStatus;

    sStatus.id  = (FMUFaultid_t)63;
    sStatus.cnt = 0;
    check_out   = 0;
    uiReg0      = FMU_REG->mcIrqStatus.euNreg[0];
    uiReg1      = FMU_REG->mcIrqStatus.euNreg[1];

    for(ucIdCnt = 0; ucIdCnt < (uint32)64; ucIdCnt++)
    {
        uiIdStatus = uiReg0 & (uint32)0x1;

        if(uiIdStatus == 1UL)
        {
            if(check_out == 0UL) {
                sStatus.id = (FMUFaultid_t)ucIdCnt;
                check_out = 1;
            } else {
                sStatus.cnt++;
                mcu_printf("[%s]status.cnt = %d\n",sStatus.cnt);
            }
        }

        uiReg0 = (((uiReg1 & (uint32)0x1) << (uint32)31) | (uiReg0 >> (uint32)1));
        uiReg1 = (uiReg1 >> (uint32)1);
    }

    return sStatus;
}

static void fmu_soft_fault_test_en
(
    uint32 uiValue
)
{
    FMU_REG->mcCfgWrPw = FMU_PW;
    FMU_REG->mcCtrl.cuBreg.ctSoftFaultTestEn = uiValue;

#ifdef FMU_DEBUG
    if (uiValue == 0) {
        mcu_printf("FMU soft fault test enabled\n");
    }
    else if (uiValue == 1) {
        mcu_printf("FMU soft fault test disabled\n");
    }
    else {
        mcu_printf("Soft fault test enable setting value is invalid\n");
    }
#endif
}

static void fmu_soft_fault_irq_isr
(
    void * pArg
)
{
    FMUSts_t sStatus;
    uint32 uiReg;

    (void) pArg;
    sStatus = get_fmu_irq_status();

    if (sStatus.id != FMU_ID_FMU_FAULT) {
        mcu_printf("IRQ for unexpected FMU IRQ ID occured\n");
    } else {
        // Restore original data
        fmu_soft_fault_test_en((uint32)0);
        FMU_REG->mcCfgWrPw = FMU_PW;
        uiReg = FMU_BASE_ADDR + (fault_id<<(uint32)2);
        FMU_WriteReg(uiReg, FMU_ReadReg(uiReg));

        (void)FMU_IsrClr(sStatus.id);

#ifdef FMU_DEBUG
        mcu_printf("FMU Fault ID-%d IRQ status cleared\n", sStatus.id);
#endif
    }

#ifdef FMU_DEBUG
    mcu_printf("Soft fault check success for FMU_CFG[%d]\n", fault_id);
#endif

    fmu_irq_done = 1;
}

static void fmu_soft_fault_en
(
    uint32 uiValue
)
{
    FMU_REG->mcCfgWrPw = FMU_PW;
    FMU_REG->mcCtrl.cuBreg.ctSoftFaultCheckEn = uiValue;

#ifdef FMU_DEBUG
    if (uiValue == 0U) {
        mcu_printf("FMU soft fault enabled\n");
    }
    else if (uiValue == 1U) {
        mcu_printf("FMU soft fault disabled\n");
    }
    else {
        mcu_printf("Soft fault enable setting value is invalid\n");
    }
#endif
}

static void FMU_SoftFaultTest
(
    void
)
{
    uint32 uiRdAddr;
    uint32 uiWrAddr;
    uint32 uiK;
    uint32 i;
    const uint32 uiDelaycnt = 5000000UL;

#ifdef FMU_DEBUG
    mcu_printf ("Soft fault test start !!\n");
#endif

    (void)FMU_IsrHandler(FMU_ID_FMU_FAULT, FMU_SVL_LOW, (FMUIntFnctPtr)&fmu_soft_fault_irq_isr, NULL);
    // FMU SW reset
    //fmu_sw_reset();

    // Set FMU for soft fault
    (void)FMU_Set(FMU_ID_FMU_FAULT);

    // Set soft fault test enable
    fmu_soft_fault_en((uint32)1);

    for(i = (uint32)2; i < (uint32)27; i++)
    {
        fault_id = i;
        if((i < (uint32)20) || (i  > (uint32)25))
        {
            fmu_soft_fault_test_en((uint32)1);
            //FMU_REG->mcCfgWrPw =0x5afeace5;

            fmu_irq_done = 0;

            FMU_REG->mcCfgWrPw = FMU_PW;
            uiRdAddr = (FMU_BASE_ADDR + (fault_id<<(uint32)2));
            uiWrAddr = FMU_BASE_ADDR + (fault_id<<(uint32)2);
            FMU_WriteReg(uiWrAddr , (FMU_ReadReg(uiRdAddr) ^ 0x11111111UL));

            for (uiK = 0UL ; (uiK < uiDelaycnt) ; uiK++)
            {
                BSP_NOP_DELAY();
                if(fmu_irq_done == 1UL)
                {
                    break;
                }
            }
            //wait_soft_fault_irq();

            mcu_printf ("****** reg [%d][0x%08X]******** \n", fault_id, FMU_BASE_ADDR + (fault_id<<2));
        }
    }


    mcu_printf ("Soft fault test end !!\n");
}

static void FMU_PasswordWriteProtection
(
    void
)
{
    uint32 i;
    //FMU_Deinit();

    for(i = 2; i < (uint32)27; i++)
    {
        if((i < (uint32)20) || (i  > (uint32)25))
        {
            uint32 uiTempAddr;

            fault_id = i;
            mcu_printf ("****** reg [%d][0x%08X]******** \n",fault_id,FMU_BASE_ADDR + (fault_id<<(uint32)2));
            uiTempAddr = FMU_BASE_ADDR + (fault_id<<(uint32)2);
            mcu_printf("original         0x%08X\n", FMU_ReadReg(uiTempAddr));

            //incorrect FMU_PW
            FMU_REG->mcCfgWrPw = FMU_PW_ERROR;
            FMU_WriteReg((uint32)(FMU_BASE_ADDR + (uint32)(fault_id<<2)) , (FMU_ReadReg(uiTempAddr) ^ 0x11111111UL));
            mcu_printf("INCURRECT_PASS   0x%08X\n", FMU_ReadReg(uiTempAddr));

            //correct FMU_PW
            FMU_REG->mcCfgWrPw = FMU_PW;
            FMU_WriteReg((uint32)(FMU_BASE_ADDR + (uint32)(fault_id<<2)) , (FMU_ReadReg(uiTempAddr) ^ 0x11111111UL));
            mcu_printf("CURRECT_PASS     0x%08X\n\n", FMU_ReadReg(uiTempAddr));
        }
    }
}

void FMU_StartFmuTest
(
    int32 ucMode
)
{

    switch (ucMode)
    {

        case 1 :
        {
            FMU_SoftFaultTest();
            break;
        }

        case 2 :
        {
            FMU_PasswordWriteProtection();
            break;
        }
        default:
        {
            mcu_printf("Invalid Test Mode Value\n");
            break;
        }
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )


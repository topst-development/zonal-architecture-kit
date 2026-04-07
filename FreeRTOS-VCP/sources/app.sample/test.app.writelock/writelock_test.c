// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : writelock_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_WRITEBACK == 1 )

#include <app_cfg.h>

#include "debug.h"
#include "reg_phys.h"
#include "uart.h"
#include "writelock_test.h"

static void Inspect_WriteLockRegister
(
    uint32                              addr,
    uint32                              str_bit,
    uint32                              end_bit
);
static void Inspect_WritePasswordRegister
(
    uint32                              PW_addr,
    uint32                              addr,
    uint32                              str_bit,
    uint32                              end_bit
);
static void Inspect_WritePasswordRegister_Using0
(
    uint32                              PW_addr,
    uint32                              addr,
    uint32                              str_bit,
    uint32                              end_bit
);
static void MID_CFG_WriteLock
(
    void
);
static void MID_CFG_PasswordWriteLock
(
    void
);
static void MC_CFG_WriteLock
(
    void
);
static void MC_CFG_PasswordWriteLock
(
    void
);
static void SYS_CFG_PasswordWriteLock
(
    void
);
static void CR_CFG_WriteLock
(
    void
);
static void UART_CFG_WriteLock
(
    void
);


static void delay1us
(
    uint32                              n
)
{
    uint32  i;
    n = n * 500;

    for (i = 0; i < n; i++)
    {
         asm("nop");
    }
}


/*********************************************************************************************************************
    Inspect_WriteLockRegister

    Description
        Inspect whether write protection working properly.
        After entering the Lock flag, Inspect whether it is impossible to write to the register.

    Parameter
        addr : address of the register to inspect for write protection.
        str_bit : start bit of register.
        end_bit : end bit of register.

    Return
        None
                                                                                                    2020/03/23  by sjw
**********************************************************************************************************************/
static void Inspect_WriteLockRegister
(
    uint32                              addr,
    uint32                              str_bit,
    uint32                              end_bit
)
{
    uint32          cnt;
    volatile uint32 baseVal = 0;
    volatile uint32 readVal = 0;
    uint32          writeVal = 0;
    //uint32        checkBit= 0;

    baseVal = SAL_ReadReg(addr);
/*
    if(0 == baseVal)    // If reset values are 0, you should use the value 1;
    {
        checkBit = 1;
    }
    else
    {
        checkBit = 0;
    }
*/
    for(cnt = str_bit; cnt <= end_bit; cnt++)
    {
        writeVal = baseVal ^ (1 << cnt);
        SAL_WriteReg(writeVal, addr);
        readVal = SAL_ReadReg(addr);

        if(readVal != writeVal)
        {
            mcu_printf("    [Lock] TEST SUCCESS(%d) !! (write: 0x%x, read: 0x%x)\n", cnt, writeVal, readVal);
        }
        else
        {
            mcu_printf("---------- [Lock] TEST FAIL(%d) !! (write: 0x%x, read: 0x%x) ----------\n", cnt, writeVal, readVal);
        }
    }
    mcu_printf("\n");
}

/*********************************************************************************************************************
    Inspect_PasswordProtectedRegister

    Description
        Inspect whether write protection working properly.
        After entering the password, Inspect whether it is possible to write to the register.

    Parameter
        PW_addr : address of Configuration Write Password Register
        addr : address of the register to inspect for write protection.
        str_bit : start bit of register.
        end_bit : end bit of register.

    Return
        None
                                                                                                    2020/03/25  by sjw
**********************************************************************************************************************/
static void Inspect_WritePasswordRegister
(
    uint32                              PW_addr,
    uint32                              addr,
    uint32                              str_bit,
    uint32                              end_bit
)
{
    uint32          cnt;
    volatile uint32 baseVal = 0;
    volatile uint32 readVal = 0;
    uint32          writeVal = 0;

    baseVal = SAL_ReadReg(addr);

    for(cnt = str_bit; cnt <= end_bit; cnt++)
    {
        SAL_WriteReg(WRITELOCK_PASSWORD, PW_addr);

        writeVal = baseVal ^ (1 << cnt);
        SAL_WriteReg(writeVal, addr);
        readVal = SAL_ReadReg(addr);

        if(readVal == writeVal)
        {
            mcu_printf("    [Password] TEST SUCCESS(%d) !! (write: 0x%x, read: 0x%x)\n", cnt, writeVal, readVal);
        }
        else
        {
            mcu_printf("---------- [Password] TEST FAIL(%d) !! (write: 0x%x, read: 0x%x) ----------\n", cnt, writeVal, readVal);
        }
    }
    mcu_printf("\n");
}

static void Inspect_WritePasswordRegister_Using0
(
    uint32                              PW_addr,
    uint32                              addr,
    uint32                              str_bit,
    uint32                              end_bit
)
{
    uint32          cnt;
    volatile uint32 baseVal = 0;
    volatile uint32 readVal = 0;
    uint32          writeVal = 0;

    baseVal = SAL_ReadReg(addr);

    for(cnt = str_bit; cnt <= end_bit; cnt++)
    {
        SAL_WriteReg(WRITELOCK_PASSWORD, PW_addr);

        writeVal = baseVal ^ (0 << cnt);
        SAL_WriteReg(writeVal, addr);
        readVal = SAL_ReadReg(addr);

        if(readVal == writeVal)
        {
            mcu_printf("    [Password] TEST SUCCESS(%d) !! (write: 0x%x, read: 0x%x)\n", cnt, writeVal, readVal);
        }
        else
        {
            mcu_printf("---------- [Password] TEST FAIL(%d) !! (write: 0x%x, read: 0x%x) ----------\n", cnt, writeVal, readVal);
        }
    }
    mcu_printf("\n");
}

/*********************************************************************************************************************
    MID_CFG_WriteLock

    Description
        Test write Lock mechanism of MID Configuration registers.

    Parameter
        None

    Return
        None
                                                                                                    2020/03/23  by sjw
**********************************************************************************************************************/
static void MID_CFG_WriteLock
(
    void
)
{
    SAL_WriteReg(WRITELOCK_PASSWORD, REG_MID_CFG_WR_PW);
    SAL_WriteReg(1, REG_MID_CFG_WR_LOCK);

    mcu_printf("AXI-0/1 Master MID Configuration Register (XS_MID)\n");
    Inspect_WriteLockRegister(REG_XS_MID, 0, 7);        // 0~7 bit
    Inspect_WriteLockRegister(REG_XS_MID, 16, 23);      // 16~23 bit
    delay1us(10000); // 10msec
    mcu_printf("AHB-2 Master MID Configuration Register (HS2_MID)\n");
    Inspect_WriteLockRegister(REG_HS2_MID, 0, 23);      // 0~23 bit
    delay1us(10000);
    mcu_printf("AHB-3 Master MID Configuration Register (HS3_MID)\n");
    Inspect_WriteLockRegister(REG_HS3_MID, 0, 23);      // 0~23 bit
    delay1us(10000);
    mcu_printf("AHB-4 Master MID Configuration-0 Register (HS4_MID)\n");
    Inspect_WriteLockRegister(REG_HS4_MID0, 0, 31);      // 0~31 bit
    delay1us(10000);
    mcu_printf("AHB-4 Master MID Configuration-1 Register (HS4_MID1)\n");
    Inspect_WriteLockRegister(REG_HS4_MID1, 0, 15);      // 0~15 bit
    delay1us(10000);
    mcu_printf("AHB-5 Master MID Configuration-0 Register (HS5_MID)\n");
    Inspect_WriteLockRegister(REG_HS5_MID0, 0, 31);      // 0~31 bit
    delay1us(10000);
    mcu_printf("AHB-5 Master MID Configuration-1 Register (HS5_MID1)\n");
    Inspect_WriteLockRegister(REG_HS5_MID1, 0, 15);      // 0~15 bit
    delay1us(10000);
}

/*********************************************************************************************************************
    MID_CFG_PasswordWriteLock

    Description
        Test Password write Lock mechanism of MID Configuration registers.

    Parameter
        None

    Return
        None
                                                                                                    2020/05/17  by sjw
**********************************************************************************************************************/
static void MID_CFG_PasswordWriteLock
(
    void
)
{
    mcu_printf("MID Configuration Write Lock Register (MID_CFG_WR_LOCK)\n");
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MID_CFG_WR_LOCK, 0, 0);            // 0 bit
    delay1us(10000);
    mcu_printf("MID Configuration Soft Fault Check Group Enable Register (SF_CHK_GRP_EN)\n");
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MID_SF_CHK_GRP_EN, 0, 8);          // 0~8 bit
    delay1us(10000);
    mcu_printf("MID Configuration Soft Fault Control Register\n");
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MID_SF_CTRL_CFG, 0, 0);            // 0 bit
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MID_SF_CTRL_CFG, 4, 4);            // 4 bit
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MID_SF_CTRL_CFG, 8, 8);            // 8 bit
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MID_SF_CTRL_CFG, 16, 27);          // 16~27 bit
    delay1us(10000);
#if 0
    mcu_printf("MID Configuration Soft Fault Status Register\n");
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MID_SF_CTRL_STS, 0, 0);            // 0 bit
    delay1us(10000);
#endif
}

/*********************************************************************************************************************
    MC_CFG_WriteLock

    Description
        Test write Lock mechanism of MC Configuration registers

    Parameter
        None

    Return
        None
                                                                                                    2020/03/24  by sjw
**********************************************************************************************************************/
static void MC_CFG_WriteLock
(
    void
)
{
    SAL_WriteReg(WRITELOCK_PASSWORD, REG_MC_CFG_WR_PW);
    SAL_WriteReg(1, REG_MC_CFG_WR_LOCK);

    mcu_printf("Peripheral Bus Clock Mask Register-0\n");
    Inspect_WriteLockRegister(REG_HCLK_MASK0, 0, 25);           // 0~25 bit
    delay1us(10000);
    mcu_printf("Peripheral Bus Clock Mask Register-1\n");
    Inspect_WriteLockRegister(REG_HCLK_MASK1, 0, 27);           // 0~27 bit
    delay1us(10000);
    mcu_printf("Peripheral Bus Clock Mask Register-2\n");
    Inspect_WriteLockRegister(REG_HCLK_MASK2, 0, 20);           // 0~20 bit
    delay1us(10000);
    mcu_printf("Peripheral SW Reset Register-0\n");
    Inspect_WriteLockRegister(REG_SW_RESET0, 0, 25);            // 0~25 bit
    delay1us(10000);
    mcu_printf("Peripheral SW Reset Register-1\n");
    Inspect_WriteLockRegister(REG_SW_RESET1, 0, 27);            // 0~27 bit
    delay1us(10000);
    mcu_printf("Peripheral SW Reset Register-2\n");
    Inspect_WriteLockRegister(REG_SW_RESET2, 0, 20);            // 0~20 bit
    delay1us(10000);
    mcu_printf("Internal Memory Controller Operation Configuration Register\n");
    Inspect_WriteLockRegister(REG_IMC_CFG, 0, 1);               // 0~1 bit
    Inspect_WriteLockRegister(REG_IMC_CFG, 4, 5);               // 4~5 bit
    delay1us(10000);
    mcu_printf("DMA Controller Request Input Select Register\n");
    Inspect_WriteLockRegister(REG_DMA_SREQ_SEL, 0, 6);          // 0~6 bit
    delay1us(10000);
    mcu_printf("AXI Asynchronous Bridge Power Management Control Register\n");
    Inspect_WriteLockRegister(REG_X2X_PWR_CTRL, 0, 0);          // 0 bit
    Inspect_WriteLockRegister(REG_X2X_PWR_CTRL, 4, 4);          // 4 bit
    Inspect_WriteLockRegister(REG_X2X_PWR_CTRL, 16, 16);        // 16 bit
    Inspect_WriteLockRegister(REG_X2X_PWR_CTRL, 20, 20);        // 20 bit
    delay1us(10000);
    mcu_printf("Secure Access ID Filter Power Management Control Register\n");
    Inspect_WriteLockRegister(REG_SID_FLT_PWR_CTRL, 0, 0);      // 0 bit
    Inspect_WriteLockRegister(REG_SID_FLT_PWR_CTRL, 4, 4);      // 4 bit
    Inspect_WriteLockRegister(REG_SID_FLT_PWR_CTRL, 8, 8);      // 8 bit
    Inspect_WriteLockRegister(REG_SID_FLT_PWR_CTRL, 12, 12);    // 12 bit
    Inspect_WriteLockRegister(REG_SID_FLT_PWR_CTRL, 16, 16);    // 16 bit
    Inspect_WriteLockRegister(REG_SID_FLT_PWR_CTRL, 20, 20);    // 20 bit
    delay1us(10000);
    mcu_printf("Memory Access ID Filter Power Management Control Register\n");
    Inspect_WriteLockRegister(REG_MID_FLT_PWR_CTRL, 0, 0);      // 0 bit
    Inspect_WriteLockRegister(REG_MID_FLT_PWR_CTRL, 4, 4);      // 4 bit
    Inspect_WriteLockRegister(REG_MID_FLT_PWR_CTRL, 8, 8);      // 8 bit
    Inspect_WriteLockRegister(REG_MID_FLT_PWR_CTRL, 12, 12);    // 12 bit
    Inspect_WriteLockRegister(REG_MID_FLT_PWR_CTRL, 16, 16);    // 16~16 bit
    Inspect_WriteLockRegister(REG_MID_FLT_PWR_CTRL, 20, 20);    // 20~20 bit
    delay1us(10000);
    mcu_printf("MICOM Sub-system Debug & DP APB Control Register\n");
    Inspect_WriteLockRegister(REG_DBG_DP_CTRL, 0, 1);           // 0~1 bit
    Inspect_WriteLockRegister(REG_DBG_DP_CTRL, 4, 6);           // 4~6 bit
    Inspect_WriteLockRegister(REG_DBG_DP_CTRL, 16, 17);         // 16~17 bit
    Inspect_WriteLockRegister(REG_DBG_DP_CTRL, 24, 24);         // 24 bit
    delay1us(10000);
    mcu_printf("System Peripheral User Mode Write Disable Register-0\n");
    Inspect_WriteLockRegister(REG_SPERI_USR_WR_DIS0, 0, 27);    // 0~27 bit
    delay1us(10000);
    mcu_printf("System Peripheral User Mode Write Disable Register-1\n");
    Inspect_WriteLockRegister(REG_SPERI_USR_WR_DIS1, 0, 4);     // 0~4 bit
    delay1us(10000);
    mcu_printf("MICOM Sub-system Ready Register\n");
    Inspect_WriteLockRegister(REG_MICOM_READY, 0, 15);          // 0~15 bit
    delay1us(10000);
}

/*********************************************************************************************************************
    MC_CFG_PasswordWriteLock

    Description
        Test Password write Lock mechanism of MC Configuration registers.

    Parameter
        None

    Return
        None
                                                                                                    2020/05/17  by sjw
**********************************************************************************************************************/
static void MC_CFG_PasswordWriteLock
(
    void
)
{
    mcu_printf("MC Configuration Write Lock Register (MC_CFG_WR_LOCK)\n");
    Inspect_WritePasswordRegister(REG_MC_CFG_WR_PW, REG_MC_CFG_WR_LOCK, 0, 0);          // 0 bit
    delay1us(10000);
    mcu_printf("MC Configuration Soft Fault Check Group Enable Register (SF_CHK_GRP_EN)\n");
    Inspect_WritePasswordRegister(REG_MC_CFG_WR_PW, REG_MC_SF_CHK_GRP_EN, 0, 7);        // 0~7 bit, 8~10 bits are reserved.
    Inspect_WritePasswordRegister(REG_MC_CFG_WR_PW, REG_MC_SF_CHK_GRP_EN, 11, 19);      // 11~19 bit
    delay1us(10000);
    mcu_printf("MC Configuration Soft Fault Control Register\n");
    Inspect_WritePasswordRegister(REG_MC_CFG_WR_PW, REG_MC_SF_CTRL_CFG, 0, 0);         // 0 bit
    Inspect_WritePasswordRegister(REG_MC_CFG_WR_PW, REG_MC_SF_CTRL_CFG, 4, 4);         // 4 bit
    Inspect_WritePasswordRegister(REG_MC_CFG_WR_PW, REG_MC_SF_CTRL_CFG, 8, 8);         // 8 bit
    Inspect_WritePasswordRegister(REG_MC_CFG_WR_PW, REG_MC_SF_CTRL_CFG, 16, 27);       // 16~27 bit
    delay1us(10000);
    mcu_printf("MC Configuration Soft Fault Status Register\n");
    Inspect_WritePasswordRegister(REG_MID_CFG_WR_PW, REG_MC_SF_CTRL_STS, 0, 0);         // 0 bit
    delay1us(10000);
}

/*********************************************************************************************************************
    SYS_CFG_PasswordWriteLock

    Description
        Test write Lock mechanism of SYS Configuration registers

    Parameter
        None

    Return
        None
                                                                                                    2020/03/25  by sjw
**********************************************************************************************************************/
static void SYS_CFG_PasswordWriteLock
(
    void
)
{
    mcu_printf("SRAM0 ECC Error Request Enable Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM0_ECC_ERR_REQ_EN, 0,31);      // 0~31 bit
    delay1us(10000);
    mcu_printf("SRAM1 ECC Error Request Enable Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM1_ECC_ERR_REQ_EN, 0, 31);     // 0~31 bit
    delay1us(10000);
#if 0
    mcu_printf("SRAM0 ECC Error Request Status Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM0_ECC_ERR_STS, 0, 31);        // 0~31 bit
    delay1us(10000);
    printf("SRAM1 ECC Error Request Status Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM1_ECC_ERR_STS, 0, 31);        // 0~31 bit
    delay1us(10000);
#endif
    mcu_printf("SFMC Buffer Memory ECC Error Request Enable Register\n");
    Inspect_WritePasswordRegister_Using0(REG_ECC_CFG_WR_PW, REG_SFMC_ECC_ERR_REQ_EN, 0, 3);       // 0~3 bit
    delay1us(10000);
#if 0
    mcu_printf("SFMC Buffer Memory ECC Error Request Status Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SFMC_ECC_ERR_STS, 0, 3);          // 0~3 bit
    delay1us(10000);
#endif
    mcu_printf("MailBox0 FIFO Memory ECC Error Request Enable Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX0_ECC_ERR_REQ_EN, 0, 3);      // 0~3 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX0_ECC_ERR_REQ_EN, 8, 11);     // 8~11 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX0_ECC_ERR_REQ_EN, 16, 19);    // 16~19 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX0_ECC_ERR_REQ_EN, 24, 27);    // 24~27 bit
    delay1us(10000);
    mcu_printf("MailBox1 FIFO Memory ECC Error Request Enable Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX1_ECC_ERR_REQ_EN, 0, 3);      // 0~3 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX1_ECC_ERR_REQ_EN, 8, 11);     // 8~11 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX1_ECC_ERR_REQ_EN, 16, 19);    // 16~19 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX1_ECC_ERR_REQ_EN, 24, 27);    // 24~27 bit
    delay1us(10000);
    mcu_printf("MailBox2 FIFO Memory ECC Error Request Enable Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX2_ECC_ERR_REQ_EN, 0, 3);      // 0~3 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_MBOX2_ECC_ERR_REQ_EN, 8, 11);     // 8~11 bit
    delay1us(10000);
    mcu_printf("ECC Error Request / Acknowledge Control Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_ECC_ERR_REQ_ACK_CTRL, 0, 11);     // 0~11 bit
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_ECC_ERR_REQ_ACK_CTRL, 16, 23);    // 16~23 bit
    delay1us(10000);
#if 0
    mcu_printf("SRAM0 Write LSB Data Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM0_WR_LSB_DATA_INV, 0, 31);    // 0~31 bit
    delay1us(10000);
    mcu_printf("SRAM0 Write MSB Data Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM0_WR_MSB_DATA_INV, 0, 31);    // 0~31 bit
    delay1us(10000);
    mcu_printf("SRAM0 Data Memory Address Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM0_DMEM_ADDR_INV, 3, 15);      // 3~15 bit
    delay1us(10000);
    mcu_printf("SRAM0 ECC Memory Address Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM0_EMEM_ADDR_INV, 3, 15);      // 3~15 bit
    delay1us(10000);
    mcu_printf("SRAM1 Write LSB Data Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM1_WR_LSB_DATA_INV, 0, 31);    // 0~31 bit
    delay1us(10000);
    mcu_printf("SRAM1 Write MSB Data Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM1_WR_MSB_DATA_INV, 0, 31);    // 0~31 bit
    delay1us(10000);
    mcu_printf("SRAM1 Data Memory Address Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM1_DMEM_ADDR_INV, 3, 15);      // 3~15 bit
    delay1us(10000);
    mcu_printf("SRAM1 ECC Memory Address Inverting Register\n");
    Inspect_WritePasswordRegister(REG_ECC_CFG_WR_PW, REG_SRAM1_EMEM_ADDR_INV, 3, 15);      // 3~15 bit
    delay1us(10000);
#endif
}

/*********************************************************************************************************************
    CR_CFG_WriteLock

    Description
        Test write Lock mechanism of CR Configuration registers

    Parameter
        None

    Return
        None
                                                                                                    2020/03/23  by sjw
**********************************************************************************************************************/
static void CR_CFG_WriteLock
(
    void
)
{
    SAL_WriteReg(WRITELOCK_PASSWORD, REG_CR5_CFG_WR_PW);
    SAL_WriteReg(1, REG_CR5_CFG_WR_LOCK);

    mcu_printf("Cortex-R5 Debug Signal Configuration and Status Register-0\n");
    Inspect_WriteLockRegister(REG_CR5_DBG0, 8, 11);         // 8~11 bit
    delay1us(10000); // 10msec
    mcu_printf("Cortex-R5 Debug Signal Configuration and Status Register-1\n");
    Inspect_WriteLockRegister(REG_CR5_DBG1, 12, 31);        // 12~31 bit
    delay1us(10000);
    mcu_printf("Cortex-R5 Debug Signal Configuration and Status Register-2\n");
    Inspect_WriteLockRegister(REG_CR5_DBG2, 12, 31);        // 12~31 bit
    delay1us(10000);
    mcu_printf("Cortex-R5 Configuration Signal Register\n");
    Inspect_WriteLockRegister(REG_CR5_CFG, 0, 5);           // 0~5 bit
    Inspect_WriteLockRegister(REG_CR5_CFG, 16, 19);         // 16~19 bit
    delay1us(10000);
    mcu_printf("Cortex-R5 Global Signal Configuration and Status Register\n");
    Inspect_WriteLockRegister(REG_CR5_GLB, 0, 0);           // 0 bit
    Inspect_WriteLockRegister(REG_CR5_GLB, 4, 4);           // 4 bit
    delay1us(10000);
}

/*********************************************************************************************************************
    CR_CFG_PasswordWriteLock

    Description
        Test Password write Lock mechanism of CR Configuration registers.

    Parameter
        None

    Return
        None
                                                                                                    2020/05/17  by sjw
**********************************************************************************************************************/
static void CR_CFG_PasswordWriteLock
(
    void
)
{
    mcu_printf("CR Configuration Write Lock Register (MC_CFG_WR_LOCK)\n");
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_CFG_WR_LOCK, 0, 0);        // 0 bit
    delay1us(10000);
    mcu_printf("CR Configuration Soft Fault Check Group Enable Register (SF_CHK_GRP_EN)\n");
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CHK_GRP_EN, 0, 3);      // 0~3 bit, 4 bit is reserved
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CHK_GRP_EN, 5, 5);      // 5 bit, 6,7 bit are reserved
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CHK_GRP_EN, 8, 9);      // 8~9 bit
    delay1us(10000);
    mcu_printf("CR Configuration Soft Fault Control Register\n");
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CTRL_CFG, 0, 0);        // 0 bit
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CTRL_CFG, 4, 4);        // 4 bit
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CTRL_CFG, 8, 8);        // 8 bit
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CTRL_CFG, 16, 27);      // 16~27 bit
    delay1us(10000);
#if 0
    mcu_printf("CR Configuration Soft Fault Status Register\n");
    Inspect_WritePasswordRegister(REG_CR5_CFG_WR_PW, REG_CR5_SF_CTRL_STS, 0, 0);         // 0 bit
    delay1us(10000);
#endif
}

/*********************************************************************************************************************
    UART_CFG_WriteLock

    Description
        Test Password write Lock mechanism of UART Configuration registers.

    Parameter
        None

    Return
        None
                                                                                                    2020/03/26  by sjw
**********************************************************************************************************************/
static void UART_CFG_WriteLock
(
    void
)
{
    SAL_WriteReg(WRITELOCK_PASSWORD, REG_UART_CFG_WR_PW);
    SAL_WriteReg(1, REG_UART_CFG_WR_LOCK);

    mcu_printf("UART SIR IO Selection Register\n");
    Inspect_WriteLockRegister(REG_UART_PORT_SEL0, 0, 31);   // 0~31 bit
    delay1us(10000); // 10msec
    mcu_printf("UART Port Selection Register\n");
    Inspect_WriteLockRegister(REG_UART_PORT_SEL1, 0, 15);   // 0~15 bit
    delay1us(10000); // 10msec
}

/*********************************************************************************************************************
    Test_WriteProtection

    Description
        Test write protection.

    Parameter
        None

    Return
        None
                                                                                                    2020/03/23  by sjw
**********************************************************************************************************************/
void Test_WriteProtection
(
    void
)
{
    /* SM-MC-MID_CFG.03, Register Write Lock with Password Write Protection */
    MID_CFG_WriteLock();
    MID_CFG_PasswordWriteLock();

    /* SM-MC_CFG-03, Register Write Lock with Password Write Protection */
    MC_CFG_WriteLock();
    MC_CFG_PasswordWriteLock();

    /* SM-MC-SYS_SM.03, Register Write Lock with Password Write Protection */
    SYS_CFG_PasswordWriteLock();

    /* SM-MC-CR_CFG.03, Register Write Lock with Password Write Protection */
    CR_CFG_WriteLock();
    CR_CFG_PasswordWriteLock();

    /* SM-MC-UART.02, Register Write Lock with Password Write Protection */
    UART_CFG_WriteLock();
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WRITEBACK == 1 )


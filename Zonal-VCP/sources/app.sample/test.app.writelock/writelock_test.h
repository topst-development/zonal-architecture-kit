// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : writelock_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_WRITELOCK_TEST_HEADER
#define MCU_BSP_WRITELOCK_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_WRITEBACK == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_UART value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )

// TCC805x PASSWORD
#define WRITELOCK_PASSWORD              0x5AFEACE5

// TCC803x PASSWORD
//#define WRITELOCK_PASSWORD            0x8030ACE5

#define UNLOCK                          0
#define LOCK                            1

#define EVEN                            0
#define ODD                             1

enum {
    STEP_UNLOCK,
    STEP_WRITEPROTECTION,
    STEP_LOCK,
};

// MEMORY ACCESS ID (MID) CONFIGURATION
#define REG_MID_CFG_WR_PW               0x1B93901C  // MID Configuration Write Password Register
#define REG_MID_CFG_WR_LOCK             0x1B939020  // MID Configuration Write Lock Register

#define REG_XS_MID                      0x1B939000  // AXI-0/1 Master MID Configuration Register
#define REG_HS2_MID                     0x1B939004  // AHB-2 Master MID Configuration Register
#define REG_HS3_MID                     0x1B939008  // AHB-3 Master MID Configuration Register
#define REG_HS4_MID0                    0x1B93900C  // AHB-4 Master MID Configuration-0 Register
#define REG_HS4_MID1                    0x1B939010  // AHB-4 Master MID Configuration-1 Register
#define REG_HS5_MID0                    0x1B939014  // AHB-5 Master MID Configuration-0 Register
#define REG_HS5_MID1                    0x1B939018  // AHB-5 Master MID Configuration-1 Register

#define REG_MID_SF_CHK_GRP_EN           0x1B939030  // MID Configuration Soft Fault Check Group Enable Register
#define REG_MID_SF_CTRL_CFG             0x1B939038  // MID Configuration Soft Fault Control Register
#define REG_MID_SF_CTRL_STS             0x1B93903C  // MID Configuration Soft Fault Status Register

// MICOM (MC) SUB-SYSTEM CONFIGURATION
#define REG_MC_CFG_WR_PW                0x1B930048  // MICOM Sub-system Configuration Write Password Register
#define REG_MC_CFG_WR_LOCK              0x1B93004C  // MICOM Sub-system Configuration Write Lock Register

#define REG_HCLK_MASK0                  0x1B930000  // Peripheral Bus Clock Mask Register-0
#define REG_HCLK_MASK1                  0x1B930004  // Peripheral Bus Clock Mask Register-1
#define REG_HCLK_MASK2                  0x1B930008  // Peripheral Bus Clock Mask Register-2
#define REG_SW_RESET0                   0x1B93000C  // Peripheral SW Reset Register-0
#define REG_SW_RESET1                   0x1B930010  // Peripheral SW Reset Register-1
#define REG_SW_RESET2                   0x1B930014  // Peripheral SW Reset Register-2
#define REG_IMC_CFG                     0x1B930018  // Internal Memory Controller Operation Configuration Register
#define REG_DMA_SREQ_SEL                0x1B93001C  // DMA Controller Request Input Select Register
#define REG_X2X_PWR_CTRL                0x1B930020  // AXI Asynchronous Bridge Power Management Control Register
#define REG_SID_FLT_PWR_CTRL            0x1B930030  // Secure Access ID Filter Power Management Control Register
#define REG_MID_FLT_PWR_CTRL            0x1B930034  // Memory Access ID Filter Power Management Control Register
#define REG_DBG_DP_CTRL                 0x1B930038  // MICOM Sub-system Debug & DP APB Control Register
#define REG_SPERI_USR_WR_DIS0           0x1B93003C  // System Peripheral User Mode Write Disable Register-0
#define REG_SPERI_USR_WR_DIS1           0x1B930040  // System Peripheral User Mode Write Disable Register-1
#define REG_MICOM_READY                 0x1B930044  // MICOM Sub-system Ready Register

#define REG_MC_SF_CHK_GRP_EN            0x1B930050  // MICOM Sub-system Configuration Soft Fault Check Group Enable Register
#define REG_MC_SF_CTRL_CFG              0x1B930070  // MICOM Sub-system Configuration Soft Fault Control Register
#define REG_MC_SF_CTRL_STS              0x1B930074  // MICOM Sub-system Configuration Soft Fault Status Register


// SYSTEM (SYS) CONFIGURATION
#define REG_ECC_CFG_WR_PW               0x1B921070  // ECC Configuration Write Password Register

#define REG_SRAM0_ECC_ERR_REQ_EN        0x1B921000  // SRAM0 ECC Error Request Enable Registr
#define REG_SRAM1_ECC_ERR_REQ_EN        0x1B92100C  // SRAM1 ECC Error Request Enable Registr
#define REG_SRAM0_ECC_ERR_STS           0x1B921004  // SRAM0 ECC Error Status Register
#define REG_SRAM1_ECC_ERR_STS           0x1B921010  // SRAM1 ECC Error Status Register
#define REG_SFMC_ECC_ERR_REQ_EN         0x1B921018  // SFMC Buffer Memory ECC Error Request Enable Register
#define REG_SFMC_ECC_ERR_STS            0x1B92101C  // SFMC Buffer Memory ECC Error Status Register
#define REG_MBOX0_ECC_ERR_REQ_EN        0x1B921024  // MailBox0 FIFO Memory ECC Error Request Enable Register
#define REG_MBOX1_ECC_ERR_REQ_EN        0x1B921030  // MailBox1 FIFO Memory ECC Error Request Enable Register
#define REG_MBOX2_ECC_ERR_REQ_EN        0x1B92103C  // MailBox2 FIFO Memory ECC Error Request Enable Register
#define REG_MBOX0_ECC_ERR_STS           0x1B921028  // Mail-box0 FIFO Memory ECC Error Status Register
#define REG_MBOX1_ECC_ERR_STS           0x1B921034  // Mail-box1 FIFO Memory ECC Error Status Register
#define REG_MBOX2_ECC_ERR_STS           0x1B921040  // Mail-box2 FIFO Memory ECC Error Status Register
#define REG_ECC_ERR_REQ_ACK_CTRL        0x1B921048  // ECC Error Request / Acknowledge Control Register
#define REG_SRAM0_WR_LSB_DATA_INV       0x1B921050  // SRAM0 Write LSB Data Inverting Register
#define REG_SRAM0_WR_MSB_DATA_INV       0x1B921054  // SRAM0 Write MSB Data Inverting Register
#define REG_SRAM0_DMEM_ADDR_INV         0x1B921058  // SRAM0 Data Memory Address Inverting Register
#define REG_SRAM0_EMEM_ADDR_INV         0x1B92105C  // SRAM0 ECC Memory Address Inverting Register
#define REG_SRAM1_WR_LSB_DATA_INV       0x1B921060  // SRAM1 Write LSB Data Inverting Register
#define REG_SRAM1_WR_MSB_DATA_INV       0x1B921064  // SRAM1 Write MSB Data Inverting Register
#define REG_SRAM1_DMEM_ADDR_INV         0x1B921068  // SRAM1 Data Memory Address Inverting Register
#define REG_SRAM1_EMEM_ADDR_INV         0x1B92106C  // SRAM1 ECC Memory Address Inverting Register

// CORETEX-R5 (CR) CONFIGURATION
#define REG_CR5_CFG_WR_PW               0x1B932020  // Cortex-R5 Configuration Write Password Register
#define REG_CR5_CFG_WR_LOCK             0x1B932024  // Cortex-R5 Configuration Write Lock Register

#define REG_CR5_DBG0                    0x1B932000  // Cortex-R5 Debug Signal Configuration and Status Register-0
#define REG_CR5_DBG1                    0x1B932004  // Cortex-R5 Debug Signal Configuration and Status Register-1
#define REG_CR5_DBG2                    0x1B932008  // Cortex-R5 Debug Signal Configuration and Status Register-2
#define REG_CR5_CFG                     0x1B93200C  // Cortex-R5 Configuration Signal Register
#define REG_CR5_GLB                     0x1B932014  // Cortex-R5 Global Signal Configuration and Status Register

#define REG_CR5_SF_CHK_GRP_EN           0x1B932050  // Cortex-R5 Configuration Soft Fault Check Group Enable Register
#define REG_CR5_SF_CTRL_CFG             0x1B932070  // Cortex-R5 Configuration Soft Fault Control Register
#define REG_CR5_SF_CTRL_STS             0x1B932074  // Cortex-R5 Configuration Soft Fault Status Register

// UART CONFIGURATION
#define REG_UART_CFG_WR_PW              0x1B2A0018  // UART Configuration Write Password Register
#define REG_UART_CFG_WR_LOCK            0x1B2A001C  // UART Configuration Write Lock Register

#define REG_UART_PORT_SEL0              0x1B2A0010  // UART Port Selection0 Register
#define REG_UART_PORT_SEL1              0x1B2A0014  // UART Port Selection1 Register


extern void Test_WriteProtection
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WRITEBACK == 1 )

#endif  // MCU_BSP_WRITELOCK_TEST_HEADER


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : midf_dev.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_MIDF_DEV_HEADER
#define MCU_BSP_MIDF_DEV_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_MIDF == 1 )

#define MIDF_FILTER_BASE                (0x1B93B000UL)
#define MIDF_FILTER_CFG_BASE            (0x1B939000UL)

//----------------------------------------------------------------------------------------------------------------------------
// Register structure
//----------------------------------------------------------------------------------------------------------------------------
#define MIDF_REGION_032KB               (0x0EUL)
#define MIDF_REGION_064KB               (0x0FUL)
#define MIDF_REGION_128KB               (0x10UL)
#define MIDF_REGION_256KB               (0x11UL)
#define MIDF_REGION_512KB               (0x12UL)
#define MIDF_REGION_001MB               (0x13UL)
#define MIDF_REGION_002MB               (0x14UL)
#define MIDF_REGION_004MB               (0x15UL)
#define MIDF_REGION_008MB               (0x16UL)
#define MIDF_REGION_016MB               (0x17UL)
#define MIDF_REGION_032MB               (0x18UL)
#define MIDF_REGION_064MB               (0x19UL)
#define MIDF_REGION_128MB               (0x1AUL)
#define MIDF_REGION_256MB               (0x1BUL)
#define MIDF_REGION_512MB               (0x1CUL)
#define MIDF_REGION_001GB               (0x1DUL)
#define MIDF_REGION_002GB               (0x1EUL)
#define MIDF_REGION_004GB               (0x1FUL)
#define MIDF_REGION_008GB               (0x20UL)

#define MIDF_SECURE_READ                (0x8UL)
#define MIDF_SECURE_WRITE               (0x4UL)

#define MIDF_NONSECURE_READ             (0x2UL)
#define MIDF_NONSECURE_WRITE            (0x1UL)

#define MIDF_REACT_OKAY                 (0x0UL<<0)
#define MIDF_REACT_DECERR               (0x1UL<<0)
#define MIDF_REACT_LOW                  (0x0UL<<1)
#define MIDF_REACT_HIGH                 (0x1UL<<1)

#define MIDF_SPECULATION_OFF           (1UL)
#define MIDF_SPECULATION_ON             (0UL)
//#define MIDF_NEW

#ifdef MIDF_NEW
typedef struct
{
    uint32                              no_of_regions   :  5;   //  [04:00]
    uint32                              :  3;                   //  [07:05]
    uint32                              address_width   :  6;   //  [13:08]
    uint32                              : 10;                   //  [23:14]
    uint32                              no_of_filters   :  2;   //  [25:24]
    uint32                              :  6;                   //  [31:26]
} CTRL;

typedef union
{
    uint32                              nReg;
    CTRL                                bReg;
} CTRL_U;

typedef struct
{
    uint32                              reaction_value  :  2;   //  [01:00]
    uint32                              : 30;                   //  [31:02]
} ACTION;

typedef union
{
    uint32                              nReg;
    ACTION                              bReg;
} ACTION_U;

typedef struct
{
    uint32                              open_request    :  8;   //  [07:00]
    uint32                              :  8;                   //  [15:08]
    uint32                              open_status     :  8;   //  [23:16]
    uint32                              :  8;                   //  [31:24]
} GATEKEEP;

typedef union
{
    uint32                              nReg;
    GATEKEEP                            bReg;
} GATE_KEEP_U;

typedef struct
{
    uint32                              read_speculation    :  1;   // [   00]
    uint32                              write_speculation   :  1;   // [   01]
    uint32                              : 30;                       // [31:02]
} SPECULATION;

typedef union
{
    uint32                              nReg;
    SPECULATION                         bReg;
} SPECULATION_U;

typedef struct
{
    uint32                              status          :  4;   //  [03:00]
    uint32                              :  4;                   //  [07:04]
    uint32                              overrun         :  4;   //  [11:08]
    uint32                              :  4;                   //  [15:12]
    uint32                              overlap         :  4;   //  [19:16]
    uint32                              : 12;                   //  [31:20]
} INT_STS;

typedef union
{
    uint32                              nReg;
    INT_STS                             bReg;
} INT_STATUS_U;

typedef struct
{
    uint32                              clear           :  4;   //  [03:00]
    uint32                              : 28;                   //  [31:04]
} INT_CLR;

typedef union
{
    uint32                              nReg;
    INT_CLR                             bReg;
} INT_CLEAR_U;

typedef struct
{
    // Fail Address Low
    uint32                              fail_addr_low   : 32;   // [31:00]
    // Fail Address High
    uint32                              fail_addr_high  : 32;   // [31:00]
    // Fail Ctrl
    uint32                              : 20;                   // [19:00]
    uint32                              privileged      :  1;   // [   20]
    uint32                              nonsecure       :  1;   // [   21]
    uint32                              :  2;                   // [23:22]
    uint32                              write           :  1;   // [24]
    uint32                              :  7;                   // [31:25]
    // Fail ID
    uint32                              id              :  8;   // [07:00]
    uint32                              : 16;                   // [23:08]
    uint32                              vnet            :  4;   // [27:24]
    uint32                              :  4;                   // [31:28]
} FAIL_CTRL;

typedef union
{
    uint32                              nReg[4];
    FAIL_CTRL                           bReg;
} FAIL_CTRL_U;

typedef struct
{
    // Base Address LOW nReg[0]
    uint32                              : 12;                   //  [11:00]
    uint32                              base_addr_low   : 20;   //  [31:12]

    // Base Address HIGH nReg[1]
    uint32                              base_addr_high  :  2;   //  [01:00]
    uint32                              : 30;                   //  [31:02]

    // Top Address LOW  nReg[2]
    uint32                              : 12;                   //  [11:00]
    uint32                              top_addr_low    : 20;   //  [31:12]

    // Top Address HIGH  nReg[3]
    uint32                              top_addr_high   :  2;   //  [01:00]
    uint32                              : 30;                   //  [31:02]

    // Attribute               nReg[4]
    uint32                              filter0_en      :  1;   //  [   00]
    uint32                              :  31;                  //  [01:31]

    // NSAID                    nReg[5]
    uint32                              nsaid_rd_en     : 16;   //  [15:00]
    uint32                              nsaid_wr_en     : 16;   //  [31:16]

    // Reserved         nReg[6,7]
    uint32                              reserved_0      : 32;   //  [31:00]
    uint32                              reserved_1      : 32;   //  [63:32]
} REGION_CTRL;

typedef union
{
    uint32                              nReg[8];
    REGION_CTRL                         bReg;
} REGION_CTRL_U;

typedef struct _MIDF_TZC400_
{
    CTRL_U                              uCTRL;                  // 0x00
    ACTION_U                            uACTION;                // 0x04
    GATE_KEEP_U                         uGATE_KEEP;             // 0x08
    SPECULATION_U                       uSPECULATION;           // 0x0C
    INT_STATUS_U                        uINT_STATUS;            // 0x10
    INT_CLEAR_U                         uINT_CLEAR;             // 0x14
    uint32                              reserved0[2];           // 0x18~0x1C
    FAIL_CTRL_U                         uFAIL_CTRL[4];          // 0x20~0x2C,0x30~0x3C,0x40~0x4C,0x50~0x5C
    uint32                              reserved1[40];          // 0x60~0xFC
    REGION_CTRL_U                       uREGION_CTRL[9];        // 0x100~0x11C,0x120~0x13C, ... 0x200~0x21C
    uint32                              reserved2[64*13+4*11];  // 0x220 ~ 0xFEC
    uint32                              uPERIPH_ID_4;           // 0xFD0
    uint32                              uPERIPH_ID_5;           // 0xFD4
    uint32                              uPERIPH_ID_6;           // 0xFD8
    uint32                              uPERIPH_ID_7;           // 0xFDC
    uint32                              uPERIPH_ID_0;           // 0xFE0
    uint32                              uPERIPH_ID_1;           // 0xFE4
    uint32                              uPERIPH_ID_2;           // 0xFE8
    uint32                              uPERIPH_ID_3;           // 0xFEC
    uint32                              uCOMPONENT_ID_0;        // 0xFF0
    uint32                              uCOMPONENT_ID_1;        // 0xFF4
    uint32                              uCOMPONENT_ID_2;        // 0xFF8
    uint32                              uCOMPONENT_ID_3;        // 0xFFC
} MIDF_TZC400_t;

//----------------------------------------------------------------------------------------------------------------------------
// Register structure
//----------------------------------------------------------------------------------------------------------------------------
// MID_XS
typedef struct
{
    uint32                              MID_AR_XS0          :  4;   // [03:00], RW
    uint32                              MID_AW_XS0          :  4;   // [07:00], RW
    uint32                              reserved0           :  8;   // [15:08]
    uint32                              MID_AR_XS1          :  4;   // [19:16], RW
    uint32                              MID_AW_XS1          :  4;   // [23:20], RW
    uint32                              reserved1           :  8;   // [31:24]
} MC_MID_XS;

typedef union
{
    uint32                              nReg;
    MC_MID_XS                           bReg;
} MC_MID_XS_U;

// MID_HS2
typedef struct
{
    uint32                              MID_HR_S2_TO_HS2    :  4;   // [03:00], RW
    uint32                              MID_HW_S2_TO_HS2    :  4;   // [07:00], RW
    uint32                              MID_HR_S3_TO_HS2    :  4;   // [11:08], RW
    uint32                              MID_HW_S3_TO_HS2    :  4;   // [15:12], RW
    uint32                              MID_HR_S4_TO_HS2    :  4;   // [19:16], RW
    uint32                              MID_HW_S4_TO_HS2    :  4;   // [23:20], RW
    uint32                              reserved0           :  8;   // [31:24]
} MC_MID_HS2;

typedef union
{
    uint32                              nReg;
    MC_MID_HS2                          bReg;
} MC_MID_HS2_U;

// MID_HS3
typedef struct
{
    uint32                              MID_HR_S0_TO_HS3    :  4;   // [03:00], RW
    uint32                              MID_HW_S0_TO_HS3    :  4;   // [07:00], RW
    uint32                              MID_HR_S1_TO_HS3    :  4;   // [11:08], RW
    uint32                              MID_HW_S1_TO_HS3    :  4;   // [15:12], RW
    uint32                              MID_HR_S2_TO_HS3    :  4;   // [19:16], RW
    uint32                              MID_HW_S2_TO_HS3    :  4;   // [23:20], RW
    uint32                              reserved0           :  8;   // [31:24]
} MC_MID_HS3;

typedef union
{
    uint32                              nReg;
    MC_MID_HS3                          bReg;
} MC_MID_HS3_U;

// MID_HS4
typedef struct
{
    uint32                              MID_HR_S0_TO_HS4    :  4;   // [0][03:00], RW
    uint32                              MID_HW_S0_TO_HS4    :  4;   // [0][07:00], RW
    uint32                              MID_HR_S1_TO_HS4    :  4;   // [0][11:08], RW
    uint32                              MID_HW_S1_TO_HS4    :  4;   // [0][15:12], RW
    uint32                              MID_HR_S2_TO_HS4    :  4;   // [0][19:16], RW
    uint32                              MID_HW_S2_TO_HS4    :  4;   // [0][23:20], RW
    uint32                              MID_HR_S3_TO_HS4    :  4;   // [0][27:24], RW
    uint32                              MID_HW_S3_TO_HS4    :  4;   // [0][31:28], RW
    uint32                              MID_HR_S4_TO_HS4    :  4;   // [1][03:00], RW
    uint32                              MID_HW_S4_TO_HS4    :  4;   // [1][07:00], RW
    uint32                              MID_HR_S5_TO_HS4    :  4;   // [1][11:08], RW
    uint32                              MID_HW_S5_TO_HS4    :  4;   // [1][15:12], RW
    uint32                              : 16;   // [1][31:16], RW
} MC_MID_HS4;

typedef union
{
    uint32                              nReg[2];
    MC_MID_HS4                          bReg;
} MC_MID_HS4_U;

// MID_HS5
typedef struct
{
    uint32                              MID_HR_S0_TO_HS5    :  4;   // [0][03:00], RW
    uint32                              MID_HW_S0_TO_HS5    :  4;   // [0][07:00], RW
    uint32                              MID_HR_S1_TO_HS5    :  4;   // [0][11:08], RW
    uint32                              MID_HW_S1_TO_HS5    :  4;   // [0][15:12], RW
    uint32                              MID_HR_S2_TO_HS5    :  4;   // [0][19:16], RW
    uint32                              MID_HW_S2_TO_HS5    :  4;   // [0][23:20], RW
    uint32                              MID_HR_S3_TO_HS5    :  4;   // [0][27:24], RW
    uint32                              MID_HW_S3_TO_HS5    :  4;   // [0][31:28], RW
    uint32                              MID_HR_S4_TO_HS5    :  4;   // [1][03:00], RW
    uint32                              MID_HW_S4_TO_HS5    :  4;   // [1][07:00], RW
    uint32                              MID_HR_S5_TO_HS5    :  4;   // [1][11:08], RW
    uint32                              MID_HW_S5_TO_HS5    :  4;   // [1][15:12], RW
    uint32                              : 16;   // [1][31:16], RW
} MC_MID_HS5;

typedef union
{
    uint32                              nReg[2];
    MC_MID_HS5                          bReg;
} MC_MID_HS5_U;

// MID configuration soft fault status
typedef struct
{
    uint32                              REG_MID_XS          :  1;   // [   00], RW
    uint32                              REG_MID_HS2         :  1;   // [   01], RW
    uint32                              REG_MID_HS3         :  1;   // [   02], RW
    uint32                              REG_MID_HS4_0       :  1;   // [   03], RW
    uint32                              REG_MID_HS4_1       :  1;   // [   04], RW
    uint32                              REG_MID_HS5_0       :  1;   // [   05], RW
    uint32                              REG_MID_HS5_1       :  1;   // [   06], RW
    uint32                              REG_MID_WR_PW       :  1;   // [   07], RW
    uint32                              REG_MID_WR_LOCK     :  1;   // [   08], RW
    uint32                              : 23;   // [31:09], Reserved
} MC_MID_SOFT_FAULT_EN;

typedef union
{
    uint32                              nReg;
    MC_MID_SOFT_FAULT_EN                bReg;
} MC_MID_SOFT_FAULT_EN_U;

// Micom configuration soft control
typedef struct
{
    uint32                              SOFT_FAULT_EN       :  1;   // [   00]
    uint32                              reserved0           :  3;   // [04:01]
    uint32                              SOFT_FAULT_TEST_EN  :  1;   // [   05]
    uint32                              reserved1           :  3;   // [07:06]
    uint32                              SOFT_FAULT_REQ_INIT :  1;   // [   08]
    uint32                              reserved2           :  7;   // [15:09]
    uint32                              TIMEOUT_VALUE       : 12;   // [27:16]
    uint32                              reserved3           :  4;   // [31:28]
} MC_MID_SOFT_FAULT_CTRL;

typedef union
{
    uint32                              nReg;
    MC_MID_SOFT_FAULT_CTRL              bReg;
} MC_MID_SOFT_FAULT_CTRL_U;

// Micom configuration soft control status
typedef struct
{
    uint32                              SOFT_FAULT_STS      :  1;   // [   00]
    uint32                              reserved0           : 15;   // [15:01]
    uint32                              SOFT_FAULT_REQ      :  1;   // [   16]
    uint32                              reserved1           :  7;   // [23:17]
    uint32                              SOFT_FAULT_ACK      :  1;   // [   24]
    uint32                              reserved2           :  7;   // [31:25]
} MC_MID_SOFT_FAULT_STS;

typedef union
{
    uint32                              nReg;
    MC_MID_SOFT_FAULT_STS               bReg;
} MC_MID_SOFT_FAULT_STS_U;

//----------------------------------------------------------------------------------------------------------------------------
// MID configuration register structure
//----------------------------------------------------------------------------------------------------------------------------
typedef struct _MIDFConfigRegister_
{
    MC_MID_XS_U                         uMID_XS;                // 0x00
    MC_MID_HS2_U                        uMID_HS2;               // 0x04
    MC_MID_HS3_U                        uMID_HS3;               // 0x08
    MC_MID_HS4_U                        uMID_HS4;               // 0x0c ~ 0x10
    MC_MID_HS5_U                        uMID_HS5;               // 0x14 ~ 0x18
    uint32                              CFG_WR_PW;              // 0x1c
    uint32                              CFG_WR_LOCK;            // 0x20
    uint32                              reserved[3];            // 0x24 ~ 0x2c
    MC_MID_SOFT_FAULT_EN_U              uSOFT_FAULT_EN;         // 0x30
    MC_MID_SOFT_FAULT_EN_U              uSOFT_FAULT_STS;        // 0x34
    MC_MID_SOFT_FAULT_CTRL_U            uSOFT_FAULT_CTRL;       // 0x38
    MC_MID_SOFT_FAULT_STS_U             uSOFT_FAULT_CTRL_STS;   // 0x3c
} MIDFConfigRegister_t;
#endif /* MIDF_NEW */

#endif  // ( MCU_BSP_SUPPORT_DRIVER_MIDF == 1 )

#endif  // MCU_BSP_MIDF_DEV_HEADER


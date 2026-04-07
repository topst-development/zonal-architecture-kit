// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : fwupdate.h
*
*   Copyright (c) Telechips Inc.
*
*   Description : Firmware update
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_FWUPDATE_HEADER
#define MCU_BSP_FWUPDATE_HEADER

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define SNOR_NO_USE                     1U

//  FW UPDATE Device
#define FWUD_DEV_UART                   (0U)
#define FWUD_DEV_SPI                    (1U)

#if (VCP_MODULE_BOARD == 1)
    #define FWUD_UART_CH                (UART_CH5)
#else
    #define FWUD_UART_CH                (UART_CH1)
#endif

#define FWUD_UART_BAUDRATE              (921600UL)

#define FWUD_SPI_CH                     (0)

//  FWUD BUNCH SIZE
#define BUNCH_SIZE                      (4096)

//  TASK STACK SIZE
#define FWUD_TASK_STK_SIZE              512UL

//  FW UPDATE Command
#define FWUD_CMD_START                  0x44555746  // "FWUD"

#define FWUD_CMD_READY                  0xAAAA0010U
#define FWUD_CMD_WRITE                  0xAAAA0011U
#define FWUD_CMD_VERSION                0xAAAA0012U
#define FWUD_CMD_CHIP_INFO              0xAAAA0013U

//  FW UPDATE Response Ack Type
#define FWUD_RSP_ACK                    0xFFFF0010U
#define FWUD_RSP_NACK                   0xFFFF0011U
#define FWUD_RSP_NYET                   0xFFFF0012U
#define FWUD_FAIL_CALC_CRC_FILE         0xFFFF0100U
#define FWUD_INVALID_CMD                0xFFFF0101U
#define FWUD_FAIL_CALC_CRC_CMD          0xFFFF0102U

//  FW UPDATE Storage Type (parameter 1 in READ/WRITE CMD)
#define FWUD_STOR_TYPE_SNOR             0xAAAA1001U
#define FWUD_STOR_TYPE_EFLASH           0xAAAA1002U

//  FW UPDATE Error Code
#define FWUD_ERR_STOR_IO                0xE002U
#define FWUD_ERR_INVAL_CMD              0xE003U
#define FWUD_ERR_INVAL_PARAM            0xE004U
#define FWUD_ERR_UNKNOWN                0xE005U

//  REMAP
#define FWUD_REMAP_FIELD                16U
#define REMAP_MODE_0                    0U
#define REMAP_MODE_1                    1U
#define REMAP_MODE_2                    2U

//  FW UPDATE Destination Address
#define eFLASH_BANK0_ADDR               0x20000000U
#define eFLASH_BANK2_ADDR               0x20100000U
#define SNOR_BANK_ADDR                  0x40000000U

#define DEST_eFLASH_BANK0               0U
#define DEST_eFLASH_BANK2               1U
#define DEST_SNOR_BANK                  2U

//  LDT1 Address
#define LDT1_BASE_ADDR                  0xA1011800U
#define LDT1_PRODUCTION_OFFSET          0x300U
#define LDT1_ECC_OFFSET                 0x310U

#define LDT1_PRODUCTION_CODE            (*(SALReg32 *)(LDT1_BASE_ADDR + LDT1_PRODUCTION_OFFSET))
#define LDT1_ECC                        (*(SALReg32 *)(LDT1_BASE_ADDR + LDT1_ECC_OFFSET))

#define ONLY_eFLASH                     0U
#define eFLASH_N_extSNOR                1U

#define SINGLE_BANK                     0U
#define DUAL_BANK                       1U


#define FWUD_MCU_FW_BODY_SIZE_ADDR      0x01042104UL
#define FWUD_MCU_FW_START_ADDR          0x01043000UL
#define FWUD_UPDATER_FW_START_ADDR      0x00000200UL
#define FWUD_UPDATER_FW_SIZE            (192 * 1024)

#define FWUD_ADDR_ALIGN_SIZE            0x10


#ifdef __GNU_C__
#define FWUD_JUMP_TO_ADDR               {__asm__ ("ldr pc, =0x00000200");}
#else
#define FWUD_JUMP_TO_ADDR               {asm("ldr pc, =0x00000200");}
#endif

/*
***************************************************************************************************
*                                            Structures
***************************************************************************************************
*/

typedef struct FWUDPacket
{
    unsigned int                        cmd_type;
    unsigned int                        param0;
    unsigned int                        param1;
    unsigned int                        param2;
    unsigned int                        param3;
    unsigned int                        param4;

} __attribute__((packed)) FWUDPacket_t;

typedef struct FWUDInfo
{
    unsigned int                        package_name;   // tcc70xx
    unsigned int                        remap_mode;     // 0,1,2
    unsigned int                        dual_bank;      // single:0, dual:1
    unsigned int                        expand_flash;   // None:0, exist:1
    unsigned int                        dest_flash;     // eflash#0:0, eflash#2:1, SNOR:2
    unsigned int                        dest_addr;      // destination address
    unsigned int                        bank_size;      // bank size

} __attribute__((packed)) FWUDInfo_t;

/*
***************************************************************************************************
*                                        EXTERN FUNCTIONS
***************************************************************************************************
*/

extern void FWUD_Init
(
    void
);

extern void FWUD_Process
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )

#endif  // MCU_BSP_FWUPDATE_HEADER


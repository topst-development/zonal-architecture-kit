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

#ifndef FWUPDATE_HEADER
#define FWUPDATE_HEADER

#include "spi_eccp.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

//  TASK STACK SIZE
#define FWUD_TASK_STK_SIZE              512UL

//#define FWUD_CMD_UPDATE_START           (0x01UL)

#if 1 //temp
typedef enum FWUDCmdType
{
    FWUD_CMD_UPDATE_START               = 0x01,
    FWUD_CMD_UPDATE_READY               = 0x02,
    FWUD_CMD_UPDATE_FW_START            = 0x03,
    FWUD_CMD_UPDATE_FW_READY            = 0x04,
    FWUD_CMD_UPDATE_FW_SEND             = 0x05,
    FWUD_CMD_UPDATE_FW_SEND_ACK         = 0x06,
    FWUD_CMD_UPDATE_FW_DONE             = 0x07,
    FWUD_CMD_UPDATE_FW_COMPLETE         = 0x08,
    FWUD_CMD_UPDATE_DONE                = 0x09,
    FWUD_CMD_UPDATE_COMPLETE            = 0x0A,
    FWUD_CMD_UPDATE_MAX                 = 0x0B
} FWUDCmdType_t;

typedef enum FWUDAckIDType
{
    FWUD_ACK                            = 0x01,
    FWUD_NACK                           = 0x02
} FWUDAckIDType_t;

typedef enum FWUDNackType
{
    FWUD_NACK_NO                        = 0x00,
    FWUD_NACK_UPDATER_LOAD_FAIL         = 0x01,
    FWUD_NACK_FLASH_INIT_FAIL           = 0x02,
    FWUD_NACK_FLASH_ACCESS_FAIL         = 0x03,
    FWUD_NACK_FLASH_ERASE_FAIL          = 0x04,
    FWUD_NACK_FLASH_WRITE_FAIL          = 0x05,
    FWUD_NACK_CRC_ERR                   = 0x06,
    FWUD_NACK_COUNT_ERR                 = 0x07,
    FWUD_NACK_MAX                       = 0x08
} FWUDNackType_t;

#endif

#if 0
#define FWUD_MCU_FW_BODY_SIZE_ADDR      0x01022104UL
#define FWUD_MCU_FW_START_ADDR          0x01023000UL
#define FWUD_UPDATER_FW_START_ADDR      0x00000200UL
#define FWUD_UPDATER_FW_SIZE            (192 * 1024)
#define FWUD_UPDATER_HEADER_SIZE        0x200UL

#define FWUD_ADDR_ALIGN_SIZE            0x10
#endif


//  REMAP
#define FWUD_REMAP_FIELD                16U
#define REMAP_MODE_0                    0U
#define REMAP_MODE_1                    1U
#define REMAP_MODE_2                    2U

//  FW UPDATE Destination Address
#define eFLASH_BANK0_ADDR               0x20000000U
#define eFLASH_BANK2_ADDR               0x20100000U
#define SNOR_BANK_ADDR                  0x40000000U
#define REMAP_MEMORY_ADDR               0x01000000U

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

#define FWUD_HSM_FW_START_ADDR          0x00002000UL
#define FWUD_HSM_FW_SIZE		        0x1F0C0UL

#define FWUD_MCU_FW_HEADER_OFFSET       0x00042000UL
#define FWUD_MCU_FW_BODY_SIZE_OFFSET    0x00042104UL
#define FWUD_MCU_FW_START_OFFSET        0x00043000UL
#define FWUD_MCU_FW_HEADER_DUMMY_SIZE   0x100UL
#define FWUD_MCU_FW_HEADER_SIZE         0x100UL
#define FWUD_MCU_FW_ID                  0x02FFUL

#define FWUD_UPDATER_FW_START_ADDR      0x00000200UL
#define FWUD_UPDATER_FW_SIZE            (192 * 1024)
#define FWUD_UPDATER_HEADER_SIZE        0x200UL
#define FWUD_UPDATER_ID                 0x03FFUL

#define FWUD_ADDR_ALIGN_SIZE            0x10

#define FWUD_SNOR_ECC_EN                (0xA0F26154U)
#define FWUD_ECC_CFG_WR_PW              (0xA0F26048U)
#define FWUD_ECC_CFG_PW                 (0x5AFEACE5U)


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

typedef struct FWUDDev
{
    ECCPDev_t *                         dDev;
} FWUDDev_t;

/*
***************************************************************************************************
*                                        EXTERN FUNCTIONS
***************************************************************************************************
*/

extern void CreateFWUDTask
(
    void
);

extern void FWUD_Start
(
    uint16                              usCmd
);

#endif /* FWUPDATE_HEADER */


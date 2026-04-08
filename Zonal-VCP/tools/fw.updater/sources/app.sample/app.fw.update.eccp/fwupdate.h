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

#define SNOR_NO_USE                     1U

//  FW UPDATE UART Channel
#define FWUD_UART_CH                    (3)
#define FWUD_SPI_CH                     (0)

//  FWUD BUNCH SIZE
#define BUNCH_SIZE                      (2048)

//  TASK STACK SIZE
#define FWUD_TASK_STK_SIZE              512UL

//  FW UPDATE Command
#define FWUD_READY_CMD			        0xAAAA0010U
#define FWUD_WRITE_CMD			        0xAAAA0011U
#define FWUD_READ_CMD			        0xAAAA0012U
#define FWUD_FW_PING				    0xAAAA0013U
#define FWUD_STORAGE_INFO_CMD	        0xAAAA0014U
#define FWUD_LOW_FORMAT_CMD		        0xAAAA0015U
#define FWUD_VERSION_CMD			    0xAAAA0016U
#define FWUD_CHIP_INFO_CMD		        0xAAAA0017U

//  FW UPDATE Response Ack Type
#define FWUD_RSP_ACK                    0xFFFF0010U
#define FWUD_RSP_NACK                   0xFFFF0011U
#define FWUD_RSP_NYET                   0xFFFF0012U
#define FWUD_FAIL_CALC_CRC_FILE         0xFFFF0100U
#define FWUD_INVALID_CMD                0xFFFF0101U
#define FWUD_FAIL_CALC_CRC_CMD          0xFFFF0102U

//  FW UPDATE Storage Type (parameter 1 in READ/WRITE CMD)
#define FWUD_STOR_TYPE_SNOR				0xAAAA1001U
#define FWUD_STOR_TYPE_EFLASH			0xAAAA1002U

//  FW UPDATE Error Code
#define FWUD_ERR_STOR_IO				0xE002U
#define FWUD_ERR_INVAL_CMD				0xE003U
#define FWUD_ERR_INVAL_PARAM			0xE004U
#define FWUD_ERR_UNKNOWN				0xE005U

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

#define LDT1_PRODUCTION_CODE            (*(SALReg32 *)(LDT1_BASE_ADDR + LDT1_PRODUCTION_OFFSET))

#define ONLY_eFLASH                     0U
#define eFLASH_N_extSNOR                1U

#define SINGLE_BANK                     0U
#define DUAL_BANK                       1U


/************************************* single update ************************************/
#define FWUD_CMD_START                  0x4E445746  // "FWDN"
#define FWUD_CMD_RECEIVE_HSM            0xFFFF0000
#define FWUD_CMD_RECEIVE_eFWDN          0xFFFF0001
#define FWUD_RESPONSE_ACK               0xFFFF0010
#define FWUD_RESPONSE_NACK              0xFFFF0011
#define FWUD_REASON_FAIL_CALC_CRC       0xFFFF0100
#define FWUD_REASON_INVALID_CMD_TYPE    0xFFFF0101
#define FWUD_REASON_INVALID_CMD_CRC     0xFFFF0102

#define FWUD_HSM_SRAM_BASE              0xA0980000
//#define FWUD_SRAM_BASE                0x00000000
#define FWUD_SRAM_BASE                  0x00040000  // Temporary address

#define FWUD_R5_FW_VERSION_INVALID_VALUE    0xFFFFFFFF

#define FWUD_R5_FW_CERT_HEADER_SIZE_VALUE   0x00000200  // 512B
#define FWUD_R5_FW_CERT_SIZE_VALUE          0x00000100  // 256B
#define FWUD_R5_FW_HEADER_SIZE_VALUE        0x00000100  // 256B
#define FWUD_R5_FW_IMAGE_ID_VALUE           0x000002FF
#define FWUD_R5_FW_BANK0_INDEX_VALUE        0x00000001
#define FWUD_R5_FW_BANK1_INDEX_VALUE        0x00000002
#define FWUD_HSM_FW_BANK0_INDEX_VALUE       0x00000004
#define FWUD_HSM_FW_BANK1_INDEX_VALUE       0x00000008
#if 1 // CS
#define FWUD_HSM_FW_SIZE_VALUE              0x0001F000  // 126976B=124*1024B
#else
#define FWUD_HSM_FW_SIZE_VALUE              0x0000F000  // 61440B
#endif

#define FWUD_eFWDN_CERT_HEADER_OFFSET       0x00000000
#define FWUD_eFWDN_CERT_HEADER_SIZE_VALUE   0x00000200  // 512B
#define FWUD_eFWDN_IMAGE_OFFSET             0x00000200
#define FWUD_eFWDN_IMAGE_ID_VALUE           0x000002FF


//#define SNOR_SECTOR_SIZE 4096
#define MAX_FW_BUF_SIZE 480
#define SNOR_UPDATE_FLAG_OFFSET	0x000FF000
#define FWUD_UPDATE_FLAG 0x44554746 //FWUD


#define FWUD_MAX_SNOR_SIZE              (2 * 1024 * 1024) /* 2MBytes */
#define FWUD_MAX_EFLASH_SIZE            (2 * 1024 * 1024) /* 2MBytes */
#define FWUD_MAX_PFLASH_SIZE            FWUD_MAX_SNOR_SIZE + FWUD_MAX_EFLASH_SIZE /* 4MBytes */
#define FWUD_FLASH_SECTOR_SIZE          0x20


#define FWUD_HEADER_SIZE                11UL /*ECCP_HEADER_SIZE*/ + 2UL /*HEADER CRC*/

#define CMD1_ID_FWUD                    0x03
#define CMD1_H_IDX                      5   /* CMD1 High */
#define CMD1_L_IDX                      6   /* CMD1 Low */
#define CMD2_H_IDX                      7   /* CMD2 High */
#define CMD2_L_IDX                      8   /* CMD2 Low */


/*
***************************************************************************************************
*                                            Structures
***************************************************************************************************
*/
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

typedef enum FWUDStatusType
{
    FWUD_STATUS_IDLE                    = 0x00,
    FWUD_STATUS_READY                   = 0x01,
    FWUD_STATUS_MAX                     = 0x02
} FWUDStatusType_t;

typedef struct FWUDCmdInfo
{
    FWUDStatusType_t                    ciStatus; /* current status */
    FWUDCmdType_t                       ciCmd; /* current command */
    uint32                              ciFwDataCnt; /* current F/W data count */
    sint32                              ciError; /* current error type */
} FWUDCmdInfo_t;

typedef struct FWUDInfo
{
    FWUDCmdInfo_t                       iCurrentCmd;
    ECCPDev_t *                         iDev;
} FWUDInfo_t;


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


#endif /* FWUPDATE_HEADER */


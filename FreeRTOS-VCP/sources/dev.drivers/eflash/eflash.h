// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eflash.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/
//#include "swl.h"

#ifndef MCU_BSP_EFLASH_HEADER
#define MCU_BSP_EFLASH_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_EFLASH == 1 )

typedef enum {
    FCONT_RET_OK                   = (uint32)0u,
    FCONT_RET_TIMEOUT              = (uint32)1u,
    FCONT_RET_PARAM_OUTOFRANGE     = (uint32)2u,
    FCONT_RET_LENGTH_FAIL          = (uint32)3u,                 /* unlikely err report, human err, need to check code */
    FCONT_RET_DRV_NOT_INIT_YET     = (uint32)4u,
    FCONT_RET_INIT_FAIL            = (uint32)5u,
    FCONT_RET_NOT_ALLOWD_AREA      = (uint32)6u,
    FCONT_RET_D_SEMA_FAIL          = (uint32)7u,
}FCONT_RET_type;






typedef enum {
    FCONT_TYPE_PFLASH              = (uint32)0u,
    FCONT_TYPE_DFLASH              = (uint32)1u,
} EF_type;



/*
***************************************************************************************************
*                                       EFLASH_Init
*
* Init write buffer
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
uint32 EFLASH_Init
(
    void
);


/*
***************************************************************************************************
*                                       EFLASH_FWDN_LowFormat
*
* Init write buffer
*
* @param    EF_type uiEfType : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_LowFormat
(
    EF_type uiEfType
);

/*
***************************************************************************************************
*                                       EFLASH_FWDN_Read
*
* Copy data from EFLASH
*
* @param    uiAddress : Read Address offset(Source)
*           uiLength  : Read Size
*           pBuf      : Dest Buff
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_Read
(
    uint32  uiAddress,
    uint32  uiLength,
    void    *pBuf,
    EF_type uiEfType
);

/*
***************************************************************************************************
*                                       EFLASH_FWDN_Write
*
* Copy data from EFLASH
*
* @param    uiAddress : Read Address offset(Dest)
*           uiLength  : Read Size
*           pBuf      : Source Buff
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_Write
(
    uint32 uiAddress,
    uint32 uiLength,
    const uint8 *pBuf,
    EF_type uiEfType
);


/*
***************************************************************************************************
*                                       EFLASH_DFLASH_WriteByte
*
*  Write data to Dflash
*
* @param    uiAddress : start address to read
*           pBuffer   : Dest buffer
*           uiLength  : Dest buffer length
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_DFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength
);


/*
***************************************************************************************************
*                                       EFLASH_Erase
*
* Erase data from EFLASH
*
* @param    uiAddress : Read Address offset(Source)
*           uiSize    : Erase Size
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_Erase
(
    uint32 uiAddress,
    uint32 uiSize,
    EF_type uiEfType
);



uint32 EFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength,
    EF_type uiEfType
);

void FLASH_ReadLDT
(
    uint32 uiAddress,
    uint32 uiCount,
    uint32 *pBuf
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH == 1 )

#endif  // MCU_BSP_EFLASH_HEADER


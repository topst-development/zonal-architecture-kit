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

enum {
    EFLASH_RET_OK                       = 0u,
    EFLASH_RET_TIMEOUT                  = 1u,
    EFLASH_RET_PARAM_OUTOFRANGE         = 2u,
    EFLASH_RET_LENGTH_FAIL              = 3u,                 /* unlikely err report, human err, need to check code */
    EFLASH_RET_DRV_NOT_INIT_YET         = 4u,
    EFLASH_RET_INIT_FAIL                = 5u,
    EFLASH_RET_NOT_ALLOWD_AREA          = 6u,
    EFLASH_RET_D_SEMA_FAIL              = 7u,
};






typedef enum {
    EF_PFLASH                           = 0u,
    EF_DFLASH                           = 1u,
} EF_type;


/******************************************************************************
 * DataType name:  EflashRegisterType
 * Description:    Structure for hardware registers in pFlash peripheral
 * Remarks:        Target DataType.
 * Requirements:   None
 ******************************************************************************/
typedef struct
{
    volatile uint32                     FRWCON;
    volatile uint32                     FSHSTAT;
    volatile uint32                     reserved[10];
    volatile uint32                     TM_LDTCON;                  /* offset 0x30 */
    volatile uint32                     TM_FSH_CON;
    volatile uint32                     TM_ECCCON_1;
    volatile uint32                     TM_ECCFADDR_1;
    volatile uint32                     TM_DCT0;
    volatile uint32                     TM_DCT1;                    /* offset 0x44 */
    volatile uint32                     reserved1[6];
    volatile uint32                     TM_LDT0;                    /* offset 0x60 */
    volatile uint32                     TM_LDT1;
    volatile uint32                     reserved2[38];
    volatile uint32                     DCYCRDCON;                  /* offset 0x100 */
    volatile uint32                     DCYCWRCON;
    volatile uint32                     reserved3[62];
    volatile uint32                     FSHADDR;                    /* offset 0x200 */
    volatile uint32                     FSHWDATA_1;
    volatile uint32                     FSHWDATA_PRT_1;
    volatile uint32                     reserved4;
    volatile uint32                     FSHWDATA_2;                 /* offset 0x210 */
    volatile uint32                     FSHWDATA_3;
    volatile uint32                     FSHWDATA_4;
    volatile uint32                     FSHWDATA_PRT_2;
}EflashRegisterType;





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
* @param    EF_type uiEfType : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
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
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
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
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_Write
(
    uint32 uiAddress,
    uint32 uiLength,
    void *pBuf,
    EF_type uiEfType
);

#if (SWL_TOTALSIZE != 0u)
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
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
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
#endif

/*
***************************************************************************************************
*                                       EFLASH_Erase
*
* Erase data from EFLASH
*
* @param    uiAddress : Read Address offset(Source)
*           uiSize    : Erase Size
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
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

#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH == 1 )

#endif  // MCU_BSP_EFLASH_HEADER


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : sav.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#ifndef MCU_BSP_SWL_HEADER
#define MCU_BSP_SWL_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_SWL == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_EFLASH value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )


#define SWL_ERASE_PAGE             (256u)//for ref
#define SWL_DFLASH_MAX_SIZE        (0x40000u)//for ref



// The constants below are configurable
#define SWL_DATA_SIZE              (1016UL)//If this value exceeds 2k, dflash write operation cannot be guaranteed when bu off.
#define SWL_STARTOFFSET            (0x30000u)//need page align
#define SWL_TOTALSIZE              (0)//((SWL_DATA_SIZE+8u)*60u)//
//////////////////////////////////////////////////////////////////////////////////

#define SWL_DATA_WRITE_ATONCE      (8u)
#define SWL_READOFFSET             (0x30000000u)//same as (DFLASH_BASE_ADDR)

#if ( SWL_DATA_SIZE < SWL_DATA_WRITE_ATONCE)
#define SWL_DATA_SIZE_PAD          (SWL_DATA_WRITE_ATONCE)
#else
#define SWL_DATA_SIZE_PAD          ((SWL_DATA_SIZE + (SWL_DATA_WRITE_ATONCE-1u))/SWL_DATA_WRITE_ATONCE*SWL_DATA_WRITE_ATONCE)
#endif
#define SWL_DATA_HEADER_SIZE       (8u)
#define SWL_REGION_SIZE            (SWL_DATA_HEADER_SIZE + SWL_DATA_SIZE_PAD)
#define SWL_REGION_NO              (SWL_TOTALSIZE/SWL_REGION_SIZE)
#define SWL_REGION_PAGE_TOTAL      (SWL_TOTALSIZE/SWL_ERASE_PAGE)
#define SWL_FLASH_ENDURANCE        (100000u)


#if ((SWL_TOTALSIZE != 0u)&&( SWL_TOTALSIZE < (SWL_ERASE_PAGE*3)))
#error "The minimum SWL_TOTALSIZE must be more than three times the erase unit."
#elif ((SWL_TOTALSIZE != 0u)&&((SWL_STARTOFFSET+SWL_TOTALSIZE) > SWL_DFLASH_MAX_SIZE ))
#error "The area to be saved must be inside dflash."
#elif ((SWL_TOTALSIZE != 0u)&&(SWL_REGION_PAGE_TOTAL < 3u ))
#error "The number of pages must be at least three."
#elif ((SWL_TOTALSIZE != 0u)&&( SWL_REGION_NO < 3u))
#error "The minimum SWL_REGION_NO must be more than three times the erase unit."
#endif

//#define WHEN_OVERWRITE_ERASEALL

#define SWL_MARKER                 (0x26365721u)

/******************************************************************************
 * DataType name:  SWLTypeHeader
 * Description:    Structure for
 * Remarks:        Target DataType.
 * Requirements:   None
 ******************************************************************************/
typedef struct
{
    volatile uint32                     Marker;
    volatile uint32                     Index;
}SWLTypeHeader;


typedef enum {
    SWL_NOSAVED_DATA                    = 0x10u,
    SWL_CASENORMAL                      = 0x11u, //prev & current index gap is one
    SWL_CASEIDXGAP                      = 0x12u, //prev & current index gap isn't one
    SWL_DATASPACENOTREADY               = 0x13u,
    SWL_WRITE_ERR                       = 0x14u,
    SWL_ERRUNEXPECTED                   = 0x15u,
    SWL_ENDURANCE_END                   = 0x16u,
} SWL_SearchIdxResultType;



/*
***************************************************************************************************
*                                       SWL_Init
*
* Init SAV drv
*   read last saved data and erase unnecessary data, and check index
*
* @param    void * lastSavedData
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
uint32 SWL_Init
(
    void * lastSavedData
);


/*
***************************************************************************************************
*                                       SWL_Write
*
*
*
* @param    void * SrcData
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
uint32 SWL_Write
(
    const void * SrcData
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SWL == 1 )

#endif  // MCU_BSP_SWL_HEADER


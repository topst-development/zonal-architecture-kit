// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : midf.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_MIDF_HEADER
#define MCU_BSP_MIDF_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_MIDF == 1 )

/**
 * MID Filter ID list.
 */
#define MIDF_ID_DMA0                    (0UL)
#define MIDF_ID_DMA1                    (1UL)
#define MIDF_ID_CAN0                    (2UL)
#define MIDF_ID_CAN1                    (3UL)
#define MIDF_ID_CAN2                    (4UL)
#define MIDF_ID_GPSB0                   (5UL)
#define MIDF_ID_GPSB1                   (6UL)
#define MIDF_ID_GPSB2                   (7UL)
#define MIDF_ID_GPSB3_4_5               (8UL)
#define MIDF_ID_UART0                   (9UL)
#define MIDF_ID_UART1                   (10UL)
#define MIDF_ID_UART2                   (11UL)
#define MIDF_ID_UART3_4_5               (12UL)
#define MIDF_ID_CR5                     (13UL)
#define MIDF_ID_JTAG                    (14UL)
#define MIDF_ID_AP                      (15UL)


#define MIDF_REGION_MIN_SIZE            (0x1000UL)
/**
 * MID Filter Action Type
 */
#define MIDF_TYPE_READ                  (1UL << 0)
#define MIDF_TYPE_WRITE                 (1UL << 1)

/**
 * MID Result
 */
#define MIDF_SUCCESS                    (0L)
#define MIDF_ERROR                      (-1L)
#define MIDF_ERROR_NOT_INITIALIZE       (-2L)
#define MIDF_ERROR_NO_RESOURCE          (-3L)
#define MIDF_ERROR_BAD_PARAMETERS       (-4L)
#define MIDF_ERROR_BAD_RANGES           (-5L)
#define MIDF_ERROR_NOT_FOUND            (-6L)

/**
 * Initialize MID Filter
 */
/*
***************************************************************************************************
*                                          MIDF_FilterInitialize
*
* Register MID Filter
*
* @param    None
* @return   None
*
* Notes
*
***************************************************************************************************
*/
void MIDF_FilterInitialize
(
    void
);

/*
***************************************************************************************************
*                                          MIDF_RegisterFilterRegion
*
* Register MID Filter
*
* @param    uiMemoryBase [in] Base address of memory buffer for 'id'. it should be 4kb aligned
* @param    uiSize [in] Size of memory buffer for 'id'. it should be 4kb aligned
* @param    uiID[in] Master id.
* @param    uiType [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/
int32 MIDF_RegisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
);

/*
***************************************************************************************************
*                                          MIDF_UnregisterFilterRegion
*
* Unregister MID Filter
*
* @param    uiMemoryBase [in] Base address of memory buffer for 'id'.
* @param    uiSize [in] Size of memory buffer for 'id'.
* @param    uiID [in] Master id.
* @param    uiType [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/
int32 MIDF_UnregisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_MIDF == 1 )

#endif  // MCU_BSP_MIDF_HEADER


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gdma_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_GDMA_TEST_HEADER
#define MCU_BSP_GDMA_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_GDMA != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_GDMA value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_GDMA != 1 )

/*
***************************************************************************************************
*                                         FUNCTION DECLEARATION
***************************************************************************************************
*/

 /*
***************************************************************************************************
*                                          GDMA_SampleForM2M
*
* Sample test function for transferring data using dma by memory to memory scheme.
*
* @param    none.
* @return
*
* Notes
*
***************************************************************************************************
*/

void GDMA_SampleForM2M
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#endif  // MCU_BSP_GDMA_TEST_HEADER


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : fmu_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef FMU_TEST_HEADER
#define FMU_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#include <sal_internal.h>
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          FMU_StartFmuTest
*
* @param    ucMode [in]
*
* Notes
*
***************************************************************************************************
*/
void FMU_StartFmuTest
(
    int32                               ucMode
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )

#endif  //_FMU_TEST_HEADER_


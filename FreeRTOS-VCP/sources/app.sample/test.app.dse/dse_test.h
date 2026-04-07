// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : dse_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_DSE_TEST_HEADER
#define MCU_BSP_DSE_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_DSE != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_DSE value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_DSE != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#include <sal_internal.h>


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                       DSE_Test_R5AccessFault
*
* Default Slave Error Sample test code
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
void DSE_Test_R5AccessFault
(
    void
);

/*
***************************************************************************************************
*                                       DSE_Test_SoftFaultCheck
*
* DSE Soft fault check test code
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void DSE_Test_SoftFaultCheck
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )

#endif  // MCU_BSP_DSE_TEST_HEADER


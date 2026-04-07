// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eflash_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_EFLASH_TEST_HEADER
#define MCU_BSP_EFLASH_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_EFLASH value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_SFMC != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_SFMC value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_SFMC != 1 )

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
*                                       EFLASHTest_ECC
*
* EFLASH ECC test
*
* @param    none
*
* @return   none
*
* Notes
*           [SM-PV] 05. Data eFLASH and Controller - DataFlash.01, ECC (Error Correcting Code)
*           Flash memory must be able to detect 1-bit or 2-bit data errors occurring in flash
*           memory through ECC.
***************************************************************************************************
*/
void EFLASHTest_ECC
(
    uint32 uiFlag
);

/*
***************************************************************************************************
*                                       EFLASHTest_RESET
*
* EFLASH RESET test
*
* @param    none
*
* @return   none
*
* Notes
*           [SM-PV] 05. Data eFLASH and Controller - DataFlash.02, Reset
*           If a system reset occurs during programming in Flash, the reset is prevented
*           until the recording is completed to ensure the integrity of the recorded data.
***************************************************************************************************
*/

void EFLASHTest_RESET
(
    uint32 uiFlag
);



/*
***************************************************************************************************
*                                       EFLASHTest_Int_Test
*
* EFLASH RESET test
*
* @param    none
*
* @return   none
*
* Notes
*           Program Flash Controller.. interrupt test
***************************************************************************************************
*/

uint32 EFLASHTest_Int_Test(void);



uint32 EFLASHTest_Int_DFLASHTest(void );

void EFLASHTest_DflashEcc(uint32 uiFlag);

void EFLASHTest_DflashAccessControl(uint32 uiFlag);
void EFLASHTest_PflashAccessControl(uint32 uiFlag);

void EFLASHTest_flashEraseWrite(uint32 type);
void EFLASHTest_snorEraseWrite(uint32 type);

#endif  // (MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )

#endif  // MCU_BSP_EFLASH_TEST_HEADER


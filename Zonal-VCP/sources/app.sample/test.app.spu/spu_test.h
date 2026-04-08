// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spu_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_SPU_TEST_HEADER
#define MCU_BSP_SPU_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_I2S != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_I2S value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_I2S != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_SPU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_SPU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_SPU != 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO != 1 )
    #error MCU_BSP_SUPPORT_TEST_APP_AUDIO value must be 1.
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO != 1 )

/*
***************************************************************************************************
*                                           SPU_SelectTestCase
*
* @param test case number
* @return
*
* Notes
*
***************************************************************************************************
*/
void SPU_SelectTestCase
(
    uint8 ucArgc,
    void* pArgv[]
);

/*
***************************************************************************************************
*                                           SPU_Test_SSL_With_Key
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
void SPU_Test_SSL_With_Key
(
    unsigned int                        key_num
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )

#endif  // MCU_BSP_SPU_TEST_HEADER


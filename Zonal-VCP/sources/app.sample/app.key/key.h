// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : key.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_KEY_APP_HEADER
#define MCU_BSP_KEY_APP_HEADER

#if ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )
    #error MCU_BSP_SUPPORT_TEST_APP_ADC value must be 0.
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ADC ==1 )

/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/


#define KEY_MANY_PUSHKEY                (0x0)
#define KEY_ONLY_ROTARY                 (0x1)

#define KEY_DEVICE_ST_NO_OPEN           (0)
#define KEY_DEVICE_ST_OPENED            (1)
#define KEY_DEVICE_ST_ENABLED           (2)
#define KEY_DEVICE_ST_DISABLED          (3)


#define KEY_APP_TASK_STK_SIZE           (256)

#define KEY_STAT_RELEASED               (0)
#define KEY_STAT_PRESSED                (1)

#define KEY_TIME_LIMIT                  (30)


/*
***************************************************************************************************
*                                         MODULE INTERFACES
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          KeyAppCreate
*
*
*
*
* Notes
*
***************************************************************************************************
*/

void KEY_AppCreate(void);

#endif  // ( MCU_BSP_SUPPORT_APP_KEY == 1 )

#endif  // MCU_BSP_KEY_APP_HEADER


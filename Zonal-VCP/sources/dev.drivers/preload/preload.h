// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : preload.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_PRELOAD_HEADER
#define MCU_BSP_PRELOAD_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_PRELOAD == 1 )


#define PRELOAD_CLOCK_CONFIG_DEFAULT
//#define PRELOAD_CLOCK_CONFIG_LEGACY_1

/***************************************************************************************************
*
*
*
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
void PRELOAD_loadOnRam
(
    void
);


/*
***************************************************************************************************
*
*
*
*
* @param
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void PRELOAD_JOB
(
    void
)__attribute__ ((section (".codeonsram")));

#endif  // ( MCU_BSP_SUPPORT_DRIVER_PRELOAD == 1 )

#endif  // MCU_BSP_PRELOAD_HEADER


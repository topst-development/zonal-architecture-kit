// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spi_led.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_SPI_LED_HEADER
#define MCU_BSP_SPI_LED_HEADER

#if ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/


#include <sal_internal.h>


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

void SPILED_CreateAppTask
(
    void
);

void SPILED_StatusOn
(
    void
);

void SPILED_StatusOff
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

#endif  // MCU_BSP_SPI_LED_HEADER


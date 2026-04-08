// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : clock.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CLOCK_HEADER
#define MCU_BSP_CLOCK_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_CLOCK == 1 )

#include <sal_com.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define CKC_MHZ(x)                      (uint32)((x) * 1000000)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

void CLOCK_Init
(
    void
);

sint32 CLOCK_SetPllRate
(
    sint32                              iId,
    uint32                              uiRate
);

uint32 CLOCK_GetPllRate
(
    sint32                              iId
);

sint32 CLOCK_SetPllDiv
(
    sint32                              iId,
    uint32                              uiPllDiv
);

sint32 CLOCK_SetClkCtrlRate
(
    sint32                              iId,
    uint32                              uiRate
);

uint32 CLOCK_GetClkCtrlRate
(
    sint32                              iId
);

sint32 CLOCK_IsPeriEnabled
(
    sint32                              iId
);

sint32 CLOCK_EnablePeri
(
    sint32                              iId
);

sint32 CLOCK_DisablePeri
(
    sint32                              iId
);

uint32 CLOCK_GetPeriRate
(
    sint32                              iId
);

sint32 CLOCK_SetPeriRate
(
    sint32                              iId,
    uint32                              uiRate
);

sint32 CLOCK_IsIobusPwdn
(
    sint32                              iId
);

sint32 CLOCK_EnableIobus
(
    sint32                              iId,
    boolean                             bEn
);

sint32 CLOCK_SetIobusPwdn
(
    sint32                              iId,
    boolean                             bEn
);

sint32 CLOCK_SetSwReset
(
    sint32                              iId,
    boolean                             bReset
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CLOCK == 1 )

#endif  // MCU_BSP_CLOCK_HEADER


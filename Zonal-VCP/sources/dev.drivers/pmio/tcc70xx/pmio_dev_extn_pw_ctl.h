// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : pmio_dev_extn_pw_ctl.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef  MCU_BSP_PMIO_DEV_EXTN_PW_CTL_HEADER
#define  MCU_BSP_PMIO_DEV_EXTN_PW_CTL_HEADER
/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <sal_internal.h>
#include <gic_enum.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define PMIO_EXTN_VA_INTERRUPT_SRC_PMIC_RST  ((uint32)(GIC_EXT3))
#define PMIO_EXTN_VA_INTERRUPT_PRIO_PMIC_RST  (1UL)

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

typedef struct
{
    uint32 uiPwrEnPin;
    uint32 uiAlivePwrConPin;
    uint32 uiStrModePin;
    uint32 uiPmicRstPin;
    uint32 uiTccRstPin;
}PMIOEXTNExtPwrPins_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
void PMIO_EXTN_Init
(
    PMIOEXTNExtPwrPins_t tExtPwrPins
);

void PMIO_EXTN_SetStr
(
    uint8 ucEn
);

void PMIO_EXTN_SetPower
(
    uint8 ucEn
);

uint32 PMIO_EXTN_GetUsingGpk32
(
    void
);


#endif /*MCU_BSP_PMIO_DEV_EXTN_PW_CTL_HEADER*/

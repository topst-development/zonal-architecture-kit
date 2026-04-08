// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : main.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_BASE == 1 )

#include <main.h>

#include <sal_api.h>
#include <app_cfg.h>
#include <debug.h>
#include <bsp.h>
#include <gic.h>
//#include <timer.h>
#include <version.h>

#include <fwupdate.h>
#include <hsm_manager.h>
#include "eflash.h"

/*
***************************************************************************************************
*                                         GLOBAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          cmain
*
* This is the standard entry point for C code.
*
* Notes
*   It is assumed that your code will call main() once you have performed all necessary
*   initialization.
*
***************************************************************************************************
*/
static void secureboot_enable(void);

void cmain (void)
{
	uint32 uiRegVal = 0;

    BSP_PreInit(); /* Initialize basic BSP functions */

    BSP_Init(); /* Initialize BSP functions */

    mcu_printf("\n Welcome to Updater F/W\n");

    mcu_printf("\nR5 Updater Version: V%d.%d.%d\n",
           SDK_MAJOR_VERSION,
           SDK_MINOR_VERSION,
           SDK_PATCH_VERSION);

    FWUD_Init();

	uiRegVal = SAL_ReadReg(0xA0F28828);
	if ((uiRegVal & 0x20000000) != 0) {
		secureboot_enable();
	}
	else {
		FWUD_Process();
	}
    while(1);
}

static void secureboot_enable()
{
	sint32 ret;
	uint32 addr = TEMP_SRAM;

	ret = HSM_GetMFCERT();
	if (ret != HSM_OK) {
		mcu_printf("Failed to get secure boot key\n");
		goto out;
	}
	mcu_printf("[SECUREBOOT-MF] STEP1 - Decrypt MFCERT to get the Key\n");

	EFLASH_Init();
	ret = EFLASH_FWDN_Read(B_LUN0_HSM_FW_IMAGE_OFFSET, HSM_FW_SIZE, (void *)addr, EF_PFLASH);
	if (ret != EFLASH_RET_OK) {
		mcu_printf("Error HAL_Write(%d)\n", ret);
		goto out;
	}

	ret = HSM_GetSecureHSM(addr);
	if (ret != HSM_OK) {
		mcu_printf("Failed to get secure HSM F/W\n");
		goto out;
	}

	ret = EFLASH_FWDN_Write(B_LUN1_HSM_FW_IMAGE_OFFSET, HSM_FW_SIZE, (void *)addr, EF_PFLASH);
	if (ret != EFLASH_RET_OK) {
		mcu_printf("Error HAL_Write(%d)\n", ret);
		goto out;
	}
	mcu_printf("[SECUREBOOT-MF] STEP2 - Convert 2ND HSM image to secure image\n");

	ret = HSM_SBEnable();
	if (ret != HSM_OK) {
		mcu_printf("HSM_SBEnable Fail\n");
		goto out;
	}
	mcu_printf("[SECUREBOOT-MF] STEP3 - Secure Boot Key Writing\n");

	ret = EFLASH_FWDN_Read(B_LUN1_HSM_FW_IMAGE_OFFSET, HSM_FW_SIZE, (void *)addr, EF_PFLASH);
	if (ret != EFLASH_RET_OK) {
		mcu_printf("Error HAL_Write(%d)\n", ret);
		goto out;
	}

	ret = EFLASH_FWDN_Write(B_LUN0_HSM_FW_IMAGE_OFFSET, HSM_FW_SIZE, (void *)addr, EF_PFLASH);
	if (ret != EFLASH_RET_OK) {
		mcu_printf("Error HAL_Write(%d)\n", ret);
		goto out;
	}
	mcu_printf("[SECUREBOOT-MF] STEP4 - Convert 1ST HSM image to secure image\n");

	PMU_LOCK = 0x5AFEACE5u;
	COLD_RESET_REQ |= (0x1u);
out:
	return;
}

#endif  // ( MCU_BSP_SUPPORT_APP_BASE == 1 )


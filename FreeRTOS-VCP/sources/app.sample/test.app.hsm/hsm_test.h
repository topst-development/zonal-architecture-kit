// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : hsm_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM TEST
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_HSM_TEST_HEADER
#define MCU_BSP_HSM_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1)

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_HSM != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_HSM value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_HSM != 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
/* HSM ioctl cmd */
// clang-format off
#define HSM_SET_KEY_FROM_NVM        (0u)

#define HSM_RUN_AES                 (1U)
#define HSM_RUN_AES_BY_KT           (2U)

#define HSM_GEN_MAC                 (3U)
#define HSM_GEN_MAC_BY_KT           (4U)

#define HSM_GEN_SHA                 (5U)

#define HSM_RUN_ECDSA               (6U)
#define HSM_RUN_RSASSA              (7U)

#define HSM_ECDH_COMPUTE_PUBKEY     (8U)
#define HSM_ECDH_PHASE_I            (9U)
#define HSM_ECDH_PHASE_II           (10U)
#define HSM_PBKDF2                  (11U)

#define HSM_CERTIFICATE_PARSE       (12U)
#define HSM_CERTIFICATE_VERIFY      (13U)

#define HSM_GET_RNG                 (14U)
#define HSM_WRITE_NVM               (15U)
#define HSM_MBOX_ECC_TEST           (16U)

#define HSM_SELECT_BANK             (17U)

#define HSM_GET_VER                 (18U)

#define HSM_FULL                    (19U)
#define HSM_AGING                   (20U)
#define HSM_BECHMARK                (21U)
// clang-format on
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
int32 HSM_Test(void *pCommand, void *pValue);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )

#endif  // MCU_BSP_HSM_TEST_HEADER


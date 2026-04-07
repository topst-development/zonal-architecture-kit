// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wdt_pmu.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_WDT_PMU_HEADER
#define MCU_BSP_WDT_PMU_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
typedef enum WDTPMUResetType
{
    WDTPMU_RST_DIRECT                   = 0x0U,
    WDTPMU_RST_FMU_HANDLER              = 0x4U

} WDTPMUResetType_t;

typedef enum WDTPMU2003Type
{
    WDTPMU_2OO3_VOTE                    = 0x0U,
    WDTPMU_ZERO_0                       = 0x1U,
    WDTPMU_ZERO_1                       = 0x2U,
    WDTPMU_ZERO_2                       = 0x3U

} WDTPMU2oo3Type_t;


enum
{
    WDTPMU_ERR_ALREADY_DONE             = (uint32)SAL_DRVID_WDT + 10UL,
    WDTPMU_ERR_NOT_INITIALIZED          = (uint32)SAL_DRVID_WDT + 11UL,
    WDTPMU_ERR_INVALID_PARAM            = (uint32)SAL_DRVID_WDT + 12UL,
    WDTPMU_ERR_NOT_SUPPORT_MODE         = (uint32)SAL_DRVID_WDT + 13UL

};//WDTPMUErrorCode_t

/*
 * Function Indexes
 */
#define WDTPMU_API_COMMON_INDEX         (0x0)
#define WDTPMU_API_STOP                 (WDTPMU_API_COMMON_INDEX + 0)
#define WDTPMU_API_SET_SM_MODE          (WDTPMU_API_COMMON_INDEX + 1)
#define WDTPMU_API_KICK_ACT             (WDTPMU_API_COMMON_INDEX + 2)
#define WDTPMU_API_RESET_SYS            (WDTPMU_API_COMMON_INDEX + 3)
#define WDTPMU_API_INIT                 (WDTPMU_API_COMMON_INDEX + 4)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          WDTPMU_KickPing
*
* This function processes kick action to counter of PMU-WDT. It also clears and resets IRQ counter.
*
* Notes
*
***************************************************************************************************
*/
void WDTPMU_KickPing
(
    void
);

/*
***************************************************************************************************
*                                          WDTPMU_ResetSystem
*
* This function reset or reboot system right away.
*
* Notes
*
***************************************************************************************************
*/
void WDTPMU_ResetSystem
(
    void
);

/*
***************************************************************************************************
*                                          WDTPMU_Stop
*
* This function stops PMU-WDT1 counter and unregister interrupt handler.
*
* Notes
*
***************************************************************************************************
*/
void WDTPMU_Stop
(
    void
);

/*
***************************************************************************************************
*                                          WDTPMU_SmMode
*
* This function sets configuration of safety mechanism on PMU-WDT1 device.
*
* @param    ucUseFmu [in] The flag to use handshake with FMU. If true, handshake mode will be enabled.
* @param    uiUseHandler [in] When handshake with FMU, external FMU handler is available.
* @param    uiVote [in] 3 WDT reset request selection mode, selecting one of 3 reset request line
*                       or 2 out of 3 voted reset requests
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t WDTPMU_SmMode
(
    boolean                             ucUseFmu,
    WDTPMUResetType_t                   uiUseHandler,
    WDTPMU2oo3Type_t                    uiVote
);

/*
***************************************************************************************************
*                                          WDTPMU_Init
*
* This function initializes configuration registers, registers interrupt handler for kick action,
* and starts PMU-WDT1 counter.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t WDTPMU_Init
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#endif  // MCU_BSP_WDT_PMU_HEADER


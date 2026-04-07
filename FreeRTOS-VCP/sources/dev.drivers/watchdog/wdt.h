// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : wdt.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_WDT_HEADER
#define MCU_BSP_WDT_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_PMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_PMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
typedef enum WDTResetType
{
    WDT_RST_DIRECT                      = 0x0U,
    WDT_RST_FMU_HANDLER                 = 0x4U

} WDTResetType_t;

typedef enum WDT2003Type
{
    WDT_2OO3_VOTE                       = 0x0U,
    WDT_ZERO_0                          = 0x1U,
    WDT_ZERO_1                          = 0x2U,
    WDT_ZERO_2                          = 0x3U

} WDT2oo3Type_t;


enum
{
    WDT_ERR_ALREADY_DONE                = (uint32)SAL_DRVID_WDT,
    WDT_ERR_NOT_INITIALIZED             = (uint32)SAL_DRVID_WDT + 1UL,
    WDT_ERR_INVALID_PARAM               = (uint32)SAL_DRVID_WDT + 2UL,
    WDT_ERR_NOT_SUPPORT_MODE            = (uint32)SAL_DRVID_WDT + 3UL

};//WDTErrorCode_t

/*
 * Function Indexes
 */
#define WDT_API_COMMON_INDEX            (0x0)
#define WDT_API_STOP                    (WDT_API_COMMON_INDEX + 0)
#define WDT_API_SET_SM_MODE             (WDT_API_COMMON_INDEX + 1)
#define WDT_API_KICK_ACT                (WDT_API_COMMON_INDEX + 2)
#define WDT_API_RESET_SYS               (WDT_API_COMMON_INDEX + 3)
#define WDT_API_INIT                    (WDT_API_COMMON_INDEX + 4)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          WDT_KickPing
*
* This function processes kick action to counter of WDT. It also clears and resets IRQ counter.
*
* Notes
*
***************************************************************************************************
*/
void WDT_KickPing
(
    void
);

/*
***************************************************************************************************
*                                          WDT_ResetSystem
*
* This function reset or reboot system right away.
*
* Notes
*
***************************************************************************************************
*/
void WDT_ResetSystem
(
    void
);

/*
***************************************************************************************************
*                                          WDT_Stop
*
* This function stops WDT counter and unregister interrupt handler.
*
* Notes
*
***************************************************************************************************
*/
void WDT_Stop
(
    void
);

/*
***************************************************************************************************
*                                          WDT_SmMode
*
* This function sets configuration of safety mechanism on WDT device.
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
SALRetCode_t WDT_SmMode
(
    boolean                             ucUseFmu,
    WDTResetType_t                      uiUseHandler,
    WDT2oo3Type_t                       uiVote
);

/*
***************************************************************************************************
*                                          WDT_Init
*
* This function initializes configuration registers, registers interrupt handler for kick action,
* and starts WDT counter.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t WDT_Init
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#endif  // MCU_BSP_WDT_HEADER


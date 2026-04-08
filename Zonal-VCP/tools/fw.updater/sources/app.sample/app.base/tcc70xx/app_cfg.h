// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : app_cfg.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_APP_CFG_HEADER
#define MCU_BSP_APP_CFG_HEADER

#if ( MCU_BSP_SUPPORT_APP_BASE == 1 )

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
/* TASK STACK SIZES : Size of the task stacks (# of WARD entries)                   */
#define ACFG_TASK_USER_STK_SIZE         (128U)
/* normal measn that task has no deep fucnction call or large local variable/array  */
#define ACFG_TASK_NORMAL_STK_SIZE       (128U)
/* medium measn that task has some fucnction call or small local variables/arrays   */
#define ACFG_TASK_MEDIUM_STK_SIZE       (256U)

#endif  // ( MCU_BSP_SUPPORT_APP_BASE == 1 )

#endif  // MCU_BSP_APP_CFG_HEADER


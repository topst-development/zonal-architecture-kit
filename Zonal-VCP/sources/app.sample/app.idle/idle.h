// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : IdleTask.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_IDLE_TASK_HEADER
#define MCU_BSP_IDLE_TASK_HEADER

#if ( MCU_BSP_SUPPORT_APP_IDLE == 1 )

/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

extern void IDLE_CreateTask(void);

#endif  // ( MCU_BSP_SUPPORT_APP_IDLE == 1 )

#endif  // MCU_BSP_IDLE_TASK_HEADER


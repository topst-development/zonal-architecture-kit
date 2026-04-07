// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : idle.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : Idle state
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_IDLE == 1 )

#include <app_cfg.h>

#include "bsp.h"
#include "idle.h"


/**************************************************************************************************
*                                  STATIC FUNCTIONS DECLARATION
**************************************************************************************************/

static void IdleTask
(
    void *                               pArg
);

/**************************************************************************************************
*                                        STATIC FUNCTIONS
**************************************************************************************************/

static void Idle
(
    void
)
{
    while(1);
}

static void IdleTask
(
    void *                               pArg
)
{
    (void)pArg;

    Idle();
}

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

void IDLE_CreateTask(void)
{
    static uint32   IdleTaskID = 0UL;
    static uint32   IdleTaskStk[32UL];

    (void)SAL_TaskCreate
    (
        &IdleTaskID,
        (const uint8 *)"Idle",
        (SALTaskFunc)&IdleTask,
        &IdleTaskStk[0],
        32UL,   //task stack size
        SAL_PRIO_LOWEST,
        NULL
    );
}

#endif  // ( MCU_BSP_SUPPORT_APP_IDLE == 1 )


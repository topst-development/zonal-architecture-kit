// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : dse.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_DSE_HEADER
#define MCU_BSP_DSE_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_DSE == 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
                                                            /* Default Slave Error configuration Flag */
#define DSE_INTERCON                    (0x00000001UL)

#define DES_SEL_ALL                     (DSE_INTERCON)
#define DES_SEL_MASK                    (DES_SEL_ALL)

#define DSE_INT_MODE                    (0x00000000UL)      /* Default Slave Error interrupt mode     */
#define DSE_RES_MODE                    (0x10000000UL)      /* Default Slave Error response  mode     */
#define DSE_RES_INT_MODE                (0x20000000UL)      /* Default Slave Error int & res mode     */
#define DSE_MODE_MASK                   (0x30000000UL)

#define DSE_SM_IRQ_MASK                 (0x2UL)
#define DSE_SM_IRQ_EN                   (0x4UL)
#define DSE_SM_SLV_CFG                  (0x8UL)
#define DSE_SM_CFG_WR_PW                (0x10000UL)
#define DSE_SM_CFG_WR_LOCK              (0x20000UL)

#define DSE_SM_SF_CHK_GRP_SEL           (DSE_SM_IRQ_MASK | DSE_SM_IRQ_EN | DSE_SM_SLV_CFG | DSE_SM_CFG_WR_PW | DSE_SM_CFG_WR_LOCK)

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

extern uint32                           gWait_irq;          /* using by unittest                      */

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                       DSE_WriteLock
*
* Default Slave Error Register Write Protection Enable/Disable
*
* @param    uiLock [in] Write Protection 1 : Enable, 0 : Disable
*
* @return   The result of Write Protection Function setting
*           SAL_RET_SUCCESS  Write Protection register setting succeed
*           SAL_RET_FAILED   Write Protection register setting fail
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_WriteLock
(
    uint32                              uiLock
);

/*
***************************************************************************************************
*                                       DSE_Deinit
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void DSE_Deinit
(
    void
);

/*
***************************************************************************************************
*                                       DSE_Init
*
* Default Slave Error Controller Deinit
*
* @param    uiModeFlag  [in] Default Slave Error configuration Flag (refer to dse.h)
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_Init
(
    uint32                              uiModeFlag
);

/*
***************************************************************************************************
*                                       DSE_GetGrpSts
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   rSF_CHK_GRP_STS : Soft Fault Check Group Status Register value
*
* Notes
*
***************************************************************************************************
*/

uint32 DSE_GetGrpSts
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_DSE == 1 )

#endif  // MCU_BSP_DSE_HEADER


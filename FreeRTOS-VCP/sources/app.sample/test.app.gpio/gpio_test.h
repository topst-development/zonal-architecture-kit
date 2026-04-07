// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gpio_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_GPIO_TEST_HEADER
#define MCU_BSP_GPIO_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define GPIO_OFFSET_DAT                 (0x000UL)
#define GPIO_OFFSET_OEN                 (0x004UL)
#define GPIO_OFFSET_SET                 (0x008UL)
#define GPIO_OFFSET_CLR                 (0x00cUL)
#define GPIO_OFFSET_XOR                 (0x010UL)
#define GPIO_OFFSET_DS0                 (0x014UL)
#define GPIO_OFFSET_DS1                 (0x018UL)
#define GPIO_OFFSET_PE                  (0x01cUL)
#define GPIO_OFFSET_PS                  (0x020UL)
#define GPIO_OFFSET_IEN                 (0x024UL)

#define GPIO_OFFSET_FNC0                (0x030UL)
#define GPIO_OFFSET_FNC1                (0x034UL)
#define GPIO_OFFSET_FNC2                (0x038UL)
#define GPIO_OFFSET_FNC3                (0x03cUL)
#define GPIO_OFFSET_EIS                 (0x280UL) /* External Interrupt Selection register             */
#define GPIO_OFFSET_ECS                 (0x2B0UL) /* External Clock Selection register                 */
#define GPIO_OFFSET_FLE                 (0x300UL) /* Function Configuration Lock Enable register       */
#define GPIO_OFFSET_OLE                 (0x380UL) /* Output Enable Configuration Lock Enable register  */
#define GPIO_OFFSET_VIR                 (0x400UL)

#define GPIO_CFG_WR_PW                  (0x8C0UL)
#define GPIO_CFG_WR_LOCK                (0x8C4UL)
#define GPIO_SOFT_FAULT_EN              (0xD00UL)
#define GPIO_SF_CTRL_CFG                (0xF00UL)
#define GPIO_SF_CTRL_STS                (0xF04UL)

#define GPIO_A_MASK                     (0x7FFFFFFFUL)
#define GPIO_B_MASK                     (0x1FFFFFFFUL)
#define GPIO_C_MASK                     (0x007FFFFFUL)
#define GPIO_K_MASK                     (0x0001FFFFUL)


#define GPIO_MAX_NTH_IRQ                (10)//5

#define GPIO_ACTIV_HIGH                 (0)
#define GPIO_ACTIV_LOW                  (1)

#define GPIO_VI2O_OEN                   (0x00U)
#define GPIO_VI2I_DAT                   (0x04U)
#define GPIO_VI2O_EN                    (0x08U)
#define GPIO_VI2I_EN                    (0x0CU)
#define GPIO_MON_DO                     (0x10U)
#define GPIO_MON_OEN                    (0x14U)

#define GPIO_MAX_NUM                    (4)

#define GPIO_CMD_NAME                   ("gpio")

#define GPIO_WriteReg(a,v)              (SAL_WriteReg(v,a))
#define GPIO_ReadReg(a)                 (SAL_ReadReg(a))

#define GPIO_WR_REG_MAX                 (112)

#define GPIO_PASSWORD                   (0x5afeace5UL)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          GPIO_StartGpioTest
*
* @param    ucMode [in]
*
* Notes
*
***************************************************************************************************
*/
void GPIO_StartGpioTest
(
    int32                               ucMode
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )

#endif  // MCU_BSP_GPIO_TEST_HEADER


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : i2c_reg.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_I2C == 1 )

#include <i2c_reg.h>

/*                                                                                                 */
/***************************************************************************************************/
/*                                             LOCAL VARIABLES                                     */
/***************************************************************************************************/

I2CDev_t i2c[I2C_CH_NUM] =
{
    {
        400,                            /* I2C Clock                */
        I2C_CH0_BASE,                   /* Channel Base Address     */
        I2C_CFG_BASE,                   /* I2C Configuration        */
        (uint32)CLOCK_PERI_I2C0,        /* Peri clock index         */
        (sint32)CLOCK_IOBUS_I2C0,       /* I/O Bus index            */
        NULL,                           /* SDA                      */
        NULL,                           /* SCL                      */
        NULL,                           /* Port number              */
        {0, },
        {0, },
        I2C_STATE_DISABLED              /* I2C State                */
    },
    {
        400,
        I2C_CH1_BASE,
        I2C_CFG_BASE,
        (uint32)CLOCK_PERI_I2C1,
        (sint32)CLOCK_IOBUS_I2C1,
        NULL,
        NULL,
        NULL,
        {0, },
        {0, },
        I2C_STATE_DISABLED
    },
    {
        400,
        I2C_CH2_BASE,
        I2C_CFG_BASE,
        (uint32)CLOCK_PERI_I2C2,
        (sint32)CLOCK_IOBUS_I2C2,
        NULL,
        NULL,
        NULL,
        {0, },
        {0, },
        I2C_STATE_DISABLED
    },
    {
        400,
        I2C_CH3_BASE,
        I2C_CFG_BASE,
        (uint32)CLOCK_PERI_I2C3,
        (sint32)CLOCK_IOBUS_I2C3,
        NULL,
        NULL,
        NULL,
        {0, },
        {0, },
        I2C_STATE_DISABLED
    },
    {
        400,
        I2C_CH4_BASE,
        I2C_CFG_BASE,
        (uint32)CLOCK_PERI_I2C4,
        (sint32)CLOCK_IOBUS_I2C4,
        NULL,
        NULL,
        NULL,
        {0, },
        {0, },
        I2C_STATE_DISABLED
    },
    {
        400,
        I2C_CH5_BASE,
        I2C_CFG_BASE,
        (uint32)CLOCK_PERI_I2C5,
        (sint32)CLOCK_IOBUS_I2C5,
        NULL,
        NULL,
        NULL,
        {0, },
        {0, },
        I2C_STATE_DISABLED
    },
};

#endif  // ( MCU_BSP_SUPPORT_DRIVER_I2C == 1 )


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : i2c_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )
#include <i2c_test.h>

#define LOOPBACK_TEST
//#define DELAY_TEST
//#define WRITE_LOCK_TEST

/*                                                                                               */
/*************************************************************************************************/
/*                                             LOCAL VARIABLES                                   */
/*************************************************************************************************/

static uint32                           g_detected_s_addr = 0;

#ifdef LOOPBACK_TEST
/*
***************************************************************************************************
*                                          I2C_TestSetLoopback
*
* Function to set internal/external loopback.
*
* @param    uiEnable [in]   : Value to set/clear LoopbackEN bit. (0 or 1)
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           uiEnable == 0   : I2C Master Internal LoopBack
*           uiEnable == 1   : I2C Master External LoopBack
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestSetLoopback
(
    uint32                              uiEnable
);

/*
***************************************************************************************************
*                                          I2C_TestSetSlaveAddr
*
* Function to set address about virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiAddr [in]     : Value of address
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           addrees bit [7:1]
**************************************************************************************************
*/

static SALRetCode_t I2C_TestSetSlaveAddr
(
    uint8                               ucCh,
    uint32                              uiAddr
);

/*
***************************************************************************************************
*                                          I2C_TestEnableSlave
*
* Function to enable virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestEnableSlave
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_TestDisableSlave
*
* Function to disable virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestDisableSlave
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_TestGetSlaveAddr
*
* Function to get virtual slave address.
*
* @param    ucCh [in]       : Value of channel to control
* @return   address of virtual slave channel
* Notes
*
**************************************************************************************************
*/

static uint32 I2C_TestGetSlaveAddr
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_TestLoopback
*
* Function to test loopback.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestLoopback
(
    uint8                               ucCh
);
#endif

#ifdef DELAY_TEST
static void I2C_SetDelay
(
    uint32                              ch,
    uint8                               delay
);

static void I2C_DelayTest
(
    void
);
#endif

#ifdef WRITE_LOCK_TEST
/*
***************************************************************************************************
*                                          I2C_TestEnableLock
*
* Function to enable write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           related register : PORT_SEL , I2C_LB_CFG
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestEnableLock
(
    void
);

/*
***************************************************************************************************
*                                          I2C_TestDisableLock
*
* Function to disable write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           related register : PORT_SEL , I2C_LB_CFG
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestDisableLock
(
    void
);

/*
***************************************************************************************************
*                                          I2C_TestWriteLock
*
* Function to test write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestWriteLock
(
    void
);
#endif

/*
***************************************************************************************************
*                                          I2C_TestScanSlave
*
* Function to scan slave
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
**************************************************************************************************
*/

static void I2C_TestScanSlave
(
    uint8                               ucCh
);

#ifdef LOOPBACK_TEST
/*
***************************************************************************************************
*                                          I2C_TestSetLoopback
*
* Function to set internal/external loopback.
*
* @param    uiEnable [in]   : Value to set/clear LoopbackEN bit. (0 or 1)
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           uiEnable == 0   : I2C Master Internal LoopBack
*           uiEnable == 1   : I2C Master External LoopBack
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestSetLoopback
(
    uint32                              uiEnable
) {
    uint32                              p_loopback_addr;

    p_loopback_addr = I2C_CFG_BASE + I2C_LB_CFG;

    SAL_WriteReg(uiEnable, p_loopback_addr);

    if((SAL_ReadReg(p_loopback_addr) & I2C_LB_CFG_BIT) != uiEnable)
    {
        mcu_printf("[I2C] %s fail. \n", __func__);

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_TestSetSlaveAddr
*
* Function to set address about virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiAddr [in]     : Value of address
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           addrees bit [7:1]
**************************************************************************************************
*/

static SALRetCode_t I2C_TestSetSlaveAddr
(
    uint8                               ucCh,
    uint32                              uiAddr
) {
    uint32                              p_vsaddr;

    p_vsaddr = I2C_CH0_VSLAVE + (ucCh * I2C_CH_OFFSET) + I2C_VSADDR;

    if(uiAddr > 0xffUL)
    {
        mcu_printf("set slave address is invaild.\n");

        return SAL_RET_FAILED;
    }

    SAL_WriteReg(((uiAddr & I2C_ADDRESS_MASK) << I2C_ADDRESS_SHIFT), p_vsaddr);

    if(((SAL_ReadReg(p_vsaddr)>> I2C_ADDRESS_SHIFT) & I2C_ADDRESS_MASK ) != uiAddr)
    {
        mcu_printf("[I2C] sm_set_slave_addr fail. \n");

        return SAL_RET_FAILED;
    }

    mcu_printf("[I2C] slave addr : 0x%x \n", SAL_ReadReg(p_vsaddr) >> 1UL);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_TestEnableSlave
*
* Function to enable virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestEnableSlave
(
    uint8                               ucCh
) {
    uint32                              p_vs_ctrl_addr;

    p_vs_ctrl_addr = I2C_CH0_VSLAVE + (ucCh * I2C_CH_OFFSET) + I2C_VSCTL;

    SAL_WriteReg((uint32)BSP_BIT_00 | (uint32)BSP_BIT_20 , p_vs_ctrl_addr);
    SAL_WriteReg(((((uint32)I2C_VSINT_EN_0|(uint32)I2C_VSINT_EN_1)  |
                (uint32)BSP_BIT_24) | (uint32)BSP_BIT_26),
                (uint32)(p_vs_ctrl_addr + (uint32)0x08UL));

    if(SAL_ReadReg(p_vs_ctrl_addr) == BSP_BIT_NONE)
    {
        mcu_printf("[I2C] sm_enable_slave fail.\n");

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_TestDisableSlave
*
* Function to disable virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestDisableSlave
(
    uint8                               ucCh
) {
    uint32                              p_vs_ctrl_addr;

    p_vs_ctrl_addr = I2C_CH0_VSLAVE + (ucCh * I2C_CH_OFFSET) + I2C_VSCTL;

    SAL_WriteReg(0x0 , p_vs_ctrl_addr);

    if(SAL_ReadReg(p_vs_ctrl_addr) != BSP_BIT_NONE)
    {
        mcu_printf("[I2C] sm_disable_slave fail. \n");

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_TestGetSlaveAddr
*
* Function to get virtual slave address.
*
* @param    ucCh [in]       : Value of channel to control
* @return   address of virtual slave channel
* Notes
*
**************************************************************************************************
*/

static uint32 I2C_TestGetSlaveAddr(uint8 ucCh)
{
    uint32                              p_vsaddr;

    p_vsaddr = I2C_CH0_VSLAVE + (I2C_CH_OFFSET * ucCh) + I2C_VSADDR;

    mcu_printf("[%s] READ(0x%x) : 0x%x \n", __func__ , p_vsaddr , SAL_ReadReg(p_vsaddr));

    return ((SAL_ReadReg(p_vsaddr)&(uint32)0xffUL)>>1UL);
}

/*
***************************************************************************************************
*                                          I2C_TestLoopback
*
* Function to test loopback.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestLoopback
(
    uint8                               ucCh
) {
    SALRetCode_t                        ret;
    uint32                              buffer;
    uint32                              buffer2;
    uint32                              data;
    uint32                              i;
    uint8                               dummy[8] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x87};
    uint8                               dummy2[8] = {0, };

    I2CXfer_t                           XferArg;

    ret = SAL_RET_SUCCESS;
    buffer = 0;
    buffer2 = 0;
    data = 0;
    i = 0;

    ret = I2C_Open(ucCh, I2C_TEST_PORT, I2C_TEST_CLK_RATE_100, NULL , NULL);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail line %d\n", __func__ ,__LINE__);

        return SAL_RET_FAILED;
    }

    g_detected_s_addr = I2C_ScanSlave(ucCh);

    ret = I2C_TestSetLoopback(I2C_LB_INTERNAL);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail line %d\n", __func__ ,__LINE__);

        return SAL_RET_FAILED;
    }

    ret = I2C_TestSetSlaveAddr(ucCh, g_detected_s_addr);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail line %d\n", __func__ ,__LINE__);

        return SAL_RET_FAILED;
    }

    ret = I2C_TestEnableSlave(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail line %d\n", __func__ ,__LINE__);

        return SAL_RET_FAILED;
    }

    XferArg.xCmdLen = (uint8) 8UL;
    XferArg.xCmdBuf = (uint8 *)&dummy;
    XferArg.xOutLen = NULL;
    XferArg.xOutBuf = (uint8 *)NULL_PTR;
    XferArg.xInLen  = (uint8) 8UL;
    XferArg.xInBuf  = (uint8 *)&dummy2;
    XferArg.xOpt    = NULL;

    // Sync Xfer
    (void)I2C_XferCmd(ucCh, (uint8)(I2C_TestGetSlaveAddr(ucCh)<<1UL), XferArg, NULL);

    // Async Xfer : virtual slave can't get data via loopback path.
    //(void)I2C_XferCmd(ucCh, (uint8)(I2C_TestGetSlaveAddr(ucCh)<<1UL), XferArg, 1);

    buffer = SAL_ReadReg((uint32)(I2C_CH0_VSLAVE + ((uint32)ucCh * I2C_CH_OFFSET) + I2C_VSMB0));
    buffer2 = SAL_ReadReg((uint32)(I2C_CH0_VSLAVE + ((uint32)ucCh * I2C_CH_OFFSET) + I2C_VSMB1));

    mcu_printf("DB 1 : %x\n",buffer);
    mcu_printf("DB 2 : %x\n",buffer2);

    for(i = 0 ; i < 8UL ; i++)
    {
        if(i < 4UL)
        {
            data = buffer;

            if(((data >> ((uint32)8UL*(i%(uint32)4UL))) & (uint32)0xffUL) == (uint32)dummy[i])
            {
                mcu_printf("Master TX Data is same VS Buffer Byte [%d] (%x) (%x)\n", i,
                    ((uint32)data >> ((uint32)8UL*(i%(uint32)4UL))) & 0xffUL , dummy[i]);
            }
            else
            {
                mcu_printf("data compare fail! \n");
            }
        }
        else
        {
            data = buffer2;

            if(((data >> ((uint32)8UL*(i%(uint32)4UL))) & (uint32)0xffUL) == (uint32)dummy[i])
            {
                mcu_printf("Master TX Data is same VS Buffer Byte [%d] (%x) (%x)\n", i,
                    ((uint32)data >> ((uint32)8UL*(i%(uint32)4UL))) & 0xffUL , dummy[i]);
            }
            else
            {
                mcu_printf("data compare fail! \n");
            }
        }


    }

    ret = I2C_TestDisableSlave(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail line %d\n", __func__ ,__LINE__);

        ret = SAL_RET_FAILED;
    }

    ret = I2C_Close(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail\n", __func__);

        ret = SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;

}

#endif

#ifdef DELAY_TEST
static void I2C_SetDelay(uint32 ch, uint8 delay)
{
    uint32 p_reg = (uint32)(0x1B300000 + (ch*0x10000) + 0x18);

    uint32 reg = SAL_ReadReg(p_reg);

    SAL_WriteReg(reg | (0xfU << 16) , p_reg);

    reg = SAL_ReadReg(p_reg);
    SAL_WriteReg(reg | ((delay & 0xff) << 8), p_reg);

    reg = SAL_ReadReg(p_reg);
    mcu_printf("[%s] TR0 : 0x%x \n", __func__ , reg);
}

static void I2C_DelayTest
(
    void
)
{
    uint8 ucCh;

    uint32                              i;
    uint8                               dummy[8] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x87};
    uint8                               dummy2[8] = {0, };

    ucCh = 0;

    (void)I2C_Open(ucCh, I2C_TEST_PORT, I2C_TEST_CLK_RATE_400, NULL , NULL);

    g_detected_s_addr = I2C_ScanSlave(ucCh);

    mcu_printf("[%s] Start. \n", __func__);
    I2C_SetDelay(ucCh, (uint8)0x0);
    for(i = 0 ; i < 10000 ; i++)
    {
        (void)I2C_XferCmd(ucCh, (uint8)(g_detected_s_addr<<1UL),
                (uint8)8UL, &dummy[0] , NULL, NULL_PTR ,
                (uint8)8UL, &dummy2[0], NULL, NULL);
    }

    mcu_printf("[%s] Done. \n", __func__);



    mcu_printf("[%s] Start (with Delay). \n", __func__);

    I2C_SetDelay(ucCh, (uint8)0x10);
    for(i = 0 ; i < 10000 ; i++)
    {
        (void)I2C_XferCmd(ucCh, (uint8)(g_detected_s_addr<<1UL),
                (uint8)8UL, &dummy[0] , NULL, NULL_PTR ,
                (uint8)8UL, &dummy2[0], NULL, NULL);
    }

    mcu_printf("[%s] Done. \n", __func__);

    (void)I2C_Close(ucCh);
}
#endif

#ifdef WRITE_LOCK_TEST
/*
***************************************************************************************************
*                                          I2C_TestEnableLock
*
* Function to enable write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           related register : PORT_SEL , I2C_LB_CFG
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestEnableLock
(
    void
) {
    SALRetCode_t                        ret;
    uint32                              tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0;

    SAL_WriteReg(I2C_LOCK_PW, (uint32)(I2C_CFG_BASE + I2C_WR_PW));
    SAL_WriteReg(BSP_BIT_00, (uint32)(I2C_CFG_BASE + I2C_WR_LOCK));

    tmp = SAL_ReadReg((uint32)(I2C_CFG_BASE + I2C_WR_LOCK));

    if ((tmp & BSP_BIT_00) == BSP_BIT_00)
    {
        mcu_printf("i2c write lock: %d\n", tmp);
    }
    else
    {
        mcu_printf("i2c write lock fail: %d\n", tmp);
        ret = SAL_RET_FAILED;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_TestDisableLock
*
* Function to disable write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           related register : PORT_SEL , I2C_LB_CFG
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestDisableLock
(
    void
) {
    SALRetCode_t                        ret;
    uint32                              tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0;

    SAL_WriteReg(I2C_LOCK_PW, (uint32)(I2C_CFG_BASE + I2C_WR_PW));
    SAL_WriteReg(BSP_BIT_NONE, (uint32)(I2C_CFG_BASE + I2C_WR_LOCK));

    tmp = SAL_ReadReg((uint32)(I2C_CFG_BASE + I2C_WR_LOCK));

    if ((tmp & BSP_BIT_00) == BSP_BIT_NONE)
    {
        mcu_printf("i2c write unlock: %d\n", tmp);
    }
    else
    {
        mcu_printf("i2c write unlock fail: %d\n", tmp);
        ret = SAL_RET_FAILED;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_TestWriteLock
*
* Function to test write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestWriteLock
(
    void
) {
    SALRetCode_t                        ret;
    uint32                              val;
    uint32                              lb_cfg;

    ret = SAL_RET_SUCCESS;
    val = 0;
    lb_cfg = 0;

    lb_cfg = SAL_ReadReg ((uint32)(I2C_CFG_BASE + I2C_LB_CFG));
    mcu_printf("Read & Store LB_CFG : 0x%x\n", lb_cfg);

    ret = I2C_TestEnableLock();

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail line %d\n", __func__, __LINE__);

        return SAL_RET_FAILED;
    }

    mcu_printf("LB_CFG(Locked) try to set 1. \n");
    SAL_WriteReg(0x1UL, (uint32)(I2C_CFG_BASE + I2C_LB_CFG));

    val = SAL_ReadReg((uint32)(I2C_CFG_BASE + I2C_LB_CFG));
    mcu_printf("LB_CFG(Locked) read value : %x \n", val);

    if(val != lb_cfg)
    {
        mcu_printf("Test (LB_CFG) Failed. read value : %x , expected value : %x\n", val, lb_cfg);
        ret = I2C_TestDisableLock();

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("[I2C-SM] %s fail\n", __func__);

            return SAL_RET_FAILED;
        }
    }
    else
    {
        mcu_printf("Test of register lock (LB_CFG) success.\n");
    }

    ret = I2C_TestDisableLock();
    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[I2C-SM] %s fail\n", __func__);

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}
#endif

/*
***************************************************************************************************
*                                          I2C_TestScanSlave
*
* Function to scan slave
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
**************************************************************************************************
*/

static void I2C_TestScanSlave
(
    uint8                               ucCh
) {
    SALRetCode_t                        ret;

    ret = SAL_RET_SUCCESS;

    ret = I2C_Open(ucCh, I2C_TEST_PORT, I2C_TEST_CLK_RATE_100, NULL , NULL);

    if(ret == SAL_RET_SUCCESS)
    {
        g_detected_s_addr = I2C_ScanSlave(ucCh);

        mcu_printf("Scaned I2C Slave Addr [0x%02x]\n", g_detected_s_addr);
    }
    else
    {
        mcu_printf("[I2C-SM] %s fail line %d\n", __func__ ,__LINE__);
    }
}

/*
***************************************************************************************************
*                                          I2C_TestMain
*
* Function to call all of test functions.
*
* @return
* Notes
*
**************************************************************************************************
*/

void I2C_TestMain
(
    void
) {

    I2C_TestScanSlave(0);

#ifdef LOOPBACK_TEST
    (void) I2C_TestLoopback(0);
#endif

#ifdef DELAY_TEST
    I2C_DelayTest();
#endif

#ifdef WRITE_LOCK_TEST
    (void) I2C_TestWriteLock();
#endif

}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )


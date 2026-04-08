// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : i2c.c
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

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*                                                                                               */
/*************************************************************************************************/
/*                                             LOCAL VARIABLES                                   */
/*************************************************************************************************/

static uint8                            dummy;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void I2C_Reinit
(
    uint8                               ucCh
);

static SALRetCode_t I2C_Enable
(
    uint8                               ucCh
);

static SALRetCode_t I2C_Reset
(
    uint8                               ucCh
);

static SALRetCode_t I2C_SetPort
(
    uint8                               ucCh,
    uint32                              uiPortSel
);

static void I2C_EnableIrq
(
    uint8                               ucCh
);

static void I2C_DisableIrq
(
    uint8                               ucCh
);

static void I2C_XferComplete
(
    uint8                               ucCh,
    uint32                              ucEvent
);

static void I2C_ClearIntr
(
    uint8                               ucCh
);

static void I2C_CheckState
(
    uint32                              status
);

static void I2C_CheckSafetyState
(
    uint32                              sm
);

static SALRetCode_t I2C_ProcessAckFail
(
    uint8                               ucCh,
    uint8                               ucSlaveAddr
);

static void I2C_PrepareSend
(
    uint8                               ucCh,
    uint8                               ucSlaveAddr
);

static SALRetCode_t I2C_CheckValidXfer
(
    uint8                               ucCh,
    uint8                               ucSlaveAddr
);

static SALRetCode_t I2C_XferSyncWrite
(
    uint8                               ucCh,
    uint8                               ucSlaveAddr,
    I2CXfer_t                           XferArg
);

static void I2C_IsrStartSend
(
    uint8                               ucCh
);

static void I2C_IsrRecvStart
(
    uint32                              ucCh
);

/*
***************************************************************************************************
*                                          I2C_EnableIrq
*
* I2C Enable interrupt function.
*
* @param    ucCh [in]       : Value of channel to control
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_EnableIrq
(
    uint8                               ucCh
)
{
    uint32  val;
    uint32  reg;

    val = 0UL;
    reg = 0UL;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CTRL))
    {
        val = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CTRL)) | BSP_BIT_06;
        reg = (uint32)(i2c[ucCh].dBase + I2C_CTRL);
        SAL_WriteReg(val, reg);
    }

    return;
}

/*
***************************************************************************************************
*                                          I2C_DisableIrq
*
* I2C Disnable interrupt function.
*
* @param    ucCh [in]       : Value of channel to control
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_DisableIrq
(
    uint8                               ucCh
)
{
    uint32  val;
    uint32  reg;

    val = 0UL;
    reg = 0UL;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CTRL))
    {
        val = (uint32)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CTRL)) & ~BSP_BIT_06);
        reg = (uint32)(i2c[ucCh].dBase + I2C_CTRL);
        SAL_WriteReg(val, reg);
    }

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
    {
        val = (uint32)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)) | BSP_BIT_00);
        reg = (uint32)(i2c[ucCh].dBase + I2C_CMD);
        SAL_WriteReg(val, reg);
    }

    return;

}

/*
***************************************************************************************************
*                                          I2C_XferComplete
*
* I2C Call callback function for notification.
*
* @param    ucCh [in]       : Value of channel to control
*           ucEvent [in]    : Value of event
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_XferComplete
(
    uint8   ucCh,
    uint32  ucEvent
)
{
    I2C_DisableIrq(ucCh);
    i2c[ucCh].dState = I2C_STATE_IDLE;

    if(i2c[ucCh].dComplete.ccCallBack != NULL_PTR)
    {
        i2c[ucCh].dComplete.ccCallBack(ucCh, (ucEvent), i2c[ucCh].dComplete.ccArg);
    }
}

/*
***************************************************************************************************
*                                          I2C_ClearIntr
*
* I2C Interrupt clear function.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_ClearIntr
(
    uint8                               ucCh
)
{
    uint32  val;
    uint32  reg;

    val = 0UL;
    reg = 0UL;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
    {
        val = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)) | BSP_BIT_00;
        reg = i2c[ucCh].dBase + I2C_CMD;
        SAL_WriteReg(val, reg);
    }

    return;
}


/*
***************************************************************************************************
*                                          I2C_ScanSlave
*
* Function to scan slave device address received ACK about selected channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   slave address that scaned second.
* Notes
*
***************************************************************************************************
*/

uint32 I2C_ScanSlave
(
    uint8                               ucCh
)
{
    uint32          i;
    uint32          j;
    SALRetCode_t    ret;
    uint32          detect_cnt;
    uint32          return_addr;
    I2CXfer_t       XferArg;

    XferArg.xCmdLen = (uint8) 1UL;
    XferArg.xCmdBuf = (uint8 *)&dummy;
    XferArg.xOutLen = NULL;
    XferArg.xOutBuf = (uint8 *)NULL_PTR;
    XferArg.xInLen  = NULL;
    XferArg.xInBuf  = (uint8 *)NULL_PTR;
    XferArg.xOpt    = NULL;

    ret         = SAL_RET_SUCCESS;
    detect_cnt  = 0UL;
    return_addr = 0UL;
    I2C_D("Start I2C scan slave \n");

    for(i = 0 ; i < 0xFFUL ; i+= 16UL)
    {
        for(j = 0 ; j < 16UL ; j++)
        {
            ret = I2C_XferCmd(ucCh, (uint8)(((i+j) & 0x7fUL) << 1UL), XferArg ,NULL);

            if(ret == SAL_RET_SUCCESS)
            {
                detect_cnt++;
                I2C_D("Detected I2C Slave Addr [%02x]\n", i+j);

                if(detect_cnt == 1UL)
                {
                    return_addr = i+j;
                }
            }
        }
    }

    I2C_D("Finish I2C scan slave. return address value : %x \n", return_addr);

    return return_addr;
}

/*
***************************************************************************************************
*                                          I2C_BusyCheck
*
* Fucntion to check whether i2c is busy.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_BusyCheck
(
    uint8                               ucCh
)
{
    uint32          status;
    SALRetCode_t    ret;
    uint32          cnt;
    uint32          do_break;

    status  = 0UL;
    ret     = SAL_RET_SUCCESS;
    cnt     = 0UL;
    do_break = 0UL;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_SR))
    {
        status = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR));
    }

    while(status != (uint32)NULL)
    {
        if((status & I2C_SR_BUSY) != (uint32)NULL)
        {
            I2C_D("I2C Busy ! \n");
        }
        else
        {
            do_break = 1UL;
        }

        if(cnt > (uint32)I2C_BUSY_TIMEOUT)
        {
            I2C_D("I2C Busy check timeout. \n");
            ret = SAL_RET_FAILED;
            do_break = 1UL;
        }
        else
        {
            cnt++;

            if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_SR))
            {
                status = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR));
            }
        }

        if(do_break == 1UL)
        {
            break;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2c_Reinit
*
* Function to Re-initialize I2C for selected channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_Reinit
(
    uint8                               ucCh
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    if (ucCh > (uint32)I2C_CH_NUM)
    {
        I2C_D("%s %d channel is wrong\n", __func__, ucCh);
    }
    else
    {
        (void)SAL_CoreCriticalEnter();

        i2c[ucCh].dState = I2C_STATE_DISABLED;

        (void)SAL_MemSet(&i2c[ucCh].dAsync, 0, sizeof(I2CAsyncXfer_t));
        (void)SAL_CoreCriticalExit();
        I2C_DisableIrq(ucCh);
        I2C_ClearIntr(ucCh);
        ret = I2C_Reset(ucCh);

        if (ret != SAL_RET_SUCCESS)
        {
            I2C_D("%s ch %d failed to reset core\n", __func__, ucCh);
        }

    	ret = I2C_SetPort(ucCh, i2c[ucCh].dPort);

        if(ret != SAL_RET_SUCCESS)
        {
            I2C_D("%s ch %d failed to set port\n", __func__, ucCh);
        }

        ret = I2C_Enable(ucCh);

        if(ret != SAL_RET_SUCCESS)
        {
            I2C_D("%s ch %d failed to enable core\n", __func__, ucCh);
        }

        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_IDLE;
        (void)SAL_CoreCriticalExit();
    }

    return;
}

/*
***************************************************************************************************
*                                          I2C_Wait
*
* Function to check transfer status and to wait TIP (Transfer in progress) complete.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           : Wait until cmd operation is working correctly
* Notice
*           : If timeout occur, check SCL and SDA signal status
*             If SCL is low by slave devices during the byte transfer, CMD is not completed
*
***************************************************************************************************
*/

static SALRetCode_t I2C_Wait
(
    uint8                               ucCh
)
{
    SALRetCode_t    ret;
    uint32          cnt;

    ret = SAL_RET_SUCCESS;
    cnt = 0UL;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
    {
        /* Check STA , STO , RD , WR is 0. */
        while((SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)) & I2C_CMD_PROGRESS_MASK) != 0UL)
        {
            if(cnt > I2C_POLL_TIMEOUT)
            {
                I2C_D("%s: ch %d non interrupt time out, cmd(0x%x)\n",
                        __func__, (uint32)ucCh, SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)));
                ret = SAL_RET_FAILED;

                break;
            }

            cnt++;
        }
    }

    if(ret == SAL_RET_SUCCESS)
    {
        /* Check TIP (Transfer in progress) is 0. */
        while(((SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR)) & I2C_SR_TIP)) != 0UL)
        {
            if(cnt > I2C_POLL_TIMEOUT)
            {
                I2C_D("%s: ch %d non interrupt time out, status(0x%x)\n",
                                __func__, (uint32)ucCh, SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR)));
                ret = SAL_RET_FAILED;

                break;
            }

            cnt++;
        }
    }

    I2C_ClearIntr(ucCh);

#if 0
    if(ret != SAL_RET_SUCCESS)
    {
        I2C_Reinit(ucCh); /* If command is not completed, try to reinit the core */
    }
#endif

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_CheckAck
*
* Function to check receive ACK after transfer complete.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           : Check whether receive the acknowledgement after send the byte
*
***************************************************************************************************
*/

static SALRetCode_t I2C_CheckAck
(
    uint8                               ucCh
)
{
    uint32  val;
    SALRetCode_t ret;

    ret = SAL_RET_SUCCESS;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_SR))
    {
        val = (uint32)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR)) & I2C_SR_RX_ACK);

        if(val == (uint32)NULL) /* Check RxACK */
        {
            ret = SAL_RET_SUCCESS;
        }
        else
        {
            ret = SAL_RET_FAILED;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_ProcessAckFail
*
* Function to process receive ACK failure.
*
* @param    ucCh [in]           : Value of channel to control
* @param    ucSlaveAddr [in]    : specific slave address to transfer
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_ProcessAckFail
(
    uint8 ucCh,
    uint8 ucSlaveAddr
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE -I2C_CMD))
    {
        SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    }

    ret = I2C_Wait(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s: i2c ch %d received NACK from 0x%x\n", __func__, ucCh, (ucSlaveAddr)>>1UL);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_PrepareSend
*
* Function to prepare send with START condition.
*
* @param    ucCh [in]           : Value of channel to control
* @param    ucSlaveAddr [in]    : specific slave address to transfer
* @return
* Notes
*
***************************************************************************************************
*/
static void I2C_PrepareSend
(
    uint8 ucCh,
    uint8 ucSlaveAddr
)
{
    /* Send START condition and slave address with write flag*/
    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_TXR))
    {
        SAL_WriteReg(((uint32)ucSlaveAddr | I2C_WR), (uint32)(i2c[ucCh].dBase + I2C_TXR));
    }

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
    {
        SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR) , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    }
}

/*
***************************************************************************************************
*                                          I2C_CheckValidXfer
*
* Function to check Sync Xfer result.
*
* @param    ucCh [in]           : Value of channel to control
* @param    ucSlaveAddr [in]    : specific slave address to transfer
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_CheckValidXfer
(
    uint8 ucCh,
    uint8 ucSlaveAddr
)
{
    SALRetCode_t ret;

    ret = SAL_RET_SUCCESS;

    ret = I2C_Wait(ucCh);

    if(ret == SAL_RET_SUCCESS)
    {
        ret = I2C_CheckAck(ucCh);

        if(ret != SAL_RET_SUCCESS)
        {
            ret = I2C_ProcessAckFail(ucCh, ucSlaveAddr);
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_XferSyncWrite
*
* Function to process sync xfer WRITE.
*
* @param    ucCh [in]           : Value of channel to control
* @param    ucSlaveAddr [in]    : specific slave address to transfer
* @param    I2CXfer_t [in]      : Xfer argument
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_XferSyncWrite
(
    uint8 ucCh,
    uint8 ucSlaveAddr,
    I2CXfer_t XferArg
)
{
    uint8                               ucCmdLen = XferArg.xCmdLen;
    const uint8 *                       ucpCmdBuf = XferArg.xCmdBuf;
    uint8                               ucOutLen = XferArg.xOutLen;
    const uint8 *                       ucpOutBuf = XferArg.xOutBuf;
    uint8                               ucInLen = XferArg.xInLen;
    uint32                              uiOpt = XferArg.xOpt;
    SALRetCode_t                        ret = SAL_RET_SUCCESS;
    uint32  i;

    I2C_PrepareSend(ucCh, ucSlaveAddr);

    ret = I2C_Wait(ucCh);

    if(ret == SAL_RET_SUCCESS)
    {
        ret = I2C_CheckAck(ucCh);

        if(ret == SAL_RET_SUCCESS)
        {
            if(ucCmdLen > (uint8)NULL)
            {
                for(i = 0 ; i < ucCmdLen ; i++)
                {
                    /* Send data */
                    if(ucpCmdBuf != (uint8 *)NULL_PTR)
                    {
                        if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_TXR))
                        {
                            SAL_WriteReg(ucpCmdBuf[i], (uint32)(i2c[ucCh].dBase + I2C_TXR));
                        }
                    }

                    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
                    {
                        SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }

                    ret = I2C_CheckValidXfer(ucCh, ucSlaveAddr);
                }
            }

            if(ret == SAL_RET_SUCCESS)
            {

                if(ucOutLen > (uint8)NULL)
                {
                    for(i = 0 ; i < ucOutLen ; i++)
                    {
                        /* Send data */
                        if(ucpOutBuf != (uint8 *)NULL_PTR)
                        {
                            if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_TXR))
                            {
                                SAL_WriteReg(ucpOutBuf[i], (uint32)(i2c[ucCh].dBase + I2C_TXR));
                            }
                        }

                        if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
                        {
                            SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                        }

                            ret = I2C_CheckValidXfer(ucCh, ucSlaveAddr);
                        }
                }

                if(ret == SAL_RET_SUCCESS)
                {
                    /* if read data is none, send STOP condition */
                    if(ucInLen == (uint8)NULL)
                    {
                        if((uiOpt & I2C_WR_WITHOUT_STOP) != 0UL)
                        {
                            ret = SAL_RET_SUCCESS;
                        }
                    }
                }
            }
        }
        else
        {
            ret = I2C_ProcessAckFail(ucCh, ucSlaveAddr);
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_XferSync
*
* Function to sync transfer data via I2C channel with param info.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    XferArg [in]    : Xfer argument
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           : this function will be called according ucAsync value in I2C_Xfer function params.
*
***************************************************************************************************
*/

static SALRetCode_t I2C_XferSync
(
    const uint8                         ucCh,
    const uint8                         ucSlaveAddr,
    I2CXfer_t                           XferArg
)
{
    SALRetCode_t    ret;
    uint32          i;
    const uint8                         ucCmdLen = XferArg.xCmdLen;
    const uint8                         ucOutLen = XferArg.xOutLen;
    const uint8                         ucInLen = XferArg.xInLen;
    uint8 *                             ucpInBuf = XferArg.xInBuf;
    uint32                              uiOpt = XferArg.xOpt;

    ret = SAL_RET_SUCCESS;

    /* Write*/
    if((ucOutLen > (uint8)NULL) || (ucCmdLen > (uint8)NULL))
    {

        ret = I2C_XferSyncWrite(ucCh, ucSlaveAddr, XferArg);
    }

    if(ret == SAL_RET_SUCCESS)
    {
        /* Read */
        if(ucInLen > (uint8)NULL)
        {

            if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_TXR))
            {
                /* Send START condition and slave address with read flag*/
                SAL_WriteReg((ucSlaveAddr | I2C_RD), (uint32)(i2c[ucCh].dBase + I2C_TXR));
            }

            if(((uiOpt & I2C_RD_WITHOUT_RPEAT_START) != (uint32) NULL))
            {
                SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
            }
            else
            {
                SAL_WriteReg((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
            }

            ret = I2C_Wait(ucCh);

            if(ret == SAL_RET_SUCCESS)
            {
                ret = I2C_CheckAck(ucCh);

                if(ret == SAL_RET_SUCCESS)
                {
                    for(i = 0 ; i < ucInLen ; i++)
                    {
                        if (i == (ucInLen - 1UL))
                        {
                            /* Last byte to read */
                            if((uiOpt & I2C_RD_WITHOUT_ACK) != (uint32)NULL)
                            {
                                if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
                                {
                                    SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                                }
                            }
                            else
                            {
                                SAL_WriteReg(((uint32)I2C_CMD_RD | (uint32)I2C_CMD_ACK), (uint32)(i2c[ucCh].dBase + I2C_CMD));
                            }
                        }
                        else
                        {
                            SAL_WriteReg(I2C_CMD_RD , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                        }

                        ret = I2C_Wait(ucCh);

                        if(ret == SAL_RET_SUCCESS)
                        {
                            /* Store read data */
                            if(ucpInBuf != NULL_PTR)
                            {
                                if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CMD))
                                {
                                    ucpInBuf[i] = (uint8)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_RXR)) & 0xffUL);
                                }
                            }
                        }
                    }

                    if(ret == SAL_RET_SUCCESS)
                    {
                        if((uiOpt & I2C_RD_WITHOUT_STOP) != (uint32)NULL)
                        {
                            ret = SAL_RET_SUCCESS;
                        }
                    }
                }
            }
        }
    }

    ret = I2C_ProcessAckFail(ucCh, ucSlaveAddr);

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_XferAsync
*
* Function to async transfer data via I2C channel with param info.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    XferArg [in]    : Xfer argument
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           : this function will be called according ucAsync value in I2C_Xfer function params.
*
***************************************************************************************************
*/
static SALRetCode_t I2C_XferAsync
(
    const uint8                          ucCh,
    const uint8                          ucSlaveAddr,
    I2CXfer_t                            XferArg
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    i2c[ucCh].dAsync.axSlaveAddr    = ucSlaveAddr;
    i2c[ucCh].dAsync.axCmdLen       = XferArg.xCmdLen;
    i2c[ucCh].dAsync.axCmdBuf       = XferArg.xCmdBuf;
    i2c[ucCh].dAsync.axOutLen       = XferArg.xOutLen;
    i2c[ucCh].dAsync.axOutBuf       = XferArg.xOutBuf;
    i2c[ucCh].dAsync.axInBuf        = XferArg.xInBuf;
    i2c[ucCh].dAsync.axInLen        = XferArg.xInLen;
    i2c[ucCh].dAsync.axOpt          = XferArg.xOpt;

    I2C_EnableIrq(ucCh);

    if((i2c[ucCh].dAsync.axCmdLen > (uint8)NULL) || (i2c[ucCh].dAsync.axOutLen > (uint8)NULL))
    {
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_SEND;
        (void)SAL_CoreCriticalExit();
        I2C_D("%s ch %d I2C_STATE_SEND\n", __func__, ucCh);
        SAL_WriteReg((ucSlaveAddr | I2C_WR), (uint32)(i2c[ucCh].dBase + I2C_TXR));
        SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR) , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    }
    else if(i2c[ucCh].dAsync.axInLen > (uint8)NULL)
    {
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_RECV_START;
        (void)SAL_CoreCriticalExit();
        I2C_D("%s ch %d I2C_STATE_RECV_START\n", __func__, ucCh);
        SAL_WriteReg((ucSlaveAddr | I2C_RD), (uint32)(i2c[ucCh].dBase + I2C_TXR));
        SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR) , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    }
    else
    {
        I2C_DisableIrq(ucCh);
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_IDLE;
        (void)SAL_CoreCriticalExit();

        ret = SAL_RET_FAILED;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_Xfer
*
* Function to transfer data.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    XferArg [in]    : Xfer argument
* @param    ucAsync   [in]  : I2C transfer mode
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           : This function can be called by application to transfer data via I2C.
*
***************************************************************************************************
*/

SALRetCode_t I2C_Xfer
(
    const uint8                         ucCh,
    const uint8                         ucSlaveAddr,
    I2CXfer_t                           XferArg,
    uint8                               ucAsync
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;
    (void)SAL_CoreCriticalEnter();

    if(i2c[ucCh].dState != I2C_STATE_IDLE)
    {
        (void)SAL_CoreCriticalExit();
        I2C_D("[%s] invaild state\n", __func__);

        ret = SAL_RET_FAILED;
    }
    else
    {
        i2c[ucCh].dState = I2C_STATE_RUNNING;
        (void)SAL_CoreCriticalExit();
        (void)I2C_BusyCheck(ucCh);

        if(ucAsync != (uint8)NULL)
        {
            ret = I2C_XferAsync(ucCh, ucSlaveAddr, XferArg);
        }
        else
        {
            ret = I2C_XferSync(ucCh, ucSlaveAddr, XferArg);
            (void)SAL_CoreCriticalEnter();
            i2c[ucCh].dState = I2C_STATE_IDLE;
            (void)SAL_CoreCriticalExit();
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_XferCmd
*
* Function to transfer data with command.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    XferArg [in]    : Xfer argument
* @param    ucAsync   [in]  : I2C transfer mode
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t I2C_XferCmd
(
    const uint8                         ucCh,
    const uint8                         ucSlaveAddr,
    I2CXfer_t                           XferArg,
    uint8                               ucAsync
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;
    (void)SAL_CoreCriticalEnter();

    if(i2c[ucCh].dState != I2C_STATE_IDLE)
    {
        (void)SAL_CoreCriticalExit();
        ret = SAL_RET_FAILED;
    }
    else
    {
        i2c[ucCh].dState = I2C_STATE_RUNNING;
        (void)SAL_CoreCriticalExit();
        (void)I2C_BusyCheck(ucCh);

        if(ucAsync != (uint8)NULL)
        {
            ret = I2C_XferAsync(ucCh, ucSlaveAddr, XferArg);
        }
        else
        {
            ret = I2C_XferSync(ucCh, ucSlaveAddr, XferArg);

            if(I2C_CheckAck(ucCh) == SAL_RET_SUCCESS)
            {
                //I2C_D("Receive ACK ! addr : %x \n", (ucSlaveAddr >> (uint8)1UL));
            }
            else
            {
                ret = SAL_RET_FAILED;
            }

            (void)SAL_CoreCriticalEnter();
            i2c[ucCh].dState = I2C_STATE_IDLE;
            (void)SAL_CoreCriticalExit();
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_IsrStartSend
*
* Function to process SEND phase in interrupt.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrStartSend
(
    uint8                               ucCh
)
{

    if(i2c[ucCh].dAsync.axCmdLen != (uint8)NULL)
    {
        /* Send cmd first, if exist */
        SAL_WriteReg(*i2c[ucCh].dAsync.axCmdBuf, (uint32)(i2c[ucCh].dBase + I2C_TXR));
        i2c[ucCh].dAsync.axCmdLen--;
        i2c[ucCh].dAsync.axCmdBuf++;

        SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    }
    else
    {
        if(i2c[ucCh].dAsync.axOutLen != (uint8)NULL)
        {
            /* Send data next to the cmd */
            SAL_WriteReg(*i2c[ucCh].dAsync.axOutBuf, (uint32)(i2c[ucCh].dBase + I2C_TXR));
            i2c[ucCh].dAsync.axOutLen--;
            i2c[ucCh].dAsync.axOutBuf++;

            SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
        }
        else
        {
            /* After send cmd and out data */
            if(i2c[ucCh].dAsync.axInLen != (uint8)NULL)
            {
                /* Start to recevie data */
                i2c[ucCh].dState = I2C_STATE_RECV_START;
                I2C_D("%s ch %d I2C_STATE_RECV_START\n", __func__, ucCh);
                SAL_WriteReg((i2c[ucCh].dAsync.axSlaveAddr | I2C_RD), (uint32)(i2c[ucCh].dBase + I2C_TXR));

                if(((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_RPEAT_START) != (uint32)NULL))
                {
                    SAL_WriteReg(I2C_CMD_WR, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
                else
                {
                    SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR), (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
            }
            else
            {
                /* Finish the transfer */
                i2c[ucCh].dState = I2C_STATE_SEND_DONE;

                if((i2c[ucCh].dAsync.axOpt & I2C_WR_WITHOUT_STOP) == (uint32)NULL)
                {
                    SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
                else
                {
                    I2C_XferComplete(ucCh, I2C_EVENT_XFER_COMPLETE);
                }
            }
        }
    }
}

/*
***************************************************************************************************
*                                          I2C_IsrRecvStart
*
* Function to process RECV phase in interrupt.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrRecvStart
(
    uint32                              ucCh
)
{
    if(i2c[ucCh].dAsync.axInLen != (uint8)NULL)
    {
        i2c[ucCh].dState = I2C_STATE_RECV;
        I2C_D("%s ch %d I2C_STATE_RECV\n", __func__, ucCh);

        if(i2c[ucCh].dAsync.axInLen == (uint8)1UL)
        {
            /* Read last byte */
            if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_ACK) != (uint32)NULL)
            {
                SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
            }
            else
            {
                SAL_WriteReg(((uint32)I2C_CMD_RD | (uint32)I2C_CMD_ACK), (uint32)(i2c[ucCh].dBase + I2C_CMD));
            }
        }
        else
        {
            SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
        }
    }
    else
    {
        i2c[ucCh].dState = I2C_STATE_RECV_DONE;
        I2C_D("%s ch %d I2C_STATE_RECV_DONE\n", __func__, ucCh);

        if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_STOP) == (uint32)NULL)
        {
            SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
        }
    }
}

/*
***************************************************************************************************
*                                          I2C_Isr
*
* Fucntion to process I2C interrupt.
*
* @param    ucCh [in]       : Value of channel to control
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_Isr
(
    uint8                               ucCh
)
{
    uint32  status;
    uint32  is_intr;
    uint32  is_ack;

    status  = 0;
    is_intr = 0;
    is_ack  = 0;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_SR))
    {
        status  = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR));
    }

    is_intr = status & I2C_SR_IF;

    if(is_intr != (uint8)NULL)
    {
        is_ack = (I2C_CheckAck(ucCh) == SAL_RET_SUCCESS) ? 1UL : 0UL;
        I2C_ClearIntr(ucCh);

        if(i2c[ucCh].dState == I2C_STATE_SEND)
        {
            if(is_ack != (uint8)NULL)
            {
				I2C_IsrStartSend(ucCh);
            }
            else
            {
                SAL_WriteReg(BSP_BIT_06 ,(uint32)( i2c[ucCh].dBase + I2C_CMD));
                I2C_D("%s: i2c ch %d received NACK from 0x%x\n", __func__, ucCh, (i2c[ucCh].dAsync.axSlaveAddr >> (uint8)1UL));
                I2C_XferComplete(ucCh, I2C_EVENT_NACK);
            }
        }
        else if(i2c[ucCh].dState == I2C_STATE_SEND_DONE)
        {
            I2C_XferComplete(ucCh, I2C_EVENT_XFER_COMPLETE);
        }
        else if(i2c[ucCh].dState == I2C_STATE_RECV_START)
        {
            if(is_ack != (uint8)NULL)
            {
                I2C_IsrRecvStart(ucCh);
            }
            else
            {
                SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                I2C_D("%s: i2c ch %d received NACK from 0x%x\n", __func__, ucCh, (i2c[ucCh].dAsync.axSlaveAddr >> (uint8)1UL));
                I2C_XferComplete(ucCh, I2C_EVENT_NACK);
            }
        }
        else if(i2c[ucCh].dState == I2C_STATE_RECV)
        {
            *i2c[ucCh].dAsync.axInBuf = (uint8)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_RXR)) & 0xffUL);
            i2c[ucCh].dAsync.axInLen--;
            i2c[ucCh].dAsync.axInBuf++;

            if(i2c[ucCh].dAsync.axInLen != (uint8)NULL)
            {
                if(i2c[ucCh].dAsync.axInLen == (uint8)1UL)
                {
                    if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_ACK) != (uint32)NULL)
                    {
                        /* Read last byte */
                        SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }
                    else
                    {
                        SAL_WriteReg(((uint32)I2C_CMD_RD | (uint32)I2C_CMD_ACK), (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }
                }
                else
                {
                    SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
            }
            else
            {
                i2c[ucCh].dState = I2C_STATE_RECV_DONE;
                I2C_D("%s ch %d I2C_STATE_RECV_DONE\n", __func__, ucCh);

                if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_STOP) == (uint32)NULL)
                {
                    SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
            }
        }
        else if(i2c[ucCh].dState == I2C_STATE_RECV_DONE)
        {
            I2C_XferComplete(ucCh, I2C_EVENT_XFER_COMPLETE);
        }
        else
        {
            /* Nothing to do */
        }
    }
}

/*
***************************************************************************************************
*                                          I2C_IsrCh0
* @param    pArg
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh0
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH0 ISR\n", __func__);
    I2C_Isr(0);
}

/*
***************************************************************************************************
*                                          I2C_IsrCh1
* @param    pArg
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh1
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH1 ISR\n", __func__);
    I2C_Isr(1);
}

/*
***************************************************************************************************
*                                          I2C_IsrCh2
* @param    pArg
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh2
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH2 ISR\n", __func__);
    I2C_Isr(2);
}

/*
***************************************************************************************************
*                                          I2C_IsrCh3
* @param    pArg
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh3
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH3 ISR\n", __func__);
    I2C_Isr(3);
}


/*
***************************************************************************************************
*                                          I2C_IsrCh4
* @param    pArg
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh4
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH4 ISR\n", __func__);
    I2C_Isr(4);
}


/*
***************************************************************************************************
*                                          I2C_IsrCh5
* @param    pArg
* @return   none
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh5
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH5 ISR\n", __func__);
    I2C_Isr(5);
}

/*
***************************************************************************************************
*                                          I2C_Reset
*
* Function to reset I2C Channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_Reset
(
    uint8                               ucCh
)
{
    sint32  err;
    SALRetCode_t ret;

    err = 0;
    ret = SAL_RET_SUCCESS;

    /* SW reset */
    if(i2c[ucCh].dIobusName < (uint32)CLOCK_IOBUS_MAX)
    {
        err = CLOCK_SetSwReset((sint32)i2c[ucCh].dIobusName, TRUE);
    }

    if(err != (sint32)NULL)
    {
        ret = SAL_RET_FAILED;
    }

    if(i2c[ucCh].dIobusName < (uint32)CLOCK_IOBUS_MAX)
    {
        err = CLOCK_SetSwReset((sint32)i2c[ucCh].dIobusName, FALSE);
    }

    if(err != (sint32)NULL)
    {
        ret = SAL_RET_FAILED;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_SetGpio
*
* Function to set gpio configuration.
*
* @param    ucCh [in]       : Value of channel to control
* @param    psInfo [in]     : Pointer of port information structure
* @return   0 and positive value means 'success'.
* Notes
*
***************************************************************************************************
*/

static sint32 I2C_SetGpio
(
    uint8                               ucCh,
    const I2CPortConfig_t *             psInfo
)
{
    SALRetCode_t    retCfg;
    SALRetCode_t    ret1;
    SALRetCode_t    ret2;
    sint32          ret;

    if (psInfo != NULL_PTR)
    {
        /* set port controller, channel */
        switch(ucCh)
        {
            case I2C_CH_0:
                retCfg = GPIO_PerichSel(GPIO_PERICH_SEL_I2CSEL_0, psInfo->pcPortSelCh);
                break;
            case I2C_CH_1:
                retCfg = GPIO_PerichSel(GPIO_PERICH_SEL_I2CSEL_1, psInfo->pcPortSelCh);
                break;
            case I2C_CH_2:
                retCfg = GPIO_PerichSel(GPIO_PERICH_SEL_I2CSEL_2, psInfo->pcPortSelCh);
                break;
            case I2C_CH_3:
                retCfg = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL0, GPIO_MFIO_I2C3, GPIO_MFIO_CFG_CH_SEL0, psInfo->pcPortSelCh);
                break;
            case I2C_CH_4:
                retCfg = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL1, GPIO_MFIO_I2C4, GPIO_MFIO_CFG_CH_SEL1, psInfo->pcPortSelCh);
                break;
            case I2C_CH_5:
                retCfg = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL2, GPIO_MFIO_I2C5, GPIO_MFIO_CFG_CH_SEL2, psInfo->pcPortSelCh);
                break;
            default:
                break;
        }

        if (retCfg != SAL_RET_FAILED)
        {
            /* set port */
            //ret1 = GPIO_Config( psInfo->pcScl, (psInfo->pcFs | GPIO_OUTPUT) );
            //ret2 = GPIO_Config( psInfo->pcSda, (psInfo->pcFs | GPIO_INPUT | GPIO_INPUTBUF_EN) );
            ret1 = GPIO_Config( psInfo->pcScl, (psInfo->pcFs | GPIO_INPUT | GPIO_INPUTBUF_EN) );
            ret2 = GPIO_Config( psInfo->pcSda, (psInfo->pcFs | GPIO_INPUT | GPIO_INPUTBUF_EN) );

            i2c[ucCh].dScl = psInfo->pcScl;
            i2c[ucCh].dSda = psInfo->pcSda;

            if( ( ret1 != SAL_RET_SUCCESS ) || ( ret2 != SAL_RET_SUCCESS ) )
            {
                ret = -1;
            }
            else
            {
                ret = 0;
            }
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_SetPort
*
* Fucntion to set chaneel port info (SDA, SCL).
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiPortSel [in]  : Value of port selection
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_SetPort
(
    uint8                               ucCh,
    uint32                              uiPortSel
)
{
    uint32  idx;
    sint32  ret;

    static const I2CPortConfig_t i2cPortCfg[I2C_PORT_NUM] =
    {
        { 0UL,  GPIO_GPB(0UL),  GPIO_GPB(1UL),  GPIO_FUNC(1UL), GPIO_PERICH_CH0 },  // I2C0, CH_0
        { 1UL,  GPIO_GPA(4UL),  GPIO_GPA(5UL),  GPIO_FUNC(2UL), GPIO_PERICH_CH1 },  // I2C0, CH_1

        { 2UL,  GPIO_GPB(2UL),  GPIO_GPB(3UL),  GPIO_FUNC(1UL), GPIO_PERICH_CH0 },  // I2C1, CH_0

        { 3UL,  GPIO_GPA(24UL), GPIO_GPA(26UL), GPIO_FUNC(2UL), GPIO_PERICH_CH0 },  // I2C2, CH_0
        { 4UL,  GPIO_GPC(4UL),  GPIO_GPC(5UL),  GPIO_FUNC(2UL), GPIO_PERICH_CH1 },  // I2C2, CH_1

        { 5UL,  GPIO_GPA(18UL), GPIO_GPA(19UL), GPIO_FUNC(3UL), GPIO_PERICH_CH0 },  // I2C3, CH_0
        { 6UL,  GPIO_GPB(2UL),  GPIO_GPB(3UL),  GPIO_FUNC(3UL), GPIO_PERICH_CH1 },  // I2C3, CH_1
        { 7UL,  GPIO_GPC(6UL),  GPIO_GPC(7UL),  GPIO_FUNC(3UL), GPIO_PERICH_CH2 },  // I2C3, CH_2
        { 8UL,  GPIO_GPK(13UL), GPIO_GPK(14UL), GPIO_FUNC(3UL), GPIO_PERICH_CH3 },  // I2C3, CH_3

        { 9UL,  GPIO_GPA(22UL), GPIO_GPA(23UL), GPIO_FUNC(3UL), GPIO_PERICH_CH0 },  // I2C4, CH_0
        { 10UL, GPIO_GPB(6UL),  GPIO_GPB(7UL),  GPIO_FUNC(3UL), GPIO_PERICH_CH1 },  // I2C4, CH_1
        { 11UL, GPIO_GPC(10UL), GPIO_GPC(11UL), GPIO_FUNC(3UL), GPIO_PERICH_CH2 },  // I2C4, CH_2

        { 12UL, GPIO_GPA(26UL), GPIO_GPA(27UL), GPIO_FUNC(3UL), GPIO_PERICH_CH0 },  // I2C5, CH_0
        { 13UL, GPIO_GPB(10UL), GPIO_GPB(11UL), GPIO_FUNC(3UL), GPIO_PERICH_CH1 },  // I2C5, CH_1
        { 14UL, GPIO_GPC(14UL), GPIO_GPC(15UL), GPIO_FUNC(3UL), GPIO_PERICH_CH2 },  // I2C5, CH_2
    };

    ret = 0;

    for(idx = 0UL ; idx <= I2C_PORT_NUM ; idx++)
    {
        if(i2cPortCfg[idx].pcI2cCh == uiPortSel)
        {
            ret = I2C_SetGpio(ucCh, &i2cPortCfg[idx]);
            break;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_ClearPort
*
* Function to clear channel port info.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_ClearPort
(
    uint32                              ucCh
)
{
    uint32          sda;
    uint32          scl;
    SALRetCode_t    ret;

    sda         = 0;
    scl         = 0;
    ret         = SAL_RET_SUCCESS;

    if(ucCh >= I2C_CH_NUM)
    {
        ret = SAL_RET_FAILED;
    }
    else
    {
        sda = i2c[ucCh].dSda;
        scl = i2c[ucCh].dScl;

        /* Reset gpio */
        ret = GPIO_Config(sda, GPIO_FUNC(0UL));

        if(ret != SAL_RET_SUCCESS)
        {
            ret = SAL_RET_FAILED;
        }
        else
        {
            ret = GPIO_Config(scl, GPIO_FUNC(0UL));

            if(ret != SAL_RET_SUCCESS)
            {
                ret = SAL_RET_FAILED;
            }
            else
			{
			    /* Reset MFIO Configuration */
			    switch(ucCh)
			    {
			        case I2C_CH_3:
			            GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL0, GPIO_MFIO_DISABLE, GPIO_MFIO_CFG_CH_SEL0, GPIO_MFIO_CH0);
			            break;
			        case I2C_CH_4:
			            GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL1, GPIO_MFIO_DISABLE, GPIO_MFIO_CFG_CH_SEL1, GPIO_MFIO_CH0);
			            break;
			        case I2C_CH_5:
			            GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL2, GPIO_MFIO_DISABLE, GPIO_MFIO_CFG_CH_SEL2, GPIO_MFIO_CH0);
			            break;
			        default:
			            break;
			    }
			}
		}

		i2c[ucCh].dPort = NULL;
	}

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_SetFilter
*
* Function to enable/disble filter and set filter value
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiValue [in]    : Value of filter
* @return   SAL_RET_SUCCESS
* Notes
*           : Bus clock freq = 300 Mhz ( 3.33 ns per tick)
*             FLCV value = 16
*             3.33ns * 16 = 53.28 ns
*             ignore noise signal width lower than 53.28 ns.
*
***************************************************************************************************
*/

static SALRetCode_t I2C_SetFilter
(
    uint8                               ucCh,
    uint32                              uiValue
)
{
    uint32  tmp;

    tmp = 0;

    if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_TIME_0))
    {
        tmp = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_TIME_0));

        tmp = tmp & ~(I2C_FCLV_MASK << I2C_FCLV_SHIFT);
        tmp = tmp | (((uiValue & I2C_FCLV_MASK) << I2C_FCLV_SHIFT));

        SAL_WriteReg(tmp, (uint32)(i2c[ucCh].dBase + I2C_TIME_0));
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_SetClock
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiInputClkKHz   : Value of GPSB speed
* @return   SAL_RET_SUCCESS
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_SetClock
(
    uint8                               ucCh,
    uint32                              uiInputClkKHz
)
{
    uint32  prescale;

    prescale    = 0;

    /* Set prescale */
    if(i2c[ucCh].dClk < (400UL + 1UL))
    {
        prescale = (uiInputClkKHz / ((i2c[ucCh].dClk) * (uint32)5UL));

        if(prescale > 0UL)
        {
            prescale = prescale - (uint32)1UL;
        }
    }

    SAL_WriteReg(prescale, (uint32)(i2c[ucCh].dBase + I2C_PRES));

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_Enable
* Function to enable I2C block
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_Enable
(
    uint8                               ucCh
)
{
    uint32  input_clk_kHz;
    sint32  err;
    SALRetCode_t ret;

    input_clk_kHz   = 0;
    err             = 0;
    ret = SAL_RET_SUCCESS;

    if(i2c[ucCh].dPeriName < (uint32)CLOCK_PERI_MAX)
    {
        /* enable i2c peri bus */
        err = CLOCK_SetPeriRate((sint32)i2c[ucCh].dPeriName, I2C_PCLK);
    }

    if(err != (sint32)0UL)
    {
        ret = SAL_RET_FAILED;
    }
    else
    {
        (void) CLOCK_EnablePeri((sint32)i2c[ucCh].dPeriName);

        if(i2c[ucCh].dIobusName < (uint32)CLOCK_IOBUS_MAX)
        {
            /* enable iobus */
            err = CLOCK_SetIobusPwdn((sint32)i2c[ucCh].dIobusName, FALSE);
        }

        if(err != (sint32)0UL)
        {
            ret = SAL_RET_FAILED;
        }
        else
        {
            /* get i2c bus clock */
            input_clk_kHz = CLOCK_GetPeriRate((sint32)i2c[ucCh].dPeriName);        /* get I2C bus clock */
            I2C_D("%s: I2C bus clock %d\n", __func__, input_clk_kHz);

            /* set i2c clock */
            (void)I2C_SetClock(ucCh, (input_clk_kHz/1000UL)); //Codesonar Unused Value (err)

            if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CTRL))
            {
                /* Enable I2C core. Set 8bit mode*/
                SAL_WriteReg(I2C_CTRL_EN_CORE , (uint32)(i2c[ucCh].dBase + I2C_CTRL));
            }

            /* Clear pending interrupt */
            I2C_ClearIntr(ucCh);
        }

    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_Open
*
* Function to initialize I2C Channel
*
* @param    ucCh [in]         : Value of channel to control
* @param    uiPortSel [in]    : Index of port selection
* @param    uiSpeedInKHz [in] : Value of SCLK speed
* @param    fnCallback [in]   : Fucntion poionter of callback
* @param    pArg
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t I2C_Open
(
    uint8                               ucCh,
    uint32                              uiPortSel,
    uint32                              uiSpeedInKHz,
    I2CCallback                         fnCallback,
    void *                              pArg
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    if(ucCh >= I2C_CH_NUM)
    {
        I2C_D("%s: %d channel is wrong\n", __func__, ucCh);

        ret = SAL_RET_FAILED;
    }
    else
    {
    	i2c[ucCh].dPort                     = uiPortSel;
        i2c[ucCh].dClk                      = uiSpeedInKHz;
        i2c[ucCh].dComplete.ccCallBack      = fnCallback;
        i2c[ucCh].dComplete.ccArg           = pArg;
        (void)SAL_MemSet(&i2c[ucCh].dAsync, 0, sizeof(I2CAsyncXfer_t));

        ret = I2C_Reset(ucCh);

        if (ret != SAL_RET_SUCCESS)
        {
            I2C_D("%s: ch %d failed to reset core\n", __func__, ucCh);
        }
        else
        {

    		ret = I2C_SetPort(ucCh, i2c[ucCh].dPort);

            if (ret != SAL_RET_SUCCESS)
            {
                I2C_D("%s: ch %d failed to set port\n", __func__, ucCh);
            }
            else
            {

                ret = I2C_Enable(ucCh);

                if (ret != SAL_RET_SUCCESS)
                {
                    I2C_D("%s: ch %d failed to enable core\n", __func__, ucCh);
                }
                else
                {
                    #ifdef I2C_USE_FILTER
                    (void)I2C_SetFilter(ucCh, 16UL);
                    #endif

                    (void)SAL_CoreCriticalEnter();
                    i2c[ucCh].dState = I2C_STATE_IDLE;
                    (void)SAL_CoreCriticalExit();
                }
            }
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_Close
*
* Function to deinitialize I2C Channel
* @param    ucCh [in]         : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t I2C_Close
(
    uint8                               ucCh
)
{
    sint32          iRet;
    SALRetCode_t    ret;

    iRet = 0;
    ret = SAL_RET_SUCCESS;

    if (ucCh > I2C_CH_NUM)
    {
        I2C_D("%s %d channel is wrong\n", __func__, ucCh);
        ret = SAL_RET_FAILED;
    }
    else
    {
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_DISABLED;
        (void)SAL_CoreCriticalExit();
        I2C_DisableIrq(ucCh);
        I2C_ClearIntr(ucCh);

        if(i2c[ucCh].dBase < (UINT_MAX_VALUE - I2C_CTRL))
        {
            SAL_WriteReg(0x0, (uint32)(i2c[ucCh].dBase + I2C_CTRL));
        }

        if(i2c[ucCh].dPeriName < (uint32) CLOCK_PERI_MAX)
        {
            (void)CLOCK_DisablePeri((sint32)i2c[ucCh].dPeriName);
        }

        if(i2c[ucCh].dIobusName < (uint32) CLOCK_IOBUS_MAX)
        {
            iRet = CLOCK_SetIobusPwdn((sint32)i2c[ucCh].dIobusName, TRUE);
        }

        if (iRet != (sint32)NULL)
        {
            ret = SAL_RET_FAILED;
        }
        else
        {
            ret = I2C_ClearPort((uint32)ucCh);

            if (ret != SAL_RET_SUCCESS)
            {
                I2C_D("%s ch %d failed to reset port\n", __func__, ucCh);
            }
            else
            {
                ret = I2C_Reset(ucCh);

                if(ret != SAL_RET_SUCCESS)
                {
                    I2C_D("%s ch %d failed to reset core\n", __func__, ucCh);
                }
            }
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_Init
*
* Function to register I2C interrupt
* @param    none
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
***************************************************************************************************
*/

void I2C_Init
(
    void
)
{
    /* Enter i2c irq handler */
    (void)GIC_IntVectSet((uint32)GIC_I2C, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh0, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C);

    (void)GIC_IntVectSet((uint32)GIC_I2C1, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh1, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C1);

    (void)GIC_IntVectSet((uint32)GIC_I2C2, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh2, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C2);

    (void)GIC_IntVectSet((uint32)GIC_I2C3, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh3, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C3);

    (void)GIC_IntVectSet((uint32)GIC_I2C4, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh4, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C4);

    (void)GIC_IntVectSet((uint32)GIC_I2C5, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh5, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C5);


    return;
}

/*
***************************************************************************************************
*                                          I2C_Deinit
*
* Function to unregister I2C interrupt
*
* @return   none
* Notes
*
***************************************************************************************************
*/

void I2C_Deinit
(
    void
)
{
    (void)GIC_IntSrcDis(GIC_I2C);
    (void)GIC_IntSrcDis(GIC_I2C1);
    (void)GIC_IntSrcDis(GIC_I2C2);
    (void)GIC_IntSrcDis(GIC_I2C3);
    (void)GIC_IntSrcDis(GIC_I2C4);
    (void)GIC_IntSrcDis(GIC_I2C5);

    return;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_I2C == 1 )


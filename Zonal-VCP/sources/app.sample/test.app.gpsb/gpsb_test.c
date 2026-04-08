// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gpsb_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <gpsb_test.h>
#include <gpsb_reg.h>
#include <bsp.h>
#include <fmu.h>
#include <mpu.h>
#include <debug.h>

/*************************************************************************************************/
/*                                             LOCAL VARIABLES                                   */
/*************************************************************************************************/

static uint32 *                         g_tx_dma;
static uint32 *                         g_rx_dma;

static uint32                           g_tx[GPSB_TEST_BUF_SIZE] = {0,};
static uint32                           g_rx[GPSB_TEST_BUF_SIZE] = {0,};

static uint8                            g_tx2[GPSB_TEST_BUF_SIZE] = {0,};
static uint8                            g_rx2[GPSB_TEST_BUF_SIZE] = {0,};

/*
***************************************************************************************************
*                                          delay1us
*
* Function to set delay with 'nop'.
*
* @param    uiUs
* @return
* Notes
*
***************************************************************************************************
*/

static void delay1us
(
    uint32                              uiUs
) {
    uint32                              i;
    uint32                              sec;

    i = 0xffUL;
    sec = 0;

    sec = uiUs * (uint32)500UL;

    for (i = 0; i < sec; i++)
    {
        BSP_NOP_DELAY();
    }

    return;
}

/*
***************************************************************************************************
*                                          GPSB_FmuIsr
*
* Function to process FMU ISR.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_FmuIsr
(
    FMUFaultid_t                        id
) {
    FMUErrTypes_t                       err;
    SALRetCode_t                        ret;

    err = FMU_OK;
    ret = SAL_RET_SUCCESS;

    mcu_printf("***************************\n");
    mcu_printf("GPSB_FmuIsr\n");
    mcu_printf("***************************\n");

    err = FMU_IsrClr((id));

    if(err != FMU_OK)
    {
        mcu_printf("FMU ISR ERROR\n");
    }

    ret = GPSB_SafetyGetStatus((uint32)id-1UL);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("GPSB_SafetyGetStatus Fail\n");
    }

    return;
}

/*
***************************************************************************************************
*                                          GPSB_FmuIsr0
*
* Function to process FMU ISR for GPSB Channel 0.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_FmuIsr0
(
    void                                *pArg
) {
    // unused
    (void)pArg;

    mcu_printf("Channel 0 fmu isr\n");
    GPSB_FmuIsr((FMUFaultid_t)FMU_ID_GPSB0);
}

/*
***************************************************************************************************
*                                          GPSB_FmuIsr1
*
* Function to process FMU ISR for GPSB Channel 1.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_FmuIsr1
(
    void                                *pArg
) {
    // unused
    (void)pArg;

    mcu_printf("Channel 1 fmu isr\n");
    GPSB_FmuIsr((FMUFaultid_t)FMU_ID_GPSB1);
}

/*
***************************************************************************************************
*                                          GPSB_FmuIsr2
*
* Function to process FMU ISR for GPSB Channel 2.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_FmuIsr2
(
    void                                *pArg
) {
    // unused
    (void)pArg;

    mcu_printf("Channel 2 fmu isr\n");
    GPSB_FmuIsr((FMUFaultid_t)FMU_ID_GPSB2);
}

/*
***************************************************************************************************
*                                          GPSB_FmuIsr3
*
* Function to process FMU ISR for GPSB Channel 3.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_FmuIsr3
(
    void                                *pArg
) {
    // unused
    (void)pArg;

    mcu_printf("Channel 3 fmu isr\n");
    GPSB_FmuIsr((FMUFaultid_t)FMU_ID_GPSB3);
}

/*
***************************************************************************************************
*                                          GPSB_FmuIsr4
*
* Function to process FMU ISR for GPSB Channel 4.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_FmuIsr4
(
    void                                *pArg
) {
    // unused
    (void)pArg;

    mcu_printf("Channel 4 fmu isr\n");
    GPSB_FmuIsr((FMUFaultid_t)FMU_ID_GPSB4);
}

/*
***************************************************************************************************
*                                          GPSB_EnableFmu
*
* Function to register FMU ISR and enable FMU GPSB interrupt.
*
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_EnableFmu
(
    void
) {
    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_GPSB0, (FMUSeverityLevelType_t)FMU_SVL_LOW,
            (FMUIntFnctPtr)&GPSB_FmuIsr0, NULL_PTR);
    (void)FMU_Set((FMUFaultid_t)FMU_ID_GPSB0);

    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_GPSB1, (FMUSeverityLevelType_t)FMU_SVL_LOW,
            (FMUIntFnctPtr)&GPSB_FmuIsr1, NULL_PTR);
    (void)FMU_Set((FMUFaultid_t)FMU_ID_GPSB1);

    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_GPSB2, (FMUSeverityLevelType_t)FMU_SVL_LOW,
            (FMUIntFnctPtr)&GPSB_FmuIsr2, NULL_PTR);
    (void)FMU_Set((FMUFaultid_t)FMU_ID_GPSB2);

    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_GPSB3, (FMUSeverityLevelType_t)FMU_SVL_LOW,
            (FMUIntFnctPtr)&GPSB_FmuIsr3, NULL_PTR);
    (void)FMU_Set((FMUFaultid_t)FMU_ID_GPSB3);

    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_GPSB4, (FMUSeverityLevelType_t)FMU_SVL_LOW,
            (FMUIntFnctPtr)&GPSB_FmuIsr4, NULL_PTR);
    (void)FMU_Set((FMUFaultid_t)FMU_ID_GPSB4);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyTestProtocolMonitor
*
* Function to test protocol monitor error.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucBpw [in]      : Value of bit per word (8/16/32)
* @return
* Notes
*       - IO Monitor test
*        : Check Protocol Error
*         Protocol error is occured with invaild SCK.
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_SafetyTestProtocolMonitor
(
    uint8                               ucCh,
    uint8                               ucBpw
) {
    SALRetCode_t                        ret;
    GPSBSafetyParam_t                   param;
    uint32                              speed;
    uint32                              data_size;
    uint32                              xfer_mode;
    uint8                               tx[8] = {1,2,3,4,5,6,7,8};
    uint8                               rx[8] = {0};
    GPSBOpenParam_t                     OpenParam;

    ret = SAL_RET_SUCCESS;
    speed = 0UL;
    xfer_mode = 0UL;

    /* Configure GPSB master */
    speed     = 1UL*1000UL*1000UL;

    if(ucBpw == (uint8)8UL)
    {
        data_size = 1UL;
    }
    else if(ucBpw == (uint8)16UL)
    {
        data_size = 4UL;
    }
    else if(ucBpw == (uint8)32UL)
    {
        data_size = 8UL;
    }
    else
    {
        data_size = 0UL;
    }
	OpenParam.uiSdo         = GPIO_GPA(2UL);
    OpenParam.uiSdi         = GPIO_GPA(3UL);
    OpenParam.uiSclk        = GPIO_GPA(0UL);
    OpenParam.pDmaAddrTx    = NULL_PTR;
    OpenParam.pDmaAddrRx    = NULL_PTR;
    OpenParam.uiDmaBufSize  = 0;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;
    ret = GPSB_Open((uint32)ucCh, OpenParam);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch 0x%x open fail. error code : %d \n", ucCh, ret);

        return SAL_RET_FAILED;
    }

    ret = GPSB_SetSpeed(ucCh, speed);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch 0x%x setuiSpeed fail. error code : %d \n", ucCh, ret);

        return SAL_RET_FAILED;
    }

    ret = GPSB_SetBpw(ucCh, ucBpw);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch 0x%x set_bpw fail. error code : %d \n", ucCh, ret);

        return SAL_RET_FAILED;
    }

    /* Configure loopback mode */
    (void)GPSB_SafetyEnableLoopback(ucCh, ucCh);

    /* Configure IO monitor for master */

    param.spCh       = ucCh;
    param.spMonitorMode    = GPSB_SM_MON_PROTOCOL;
    param.spWidth = data_size * 8;  // 8 : gpsb data write count

    mcu_printf("======================================================\n");
    mcu_printf("======== Protocol Error Test : BPW %d WIDTH %d=========\n" , ucBpw, param.spWidth);
    (void)GPSB_SafetyEnableMonitor(ucCh , param.spMonitorMode, (uint32)param.spWidth);

    tx[0] = (uint8)0x12UL;
    tx[1] = (uint8)0x34UL;
    tx[3] = (uint8)0x78UL;
    tx[2] = (uint8)0x56UL;

    /* Transfer data */
    ret = GPSB_Xfer((uint32)ucCh, tx, rx, data_size, xfer_mode);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ucCh, ret);

        return SAL_RET_FAILED;
    }

    (void)GPSB_SafetyDisableMonitor((uint32)param.spCh);
    //mcu_printf("mst_ch%d: sm status: %d\n", ucCh, GPSB_SafetyGetStatus(param.spCh));

    // Change width
    param.spWidth = 3;//data_size;
    mcu_printf("======================================================\n");
    mcu_printf("======== Protocol Error Test : BPW %d WIDTH %d=========\n" , ucBpw, param.spWidth);
    (void)GPSB_SafetyEnableMonitor(ucCh , param.spMonitorMode, (uint32)param.spWidth);

    /* Transfer data */
    ret = GPSB_Xfer((uint32)ucCh, tx, rx, data_size, xfer_mode);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ucCh, ret);

        return SAL_RET_FAILED;
    }

    (void)GPSB_SafetyDisableMonitor((uint32)param.spCh);
    //mcu_printf("mst_ch%d: sm status: %d\n", ucCh, GPSB_SafetyGetStatus(param.spCh));

    param.spWidth = data_size * 8;  // 8 gpsb data write count

    mcu_printf("======================================================\n");
    mcu_printf("======== Protocol Error Test : BPW %d WIDTH %d=========\n" , ucBpw, param.spWidth);
    (void)GPSB_SafetyEnableMonitor(ucCh , param.spMonitorMode, (uint32)param.spWidth);

    tx[0] = (uint8)0x12UL;
    tx[1] = (uint8)0x34UL;
    tx[3] = (uint8)0x78UL;
    tx[2] = (uint8)0x56UL;

    /* Transfer data */
    ret = GPSB_Xfer((uint32)ucCh, tx, rx, data_size, xfer_mode);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ucCh, ret);

        return SAL_RET_FAILED;
    }

    (void)GPSB_SafetyDisableMonitor((uint32)param.spCh);

    ret = GPSB_Close((uint32)ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d close fail. error code : %d \n", ucCh, ret);

        return SAL_RET_FAILED;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyTestStuckMonitor
*
* Function to test stuck monitor error.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*        - IO Monitor test
*          : Check Data stuck error
*           Stuck error is occured with invaild SDI.
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_SafetyTestStuckMonitor
(
    uint8                               ucMstCh
) {
    SALRetCode_t                        ret;
    GPSBSafetyParam_t                   param;
    uint32                              speed;
    uint32                              data_size;
    uint32                              xfer_mode;
    uint32                              cs;
    uint8                               bpw;
    uint8                               tx[8] = {1,2,3,4,5,6,7,8};
    uint8                               rx[8] = {0};
    GPSBOpenParam_t                     OpenParam;

    ret = SAL_RET_SUCCESS;

    /* Configure GPSB slave */
    cs          = GPIO_GPA(1UL);

    /* Configure GPSB master */
    speed     = 1UL*1000UL*1000UL;
    bpw       = (uint8)8UL;
    data_size = 1UL;

    OpenParam.uiSdo         = GPIO_GPA(2UL);
    OpenParam.uiSdi         = GPIO_GPA(3UL);
    OpenParam.uiSclk        = GPIO_GPA(0UL);
    OpenParam.pDmaAddrTx    = NULL_PTR;
    OpenParam.pDmaAddrRx    = NULL_PTR;
    OpenParam.uiDmaBufSize  = 0;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;
	OpenParam.uiIsSlave		= GPSB_MASTER_MODE;

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;

    if(ucMstCh >= GPSB_CH_NUM)
    {
        mcu_printf("%s: %d channel is wrong\n", __func__, ucMstCh);
        ret = SAL_RET_FAILED;
    }
    else
    {

        (void)GPSB_Open((uint32)ucMstCh, OpenParam);

        (void)GPSB_SetSpeed(ucMstCh, speed);

        (void)GPSB_SetBpw(ucMstCh, bpw);

        (void)GPSB_CsAlloc(ucMstCh, cs);

        /* Configure loopback mode */
        (void)GPSB_SafetyEnableLoopback(ucMstCh, ucMstCh);

        /* Configure IO monitor for master */
        param.spCh     = ucMstCh;
        param.spMonitorMode = GPSB_SM_MON_STUCK_BOTH;
        param.spWidth    = bpw;

        mcu_printf("======================================================\n");
        mcu_printf("============ TEST STUCK ERROR [Case : pass] ==========\n");
        mcu_printf("======================================================\n");

        (void)GPSB_SafetyEnableMonitor(param.spCh , param.spMonitorMode, (uint32)param.spWidth);

        tx[0] = (uint8)0x12UL;
        tx[1] = (uint8)0x23UL;
        tx[2] = (uint8)0x34UL;
        tx[3] = (uint8)0x56UL;

        /* Transfer data */
        (void)GPSB_Xfer(ucMstCh, tx, rx, data_size, xfer_mode);

        mcu_printf("mst_ch%d: sm status: %d\n", ucMstCh, GPSB_SafetyGetStatus(param.spCh));

        (void)GPSB_SafetyDisableMonitor(param.spCh);

        (void)GPSB_Close(ucMstCh);

        (void)GPSB_Open(ucMstCh, OpenParam);

        (void)GPSB_SetSpeed(ucMstCh, speed);

        (void)GPSB_SetBpw(ucMstCh, bpw);

        (void)GPSB_CsAlloc(ucMstCh, cs);

        /* Configure loopback mode */

        (void)GPSB_SafetyEnableLoopback((uint32)ucMstCh, (uint32)ucMstCh);

        param.spCh     = ucMstCh;
        param.spMonitorMode = (uint32)GPSB_SM_MON_STUCK_HIGH;
        param.spWidth    = bpw;

        mcu_printf("======================================================\n");
        mcu_printf("============ TEST STUCK ERROR [Case : high] ==========\n");
        mcu_printf("======================================================\n");

        (void)GPSB_SafetyEnableMonitor(param.spCh , param.spMonitorMode, (uint32)param.spWidth);

        tx[0] = (uint8)0xffUL;
        tx[1] = (uint8)0xffUL;
        tx[2] = (uint8)0xffUL;
        tx[3] = (uint8)0xffUL;

        /* Transfer data */
        ret = GPSB_Xfer((uint32)ucMstCh, tx, rx, data_size, xfer_mode);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ucMstCh, ret);

            return SAL_RET_FAILED;
        }

        mcu_printf("mst_ch%d: sm status: %d\n", ucMstCh, GPSB_SafetyGetStatus(param.spCh));

        (void)GPSB_SafetyDisableMonitor((uint32)param.spCh);

        (void)GPSB_Close((uint32)ucMstCh);

        /* Configure GPSB master */

        (void)GPSB_Open((uint32)ucMstCh, OpenParam);

        (void)GPSB_SetSpeed((uint32)ucMstCh, speed);

        (void)GPSB_SetBpw((uint32)ucMstCh, bpw);

        (void)GPSB_CsAlloc((uint32)ucMstCh, cs);

        /* Configure loopback mode */
        (void)GPSB_SafetyEnableLoopback((uint32)ucMstCh, (uint32)ucMstCh);

        param.spCh     = ucMstCh;
        param.spMonitorMode = (uint32)GPSB_SM_MON_STUCK_LOW;
        param.spWidth    = bpw;

        mcu_printf("======================================================\n");
        mcu_printf("============ TEST STUCK ERROR [Case : low] ==========\n");
        mcu_printf("======================================================\n");

        (void)GPSB_SafetyEnableMonitor(param.spCh , param.spMonitorMode, (uint32)param.spWidth);

        tx[0] = (uint8)0x00UL;
        tx[1] = (uint8)0x00UL;
        tx[2] = (uint8)0x00UL;
        tx[3] = (uint8)0x00UL;

        /* Transfer data */
        ret = GPSB_Xfer((uint32)ucMstCh, tx, rx, data_size, xfer_mode);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ucMstCh, ret);

            return SAL_RET_FAILED;
        }

        mcu_printf("mst_ch%d: sm status: %d\n", ucMstCh, GPSB_SafetyGetStatus(param.spCh));

        (void)GPSB_SafetyDisableMonitor((uint32)param.spCh);

        (void)GPSB_Close((uint32)ucMstCh);

    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyTestLoopback
*
* Function to test Safety loopback.
*
* @param    ucMstCh [in]    : Master Channel number for test
* @param    ucSlvCh [in]    : Slave Channel number for test
* @return
* Notes
*           - IO Monitor test
*           : Check Loopback data (TX FIFO Data <-> LB RX FIFO Data)
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_SafetyTestLoopback
(
    uint8                               ucMstCh,
    uint8                               ucSlvCh
) {
    SALRetCode_t                        ret;
    uint32                              speed;
    uint8                               bpw;
    uint32                              data_size;
    uint32                              xfer_mode;
    uint32                              i;
    uint8                               tx[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8                               m_rx[8] = {0};
    uint8                               s_rx[8] = {0};
    GPSBOpenParam_t                     OpenParam;

    ret = SAL_RET_SUCCESS;
    speed = 0UL;
    bpw = (uint8)0UL;
    data_size = 0UL;
    xfer_mode = 0UL;
    i = 0xffUL;

    if (ucMstCh != ucSlvCh)
    {
        /* Configure GPSB slave */
        bpw         = (uint8)8UL;
        data_size   = 4UL;
		OpenParam.uiSdo         = GPIO_GPA(2UL);
        OpenParam.uiSdi         = GPIO_GPA(3UL);
        OpenParam.uiSclk        = GPIO_GPA(0UL);
        OpenParam.pDmaAddrTx    = NULL_PTR;
        OpenParam.pDmaAddrRx    = NULL_PTR;
        OpenParam.uiDmaBufSize  = 0;
        OpenParam.fbCallback    = NULL_PTR;
        OpenParam.pArg          = NULL_PTR;
		OpenParam.uiIsSlave		= GPSB_SLAVE_MODE;

        ret = GPSB_Open((uint32)ucSlvCh, OpenParam);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("[GPSB-test] ch %d open fail. error code : %d \n", ucSlvCh, ret);
            ret = SAL_RET_FAILED;
        }

        SAL_WriteReg((uint32)0x704, (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL*(uint32)ucSlvCh)+0xCUL)); /* slave mode */
        SAL_WriteReg((uint32)0x70C, (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL*(uint32)ucSlvCh)+0xCUL)); /* enable slave */

        SAL_WriteReg((uint32)tx[0], (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL*(uint32)ucSlvCh))); /* write data */
        SAL_WriteReg((uint32)tx[1], (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL*(uint32)ucSlvCh))); /* write data */
        SAL_WriteReg((uint32)tx[2], (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL*(uint32)ucSlvCh))); /* write data */
        SAL_WriteReg((uint32)tx[3], (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL*(uint32)ucSlvCh))); /* write data */

    }

    /* Configure loopback mode */
    ret = GPSB_SafetyEnableLoopback((uint32)ucMstCh, (uint32)ucSlvCh);
    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] sm_enable_loopback fail. error code :%d \n", ret);
        ret = SAL_RET_FAILED;
    }

    /* Configure GPSB master */
    speed       = 2*1000*1000UL;
    bpw         = (uint8)8UL;
    data_size   = 4UL;

	OpenParam.uiSdo         = GPIO_GPB(6UL);
    OpenParam.uiSdi         = GPIO_GPB(7UL);
    OpenParam.uiSclk        = GPIO_GPB(4UL);
    OpenParam.pDmaAddrTx    = NULL_PTR;
    OpenParam.pDmaAddrRx    = NULL_PTR;
    OpenParam.uiDmaBufSize  = 0;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;
	OpenParam.uiIsSlave		= GPSB_MASTER_MODE;

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;
    (void)GPSB_Open((uint32)ucMstCh, OpenParam);

    (void)GPSB_SetSpeed((uint32)ucMstCh, speed);

    (void)GPSB_SetBpw((uint32)ucMstCh, bpw);

    ret = GPSB_Xfer((uint32)ucMstCh, tx, m_rx, data_size,xfer_mode);
    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ucMstCh, ret);
        ret = SAL_RET_FAILED;
    }

    for (i = 0; i < data_size; i++)
    {
        s_rx[i] = (uint8)SAL_ReadReg((uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL*(uint32)ucSlvCh))); /* read data */
    }

    /* Disable slave */
    SAL_WriteReg((uint32)0x0UL , (uint32)((uint32)MCU_BSP_GPSB_BASE+(uint32)((uint32)0x10000UL*(uint32)ucSlvCh)+(uint32)0xCUL));

    /* Disable loopback mode */
    (void)GPSB_SafetyDisableLoopback(ucMstCh, ucSlvCh);

    ret = GPSB_Close(ucMstCh);
    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d close fail. error code : %d \n", ucSlvCh, ret);
        ret = SAL_RET_FAILED;
    }
    ret = GPSB_Close(ucSlvCh);
    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d close fail. error code : %d \n", ucSlvCh, ret);
        ret = SAL_RET_FAILED;
    }

    /* Compare the data */
    for (i = 0; i < data_size; i++)
    {
        if (tx[i] != s_rx[i])
        {
            mcu_printf("[%d]m_tx:s_rx --> %02x:%02x\n", i, tx[i], s_rx[i]);
            ret = SAL_RET_FAILED;
        }
    }

    if(ret == SAL_RET_FAILED)
    {
        mcu_printf("===== loopback test: ch%d --> ch%d fail\n", ucMstCh, ucSlvCh);
    }
    else
    {
        mcu_printf("===== loopback test: ch%d --> ch%d clear\n", ucMstCh, ucSlvCh);
    }

    delay1us(1000);
    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyTestWriteLock
*
* Function to test write lock
*
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_SafetyTestWriteLock
(
    void
) {
    SALRetCode_t                        ret;
    uint32                              test_value;
    uint32                              lb_cfg;
    uint32                              val;

    ret = SAL_RET_SUCCESS;
    test_value = 0x1c2c3c4cUL;
    lb_cfg = 0;
    val = 0;

    mcu_printf("LB_CFG(Unlocked) try to set %x. \n",test_value);
    SAL_WriteReg(test_value, (uint32)(GPSB_PCFG_BASE + GPSB_LB_CFG));

    mcu_printf("===================================================\n");

    lb_cfg = SAL_ReadReg((uint32)(GPSB_PCFG_BASE + GPSB_LB_CFG));
    mcu_printf("Read & Store LB_CFG 0x%x\n", lb_cfg);

    ret = GPSB_SafetyEnableLock();

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-SM] enable_lock failed. \n");

        return SAL_RET_FAILED;
    }

    mcu_printf("===================================================\n");
    mcu_printf("LB_CFG (Locked) try to set 0xfffffff. \n");

    SAL_WriteReg((uint32)0xffffffffUL, (uint32)(GPSB_PCFG_BASE + GPSB_LB_CFG));
    val = SAL_ReadReg((uint32)(GPSB_PCFG_BASE + GPSB_LB_CFG));

    if(val != lb_cfg)
    {
        mcu_printf("LB_CFG Test Failed. read value : %x , expected value : %x\n", val, lb_cfg);
    }
    else
    {
        mcu_printf("LB_CFG read : 0x%x , expected : 0x%x\n", ret , lb_cfg);
        mcu_printf("Test of register lock (LB_CFG) success.\n");
    }

    mcu_printf("===================================================\n");

    ret = GPSB_SafetyDisableLock();

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-SM] disable_lock failed. \n");

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}
#if 0
/*
***************************************************************************************************
*                                          GPSB_TestSdm
*
* Function to test smd with internal path.
*
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_TestSdm
(
 void
 ) {
    SALRetCode_t                        ret;
    uint32                              speed;
    uint8                               bpw;
    uint32                              data_size;
    uint32                              xfer_mode;
    uint32                              i;
    uint32                              sclk;
    uint32                              cs;
    uint32                              sdo;
    uint32                              sdi;
    uint32                              data;
    uint32                              checksum;
    uint32                              tx[4] = {0};
    uint32                              m_rx[4] = {0};
    uint32                              s_rx[8] = {0};
    uint32                              addr;
    uint8                               mst_ch;
    uint8                               slv_ch;

    ret = SAL_RET_SUCCESS;
    i = 0UL;
    data = 0UL;
    checksum = 0UL;
    addr = 0UL;
    mst_ch = (uint8)3UL;
    slv_ch = (uint8)0UL;

    mcu_printf("Internal MICOM SDM Access test, Master : GPSB-%d", mst_ch);

    /* Configure GPSB master */
    speed       = 20000000UL;
    bpw         = (uint8)32UL;
    data_size   = 12UL;

    sclk        = GPIO_GPG(7UL);
    cs          = GPIO_GPG(8UL);
    sdo         = GPIO_GPG(9UL);
    sdi         = GPIO_GPG(10UL);

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;

    (void)GPSB_Open((uint32)mst_ch, sdo, sdi, sclk, NULL, NULL, 0, NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);

    (void)GPSB_SetSpeed((uint32)mst_ch, speed);

    (void)GPSB_SetBpw((uint32)mst_ch, bpw);

    (void)GPSB_CsAlloc((uint32)mst_ch, cs);

    /* Configure GPSB slave */

    (void)GPSB_Open((uint32)slv_ch, NULL, NULL, NULL, NULL, NULL, 0, NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);

    (void)GPSB_SetBpw((uint32)slv_ch, bpw);

    SAL_WriteReg((uint32)0x704UL, (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL * (uint32)slv_ch) + (uint32)0xCUL)); /* slave mode */
    SAL_WriteReg((uint32)0x70CUL, (uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL * (uint32)slv_ch) + (uint32)0xCUL)); /* enable slave */

    SAL_WriteReg((uint32)tx[0], (uint32)((uint32)MCU_BSP_GPSB_BASE+(0x10000UL * (uint32)slv_ch))); /* write data */
    SAL_WriteReg((uint32)tx[1], (uint32)((uint32)MCU_BSP_GPSB_BASE+(0x10000UL * (uint32)slv_ch))); /* write data */
    SAL_WriteReg((uint32)tx[2], (uint32)((uint32)MCU_BSP_GPSB_BASE+(0x10000UL * (uint32)slv_ch))); /* write data */
    SAL_WriteReg((uint32)tx[3], (uint32)((uint32)MCU_BSP_GPSB_BASE+(0x10000UL * (uint32)slv_ch))); /* write data */

    /* Configure loopback mode */
    (void)GPSB_SafetyEnableLoopback((uint32)mst_ch, (uint32)slv_ch);

    // Write
    data = 0x1ACCE551UL;
    checksum = ((data&0xff000000UL) >> 24UL) + ((data&0xff0000UL) >> 16UL) + ((data&0xff00UL) >> 8UL) + (data&0xffUL);
    checksum = checksum & 0xffffUL;
    tx[0] = (((uint32)2UL & (uint32)0xffUL) << (uint32)24UL) | (addr & (uint32)0xffffffUL);
    tx[1] = ((uint32)0x03000000UL | (((uint32)data&(uint32)0xffffff00UL) >> (uint32)8UL));
    tx[2] = (((uint32)data&0xffUL) << (uint32)24UL)|((uint32)checksum << 8UL);

    ret = GPSB_Xfer((uint32)mst_ch, tx, m_rx, data_size, xfer_mode);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", mst_ch, ret);
        ret = SAL_RET_FAILED;
    }

    for (i = 0; i < (data_size/(uint32)4UL); i++)
    {
        s_rx[i] = SAL_ReadReg((uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL * (uint32)slv_ch))); /* read data */
    }

    if(((tx[0UL] == s_rx[0UL]) && (tx[1UL] == s_rx[1UL]) && (tx[2UL] == s_rx[2UL])))
    {
        mcu_printf("GPSB transfer to SDM is correct \n");
    }
    else
    {
        GPSB_D ("GPSB transfer to SDM is in-correct \n");
        GPSB_D ("TX0 : 0x%x / RX0 : 0x%x \n", tx[0], s_rx[0]);
        GPSB_D ("TX1 : 0x%x / RX1 : 0x%x \n", tx[1], s_rx[1]);
        GPSB_D ("TX2 : 0x%x / RX2 : 0x%x \n", tx[2], s_rx[2]);
    }

    // Read
    addr   = 0x820000UL;
    data   = 0x1ACCE551UL;
    checksum = 0xffffUL;

    tx[0] = ((11UL & 0xffUL) << 24UL) | (addr&0xffffffUL);
    tx[1] = 0x03000000UL | ((data&0xffffff00UL) >> 8UL);
    tx[2] = ((data&0xffUL) << 24UL)|(checksum << 8UL);

    ret = GPSB_Xfer((uint32)mst_ch, tx, m_rx, data_size, xfer_mode);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", mst_ch, ret);
        return SAL_RET_FAILED;
    }

    m_rx[0] = ((m_rx[1]&0xffffffUL) << 8UL) | ((m_rx[2]&0xff000000UL)>>24UL);
    checksum = ((m_rx[0]&0xff000000UL) >> 24UL) + ((m_rx[0]&0xff0000UL) >> 16UL) + ((m_rx[0]&0xff00UL) >> 8UL) + (m_rx[0]&0xffUL);
    checksum = checksum & 0xffffUL;

    if(checksum == ((m_rx[2]&0xffff00UL)>>8UL))
    {
        GPSB_D ("read data check : %x \n", m_rx[0]);
    }

    for (i = 0; i < (data_size/(uint32)4UL); i++)
    {
        s_rx[i] = SAL_ReadReg((uint32)((uint32)MCU_BSP_GPSB_BASE+((uint32)0x10000UL * (uint32)slv_ch))); /* read data */
    }

    if((((tx[0] == s_rx[0]) && (tx[1] == s_rx[1])) && (tx[2] == s_rx[2])))
    {
        mcu_printf("GPSB transfer to SDM is correct \n");
    }
    else
    {
        GPSB_D ("GPSB transfer to SDM is in-correct \n");
        GPSB_D ("TX0 : 0x%x / RX0 : 0x%x \n", tx[0], s_rx[0]);
        GPSB_D ("TX1 : 0x%x / RX1 : 0x%x \n", tx[1], s_rx[1]);
        GPSB_D ("TX2 : 0x%x / RX2 : 0x%x \n", tx[2], s_rx[2]);
    }

    /* Disable slave */
    SAL_WriteReg(0x0 , (uint32)((uint32)MCU_BSP_GPSB_BASE+(uint32)((uint32)0x10000UL * (uint32) slv_ch) + (uint32)0xCUL));
    /* Disable loopback mode */
    (void) GPSB_SafetyDisableLoopback((uint32)mst_ch, (uint32)slv_ch);

    ret = GPSB_Close((uint32)mst_ch);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d close fail. error code : %d \n", slv_ch, ret);
        return SAL_RET_FAILED;
    }

    ret = GPSB_Close((uint32)slv_ch);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[GPSB-test] ch %d close fail. error code : %d \n", slv_ch, ret);
        return SAL_RET_FAILED;
    }

    delay1us(1000UL);

    return ret;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_UnitTest1
*
* Function to test GPSB port/control/xfer.
*
* @return
* Notes
*           : this function is not supported.
***************************************************************************************************
*/

static SALRetCode_t GPSB_UnitTest1
(
    void
) {
    // Unit_test1 can check belows.
    // - GPSB controller is vaild. (channel 0~5)
    // - Buffer
    SALRetCode_t                        ret;
    uint32                              speed;
    uint8                               bpw;
    uint32                              data_size;
    uint32                              xfer_mode;
    uint32                              sclk;
    uint32                              cs;
    uint32                              sdo;
    uint32                              sdi;
    uint8                               tx[8] = {0};
    uint8                               rx[8] = {0};
    uint8                               ch;
    uint8                               port_num;

    uint32 test_port[GPSB_PORT_NUM][4] = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {GPIO_GPA(0UL)  , GPIO_GPA(1UL)  , GPIO_GPA(2UL)  , GPIO_GPA(3UL)},
        {GPIO_GPA(6UL)  , GPIO_GPA(7UL)  , GPIO_GPA(8UL)  , GPIO_GPA(9UL)},
        {GPIO_GPA(12UL) , GPIO_GPA(13UL) , GPIO_GPA(14UL) , GPIO_GPA(15UL)},
        {GPIO_GPA(18UL) , GPIO_GPA(19UL) , GPIO_GPA(20UL) , GPIO_GPA(21UL)},
        {GPIO_GPA(24UL) , GPIO_GPA(25UL) , GPIO_GPA(26UL) , GPIO_GPA(27UL)},
        {GPIO_GPB(0UL)  , GPIO_GPB(1UL)  , GPIO_GPB(2UL)  , GPIO_GPB(3UL)},
        {GPIO_GPB(6UL)  , GPIO_GPB(7UL)  , GPIO_GPB(8UL)  , GPIO_GPB(9UL)},
        {GPIO_GPB(12UL) , GPIO_GPB(13UL) , GPIO_GPB(14UL) , GPIO_GPB(15UL)},
        {GPIO_GPB(19UL) , GPIO_GPB(20UL) , GPIO_GPB(21UL) , GPIO_GPB(22UL)},
        {GPIO_GPB(25UL) , GPIO_GPB(26UL) , GPIO_GPB(27UL) , GPIO_GPB(28UL)},
        {GPIO_GPC(0UL)  , GPIO_GPC(1UL)  , GPIO_GPC(2UL)  , GPIO_GPC(3UL)},
        {GPIO_GPC(4UL)  , GPIO_GPC(5UL)  , GPIO_GPC(6UL)  , GPIO_GPC(7UL)},
        {GPIO_GPC(12UL) , GPIO_GPC(13UL) , GPIO_GPC(14UL) , GPIO_GPC(15UL)},
        {GPIO_GPC(16UL) , GPIO_GPC(17UL) , GPIO_GPC(18UL) , GPIO_GPC(19UL)},
        {GPIO_GPC(20UL) , GPIO_GPC(21UL) , GPIO_GPC(22UL) , GPIO_GPC(23UL)},
        {GPIO_GPC(26UL) , GPIO_GPC(27UL) , GPIO_GPC(28UL) , GPIO_GPC(29UL)},
        {GPIO_GPG(0UL)  , GPIO_GPG(1UL)  , GPIO_GPG(2UL)  , GPIO_GPG(3UL)},
        {GPIO_GPG(7UL)  , GPIO_GPG(8UL)  , GPIO_GPG(9UL)  , GPIO_GPG(10UL)},
        {GPIO_GPE(5UL)  , GPIO_GPE(6UL)  , GPIO_GPE(7UL)  , GPIO_GPE(8UL)},
        {GPIO_GPE(11UL) , GPIO_GPE(12UL) , GPIO_GPE(13UL) , GPIO_GPE(14UL)},
        {GPIO_GPE(16UL) , GPIO_GPE(17UL) , GPIO_GPE(18UL) , GPIO_GPE(19UL)},
        {GPIO_GPH(4UL)  , GPIO_GPH(5UL)  , GPIO_GPH(6UL)  , GPIO_GPH(7UL)},
        {GPIO_GPMA(0UL) , GPIO_GPMA(1UL) , GPIO_GPMA(2UL) , GPIO_GPMA(3UL)},
        {GPIO_GPMA(6UL) , GPIO_GPMA(7UL) , GPIO_GPMA(8UL) , GPIO_GPMA(9UL)},
        {GPIO_GPMA(12UL), GPIO_GPMA(13UL), GPIO_GPMA(14UL), GPIO_GPMA(15UL)},
        {GPIO_GPMA(18UL), GPIO_GPMA(19UL), GPIO_GPMA(20UL), GPIO_GPMA(21UL)},
        {GPIO_GPMA(24UL), GPIO_GPMA(25UL), GPIO_GPMA(26UL), GPIO_GPMA(27UL)},
        {GPIO_GPH(0UL)  , GPIO_GPH(1UL)  , GPIO_GPH(2UL)  , GPIO_GPH(3UL)},

        {GPIO_GPMB(0UL) , GPIO_GPMB(1UL) , GPIO_GPMB(2UL) , GPIO_GPMB(3UL)},
        {GPIO_GPMB(6UL) , GPIO_GPMB(7UL) , GPIO_GPMB(8UL) , GPIO_GPMB(9UL)},
        {GPIO_GPMB(12UL) , GPIO_GPMB(13UL) , GPIO_GPMB(14UL) , GPIO_GPMB(15UL)},
        {GPIO_GPMB(18UL) , GPIO_GPMB(19UL) , GPIO_GPMB(20UL) , GPIO_GPMB(21UL)},
        {GPIO_GPMB(24UL) , GPIO_GPMB(25UL) , GPIO_GPMB(26UL) , GPIO_GPMB(27UL)},

        {GPIO_GPMC(0UL) ,  GPIO_GPMC(1UL) ,  GPIO_GPMC(2UL) ,  GPIO_GPMC(3UL)},
        {GPIO_GPMC(6UL) ,  GPIO_GPMC(7UL) ,  GPIO_GPMC(8UL) ,  GPIO_GPMC(9UL)},
        {GPIO_GPMC(12UL) , GPIO_GPMC(13UL) , GPIO_GPMC(14UL) , GPIO_GPMC(15UL)},
        {GPIO_GPMC(18UL) , GPIO_GPMC(19UL) , GPIO_GPMC(20UL) , GPIO_GPMC(21UL)},

        {GPIO_GPMD(0UL) , GPIO_GPMD(1UL) , GPIO_GPMD(2UL) , GPIO_GPMD(3UL)},
        {GPIO_GPMD(12UL) , GPIO_GPMD(13UL) , GPIO_GPMD(14UL) , GPIO_GPMD(15UL)}
    };

    ret = SAL_RET_SUCCESS;

    xfer_mode = 0UL;
    sclk = 0UL;
    cs = 0UL;
    sdo = 0UL;
    sdi = 0UL;
    port_num = (uint8)0UL;

    /* Configure GPSB master */
    speed     = 50UL*1000UL*1000UL;
    bpw       = (uint8)8UL;
    data_size = 4UL;

    // TODO : Check vaild GPSB PIN set in tcc805x.
    mcu_printf("Start Unit test : DMA Interrupt check.\n");

    for(ch = 0 ; ch < GPSB_CH_NUM ; ch++)
    {
        for(port_num = 0 ; port_num < GPSB_PORT_NUM ; port_num++)
        {
            sclk      = test_port[port_num][0];
            cs        = test_port[port_num][1];
            sdo       = test_port[port_num][2];
            sdi       = test_port[port_num][3];

            OpenParam.uiSdo         = test_port[port_num][2];
            OpenParam.uiSdi         = test_port[port_num][3];
            OpenParam.uiSclk        = test_port[port_num][0];
            OpenParam.pDmaAddrTx    = NULL_PTR;
            OpenParam.pDmaAddrRx    = NULL_PTR;
            OpenParam.uiDmaBufSize  = 0;
            OpenParam.fbCallback    = NULL_PTR;
            OpenParam.pArg          = NULL_PTR;
			OpenParam.uiIsSlave		= GPSB_MASTER_MODE;

            xfer_mode = GPSB_XFER_MODE_DMA | GPSB_XFER_MODE_WITH_INTERRUPT;

            g_tx_dma = (uint32 *)MPU_GetDMABaseAddress();
            g_rx_dma = (uint32 *)(g_tx_dma + 1024UL);
            mcu_printf("g_tx_dma: 0x%08X\n", g_tx_dma);
            mcu_printf("g_rx_dma: 0x%08X\n", g_rx_dma);

            (void)GPSB_Open(ch, OpenParam);

            (void)GPSB_SetSpeed((uint32)ch, (uint32)speed);

            (void)GPSB_SetBpw((uint32)ch,(uint32)bpw);

            (void)GPSB_CsAlloc((uint32)ch, (uint32)cs);

            ret = GPSB_Xfer((uint32)ch, tx, rx, data_size,xfer_mode);

            if(ret != SAL_RET_SUCCESS)
            {
                mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ch, ret);
                return SAL_RET_FAILED;
            }

            ret = GPSB_Close((uint32)ch);

            if(ret != SAL_RET_SUCCESS)
            {
                mcu_printf("[GPSB-test] ch %d close fail. error code : %d \n", ch, ret);
                return SAL_RET_FAILED;
            }
        }
    }

    mcu_printf("Start Unit test : Interrupt check.\n");

    for(ch = (uint8)0UL; ch < GPSB_CH_NUM ; ch++)
    {
        xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITH_INTERRUPT;
        ret = GPSB_Open(ch, OpenParam);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("[GPSB-test] ch %d open fail. error code : %d \n", ret);
            return SAL_RET_FAILED;
        }

        (void)GPSB_SetSpeed((uint32)ch, (uint32)speed);

        (void)GPSB_SetBpw((uint32)ch, (uint32)bpw);

        (void)GPSB_CsAlloc((uint32)ch, (uint32)cs);

        ret = GPSB_Xfer((uint32)ch, tx, rx, data_size,xfer_mode);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("[GPSB-test] ch %d xfer fail. error code : %d \n", ch, ret);
            return SAL_RET_FAILED;
        }

        ret = GPSB_Close((uint32)ch);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("[GPSB-test] ch %d close fail. error code : %d \n", ch, ret);
            return SAL_RET_FAILED;
        }
    }
    return SAL_RET_SUCCESS;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_UnitTest2
*
* Function to check register value according control sequence.
*
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_UnitTest2
(
    void
) {
    SALRetCode_t                         ret;
    uint32                               reg;
    uint8                                tx[8] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,};
    uint8                                ch;
    uint8                                i;
    uint32                               base_addr;

    ret = SAL_RET_SUCCESS;
    reg = 0UL;
    ch = (uint8)0UL;
    i = (uint8)0xffUL;
    base_addr = (uint32)MCU_BSP_GPSB_BASE + (uint32)((uint32)ch * (uint32)0x10000)           ;

    // FIFO Test : 16-bit width X 16-depth
    ret = GPSB_Reset((uint32)ch);

    if(ret != SAL_RET_SUCCESS)
    {
        return ret;
    }

    reg = SAL_ReadReg((uint32)(base_addr + GPSB_STAT));

    if((reg & (uint32)BSP_BIT_03) !=      (uint32) BSP_BIT_03) // Check WE init value : 0x1
    {
        mcu_printf("[%s : %d] Invaild WE %x (expected : 0x1)\n", __func__ , __LINE__ , reg & (uint32)BSP_BIT_03);
        return SAL_RET_FAILED;
    }

    if(((reg >> 16UL)& 0x1FUL) != 0UL) // Check RBVCNT init value : 0x0
    {
        mcu_printf("[%s : %d] Invaild RBVCNT (expected : 0x0)\n", __func__ , __LINE__);
        return SAL_RET_FAILED;
    }

    if((((reg >> 24UL)& 0x1FUL) << 24UL) != 0UL) // Check WBVCNT init value : 0x0
    {
        mcu_printf("[%s : %d] Invaild WBVCNT  (expected : 0x0)\n", __func__ , __LINE__);
        return SAL_RET_FAILED;
    }

    SAL_WriteReg((uint32)((uint32)0xfUL << (uint32)8UL) , (uint32)( base_addr + GPSB_MODE)); // Set BWS -> FIFO 16 X 16 Bits

    for(i = 0 ; i < 16UL ; i++)
    {
        SAL_WriteReg(tx[i%(uint8)7UL] , (uint32)(base_addr + GPSB_PORT));  // Repeat 16 times data write to PORT Register.
    }

    reg = SAL_ReadReg((uint32)(base_addr + GPSB_STAT));

    if((((reg >> 24UL) & 0x1FUL)) != 0x10UL)
    {
        mcu_printf("[%s : %d] Invaild WBVCNT value : %x (expected : 0x10) (reg:%x)\n" ,
                __func__, __LINE__, reg & (0x1FUL << 24UL), reg); // Check WBVCNT value

        return SAL_RET_FAILED;
    }

    SAL_WriteReg((uint32)((uint32)BSP_BIT_06 & (uint32)(~BSP_BIT_02)), (uint32)(base_addr + GPSB_MODE)); // Set LB , Clear SLV

    SAL_WriteReg(0, (uint32)(base_addr + GPSB_INTEN));

    SAL_WriteReg((uint32)BSP_BIT_03 | SAL_ReadReg((uint32)(base_addr + GPSB_MODE)) , (uint32)(base_addr + GPSB_MODE));

    reg = SAL_ReadReg((uint32)(base_addr + GPSB_STAT));

    if(((reg >> 24UL) & 0x1FUL) != 0x00UL)
    {
        mcu_printf("[%s : %d] Invaild WBVCNT value %x (reg:%x)(expected : 0x00)\n",
                __func__, __LINE__ , (reg>>24)&0x1f , reg); // Check WBVCNT value

        return SAL_RET_FAILED;
    }

    if((reg & (uint32)BSP_BIT_03) != (uint32)BSP_BIT_03) // Check WE 0x1
    {
        mcu_printf("[%s : %d] Invaild WE %x (expected : 0x1)\n", __func__, __LINE__, reg & (uint32)BSP_BIT_03);

        return SAL_RET_FAILED;
    }

    if((reg & (uint32)BSP_BIT_04) != (uint32)BSP_BIT_04) // Check RF 0x1
    {
        mcu_printf("[%s : %d] Invaild RF %x (expected : 0x1)\n", __func__ , __LINE__, reg & (uint32)BSP_BIT_04);

        return SAL_RET_FAILED;
    }

    if(((reg >> 24UL)& 0x1FUL) != 0x0UL) // Check WBVCNT 0x0
    {
        mcu_printf("[%s : %d] Invaild WBVCNT (expected : 0x0)\n", __func__, __LINE__);

        return SAL_RET_FAILED;
    }

    if(((reg >> 16UL) & 0x1FUL) != 0x10UL) // Check RBVCNT 0x10
    {
        mcu_printf("[%s : %d] Invaild RBVCNT (expected : 0x10)\n", __func__, __LINE__);

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_UnitTest3
*
* Function to check register value according control sequence.
*
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_UnitTest3
(
    void
) {
    SALRetCode_t                        ret;
    uint32                              reg;
    uint8                               tx[8] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,};
    uint8                               ch;
    uint8                               i;
    uint32                              base_addr;

    ret = SAL_RET_SUCCESS;
    reg = 0UL;
    ch = (uint8)0UL;
    i = (uint8)0xffUL;
    base_addr = (uint32)MCU_BSP_GPSB_BASE + (uint32)((uint32)ch * (uint32)0x10000UL);

    // FIFO Test : 32-bit width X 8-depth
    ret = GPSB_Reset((uint32)ch);

    if(ret != SAL_RET_SUCCESS)
    {
        return ret;
    }

    reg = SAL_ReadReg((uint32)(base_addr + GPSB_STAT));

    if(((reg & (uint32)BSP_BIT_03)) != (uint32)(BSP_BIT_03)) // Check WE init value : 0x1
    {
        mcu_printf("[%s : %d] Invaild WE     (expected : 0x1)\n",     __func__ , __LINE__);
        return SAL_RET_FAILED;
    }

    if(((reg >> (uint32)16UL) & (uint32)0x1F) != 0UL) // Check RBVCNT init value : 0x0
    {
        mcu_printf("[%s : %d] Invaild RBVCNT      (expected : 0x1)\n",    __func__ , __LINE__);
        return SAL_RET_FAILED;
    }

    if(((reg >> (uint32)24UL) & (uint32)0x1FUL) != 0UL) // Check WBVCNT init value : 0x0
    {
        mcu_printf("[%s : %d] Invaild WBVCNT      (expected : 0x1)\n",    __func__ , __LINE__);
        return SAL_RET_FAILED;
    }

    SAL_WriteReg((uint32)((uint32) 0x1fUL << (uint32)8UL) , (uint32)(base_addr + GPSB_MODE)); // Set BWS -> FIFO 8 X 32 Bits

    for(i = 0 ; i < 8UL ; i++)
    {
        SAL_WriteReg((uint32)tx[i] , (uint32)(base_addr + GPSB_PORT));  // Repeat 16 times data write to PORT Register.
    }

    reg = SAL_ReadReg((uint32)(base_addr + GPSB_STAT));

    if(((reg >> (uint32)24UL) & (uint32)0x1FUL) != 0x08UL)
    {
        mcu_printf("[%s : %d] Invaild WBVCNT value       (expected : 0x08)\n" , __func__, __LINE__); // Check WBVCNT value

        return SAL_RET_FAILED;
    }

    SAL_WriteReg((uint32)((uint32)BSP_BIT_06 & (uint32)(~BSP_BIT_02)), (uint32)(base_addr + GPSB_MODE)); // Set LB , Clear SLV

    SAL_WriteReg((uint32)0UL, (uint32)(base_addr + GPSB_INTEN)); // select Interrupt type , 0 : all disable

    SAL_WriteReg((uint32)((uint32)BSP_BIT_03 | SAL_ReadReg((uint32)(base_addr + GPSB_MODE))) , (uint32)(base_addr + GPSB_MODE)); // Enable interrupt

    reg = SAL_ReadReg((uint32)(base_addr + GPSB_STAT));

    if(((reg >> (uint32)24UL) & (uint32)0x1FUL) !=(uint32)0x00UL)
    {
        mcu_printf("[%s : %d] Invaild WBVCNT value %x (reg:%x) (expected : 0x00)\n",
                 __func__, __LINE__, ((reg >> 24) & 0x1f), reg); // Check WBVCNT value

        return SAL_RET_FAILED;
    }

    if((reg & (uint32)BSP_BIT_03) != (uint32)BSP_BIT_03) // Check WE 0x1
    {
        mcu_printf("[%s : %d] Invaild WE (expected : 0x1)\n",         __func__, __LINE__);

        return SAL_RET_FAILED;
    }

    if((reg & (uint32)BSP_BIT_04) != (uint32)BSP_BIT_04) // Check RF 0x1
    {
        mcu_printf("[%s : %d] Invaild RF      (expected : 0x1)\n",   __func__ , __LINE__);

        return SAL_RET_FAILED;
    }

    if(((reg >> (uint32)24UL) & (uint32)0x1FUL) != 0x0UL) // Check WBVCNT 0x0
    {
        mcu_printf("[%s : %d] Invaild WBVCNT  (expected : 0x0)\n",       __func__, __LINE__);

        return SAL_RET_FAILED;
    }

    if(((reg >> (uint32)16UL) & (uint32)0x1FUL) != 0x08UL) // Check RBVCNT 0x08
    {
        mcu_printf("[%s : %d] Invaild RBVCNT      (expected : 0x10)\n",    __func__, __LINE__);

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_TestIpVerification
*
* Function to call IP test functions and check result.
*
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_TestIpVerification
(
    void
) {
    SALRetCode_t                               ret;

    ret = SAL_RET_SUCCESS;

    gpsb[0].dChannel = 0;

#if 0 // Not support
    ret = tcc_GPSB_unit_test_1();

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("GPSB Unit Test 1 was failed.\n");
        return SAL_RET_FAILED;
    }
    else
    {
        mcu_printf("GPSB Unit Test 1 success.\n");
    }
#endif

    ret = GPSB_UnitTest2(); //FIFO Test 16*16

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("GPSB Unit Test 2 was failed.\n");
        return SAL_RET_FAILED;
    }
    else
    {
        mcu_printf("GPSB Unit Test 2 success.\n");
    }

    ret = GPSB_UnitTest3(); //FIFO Test 8*32

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("GPSB Unit Test 3 was failed.\n");
        return SAL_RET_FAILED;
    }
    else
    {
        mcu_printf("GPSB Unit Test 3 success.\n");
    }

    return SAL_RET_SUCCESS;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_WriteSdm
*
* Function to transfer TX data to sdm.
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiAddr [in]     : SDM reistger address
* @param    pucValue [in]   : Write data
* @param    uiSize [in]     : Write data size
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_WriteSdm
(
    uint8                               ucCh,
    uint32                              uiAddr,
    const uint8 *                       pucValue,
    uint32                              uiSize
) {
    uint32                              uiAddr_temp;
    uint32                              uiSize_temp;
    uint8                               opcode[SDM_GPSB_MAX_WRITE_SIZE+16UL];
    uint8                               dummy[SDM_GPSB_MAX_WRITE_SIZE+16UL];
    SALRetCode_t                        iRet;
    uint16                              checksum;
    uint32                              index;
    uint32                              send_data_size;
    uint32                              i;
    uint32                              pos;

    uiAddr_temp = uiAddr;
    uiSize_temp = uiSize;
    iRet = SAL_RET_SUCCESS;
    checksum = 0;
    index = 0;
    send_data_size = 4;
    i = 0;
    pos = 0;

    while(uiSize_temp > (uint32)0UL)
    {
        SAL_MemSet(opcode, 0, (uint32)SDM_GPSB_MAX_WRITE_SIZE+(uint32)6UL);

        /* OP code */
        opcode[index] = 0x02;
        index++;
        opcode[index] = (uint8)(uiAddr_temp >> (uint32)16UL); /*MSB */
        index++;
        opcode[index] = (uint8)(uiAddr_temp >> (uint32)8UL);
        index++;
        opcode[index] = (uint8)(uiAddr_temp & (uint32)0xffUL);
        index++;

        if(uiSize_temp > SDM_GPSB_MAX_WRITE_SIZE)
        {
            send_data_size = (uint8)(SDM_GPSB_MAX_WRITE_SIZE);
        }
        else
        {
            send_data_size = (uint8)(uiSize_temp);
        }

        send_data_size = ((send_data_size + 3UL)/4UL)*4UL;

        opcode[index] = (uint8)(send_data_size - 1UL);
        index++;

        /* write data */
        SAL_MemCopy(&opcode[index], &pucValue[pos], send_data_size);

        for(i = index; i < (send_data_size + index) ; i++)
        {
            checksum += opcode[i];
        }

        index += send_data_size;
        opcode[index] = (uint8)(checksum >> 8UL); /*MSB */
        index++;
        opcode[index] = (uint8)(checksum & 0xffUL);
        index++;
        index = ((index +3UL)/4UL)*4UL;

        uiSize_temp -= send_data_size;

        iRet = GPSB_Xfer((uint32)ucCh, opcode, dummy, index, GPSB_XFER_MODE_WITHOUT_INTERRUPT);

        if(iRet != SAL_RET_SUCCESS)
        {
            GPSB_D ("[%s : %d] GPSB Xfer fail\n");
            iRet = SAL_RET_FAILED;
        }

        //iRet = GPSB_Xfer(SDM_GPSB_CH, opcode, dummy, index, GPSB_XFER_MODE_WITHOUT_INTERRUPT);

        uiAddr_temp += send_data_size;
        pos += send_data_size;
        index = 0;
        checksum = 0;
    }

    return iRet;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_ReadSdm
*
* Function to receive RX data from sdm.
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiAddr [in]     : SDM reistger address
* @param    pucValue [in]   : Read data
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_ReadSdm
(
    uint8                               ucCh,
    uint32                              uiAddr,
    uint32 *                            puiValue
) {
    uint8                               opcode[SDM_GPSB_MAX_WRITE_SIZE+3UL] = {0};
    uint8                               dummy[SDM_GPSB_MAX_WRITE_SIZE+3UL] = {0};
    SALRetCode_t                        lRet;
    uint16                              checksum;
    uint16                              readed_cs;
    uint32                              index;

    lRet = SAL_RET_SUCCESS;
    checksum = 0;
    readed_cs = 0;
    index = 0;

    SAL_MemSet(opcode, 0, SDM_GPSB_MAX_WRITE_SIZE+3UL);
    /* OP code */
    opcode[index] = 0x0B;
    index++;
    opcode[index] = (uint8)(uiAddr >> 16UL);
    index++;
    opcode[index] = (uint8)(uiAddr >> 8UL);
    index++;
    opcode[index] = (uint8)(uiAddr & 0xffUL);
    index++;
    opcode[index] = (uint8)(3UL);
    index++;
    /* write data */

    // Transfer data to sdm
    lRet = GPSB_Xfer(ucCh, opcode, dummy, 12UL, GPSB_XFER_MODE_WITHOUT_INTERRUPT);

    checksum = ((uint16)dummy[index] + (uint16)dummy[index + 1UL] + (uint16)dummy[index + 2UL] + (uint16)dummy[index + 3UL]);

    readed_cs |= (uint16)((uint32)dummy[index+4UL] << (uint32)8UL);
    readed_cs |= (uint16)dummy[index + 5UL];

    if((lRet == SAL_RET_SUCCESS) && (checksum == readed_cs))
    {
        if(puiValue != NULL_PTR)
        {
            *puiValue = (uint32)dummy[index] << 24UL;
            *puiValue |= (uint32)dummy[index + 1UL] << 16UL;
            *puiValue |= (uint32)dummy[index + 2UL] << 8UL;
            *puiValue |= (uint32)dummy[index + 3UL];
        }
    }
    else
    {
        mcu_printf("%s] Invalid crc calulate x%x, readed x%x or xfer ret x%x\n",
                __func__,checksum,readed_cs,lRet);
        mcu_printf("%s] read data x%02x, x%02x, x%02x, x%02x\n",
                __func__, dummy[index], dummy[index+1UL], dummy[index+2UL], dummy[index+3UL]);
        lRet = SAL_RET_FAILED;
    }

    return lRet;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_SdmExtTest
*
* Function to test sdm external path
*
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SdmExtTest
(
    void
) {
    uint32                              speed;
    SALRetCode_t                        ret;
    uint32                              w_test_cnt;
    uint8                               w_va[4] = {0x1a, 0xcc, 0xe5, 0x51};
    uint8                               reg[4] = {0x00, 0x00, 0x00, 0x01};
    uint32                              w_va_num;
    uint32                              r_va;
    uint32                              cs_high;

    speed = 22UL*1000UL*1000UL;
    ret = SAL_RET_SUCCESS;
    w_test_cnt = 0xffUL;
    w_va_num = 0;
    r_va=0;
    cs_high = FALSE;

    // Set MICOM GPIO Control
    SAL_WriteReg(0x0, 0x1b936150UL);
    SAL_WriteReg(0x0, 0x1b936158UL);

    mcu_printf("GPIOC_SEL : %x \n", SAL_ReadReg(0x1b936150UL));
    mcu_printf("GPIOG_SEL : %x \n", SAL_ReadReg(0x1b936158UL));

    // SDM GPSB Channel open & Set param

    (void)GPSB_Open(GPSB_CH_3, NULL, NULL, NULL, NULL, NULL, NULL, NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);

    (void)GPSB_SetSpeed(GPSB_CH_3, speed);

    (void)GPSB_SetSdmChannel(GPSB_CH_3);

    (void)GPSB_SetBpw(GPSB_CH_3, 32UL);

    GPSB_SetMode(GPSB_CH_3, GPSB_MODE_0);

    // Set SDM External path
    ret = GPSB_WriteSdm((uint8)GPSB_CH_3, (uint32)0x820000UL, w_va, (uint32)4UL);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("!!!! %d Write FAIL cnt %d, va x%x, x%x, x%x, x%x, ret x%x\n",
                __LINE__, w_test_cnt, w_va[0], w_va[1], w_va[2], w_va[3], ret);
        ret = SAL_RET_FAILED;
    }

    ret = GPSB_ReadSdm((uint8)GPSB_CH_3, (uint32)0x820000UL, &r_va);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("!!!! %d Read FAIL x%x\n", __LINE__, ret);
        ret =  SAL_RET_FAILED;
    }

    if(r_va != 0x1acce551UL)
    {
        mcu_printf("SDM (0x820000) : %x \n", r_va);
    }

    GPSB_WriteSdm((uint8)GPSB_CH_3, (uint32)0x100058UL , reg, 4UL);

    ret = GPSB_ReadSdm((uint8)GPSB_CH_3, (uint32)0x100058UL , &r_va);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("!!!! %d Read FAIL x%x\n", __LINE__, ret);
        ret = SAL_RET_FAILED;
    }
    else
    {
        mcu_printf("SDM interface select : %x \n", r_va);
        ret = SAL_RET_FAILED;
    }

    // Set Ext SDM port sel

    SAL_WriteReg((SAL_ReadReg((uint32)0x1B160004UL) & (~((uint32)0x3fUL << (uint32)16UL)))              | ((uint32)19UL<< (uint32)16UL)
            , (uint32)0x1B160004UL);

    mcu_printf("[%s] == Set SDM External port : 0x%x \n",
            __func__ , SAL_ReadReg(0x1b160004));

    SAL_WriteReg((uint32)0x707UL, (uint32)0x1B160008UL);

    mcu_printf("[%s] == Set SDM Diable internal select : 0x%x \n",
            __func__ , SAL_ReadReg((uint32)0x1b160008UL));

    // Close sdm channel

    ret = GPSB_Close(GPSB_CH_3);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[%s] == GPSB channel close fail. ch : 3 \n", __func__);
    }

    ret = GPIO_Config(GPIO_GPC(22UL), GPIO_FUNC(6UL)| GPIO_DS(3UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = GPIO_Config(GPIO_GPC(23UL), GPIO_FUNC(6UL)| GPIO_DS(3UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = GPIO_Config(GPIO_GPC(20UL), GPIO_FUNC(6UL)| GPIO_DS(3UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = GPIO_Config(GPIO_GPC(21UL), GPIO_FUNC(6UL)| GPIO_DS(3UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    // Open Master channel with port number
    g_gpio_sclk = GPIO_GPG(7UL);
    g_gpio_cs   = GPIO_GPG(8UL);
    g_gpio_sdo  = GPIO_GPG(9UL);
    g_gpio_sdi  = GPIO_GPG(10UL);

    (void)GPSB_Open(GPSB_CH_0, g_gpio_sdo, g_gpio_sdi, g_gpio_sclk, NULL, NULL, NULL, NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);

    (void)GPSB_SetSpeed(GPSB_CH_0, speed);

    (void)GPSB_SetBpw(GPSB_CH_0, 32UL);

    (void)GPSB_CsInit(GPSB_CH_0, g_gpio_cs, cs_high);

    mcu_printf("Write Test Start (%d).\n", g_test_num);

    for(w_test_cnt = 0; w_test_cnt < g_test_num; w_test_cnt++)
    {
        w_va[0] = (uint8)w_va_num;
        w_va[1] = (uint8)w_va_num;
        w_va[2] = (uint8)w_va_num;
        w_va[3] = (uint8)w_va_num;

        ret = GPSB_WriteSdm(GPSB_CH_0, (uint32)0x820108UL/*VCRC_REGION_REF_CRC*/, w_va, 4UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("!!!! %d Write FAIL cnt %d, va x%x, x%x, x%x, x%x, ret x%x\n",
                    __LINE__, w_test_cnt, w_va[0], w_va[1], w_va[2], w_va[3], ret);
        }

        ret = GPSB_ReadSdm(GPSB_CH_0, 0x820108UL, &r_va);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("!!!! %d Read FAIL cnt %d, ret x%x\n",        __LINE__, w_test_cnt, ret);
        }

        if(r_va != ((w_va_num<<24UL)|(w_va_num<<16UL)|(w_va_num<<8UL)|(w_va_num)))
        {
            mcu_printf("!!!! %d Mismatch FAIL r_va x%x, w_va x%x\n",
                    __LINE__, r_va, ((w_va_num<<24)|(w_va_num<<16)|(w_va_num<<8)|(w_va_num)));
        }

        w_va_num++;
    }

    mcu_printf("Write Test Done.\n");

    return SAL_RET_SUCCESS;
}
#endif
#if 0
/*
***************************************************************************************************
*                                          GPSB_SdmIntTest
*
* Function to test sdm internal path
*
* @param    uiSclk [in]     : SCLK clock speed
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SdmIntTest
(
    uint32                              uiSclk
) {
    uint32                              speed;
    SALRetCode_t                        ret;
    uint32                              w_test_cnt;
    uint8                               w_va[4] = {0x1a, 0xcc, 0xe5, 0x51};
    uint32                              w_va_num;
    uint32                              r_va;

    ret = SAL_RET_SUCCESS;
    w_test_cnt = 0;
    w_va_num = 0;
    r_va = 0;

    speed = uiSclk;

    // SDM GPSB Channel open & Set param

    (void)GPSB_Open(GPSB_CH_3, NULL, NULL, NULL, NULL, NULL, NULL, NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);

    (void)GPSB_SetSpeed(GPSB_CH_3, speed);

    (void)GPSB_SetSdmChannel(GPSB_CH_3);

    (void)GPSB_SetBpw(GPSB_CH_3, 32UL);

    GPSB_SetMode(GPSB_CH_3, GPSB_MODE_0);

    ret = GPSB_WriteSdm(GPSB_CH_3, 0x820000UL, w_va, 4UL);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("!!!! %d Write FAIL cnt %d, va x%x, x%x, x%x, x%x, ret x%x\n",
                __LINE__, w_test_cnt, w_va[0], w_va[1], w_va[2], w_va[3], ret);
    }

    ret = GPSB_ReadSdm(3, 0x820000, &r_va);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("!!!! %d Read FAIL x%x\n", __LINE__, ret);
    };

    if(r_va != 0x1acce551UL)
    {
        mcu_printf("SDM (0x820000) : %x \n", r_va);
    }

    mcu_printf("Write Test Start (%d).\n", g_test_num);

    for(w_test_cnt = 0; w_test_cnt < g_test_num; w_test_cnt++)
    {
        w_va[0]= (uint8)(w_va_num & 0xFFUL);
        w_va[1]= (uint8)(w_va_num & 0xFFUL);
        w_va[2]= (uint8)(w_va_num & 0xFFUL);
        w_va[3]= (uint8)(w_va_num & 0xFFUL);

        ret = GPSB_WriteSdm(GPSB_CH_3, 0x820108UL/*VCRC_REGION_REF_CRC*/, w_va, 4UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("!!!! %d Write FAIL cnt %d, va x%x, x%x, x%x, x%x, ret x%x\n",
                    __LINE__, w_test_cnt, w_va[0], w_va[1], w_va[2], w_va[3], ret);
        }

        ret = GPSB_ReadSdm(GPSB_CH_3, 0x820108UL, &r_va);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("!!!! %d Read FAIL cnt %d, ret x%x\n", __LINE__, w_test_cnt, ret);
        }

        if(r_va != ((w_va_num<<24UL)|(w_va_num<<16UL)|(w_va_num<<8UL)|(w_va_num)))
        {
            mcu_printf("!!!! %d Mismatch FAIL r_va x%x, w_va x%x\n",
                    __LINE__, r_va, ((w_va_num<<24UL)|(w_va_num<<16UL)|(w_va_num<<8UL)|(w_va_num)));
        }

        w_va_num++;
    }

    // Close sdm channel

    ret = GPSB_Close(GPSB_CH_3);

    if(ret != SAL_RET_SUCCESS)
    {
        mcu_printf("[%s] == GPSB channel close fail. ch : 3 \n", __func__);
    }

    mcu_printf("Write Test Done (Speed : %d).\n", speed);

    return SAL_RET_SUCCESS;
}
#endif
/*
***************************************************************************************************
*                                          GPSB_InternalLoopbackTest
*
* Function to test internal loopback between mst & slv channels.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucBpw [in]      : Value of bit per word
* @param    ucXferMode      : transfer mode
* @param    uiSpeed         : SCLK clock speed
* @return   Success or error code
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_InternalLoopbackTest
(
    uint8                               ucCh,
    uint8                               ucBpw,
    uint8                               ucXferMode,
    uint32                              uiSpeed
) {
    SALRetCode_t                        ret;
    uint32                              i;
    uint32                              cnt;
    uint32                              data_size;
    uint32                              gpio_cs;
    uint32                              tmp_cnt;
    uint32                              dma_buf_size;
    uint8                               cs_high;
    uint32                              speed;
    uint8                               xfer_mode;
    GPSBOpenParam_t                     OpenParam;

    ret = SAL_RET_SUCCESS;
    cnt = 0;
    data_size = 0;
    gpio_cs = 0;
    tmp_cnt = 0;
    dma_buf_size = 0;
    cs_high = 0;
    speed = 0;
    xfer_mode = 0;

    xfer_mode = ucXferMode;
    speed = uiSpeed;

    dma_buf_size = DMA_BUF_SIZE;

    data_size = GPSB_TEST_BUF_SIZE;

    if (((uint32)ucXferMode & GPSB_XFER_MODE_DMA) != (uint32)NULL)
    {
        g_tx_dma = (uint32 *)MPU_GetDMABaseAddress();
        g_rx_dma = (uint32 *)(MPU_GetDMABaseAddress() + dma_buf_size);
    }
    else
    {
        g_tx_dma = (uint32 *)NULL_PTR;
        g_rx_dma = (uint32 *)NULL_PTR;
    }

    if(g_rx_dma == (uint32 *)NULL_PTR)
    {
        mcu_printf("%s only tx transfer test\n", __func__);
    }

    OpenParam.uiSdo         = GPIO_GPA(2UL);
    OpenParam.uiSdi         = GPIO_GPA(3UL);
    OpenParam.uiSclk        = GPIO_GPA(0UL);
    OpenParam.pDmaAddrTx    = g_tx_dma;
    OpenParam.pDmaAddrRx    = g_rx_dma;
    OpenParam.uiDmaBufSize  = dma_buf_size;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;
	OpenParam.uiIsSlave		= GPSB_MASTER_MODE;

    (void) GPSB_Open(ucCh, OpenParam);

    (void) GPSB_SetSpeed(ucCh, speed);

    (void) GPSB_SetBpw(ucCh, ucBpw);

    GPSB_SetMode(ucCh, GPSB_LOOP);

    if(((uint32)xfer_mode & GPSB_XFER_MODE_DMA) != (uint32)NULL)
    {
        (void)GPSB_CsAlloc(ucCh, gpio_cs);
    }
    else
    {
        (void)GPSB_CsInit(ucCh, gpio_cs, cs_high);
    }

    (void)SAL_MemSet(g_tx, 0, data_size);

    if(g_tx_dma != (uint32 *)NULL_PTR)
    {
        for(i=0;i<(data_size/4UL);i++)
        {
            g_tx[i] = (uint8)(((uint8)i + (uint8)1UL) & (uint8)0xFFUL);
        }
    }
    else
    {
        for(i=0;i<(data_size);i++)
        {
            g_tx2[i] = (uint8)(((uint8)i + (uint8)1UL) & (uint8)0xFFUL);
        }
    }

    tmp_cnt = 0;

    while(1)
    {
        (void)SAL_MemSet(g_rx, 0x0, data_size);
        (void)SAL_MemSet(g_rx2, 0x0, data_size);

        if(((uint32)xfer_mode & GPSB_XFER_MODE_DMA) == (uint32)NULL)
        {
            (void)GPSB_CsActivate((uint32)ucCh, gpio_cs, cs_high);
        }

        if((xfer_mode & GPSB_XFER_MODE_WITHOUT_INTERRUPT) != 0UL)
        {
            ret = GPSB_Xfer((uint32)ucCh, (uint8 *)g_tx2, (uint8 *)g_rx2, data_size, xfer_mode);
        }
        else if((xfer_mode & GPSB_XFER_MODE_WITH_INTERRUPT) != 0UL)
        {
            ret = GPSB_AsyncXfer((uint32)ucCh, g_tx, g_rx, data_size, xfer_mode);
        }

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("%s ch %d xfer error ret %d\n", __func__, ucCh, ret);

            goto err;
        }

        if(((uint32)xfer_mode & GPSB_XFER_MODE_DMA) == (uint32)NULL)
        {
            (void)GPSB_CsDeactivate((uint32)ucCh, gpio_cs, cs_high);
        }

        cnt = 0;
        delay1us(1);

        for(i=0;i<(data_size / 4UL);i++)
        {
            if(g_tx[i] != g_rx[i])
            {

                cnt++;
            }
        }

        if(cnt > (uint32)NULL)
        {
            mcu_printf("%s ch %d loopback test fail cnt %d %d\n", __func__, ucCh, tmp_cnt, cnt);
            ret =  SAL_RET_FAILED;
            break;
        }
        else
        {
            mcu_printf("%s ch %d loopback test success %d\n", __func__, ucCh, tmp_cnt);
        }

        tmp_cnt++;

        if(tmp_cnt > GPSB_TEST_NUM)
        {
            ret = SAL_RET_SUCCESS;
            break;
        }

    }

    (void)GPSB_Close(ucCh);
    mcu_printf("%s ret %d\n", __func__, ret);

    return ret;

err :
    (void)GPSB_Close(ucCh);
    mcu_printf("%s ret %d\n", __func__, ret);

    return ret;
}

#if 0
/*
***************************************************************************************************
*                                          GPSB_ExtDevTest
*
* Function to test external device
*
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_ExtDevTest
(
    void
) {
    SALRetCode_t                        ret;
    uint32                              data_size;
    uint32                              gpio_sdi;
    uint32                              gpio_sdo;
    uint32                              gpio_sclk;
    uint32                              gpio_cs;
    uint32                              xfer_mode;
    uint32                              dma_buf_size;
    uint8                               cs_high;
    uint32                              speed;
    uint32                              ch;
    uint32                              bpw;
    uint8                               tx_buf[8] = {0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71};
    uint8                               rx_buf[8] = {0,};
    uint32                              test_cnt;
    uint8                               port;

    ret = SAL_RET_SUCCESS;
    data_size = 0;
    gpio_sdi = 0;
    gpio_sdo = 0;
    gpio_sclk = 0;
    gpio_cs = 0;
    xfer_mode = 0;
    dma_buf_size = 0;
    cs_high = 1;
    speed = 0;
    bpw = 0;
    test_cnt = 0;

    //xfer_mode = GPSB_XFER_MODE_DMA|GPSB_XFER_MODE_WITH_INTERRUPT;

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT | GPSB_XFER_MODE_WITHOUT_CTF;
    //xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;

    dma_buf_size = DMA_BUF_SIZE;

    speed = 1000*1000;
    bpw = 8;

    data_size = 8;//GPSB_TEST_BUF_SIZE;

    ch = 1;
    port = 0;

    gpio_sclk   = gpsbport[port][GPSB_SCLK];
    gpio_cs     = gpsbport[port][GPSB_CS];
    gpio_sdo    = gpsbport[port][GPSB_SDO];
    gpio_sdi    = gpsbport[port][GPSB_SDI];

    /* for dma test */
    g_tx_dma = MPU_GetDMABaseAddress();
    g_rx_dma = g_tx_dma + dma_buf_size;

    (void)GPSB_Open(ch, gpio_sdo, gpio_sdi, gpio_sclk, g_tx_dma, g_rx_dma, dma_buf_size,
            NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);

    (void)GPSB_SetSpeed(ch, speed);

    (void)GPSB_SetBpw(ch, bpw);

    /* Without CTF mode */
    (void)GPSB_CsInit(ch, gpio_cs, cs_high);

    // CTF mode
    //(void)GPSB_CsAlloc(ch, gpio_cs, gpsbport[port][GPSB_FUNC]);
    //(void)GPIO_Config(gpio_cs, gpsbport[port][GPSB_FUNC] | GPIO_OUTPUT);

    SAL_MemSet(tx_buf, 0, sizeof(tx_buf));
    SAL_MemSet(rx_buf, 0x0, sizeof(rx_buf));

    tx_buf[0] = 0x00;
    tx_buf[1] = 0x12;
    tx_buf[2] = 0x12;
    tx_buf[3] = 0x34;
    tx_buf[4] = 0x34;
    tx_buf[5] = 0x56;
    tx_buf[6] = 0x56;
    tx_buf[7] = 0x77;

    while(test_cnt < 1000000UL)
    {
        if((xfer_mode & GPSB_XFER_MODE_WITHOUT_CTF) == (uint32)NULL)
        {
            (void)GPSB_CsActivate(ch, gpio_cs, (uint32)cs_high);
        }
        ret = GPSB_Xfer(ch, tx_buf, rx_buf, data_size, xfer_mode);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("%s ch %d xfer error ret %d\n", __func__, ch, ret);

            return ret;
        }

        mcu_printf("RX Data : %02x %02x %02x %02x\n", rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3]);
        mcu_printf("RX Data : %02x %02x %02x %02x\n", rx_buf[4], rx_buf[5], rx_buf[6], rx_buf[7]);

        if((xfer_mode & GPSB_XFER_MODE_WITHOUT_CTF) == (uint32)NULL)
        {
            (void)GPSB_CsDeactivate(ch, gpio_cs, (uint32)cs_high);
        }
        test_cnt++;
        delay1us(5000);
    }

    (void)GPSB_Close(ch);

    return ret;

}
#endif

uint32                              slave_rx_data[32] = {0,};
#if 0   // SPI Slave sample code for Joynext
uint8 g_bpw = 0;
uint32 tx_dma_addr = 0x7C000;
uint32 rx_dma_addr = 0x7C400;

#define CLEAR_REG32_BIT(reg_addr, mask)             (*((volatile uint32*)(reg_addr)) &= ~(uint32)(mask))
#define SET_REG32_BITFIELD(reg_addr, val)           (*((volatile uint32*)(reg_addr)) |= (uint32)(val))
#define SET_REG32_BIT(reg_addr, mask)               (*((volatile uint32*)(reg_addr)) |= (uint32)(mask))
#define GET_REG32_BIT(reg_addr, mask)               (*((volatile uint32*)(reg_addr)) & (uint32)(mask))
#define WRITE_REG32(reg_addr, val)                  (*((volatile uint32*)(reg_addr)) = (uint32)(val))
#define READ_REG32(reg_addr)                        (*((volatile uint32*)(reg_addr)))

#define CKC_PCLK_CFG_xxx_REGADDR(ctrl_baseaddr, ch) ((ctrl_baseaddr) + (ch) + 0x28U)
#define CMU_PCLK_CFG_GPSB3_REGID                    (0x1C)
#define CKC_PCLK_CFG_xxx_OUT_STS_OFFSET             (31U)
#define CKC_PCLK_CFG_xxx_OUT_STS_MASK               (1U << CKC_PCLK_CFG_xxx_OUT_STS_OFFSET)
#define CKC_PCLK_CFG_xxx_OUT_EN_OFFSET              (30U)
#define CKC_PCLK_CFG_xxx_OUT_EN_MASK                (1U << CKC_PCLK_CFG_xxx_OUT_EN_OFFSET)
#define CKC_PCLK_CFG_xxx_DIV_EN_OFFSET              (29U)
#define CKC_PCLK_CFG_xxx_DIV_EN_MASK                (1U << CKC_PCLK_CFG_xxx_DIV_EN_OFFSET)
#define CKC_PCLK_CFG_xxx_CLK_SEL_OFFSET             (24U)
#define CKC_PCLK_CFG_xxx_CLK_SEL_MASK               (0x1FU << CKC_PCLK_CFG_xxx_CLK_SEL_OFFSET)
#define CKC_PCLK_CFG_xxx_CLK_DIV_OFFSET             (0U)
#define CKC_PCLK_CFG_xxx_CLK_DIV_MASK               (0x0FFFU << CKC_PCLK_CFG_xxx_CLK_DIV_OFFSET)

#define MICOM_GPIOA_EN  (MCU_BSP_GPIO_BASE + 0x04)
#define MICOM_GPIOA_IEN (MCU_BSP_GPIO_BASE + 0x24)
#define MICOM_GPIOA_FN0 (MCU_BSP_GPIO_BASE + 0x30)
#define MICOM_GPIOA_FN1 (MCU_BSP_GPIO_BASE + 0x34)
#define MICOM_GPIOA_FN2 (MCU_BSP_GPIO_BASE + 0x38)
#define MICOM_GPIOA_FN3 (MCU_BSP_GPIO_BASE + 0x3C)
#define MICOM_GPIO_PERICHSEL    (0xA0F222B8)
#define MICOM_GPIO_MFIO         (0xA0F222B4)

#define SPI_WRITE_REG32(reg_addr, val)                  (*((volatile uint32*)(reg_addr)) = (uint32)(val))
#define SPI_READ_REG32(reg_addr)                        (*((volatile uint32*)(reg_addr)))

#define SPI_MICOM_SW_RESET_0                ((uint32) 0xA0F20010U)
#define SPI_GPSB_SW_RESET_BIT_OFFSET(x)     ((uint8) ((x) + 22U))
#define SPI_GPSB_SW_RESET_MASK(x)           ((uint32) ((uint32)1U << SPI_GPSB_SW_RESET_BIT_OFFSET(x)))

#define SPI_SLAVE_BASE_ADDRESS      0xA0130000U

#define SPI_REG_OFFSET_PORT     0x00
#define SPI_REG_OFFSET_STATUS   0x04
#define SPI_REG_OFFSET_INTEN    0x08
#define SPI_REG_OFFSET_MODE     0x0C
#define SPI_REG_OFFSET_TXBASE   0x20
#define SPI_REG_OFFSET_RXBASE   0x24
#define SPI_REG_OFFSET_PACKET   0x28
#define SPI_REG_OFFSET_DMACTR   0x2C
#define SPI_REG_OFFSET_DMAICR   0x34

#define SPI_DATA_LEN            8

static void Spi_DmaStart(void)
{
    uint32 addr = 0;
    uint32 reg = 0;

    // Enable DMA request for TX / RX FIFO.
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_INTEN;
    reg = SPI_READ_REG32(addr);
    reg = reg | ((1<<31) | (1<<30));
    SPI_WRITE_REG32(addr, reg);

    // Enable DMA TX / RX request.
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMACTR;
    reg = SPI_READ_REG32(addr);
    reg = reg | ((1<<31) | (1<<30));
    SPI_WRITE_REG32(addr , reg);

    // Clear DMA done and packet interrupt status.
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMAICR;
    reg = SPI_READ_REG32(addr);
    reg = reg | (1<<17);
    reg = reg & (~(1<<16) | ~(1<<20));
    SPI_WRITE_REG32(addr , reg);

    // Enable DMA operation.
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMACTR;
    reg = SPI_READ_REG32(addr);
    reg = reg | ((1<<0));
    SPI_WRITE_REG32(addr , reg);

}

static void Spi_DmaStop(void)
{
    uint32 addr = 0;
    uint32 reg = 0;

    // Disable DMA request for TX / RX FIFO.

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_INTEN;
    reg = SPI_READ_REG32(addr);
    reg = reg & (~(1<<31) | ~(1<<30));
    SPI_WRITE_REG32(addr, reg);

    // Disable DMA TX / RX request.
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMACTR;
    reg = SPI_READ_REG32(addr);
    reg = reg & (~(1<<31) | ~(1<<30));
    SPI_WRITE_REG32(addr , reg);

    // Clear DMA done and packet interrupt status.
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMAICR;
    reg = SPI_READ_REG32(addr);
    reg = reg | ((1<<29) | (1<<28));
    SPI_WRITE_REG32(addr , reg);

    // Disable DMA operation.
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMACTR;
    reg = SPI_READ_REG32(addr);
    reg = reg & (~(1<<0));
    SPI_WRITE_REG32(addr , reg);

}

static void Spi_PrepareDMA(void)
{
    uint32 addr = 0;
    uint32 reg = 0;

    if(g_bpw != 32)
    {
        addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMACTR;
        reg = SPI_READ_REG32(addr);
        reg = reg | (1<<28);
        SPI_WRITE_REG32(addr, reg);
    }
    else
    {
        addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_INTEN;
        reg = SPI_READ_REG32(addr);
        reg = reg & (~(1<<24) | ~(1<<25) | ~(1<<27) | ~(1<<28));
        SPI_WRITE_REG32(addr, reg);

        addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMACTR;
        reg = SPI_READ_REG32(addr);
        reg = reg & ~(1<<28);
        SPI_WRITE_REG32(addr, reg);
    }

    // Clear packet counter
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMACTR;
    reg = SPI_READ_REG32(addr);
    reg = reg | (1<<2);
    SPI_WRITE_REG32(addr, reg);
    reg = reg & ~(1<<2);
    SPI_WRITE_REG32(addr, reg);

    // Set multiple packet address mode for TX/RX
    reg = reg & (~(1<<14) | ~(1<<15) | ~(1<<16) | ~(1<<17));
    SPI_WRITE_REG32(addr, reg);

    // Set TX DMA base address
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_TXBASE;
    reg = tx_dma_addr;
    SPI_WRITE_REG32(addr, reg);

    // Set RX DMA base address
    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_RXBASE;
    reg = rx_dma_addr;
    SPI_WRITE_REG32(addr, reg);
}

static void Spi_SetPacketSize(uint32 size)
{
    uint32 addr = 0;
    uint32 reg = 0;

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_PACKET;
    reg = SPI_READ_REG32(addr);
    reg = reg | ((size & 0x1FFF)<<0);
    SPI_WRITE_REG32(addr , reg);
}

static void Spi_Disable(void)
{
    uint32 addr = 0;
    uint32 reg = 0;

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_MODE;
    reg = SPI_READ_REG32(addr);
    reg = reg & (~(1<<3));
    SPI_WRITE_REG32(addr , reg);
}

static void Spi_Enable(void)
{
    uint32 addr = 0;
    uint32 reg = 0;

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_MODE;
    reg = SPI_READ_REG32(addr);
    reg = reg | (1<<3);
    SPI_WRITE_REG32(addr , reg);
}

static void Spi_HwInit(void)
{
    uint32 addr = 0;
    uint32 reg = 0;
    uint32 divldv = 0;
    uint32 tx_threshold = 1;
    uint32 rx_threshold = 0;

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_MODE;
    reg = SPI_READ_REG32(addr);
    reg = reg | ((divldv & 0xFF) << 24);
    reg = reg | (1<<2); // set slave mode
    SPI_WRITE_REG32(addr , reg);

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_INTEN;
    reg = SPI_READ_REG32(addr);
    reg = reg & ~(0x3FF);
    reg = reg | ((1<<5)|(1<<6)|(1<<7)|(1<<8)); // interrupt error signal enable
    reg = reg | ((tx_threshold << 20) | (rx_threshold << 16)); // set tx/rx threshold
    SPI_WRITE_REG32(addr , reg);

}

static void Spi_SetBPW(uint8 bpw)
{
    uint32 addr = 0;
    uint32 reg = 0;
    uint8 bpw_value = 0;

    if((bpw != 8) && (bpw != 16) && (bpw != 32))
    {
        bpw_value = 8;
    }
    else
    {
        bpw_value = bpw;
    }

    g_bpw = bpw_value;

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_MODE;
    reg = SPI_READ_REG32(addr);
    reg = reg & (~(0x1F<<8));
    reg = reg | (((bpw_value - 1) & 0x1F) << 8);
    SPI_WRITE_REG32(addr , reg);

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_INTEN;
    reg = SPI_READ_REG32(addr);
    reg = reg & (~(0xF << 24));

    if(bpw_value == 8)
    {
        reg = reg | (0 << 24);
    }
    else if(bpw_value == 16)
    {
        reg = reg | ((1 << 24) | (1 << 26));
    }
    else if(bpw_value == 32)
    {
        reg = reg | ((1 << 24) | (1 << 25) | (1 << 26) | (1 << 27));
    }

}

static void Spi_FillTxData(const void * buf_ptr, uint32 size)
{
    uint32 idx = 0;

    if(size > 256)
    {
        // invalid data size.
        // return error?
    }

    if(g_bpw == 8)
    {
        const uint8 * data_8 = (const uint8 *) buf_ptr;

        for(idx = 0 ; idx < size ; idx++)
        {
            SPI_WRITE_REG32(SPI_SLAVE_BASE_ADDRESS , data_8[idx]);
        }
    }
    else if(g_bpw == 16)
    {
        const uint16 * data_16 = (const uint16 *) buf_ptr;

        for(idx = 0 ; idx < (size/2) ; idx++)
        {
            SPI_WRITE_REG32(SPI_SLAVE_BASE_ADDRESS , data_16[idx]);
        }
    }
    else if(g_bpw == 32)
    {
        const uint32 * data_32 = (const uint32 *) buf_ptr;

        for(idx = 0 ; idx < (size/4) ; idx++)
        {
            SPI_WRITE_REG32(SPI_SLAVE_BASE_ADDRESS , data_32[idx]);
        }
    }

}

static void Spi_Slave_InterruptHandleing(void * pArg)
{
    // For DMA interrupt handling.
    uint32 status;
    uint32 dmaicr;
    uint32 copy_length;
    uint32 addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_STATUS;

    copy_length = 0;

    // Check error status
    status = READ_REG32(addr);

    if((status & (0xFUL << 5UL)) != 0UL)
    {
        //error status occur

        if((status & (1<<5)) != 0UL)
        {
            //read fifo over-run error
        }

        if((status & (1<<6)) != 0UL)
        {
            //write fifo under-run error
        }

        if((status & (1<<7)) != 0UL)
        {
            //read fifo under-run error
        }

        if((status & (1<<8)) != 0UL)
        {
            //write fifo over-run error
        }

        /* Clear error status */
        WRITE_REG32(addr, 0x1E0); // Bit 5, 6, 7, 8

        return;
    }

    addr = SPI_SLAVE_BASE_ADDRESS + SPI_REG_OFFSET_DMAICR;
    dmaicr = READ_REG32(addr);

    if((dmaicr & (1<<29)) != 0UL)
    {
        //dma done interrupt
        WRITE_REG32(addr , dmaicr);

        copy_length = SPI_DATA_LEN;

        (void)SAL_MemCopy(&slave_rx_data[0], (void *)(rx_dma_addr), copy_length);

        Spi_DmaStart();
    }
}


static void GPSB_Slave_Test(void * pArg)
{
    uint32                              ucMstCh;
    uint32                              sclk;
    uint32                              sdo;
    uint32                              sdi;
    uint32                              xfer_mode;
    uint32                              i;
    uint32                              data_len;
    uint8                               tx[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8                               m_rx[8] = {0};

    ucMstCh     = 4;
    sclk        = GPIO_GPA(27UL);
    sdo         = GPIO_GPA(24UL);
    sdi         = GPIO_GPA(25UL);

    data_len = 8;

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;

    (void)GPSB_Open((uint32)ucMstCh, sdo, sdi, sclk, 0, 0, 0, NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);
    (void)GPSB_SetSpeed((uint32)ucMstCh, 2*1000*1000);
    (void)GPSB_SetBpw((uint32)ucMstCh, 32);

    //uint8 spi_slave_ch = 3;
    uint8 bpw = 32;
    uint32 default_slave_tx_data[16] = {0x99918174, 0x23225155, 0x00010203, 0x11223344};
    uint8 data_size = SPI_DATA_LEN;

    (void)GIC_IntVectSet((uint32)GIC_GPSB3_DMA, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &Spi_Slave_InterruptHandleing, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB3_DMA);

    CLEAR_REG32_BIT(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), CKC_PCLK_CFG_xxx_OUT_EN_MASK);
    CLEAR_REG32_BIT(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), CKC_PCLK_CFG_xxx_DIV_EN_MASK);

    while(GET_REG32_BIT(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), CKC_PCLK_CFG_xxx_OUT_STS_MASK) != 0U)
    {
        /* wait */
    }

    SET_REG32_BITFIELD(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), (5U << CKC_PCLK_CFG_xxx_CLK_SEL_OFFSET) & CKC_PCLK_CFG_xxx_CLK_SEL_MASK);
    CLEAR_REG32_BIT(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), (CKC_PCLK_CFG_xxx_CLK_DIV_MASK));
    SET_REG32_BITFIELD(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), (2U << CKC_PCLK_CFG_xxx_CLK_DIV_OFFSET) & CKC_PCLK_CFG_xxx_CLK_DIV_MASK);

    SET_REG32_BIT(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), CKC_PCLK_CFG_xxx_DIV_EN_MASK);
    SET_REG32_BIT(CKC_PCLK_CFG_xxx_REGADDR(MCU_BSP_CKC_BASE, CMU_PCLK_CFG_GPSB3_REGID), CKC_PCLK_CFG_xxx_OUT_EN_MASK);

    // Joynext : for slave SPI controller
    WRITE_REG32(MICOM_GPIOA_EN, 0);
    WRITE_REG32(MICOM_GPIOA_IEN, 0);

    // GPIO_A20 : SDO
    // GPIO_A21 : SDI
    // GPIO_A22 : CMD
    // GPIO_A23 : SCK
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) & ~(0x1 << 23))); // SCK input for slave
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) & ~(0x1 << 22))); // CMD input for slave
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) | (0x1 << 20)));
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) & ~(0x1 << 21)));
    WRITE_REG32(MICOM_GPIOA_IEN, (READ_REG32(MICOM_GPIOA_IEN) | (0x1 << 21) | (0x1 << 22) | (0x1 << 23)));


    WRITE_REG32(MICOM_GPIOA_FN2, (READ_REG32(MICOM_GPIOA_FN2) & (0x0000FFFF)));
    WRITE_REG32(MICOM_GPIOA_FN2, (READ_REG32(MICOM_GPIOA_FN2) | (0x33330000)));

    // Joynext : for master SPI controller
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) | (0x1 << 27)));
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) | (0x1 << 26)));
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) | (0x1 << 24)));
    WRITE_REG32(MICOM_GPIOA_EN, (READ_REG32(MICOM_GPIOA_EN) & ~(0x1 << 25)));
    WRITE_REG32(MICOM_GPIOA_IEN, (READ_REG32(MICOM_GPIOA_IEN) | (0x1 << 25)));

    WRITE_REG32(MICOM_GPIOA_FN3, (READ_REG32(MICOM_GPIOA_FN3) & (0xFFFF0000)));
    WRITE_REG32(MICOM_GPIOA_FN3, (READ_REG32(MICOM_GPIOA_FN3) | (0x00003333)));

    //clear bit
    WRITE_REG32(MICOM_GPIO_PERICHSEL, 0x0);
    WRITE_REG32(MICOM_GPIO_MFIO, 0x0);
    WRITE_REG32(MICOM_GPIO_MFIO, (1<<20) | (1<<12));

    SPI_WRITE_REG32(SPI_MICOM_SW_RESET_0, (SPI_READ_REG32(SPI_MICOM_SW_RESET_0) & ~(SPI_GPSB_SW_RESET_MASK((uint8) 3))));
    SPI_WRITE_REG32(SPI_MICOM_SW_RESET_0, (SPI_READ_REG32(SPI_MICOM_SW_RESET_0) | (SPI_GPSB_SW_RESET_MASK((uint8) 3))));

    Spi_DmaStop();
    Spi_Disable();
    Spi_HwInit();
    Spi_Enable();
    Spi_SetBPW(bpw);
    Spi_PrepareDMA();
    Spi_SetPacketSize(data_size);
    Spi_Enable();
    Spi_DmaStart();
    SAL_MemCopy((void*)tx_dma_addr, &default_slave_tx_data[0], SPI_DATA_LEN);

    while(1)
    {
        (void)GPSB_Xfer((uint32)ucMstCh, tx, m_rx, data_len, xfer_mode);
        for(i = 0; i < data_len ; i++)
        {
            tx[i]++;
        }

        mcu_printf("\n==============================\n");
        mcu_printf("[GPSB-S] Read Data \n");
        {
            mcu_printf("0x%x 0x%x",slave_rx_data[0], slave_rx_data[1]);
        }
        mcu_printf("\n==============================\n");
        mcu_printf("[GPSB-M] Read Data \n");
        for(i = 0 ; i < data_len ; i++)
        {
            mcu_printf("%x ", m_rx[i]);
        }
        mcu_printf("\n==============================\n");

        SAL_TaskSleep(200);
    }


}
#endif

#define SLAVE_FIFO_TEST

// If not define SLAVE_FIFO_TEST, this funtion do test for dma.
static void GPSB_MasterSlave_Test
(
    void *                              pArg
)
{
    uint32                              ucSlvCh;
    uint32                              ucMstCh;
    uint8                               bpw;
#ifdef SLAVE_FIFO_TEST
    uint32                              reg;
    uint32                              rx_cnt;
#endif
    uint32                              i;
    uint32                              data_len;
    uint32                              speed;
    uint32                              xfer_mode;
#ifdef SLAVE_FIFO_TEST
    uint8                               default_slave_tx_data[8] = {0x99, 0x91, 0x81, 0x74, 0x23, 0x22, 0x51, 0x55};
#else
    uint32                              default_slave_tx_data[2] = {0x99918174, 0x23225155};
    //uint32                                slave_rx_data[32] = {0,};
    uint32                              dma_buf_size;
#endif
    uint8                               tx[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8                               m_rx[8] = {0};
    GPSBOpenParam_t                     OpenParam;

    ucSlvCh = 3;
    ucMstCh = 4;

    speed = 2*1000*1000;
    bpw = (uint8)0UL;
    data_len = 8;

#ifdef SLAVE_FIFO_TEST
#else
    dma_buf_size = DMA_BUF_SIZE;
    g_tx_dma = MPU_GetDMABaseAddress();
    g_rx_dma = g_tx_dma + dma_buf_size;
#endif

    /* Configure GPSB slave */
    // FUNC3 MFIO_1_CH0 -> GPSB Controller 3 (CH3) -> Set Slave
    // GPIO_A20 : SDO
    // GPIO_A21 : SDI
    // GPIO_A22 : CMD
    // GPIO_A23 : SCK
    OpenParam.uiSdo         = GPIO_GPA(20UL);
    OpenParam.uiSdi         = GPIO_GPA(21UL);
    OpenParam.uiSclk        = GPIO_GPA(23UL);
    OpenParam.uiIsSlave     = GPSB_SLAVE_MODE;

#ifdef SLAVE_FIFO_TEST
    //(void)GPSB_Open((uint32)ucSlvCh, sdo, sdi, sclk, 0, 0, 0, NULL_PTR, NULL_PTR, GPSB_SLAVE_MODE);
    OpenParam.pDmaAddrTx    = NULL_PTR;
    OpenParam.pDmaAddrRx    = NULL_PTR;
    OpenParam.uiDmaBufSize  = 0;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;

    (void)GPSB_Open((uint32)ucSlvCh, OpenParam);
#else
    OpenParam.pDmaAddrTx    = g_tx_dma;
    OpenParam.pDmaAddrRx    = g_rx_dma;
    OpenParam.uiDmaBufSize  = dma_buf_size;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;

    //(void)GPSB_Open((uint32)ucSlvCh, sdo, sdi, sclk, g_tx_dma, g_rx_dma, dma_buf_size, NULL_PTR, NULL_PTR, GPSB_SLAVE_MODE);
    (void)GPSB_Open((uint32)ucSlvCh, OpenParam);
#endif

#ifdef SLAVE_FIFO_TEST
    bpw         = (uint8)8UL;
#else
    bpw         = (uint8)32UL;
#endif

    (void)GPSB_SetBpw(ucSlvCh, bpw);

#ifdef SLAVE_FIFO_TEST
    (void)GPSB_FillSlaveTXData(ucSlvCh, (const void*)&default_slave_tx_data[0], 8);
#else   // slave dma test
    SAL_MemCopy((void*)g_tx_dma, &default_slave_tx_data[0], 8);
    GPSB_SetSlaveDMAMode(ucSlvCh, (const void *)&default_slave_tx_data[0], (void *) &slave_rx_data[0], 8);
#endif

    mcu_printf("[GPSB-Slave-%d] =============================== \n", ucSlvCh);
    mcu_printf("[GPSB-Slave-%d] [Test Info] \n", ucSlvCh);
    mcu_printf("[GPSB-Slave-%d] BPW(Bit Per Word) : %d \n", ucSlvCh, bpw);
    mcu_printf("[GPSB-Slave-%d] Data Unit Length : %d Byte \n", ucSlvCh, data_len);
    mcu_printf("[GPSB-Slave-%d] =============================== \n", ucSlvCh);

    // FUNC3 MFIO_2_CH0 -> GPSB Controller 4 (CH4) -> Set Master
    // GPIO_A24 : SDO
    // GPIO_A25 : SDI
    // GPIO_A26 : CMD
    // GPIO_A27 : SCK

    OpenParam.uiSdo         = GPIO_GPA(24UL);
    OpenParam.uiSdi         = GPIO_GPA(25UL);
    OpenParam.uiSclk        = GPIO_GPA(27UL);
    OpenParam.pDmaAddrTx    = NULL_PTR;
    OpenParam.pDmaAddrRx    = NULL_PTR;
    OpenParam.uiDmaBufSize  = 0;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;
    OpenParam.uiIsSlave     = GPSB_MASTER_MODE;

    //(void)GPSB_Open((uint32)ucMstCh, sdo, sdi, sclk, 0, 0, 0, NULL_PTR, NULL_PTR, GPSB_MASTER_MODE);
    (void)GPSB_Open((uint32)ucMstCh, OpenParam);
    (void)GPSB_SetSpeed((uint32)ucMstCh, speed);
    (void)GPSB_SetBpw((uint32)ucMstCh, bpw);

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;

    while(1)
    {
        (void)GPSB_Xfer((uint32)ucMstCh, tx, m_rx, data_len, xfer_mode);
        for(i = 0; i < data_len ; i++)
        {
            tx[i]++;
        }
#ifdef SLAVE_FIFO_TEST
        reg = GPSB_GetStatus(ucSlvCh);
        if((reg & GPSB_WCNT_MASK) == 0)
        {
            (void)GPSB_FillSlaveTXData(ucSlvCh, (const void*)&default_slave_tx_data[0], data_len);
        }

        rx_cnt = ((reg & GPSB_RCNT_MASK) >> GPSB_RCNT_SHIFT);

        if(rx_cnt > 0)
        {
            mcu_printf("[GPSB-S-CH%d] Read Data \n", ucSlvCh);
            for(i = 0 ; i < rx_cnt ; i++)
            {
                reg = GPSB_GetSlaveRXData(ucSlvCh);
                mcu_printf("%x ", reg);
            }
            mcu_printf("\n==============================\n");
            mcu_printf("[GPSB-M-CH%d] Read Data \n", ucMstCh);
            for(i = 0 ; i < data_len ; i++)
            {
                mcu_printf("%x ", m_rx[i]);
            }
            mcu_printf("\n==============================\n");
        }
#else   // slave dma test
        mcu_printf("\n==============================\n");
        mcu_printf("[GPSB-S-CH%d] Read Data \n", ucSlvCh);
        {
            SAL_MemCopy(&slave_rx_data[0] , (void*)g_rx_dma, 8);
            mcu_printf("0x%x 0x%x",slave_rx_data[0], slave_rx_data[1]);
        }
        mcu_printf("\n==============================\n");
        mcu_printf("[GPSB-M-CH%d] Read Data \n", ucMstCh);
        for(i = 0 ; i < data_len ; i++)
        {
            mcu_printf("%x ", m_rx[i]);
        }
        mcu_printf("\n==============================\n");
#endif
        SAL_TaskSleep(500);
    }

    (void)GPSB_Close(ucSlvCh);
}

void GPSB_CreateSlaveTestTask
(
    void
)
{
    static uint32 Gpsb_Slave_TaskId = 0;
    static uint32 Gpsb_Slave_TaskStack[512];
#if 1
    if((SAL_TaskCreate(&Gpsb_Slave_TaskId,  (const uint8 *)"GPSB Slave Task", \
                         (SALTaskFunc)&GPSB_MasterSlave_Test, \
                         &(Gpsb_Slave_TaskStack[0]), \
                         (uint32)512, \
                         SAL_PRIO_ETH_TASK, \
                         NULL)) != SAL_RET_SUCCESS)
#else
     if((SAL_TaskCreate(&Gpsb_Slave_TaskId,  (const uint8 *)"GPSB Slave Task", \
     (SALTaskFunc)&GPSB_Slave_Test, \
     &(Gpsb_Slave_TaskStack[0]), \
     (uint32)512, \
     SAL_PRIO_ETH_TASK, \
     NULL)) != SAL_RET_SUCCESS)
#endif
    {
        mcu_printf("Create GPSB Slave Test Task : Fail\n ");
    }
    else
    {
        mcu_printf("Create GPSB Slave Test Task : Done\n ");
    }

}


/*
***************************************************************************************************
*                                          GPSB_Test
*
* Functions to call all of test functions.
*
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_Test
(
    void
) {
    SALRetCode_t                        ret;
    uint8                               ch_number;
    uint8                               mst_ch;
    uint8                               slv_ch;

    ret = SAL_RET_SUCCESS;
    ch_number = 0;
    slv_ch = 0;

    mcu_printf("--------------------------\n");
    mcu_printf("Start GPSB Test\n");
    mcu_printf("--------------------------\n");
    GPSB_EnableFmu();

    mcu_printf("\n");
    mcu_printf("========================================================\n");
    mcu_printf("\n");

    //for port test
    //GPSB_ExtDevTest();

    //GPSB_TestIpVerification();

    // Internal Loopback Test

    for(ch_number = 0 ; ch_number < GPSB_CH_NUM ; ch_number++)
    {
        mcu_printf("Test : Internal LB (CH %d , BPW 8 , Non-DMA , 20Mhz)\n", ch_number);
        ret = GPSB_InternalLoopbackTest(ch_number, (uint8)8UL, (uint8)GPSB_XFER_MODE_WITHOUT_INTERRUPT, 20UL*1000UL*1000UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("GPSB Internal Non-DMA LB Test Fail. [line :%d]\n", __LINE__);
        }
        mcu_printf("Test : Internal LB (CH %d , BPW 16 , Non-DMA , 40Mhz)\n", ch_number);
        ret = GPSB_InternalLoopbackTest(ch_number, (uint8)16UL, (uint8)GPSB_XFER_MODE_WITHOUT_INTERRUPT, 40UL*1000UL*1000UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("GPSB Internal Non-DMA LB Test Fail. [line :%d]\n", __LINE__);
        }

        mcu_printf("Test : Internal LB (CH %d , BPW 32 , Non-DMA , 50Mhz)\n", ch_number);
        ret = GPSB_InternalLoopbackTest(ch_number, (uint8)32UL, (uint8)GPSB_XFER_MODE_WITHOUT_INTERRUPT, 50UL*1000UL*1000UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("GPSB Internal Non-DMA LB Test Fail. [line :%d]\n", __LINE__);
        }

        mcu_printf("\n");
        mcu_printf("========================================================\n");
        mcu_printf("\n");

        mcu_printf("Test : Internal LB (CH %d , BPW 8 , DMA , 20Mhz)\n", ch_number);
        ret = GPSB_InternalLoopbackTest(ch_number, (uint8)8UL, (uint8)GPSB_XFER_MODE_DMA | (uint8)GPSB_XFER_MODE_WITH_INTERRUPT, 20UL*1000UL*1000UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("GPSB Internal DMA LB Test Fail. [line :%d]\n", __LINE__);
        }

        mcu_printf("Test : Internal LB (CH %d , BPW 16 , DMA , 40Mhz)\n", ch_number);
        ret = GPSB_InternalLoopbackTest(ch_number, (uint8)16UL, (uint8)GPSB_XFER_MODE_DMA | (uint8)GPSB_XFER_MODE_WITH_INTERRUPT, 40UL*1000UL*1000UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("GPSB Internal DMA LB Test Fail. [line :%d]\n", __LINE__);
        }

        mcu_printf("Test : Internal LB (CH %d , BPW 32 , DMA , 50Mhz)\n", ch_number);
        ret = GPSB_InternalLoopbackTest(ch_number, (uint8)32UL, (uint8)GPSB_XFER_MODE_DMA | (uint8)GPSB_XFER_MODE_WITH_INTERRUPT, 50UL*1000UL*1000UL);

        if(ret != SAL_RET_SUCCESS)
        {
            mcu_printf("GPSB Internal DMA LB Test Fail. [line :%d]\n", __LINE__);
        }

        mcu_printf("\n");
        mcu_printf("========================================================\n");
        mcu_printf("\n");
    }

    // SM Test 01

    for(mst_ch = 0 ; mst_ch < GPSB_CH_NUM ; mst_ch++)
    {
        mcu_printf("\n");
        mcu_printf("========================================================\n");
        mcu_printf("\n");

        (void)GPSB_SafetyTestProtocolMonitor(mst_ch, (uint8)8UL);
        delay1us (1000);

        mcu_printf("\n");
        mcu_printf("========================================================\n");
        mcu_printf("\n");

        (void)GPSB_SafetyTestProtocolMonitor(mst_ch, (uint8)16UL);
        delay1us (1000);

        mcu_printf("\n");
        mcu_printf("========================================================\n");
        mcu_printf("\n");

        (void)GPSB_SafetyTestProtocolMonitor(mst_ch, (uint8)32UL);
        delay1us (1000);

    }

    // SM Test 02
    mcu_printf("\n");
    mcu_printf("========================================================\n");
    mcu_printf("\n");

    for(mst_ch = 0 ; mst_ch < GPSB_CH_NUM ; mst_ch++)
    {
        (void)GPSB_SafetyTestStuckMonitor (mst_ch);

        delay1us(10);
    }

    mcu_printf("\n");
    mcu_printf("========================================================\n");
    mcu_printf("\n");

    // SM Test 03

    for(mst_ch = 0 ; mst_ch < GPSB_CH_NUM ; mst_ch++)
    {
        for(slv_ch = 0; slv_ch < GPSB_CH_NUM ; slv_ch++)
        {
            if(mst_ch == slv_ch)
            {
                continue;
            }

            ret = GPSB_SafetyTestLoopback(mst_ch, slv_ch);

            if(ret != SAL_RET_SUCCESS)
            {
                mcu_printf("GPSB SM Test3 (Loopback) Fail. [line :%d]\n", __LINE__);
            }

            delay1us(10);
        }
    }

    // SM Test 04
    (void)GPSB_SafetyTestWriteLock();

    (void)FMU_Deinit();

    mcu_printf("--------------------------\n");
    mcu_printf("Finish GPSB Test\n");
    mcu_printf("--------------------------\n");

    return;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )


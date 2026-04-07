// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : uart_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )

#include "string.h"
#include "sal_com.h"
#include <app_cfg.h>

#include <debug.h>
#include <reg_phys.h>
#include <uart.h>
#include <uart_test.h>
#include <bsp.h>

#define MAX_BUFFER_SIZE (32UL)
static uint8 gTxBuf[MAX_BUFFER_SIZE];
static uint8 gRxBuf[MAX_BUFFER_SIZE];

static UartParam_t UART_TestParam;

static uint8 gIsProbed[UART_CH_MAX] = {0, 0, 1, 0, 0, 0};   // UART_DEBUG_CH is opened before run console task

static void Uart_delay1us
(
    uint32                              n
)
{
    uint32  i;
    uint32  val;

    val = n * 500UL;
    for (i = 0UL; i < val; i++)
    {
         BSP_NOP_DELAY();
    }
}

static uint32 UART_Reg_Read
(
    uint8                               ucCh,
    uint32                              uiAddr
)
{
    uint32 uiRet;
    uint32 uiBaseAddr;
    uint32 uiRegAddr;

    uiRet       = 0;

    uiBaseAddr  = (UART_BASE_ADDR + (0x10000UL * (ucCh))) & 0xAFFFFFFFU;
    uiAddr      &= 0xFFFFU;
    uiRegAddr   = uiBaseAddr + uiAddr;
    uiRet       = SAL_ReadReg(uiRegAddr);

    return uiRet;
}

static void UART_Reg_Write
(
    uint8                               ucCh,
    uint32                              uiAddr,
    uint32                              uiSetValue
)
{
    uint32 uiBaseAddr;
    uint32 uiRegAddr;

    uiBaseAddr  = (UART_BASE_ADDR + (0x10000UL * (ucCh))) & 0xAFFFFFFFU;
    uiAddr      &= 0xFFFFU;
    uiRegAddr   = uiBaseAddr + uiAddr;
    SAL_WriteReg(uiSetValue, uiRegAddr);
}


static void UartDmaTxTest
(
    void
)
{
    uint32  i;

    UART_TestParam.sCh          = UART_DEBUG_CH;
    UART_TestParam.sPriority    = 10U;
    UART_TestParam.sBaudrate    = 115200U;
    UART_TestParam.sMode        = UART_DMA_MODE;
    UART_TestParam.sCtsRts      = UART_CTSRTS_OFF;
    UART_TestParam.sPortCfg     = 4U;
    UART_TestParam.sWordLength  = WORD_LEN_8;
    UART_TestParam.sFIFO        = ENABLE_FIFO;
    UART_TestParam.s2StopBit    = TWO_STOP_BIT_OFF;
    UART_TestParam.sParity      = PARITY_SPACE;
    UART_TestParam.sFnCallback  = (GICIsrFunc)&UART_ISR;

    /* Close the UART channel in case it was previously used */
    UART_Close(UART_TestParam.sCh);

    (void)UART_Open(&UART_TestParam);

    (void)SAL_MemSet(gTxBuf, 0, MAX_BUFFER_SIZE);

    // Write temporary data
    for(i = 0UL; i < MAX_BUFFER_SIZE; i++)
    {
        gTxBuf[i] = (uint8)'A' + (uint8)(i & 0xFFUL);
    }

    (void)UART_Write(UART_TestParam.sCh, gTxBuf, MAX_BUFFER_SIZE);

    Uart_delay1us(1000);

    UART_Close(UART_TestParam.sCh);
}

static void UartDmaRxTest
(
    void
)
{
    UART_TestParam.sCh          = UART_DEBUG_CH;
    UART_TestParam.sPriority    = 10U;
    UART_TestParam.sBaudrate    = 115200U;
    UART_TestParam.sMode        = UART_DMA_MODE;
    UART_TestParam.sCtsRts      = UART_CTSRTS_OFF;
    UART_TestParam.sPortCfg     = 4U;
    UART_TestParam.sWordLength  = WORD_LEN_8;
    UART_TestParam.sFIFO        = ENABLE_FIFO;
    UART_TestParam.s2StopBit    = TWO_STOP_BIT_OFF;
    UART_TestParam.sParity      = PARITY_SPACE;
    UART_TestParam.sFnCallback  = (GICIsrFunc)&UART_ISR;

    /* Close the UART channel in case it was previously used */
    (void)UART_Close(UART_TestParam.sCh);

    (void)UART_Open(&UART_TestParam);

    (void)SAL_MemSet(gRxBuf, 0, MAX_BUFFER_SIZE);

    /* STOP */
    /* Type the text in the console to test GDMA Rx */

    (void)UART_Read(UART_TestParam.sCh, gRxBuf, MAX_BUFFER_SIZE);

    Uart_delay1us(1000);

    UART_Close(UART_TestParam.sCh);
}

static void UartInterruptTest
(
    void
)
{
    sint32  size = 0;
    sint32  i;

    UART_TestParam.sCh          = UART_DEBUG_CH;
    UART_TestParam.sPriority    = 10U;
    UART_TestParam.sBaudrate    = 115200U;
    UART_TestParam.sMode        = UART_INTR_MODE;
    UART_TestParam.sCtsRts      = UART_CTSRTS_OFF;
    UART_TestParam.sPortCfg     = 4U;
    UART_TestParam.sWordLength  = WORD_LEN_8;
    UART_TestParam.sFIFO        = ENABLE_FIFO;
    UART_TestParam.s2StopBit    = TWO_STOP_BIT_OFF;
    UART_TestParam.sParity      = PARITY_SPACE;
    UART_TestParam.sFnCallback  = (GICIsrFunc)&UART_ISR;

    /* Close the UART channel in case it was previously used */
    UART_Close(UART_TestParam.sCh);

    (void)UART_Open(&UART_TestParam);

    while(TRUE)
    {
        /**** UART Interrupt Rx Test ****/
        size = UART_Read(UART_TestParam.sCh, gRxBuf, MAX_BUFFER_SIZE);

        if(size == 0)
        {
            continue;
        }
        else if (size < 0)
        {
            break;
        }
        else
        {
            for(i = 0; i < size; i ++)
            {
                gTxBuf[i] = gRxBuf[i];
            }

            /**** UART Interrupt Tx Test ****/
            (void)UART_Write(UART_TestParam.sCh, gTxBuf, (uint32)size);

            Uart_delay1us(1000);

            if(gTxBuf[0] == (uint8)'q') //input 'Q' to exit
            {
                UART_Close(UART_TestParam.sCh);
                break;
            }

            (void)SAL_MemSet(gTxBuf, 0, MAX_BUFFER_SIZE);
            (void)SAL_MemSet(gRxBuf, 0, MAX_BUFFER_SIZE);
        }
    }
}


static void UartPollingTest
(
    void
)
{
    sint32  data;
    sint8   getc_err;
    uint32  val;

    data        = 0;
    getc_err    = 0;
    val         = 0UL;

    UART_TestParam.sCh          = UART_DEBUG_CH;
    UART_TestParam.sPriority    = GIC_PRIORITY_NO_MEAN;
    UART_TestParam.sBaudrate    = 115200U;
    UART_TestParam.sMode        = UART_POLLING_MODE;
    UART_TestParam.sCtsRts      = UART_CTSRTS_OFF;
    UART_TestParam.sPortCfg     = 4U;
    UART_TestParam.sWordLength  = WORD_LEN_8;
    UART_TestParam.sFIFO        = ENABLE_FIFO;
    UART_TestParam.s2StopBit    = TWO_STOP_BIT_OFF;
    UART_TestParam.sParity      = PARITY_SPACE;
    UART_TestParam.sFnCallback  = NULL_PTR; /* if polling mode, callback function is NULL_PTR */

    /* Close the UART channel in case it was previously used */
    UART_Close(UART_TestParam.sCh);

    (void)UART_Open(&UART_TestParam);

    while(TRUE)
    {
        data = UART_GetChar(UART_TestParam.sCh, (sint32)0, (sint8 *)&getc_err);

        if (getc_err < 0)
        {
           ; // Handle Error
        }
        else
        {
            if(data >= 0)
            {
                val = (uint32)data;
                (void)UART_PutChar(UART_TestParam.sCh, (uint8)(val & 0xFFUL));

                Uart_delay1us(1000);

                if(data == (sint32)'q') //input 'q' to exit
                {
                    UART_Close(UART_TestParam.sCh);
                    break;
                }
            }
        }

        getc_err = 0;
    }
}

static void UartLoopBackTest
(
    void
)
{
    uint8  ch;
    uint32  i;
    uint32  max_ch;
    uint32  max_data;
    uint32  wdata;
    uint32  rdata;
    uint32  cr_data;

    max_ch   = UART_CH_MAX;
    max_data = 30UL;
    wdata    = 0;
    rdata    = 0;
    cr_data  = 0;

    /********************** UART Loopback sequence **********************/
    /* Uart baudrate -> UARTEN -> write Tx data -> UART RXE -> UART TXE */

    for (ch = 0U; ch < max_ch; ch++)
    {
        /* Disable the UART ch in case it was previously used */
        SAL_WriteReg(NULL, MCU_BSP_UART_BASE + (0x10000UL * (ch)) + UART_REG_CR);

        // Set baudrate 115200
        UART_Reg_Write(ch, OFFSET_IBRD, 0x1AUL);
        UART_Reg_Write(ch, OFFSET_FBRD, 0x2UL);
        // Set word length is 8bits and Enables FIFOs
        UART_Reg_Write(ch, OFFSET_LCRH, 0x70UL);

        // UART Enable
        cr_data = CR_UARTEN;
        UART_Reg_Write(ch, OFFSET_CR, cr_data);

        // Loopback enable
        cr_data |= CR_LBE;
        UART_Reg_Write(ch, OFFSET_CR, cr_data);

        // Write data
        for(i = 0; i < max_data; i++)
        {
            wdata = (uint32)'A' + (uint32)(i & 0xFFUL);

            // Write Data
            UART_Reg_Write(ch, OFFSET_DR, wdata);

            // Receive Enable
            cr_data |= CR_RXE;
            UART_Reg_Write(ch, OFFSET_CR, cr_data);

            // Transmit Enable
            cr_data |= CR_TXE;
            UART_Reg_Write(ch, OFFSET_CR, cr_data);

            Uart_delay1us(1000);

            // Read Data
            rdata = UART_Reg_Read(ch, OFFSET_DR);

            if (wdata != rdata)
            {
                break;
            }

            wdata = 0;
            rdata = 0;
        }

        /* Disable the UART ch */
        SAL_WriteReg(NULL, MCU_BSP_UART_BASE + (0x10000UL * (ch)) + UART_REG_CR);
    }
}

sint32 UART_ConsoleTxTest
(
    uint8                               ucTxCh,
    uint8                               uiPortCfg,
    const uint8 *                       pucBuf,
    uint32                              len
)
{
    sint32  ret = 0;
    uint32  i;

    UART_TestParam.sCh          = ucTxCh;
    UART_TestParam.sPriority    = 10U;
    UART_TestParam.sBaudrate    = 115200U;
    UART_TestParam.sMode        = UART_POLLING_MODE;
    UART_TestParam.sCtsRts      = UART_CTSRTS_OFF;
    UART_TestParam.sPortCfg     = uiPortCfg;
    UART_TestParam.sWordLength  = WORD_LEN_8;
    UART_TestParam.sFIFO        = ENABLE_FIFO;
    UART_TestParam.s2StopBit    = TWO_STOP_BIT_OFF;
    UART_TestParam.sParity      = PARITY_SPACE;
    UART_TestParam.sFnCallback  = NULL_PTR; /* if polling mode, callback function is NULL_PTR */

    /* Open input Uart ch */
    if(gIsProbed[ucTxCh] == 0U)
    {
        if(uiPortCfg < UART_PORT_CFG_MAX)
        {
            ret = UART_Open(&UART_TestParam);
            gIsProbed[ucTxCh] = 1U;
        }
        else
        {
            mcu_printf("Input PortCfg val is larger than UART_PORT_CFG_MAX(%d)\n", UART_PORT_CFG_MAX);
            ret = -1;
        }
    }
    else
    {
        /* Selected ch is already open */
        ret = 0;
    }

    /* Tx by input ch */
    if(ret != -1)
    {
        if(len <= MAX_BUFFER_SIZE)
        {
            for(i = 0; i < len ; i++)
            {
                gTxBuf[i] = pucBuf[i];
            }

            /* Write data */
            ret = UART_Write(UART_TestParam.sCh, gTxBuf, len);

            if(ret != -1)
            {
                mcu_printf("Tx done (Uart ch%d)\n", UART_TestParam.sCh, len, ret);
                (void)SAL_MemSet(gTxBuf, 0, MAX_BUFFER_SIZE);
            }
            else
            {
                mcu_printf("Tx failed (Uart ch%d)\n", UART_TestParam.sCh);
            }
        }
        else
        {
            mcu_printf("Size of input data(%d) is larger than Uart FIFO size(%d)\n", len, MAX_BUFFER_SIZE);
            ret = -1;
        }
    }
    else
    {
        mcu_printf("Fail to open Uart ch%d\n", UART_TestParam.sCh);
    }

    return ret;
}

void UART_SelectTestCase
(
    uint32                              uiTestCase
)
{
    switch(uiTestCase)
    {
        case    1:
        {
            UartPollingTest();
            break;
        }
        case    2:
        {
            UartInterruptTest();
            break;
        }
        case    3:
        {
            UartDmaTxTest();
            break;
        }
        case    4:
        {
            UartDmaRxTest();
            break;
        }
        case    5:
        {
            UartLoopBackTest();
            break;
        }
        default :
        {
            mcu_printf("== Invaild Test Case ==\n");
            break;
        }
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )


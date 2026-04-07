// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : uart.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_UART == 1 )

#include <bsp.h>
#include "reg_phys.h"
#include "gdma.h"
#include "uart.h"
#include "clock.h"
#include "clock_dev.h"
#include "gpio.h"
#include "mpu.h" //mem_get_dma_base

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#if 0//(DEBUG_ENABLE)
#   include <debug.h>
#   define UART_D(fmt, args...)          {LOGD(DBG_TAG_UART, fmt, ## args)}
#   define UART_E(fmt, args...)          {LOGE(DBG_TAG_UART, fmt, ## args)}
#else
#   define UART_D(fmt, args...)
#   define UART_E(fmt, args...)
#endif

/*
***************************************************************************************************
*                                       LOCAL VARIABLES
***************************************************************************************************
*/

static UartParam_t UART_Param;

static UartStatus_t uart[UART_CH_MAX];

static uint8 uart_buff0[2][UART_BUFF_SIZE];
static uint8 uart_buff1[2][UART_BUFF_SIZE];
static uint8 uart_buff2[2][UART_BUFF_SIZE];
static uint8 uart_buff3[2][UART_BUFF_SIZE];
static uint8 uart_buff4[2][UART_BUFF_SIZE];
static uint8 uart_buff5[2][UART_BUFF_SIZE];

static uint8 *uart_buff[UART_CH_MAX][2] =
{
    { uart_buff0[0], uart_buff0[1] },
    { uart_buff1[0], uart_buff1[1] },
    { uart_buff2[0], uart_buff2[1] },
    { uart_buff3[0], uart_buff3[1] },
    { uart_buff4[0], uart_buff4[1] },
    { uart_buff5[0], uart_buff5[1] }
};

/*
***************************************************************************************************
                                     STATIC FUNCTION PROTOTYPES
***************************************************************************************************
*/

static uint32 UART_RegRead
(
    uint8                               ucCh,
    uint32                              uiAddr
);

static void UART_RegWrite
(
    uint8                               ucCh,
    uint32                              uiAddr,
    uint32                              uiSetValue
);

static SALRetCode_t UART_Reset
(
    uint8                               ucCh
);

static sint32 UART_SetGpio
(
    uint8                               ucCh,
    const UartBoardPort_t *             psInfo
);

static sint32 UART_SetPortConfig
(
    uint8                               ucCh,
    uint32                              uiPort
);

static SALRetCode_t UART_ClearGpio
(
    uint8                               ucCh
);

static sint32 UART_SetChannelConfig
(
    UartParam_t                         * pUartCfg
);

static sint32 UART_SetBaudRate
(
    uint8                               ucCh,
    uint32                              uiBaud
);

static void UART_StatusInit
(
    uint8                               ucCh
);

static sint32 UART_Probe
(
    UartParam_t                         * pUartCfg
);

static sint32 UART_Rx
(
    uint8                               ucCh
);

static sint32 UART_TxChar
(
    uint8                               ucCh,
    uint8                               cChar
);

static sint32 UART_Tx
(
    uint8                               ucCh
);

static void UART_DmaRxIrq
(
    uint8                               ucCh
);

static void UART_EnableInterrupt
(
    uint8                               ucCh,
    uint32                              uiPriority,
    uint8                               ucFIFO,
    GICIsrFunc                          fnCallback
);

static void UART_DisableInterrupt
(
    uint8                               ucCh
);

static void UART_InterruptTxProbe
(
    uint8                               ucCh
);

static void UART_InterruptRxProbe
(
    uint8                               ucCh
);

static void UART_InterruptProbe
(
    uint8                               ucCh
);

static sint32 UART_InterruptWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
);

static sint32 UART_InterruptRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
);

static void UART_DmaTxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
);

static void UART_DmaRxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
);

static void UART_DmaProbe
(
    uint8                               ucCh
);

static sint32 UART_DmaTxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const GDMAInformation_t *           psDmacon
);

static sint32 UART_DmaRxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const GDMAInformation_t *           psDmacon
);

static sint32 UART_DmaWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
);

static sint32 UART_DmaRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
);

static sint32 UART_DmaRxTriggerDma
(
    uint8                               ucCh
);


static void UART_DmaRxISR
(
    void * pArg
)
;

/*
***************************************************************************************************
*                                         INTERFACE FUNCTIONS
***************************************************************************************************
*/

sint32 UART_Open
(
    UartParam_t                         * pUartCfg
)
{
    uint8           ucCh    = pUartCfg->sCh;
    sint32          ret     = -1;

    UART_StatusInit(ucCh);

    if (pUartCfg->sPortCfg <= UART_PORT_CFG_MAX)
    {
        ret = UART_Probe(pUartCfg);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }

    return ret;
}

void UART_Close
(
    uint8                              ucCh
)
{
    sint32          iClkBusId;
    SALRetCode_t    ret;

    if (ucCh < UART_CH_MAX)
    {
        /* Disable the UART Interrupt */
        if (uart[ucCh].sOpMode == UART_INTR_MODE)
        {
            UART_DisableInterrupt(ucCh);
        }

        /* Disable the UART controller Bus clock */
        iClkBusId   = (sint32)CLOCK_IOBUS_UART0 + (sint32)ucCh;
        (void)CLOCK_SetIobusPwdn(iClkBusId, TRUE);

        if (uart[ucCh].sOpMode == UART_DMA_MODE)
        {
            /* Disable the GDMA controller Bus clock */
            iClkBusId   = (sint32)CLOCK_IOBUS_DMA_CON0 + (sint32)ucCh;
            (void)CLOCK_SetIobusPwdn(iClkBusId, TRUE);
        }

        ret = UART_ClearGpio(ucCh);

        if (ret != SAL_RET_SUCCESS)
        {
            UART_D("%s ch %d failed to reset port\n", __func__, ucCh);
        }

        /* Disable the UART ch */
        SAL_WriteReg(NULL, MCU_BSP_UART_BASE + (0x10000UL * (ucCh)) + UART_REG_CR);

        /* Disable the GDMA */
        if (uart[ucCh].sOpMode == UART_DMA_MODE)
        {
            GDMA_ChannelDisable(&uart[ucCh].sRxDma);
            GDMA_ChannelDisable(&uart[ucCh].sTxDma);
        }

        /* Initialize UART Structure */
        SAL_MemSet(&uart[ucCh], 0, sizeof(UartStatus_t));

        /* UART SW Reset */
        (void)UART_Reset(ucCh);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

sint32 UART_Read
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    sint32  ret;
    sint8   getc_err;

    ret = -1;

    if ((pucBuf != NULL_PTR) && (ucCh < UART_CH_MAX))
    {
        if (OFF != uart[ucCh].sIsProbed)
        {
            if (uart[ucCh].sOpMode == UART_DMA_MODE)
            {
                ret = UART_DmaRead(ucCh, pucBuf, uiSize);
            }
            else if (uart[ucCh].sOpMode == UART_INTR_MODE)
            {
                ret = UART_InterruptRead(ucCh, pucBuf, uiSize);
            }
            else
            {
                for (i = 0 ; i < uiSize ; i++)
                {
                    ret = UART_GetChar(ucCh, 0, (sint8 *)&getc_err);

                    if (ret >= 0)
                    {
                        pucBuf[i] = (uint8)((uint32)ret & 0xFFUL);
                    }
                    else
                    {
                        break;
                    }
                }

                ret = (sint32)i;
            }
        }
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }

    return ret;
}

sint32 UART_Write
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    sint32  ret;

    ret = -1;

    if (pucBuf != NULL_PTR)
    {
        if (OFF != uart[ucCh].sIsProbed)
        {
            if (ucCh >= UART_CH_MAX)
            {
                (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
            }
            else
            {
                if (uart[ucCh].sOpMode == UART_DMA_MODE)
                {
                    ret = UART_DmaWrite(ucCh, pucBuf, uiSize);
                }
                else if (uart[ucCh].sOpMode == UART_INTR_MODE)
                {
                    ret = UART_InterruptWrite(ucCh, pucBuf, uiSize);
                }
                else
                {
                    for (i = 0; i < uiSize; i++)
                    {
                        ret = UART_PutChar(ucCh, pucBuf[i]);
                    }
                }
            }
        }
    }

    return ret;
}

uint32 UART_GetData
(
    uint8                               ucCh,
    sint32                              iWait,
    sint8 *                             pcErr
)
{
    /* Use for Lin */
    uint32  data;
    uint32  ret;
    sint32  timeout;

    data    = 0;
    ret     = 0;
    timeout = 0;

    if (pcErr != NULL_PTR)
    {
        if ((ucCh >= UART_CH_MAX) || (OFF == uart[ucCh].sIsProbed))
        {
            (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
            *pcErr = (sint8)-1;
        }
        else
        {
            while ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) != 0UL)
            {
                timeout++;

                if( timeout > iWait )
                {
                    *pcErr = (sint8)-1;
                    break;
                }
            }

            if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) == 0UL)
            {
                data    = UART_RegRead(ucCh, UART_REG_DR);
                data    &= 0x7FFFFFFFU;
                ret     = data;
            }
        }
    }

    return ret;
}

sint32 UART_GetChar
(
    uint8                               ucCh,
    sint32                              iWait,
    sint8 *                             pcErr
)
{
    uint32  data;
    sint32  ret;

    data    = 0;
    ret     = -1;

    if (pcErr != NULL_PTR)
    {
        if ((ucCh >= UART_CH_MAX) || (OFF == uart[ucCh].sIsProbed))
        {
            (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
            *pcErr = (sint8)-1;
        }
        else
        {
            if (iWait == 1)
            {
                while ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) != 0UL)
                {
                    if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) == 0UL)
                    {
                        break;
                    }
                }

                data    = UART_RegRead(ucCh, UART_REG_DR);
                data    &= 0x7FFFFFFFU;
                ret     = (sint32) data;
            }
            else
            {
                if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) != 0UL)
                {
                    *pcErr = (sint8)(-1);
                }
                else
                {
                    data    = UART_RegRead(ucCh, UART_REG_DR);
                    data    &= 0x7FFFFFFFU;
                    ret     = (sint32) data;
                }
            }
        }
    }

    return ret;
}

sint32 UART_PutChar
(
    uint8                               ucCh,
    uint8                               ucChar
)
{
    sint32  ret;

    ret = -1;

    if ((ucCh >= UART_CH_MAX) || (OFF == uart[ucCh].sIsProbed))
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
    else
    {
        while ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_TXFF) != 0UL)
        {
            if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_TXFF) == 0UL)
            {
                break;
            }
        }

        UART_RegWrite(ucCh, UART_REG_DR, ucChar);
        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

void UART_SetLineControlReg
(
    uint8                               ucCh,
    uint32                              uiBits,
    uint8                               uiEnabled
)
{
    uint32  uiLcrh;

    uiLcrh = UART_RegRead(ucCh, UART_REG_LCRH);

    if (uiEnabled == 1U)
    {
        uiLcrh = uiLcrh | uiBits;
    }
    else // (uiEnabled == 0U)
    {
        uiLcrh = uiLcrh & ~(uiBits);
    }

    UART_RegWrite(ucCh, UART_REG_LCRH, uiLcrh);
}

void UART_SetInterruptClearReg
(
    uint8                               ucCh,
    uint32                              uiSetValue
)
{
    UART_RegWrite(ucCh, UART_REG_ICR, uiSetValue);
}

void UART_SetErrorClearReg
(
    uint8                               ucCh,
    uint32                              uiSetValue
)
{
    UART_RegWrite(ucCh, UART_REG_ECR, uiSetValue);
}

uint32 UART_GetReceiveStatusReg
(
    uint8                               ucCh
)
{
    uint32  uiRet;
    uiRet = UART_RegRead(ucCh, UART_REG_RSR);

    return uiRet;
}

uint32 UART_GetRawInterruptStatusReg
(
    uint8                               ucCh
)
{
    uint32  uiRet;
    uiRet = UART_RegRead(ucCh, UART_REG_RIS);

    return uiRet;
}

void UART_Init
(
    void
)
{
    UART_Param.sCh          = UART_DEBUG_CH;
    UART_Param.sPriority    = GIC_PRIORITY_NO_MEAN;
    UART_Param.sBaudrate    = 115200U;
    UART_Param.sMode        = UART_POLLING_MODE;
    UART_Param.sCtsRts      = UART_CTSRTS_OFF;
#if ( VCP_MODULE_BOARD == 1 )
    UART_Param.sPortCfg     = 2U;
#else
    UART_Param.sPortCfg     = 4U;
#endif
    UART_Param.sWordLength  = WORD_LEN_8;
    UART_Param.sFIFO        = DISABLE_FIFO;
    UART_Param.s2StopBit    = TWO_STOP_BIT_OFF;
    UART_Param.sParity      = PARITY_SPACE;
    UART_Param.sFnCallback  = NULL_PTR;

    // For LOG output
    (void)UART_Close(UART_Param.sCh);
    (void)UART_Open(&UART_Param);
}

void UART_ISR
(
    void *                              pArg
)
{
    const UartStatus_t *  uartStat = NULL_PTR;
    uint32          status;
    uint32          imsc;
    sint32          max_count;

    (void) SAL_MemCopy(&uartStat, &pArg, sizeof(UartStatus_t *));
    max_count   = (sint32)UART_BUFF_SIZE;

    if (uartStat != NULL_PTR)
    {
        imsc    = UART_RegRead(uartStat->sCh, UART_REG_IMSC);
        status  = UART_RegRead(uartStat->sCh, UART_REG_MIS) & imsc;

        if (status != 0UL)
        {
            do
            {
                UART_RegWrite(uartStat->sCh, UART_REG_ICR, status & ~(UART_INT_RXIS | UART_INT_RTIS | UART_INT_TXIS));

                if ((status & (UART_INT_RTIS | UART_INT_RXIS)) != 0UL)
                {
                    if (uart[uartStat->sCh].sOpMode == UART_DMA_MODE)
                    {
                        UART_DmaRxIrq(uartStat->sCh);
                    }
                    else
                    {
                        (void)UART_Rx(uartStat->sCh);
                    }

                    UART_RegWrite(uartStat->sCh, UART_REG_ICR, UART_INT_RXIS | UART_INT_RTIS);
                }

                if ((status & UART_INT_TXIS) != 0UL)
                {
                    (void)UART_Tx(uartStat->sCh);
                }

                status = UART_RegRead(uartStat->sCh, UART_REG_MIS) & imsc;

                if (max_count > (sint32)0)
                {
                    max_count--;
                }
                else
                {
                    break;
                }
            } while (status != 0UL);
        }
    }
}

/*
***************************************************************************************************
*                                       STATIC FUNCTIONS
***************************************************************************************************
*/

static uint32 UART_RegRead
(
    uint8                               ucCh,
    uint32                              uiAddr
)
{
    uint32 uiRet;
    uint32 uiBaseAddr;
    uint32 uiRegAddr;

    uiRet       = 0;

    if (uart[ucCh].sBase == 0UL)
    {
        uart[ucCh].sBase = UART_GET_BASE(ucCh);
    }

    uiBaseAddr  = uart[ucCh].sBase & 0xAFFFFFFFU;
    uiAddr      &= 0xFFFFU;
    uiRegAddr   = uiBaseAddr + uiAddr;
    uiRet       = SAL_ReadReg(uiRegAddr);

    return uiRet;
}

static void UART_RegWrite
(
    uint8                               ucCh,
    uint32                              uiAddr,
    uint32                              uiSetValue
)
{
    uint32 uiBaseAddr;
    uint32 uiRegAddr;

    if (uart[ucCh].sBase == 0UL)
    {
        uart[ucCh].sBase = UART_GET_BASE(ucCh);
    }

    uiBaseAddr  = uart[ucCh].sBase & 0xAFFFFFFFU;
    uiAddr      &= 0xFFFFU;
    uiRegAddr   = uiBaseAddr + uiAddr;
    SAL_WriteReg(uiSetValue, uiRegAddr);
}

/*
***************************************************************************************************
*                                          UART_Reset
*
* Function to reset UART Channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t UART_Reset
(
    uint8                               ucCh
)
{
    SALRetCode_t tRet;
    sint32 iRet;
    sint32 iClkBusId;

    tRet = SAL_RET_SUCCESS;
    iClkBusId   = (sint32)CLOCK_IOBUS_UART0 + (sint32)ucCh;

    /* SW reset */
    iRet = CLOCK_SetSwReset(iClkBusId, TRUE);

    if (iRet != (sint32)NULL)
    {
        tRet = SAL_RET_FAILED;
    }
    else
    {
        /* Bit Clear */
        iRet = CLOCK_SetSwReset(iClkBusId, FALSE);

        if(iRet != (sint32)NULL)
        {
            tRet = SAL_RET_FAILED;
        }
    }

    return tRet;
}

static sint32 UART_SetGpio
(
    uint8                               ucCh,
    const UartBoardPort_t *             psInfo
)
{
    sint32          ret;
    SALRetCode_t    ret1;
    SALRetCode_t    ret2;
    SALRetCode_t    ret3;
    SALRetCode_t    ret4;
    SALRetCode_t    retCfg;

    ret = -2;
    retCfg = SAL_RET_FAILED;

    if (psInfo != NULL_PTR)
    {
        /* set port controller, channel */
        switch (ucCh)
        {
            case UART_CH0:
                retCfg = GPIO_PerichSel(GPIO_PERICH_SEL_UARTSEL_0, psInfo->bPortCH);
                break;
            case UART_CH1:
                retCfg = GPIO_PerichSel(GPIO_PERICH_SEL_UARTSEL_1, psInfo->bPortCH);
                break;
            case UART_CH2:
                retCfg = GPIO_PerichSel(GPIO_PERICH_SEL_UARTSEL_2, psInfo->bPortCH);
                break;
            case UART_CH3:
                retCfg = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL0, GPIO_MFIO_UART3, GPIO_MFIO_CFG_CH_SEL0, psInfo->bPortCH);
                break;
            case UART_CH4:
                retCfg = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL1, GPIO_MFIO_UART4, GPIO_MFIO_CFG_CH_SEL1, psInfo->bPortCH);
                break;
            case UART_CH5:
                retCfg = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL2, GPIO_MFIO_UART5, GPIO_MFIO_CFG_CH_SEL2, psInfo->bPortCH);
                break;
            default:
                retCfg = SAL_RET_FAILED;
                break;
        }

        if (retCfg != SAL_RET_FAILED)
        {
            /* set debug port */
            ret1 = GPIO_Config(psInfo->bPortTx, (psInfo->bPortFs)); // TX
            ret2 = GPIO_Config(psInfo->bPortRx, (psInfo->bPortFs | GPIO_INPUT | GPIO_INPUTBUF_EN)); // RX

            uart[ucCh].sPort.bPortCfg = psInfo->bPortCfg;
            uart[ucCh].sPort.bPortTx = psInfo->bPortTx;
            uart[ucCh].sPort.bPortRx = psInfo->bPortRx;
            uart[ucCh].sPort.bPortFs = psInfo->bPortFs;

            if (uart[ucCh].sCtsRts != 0UL)
            {
                ret3 = GPIO_Config(psInfo->bPortRts, psInfo->bPortFs); // RTS
                ret4 = GPIO_Config(psInfo->bPortCts, psInfo->bPortFs); // CTS

                if ((ret1 != SAL_RET_SUCCESS) || (ret2 != SAL_RET_SUCCESS) || (ret3 != SAL_RET_SUCCESS) || (ret4 != SAL_RET_SUCCESS))
                {
                    ret = -1;
                }
                else
                {
                    uart[ucCh].sPort.bPortCts = psInfo->bPortCts;
                    uart[ucCh].sPort.bPortRts = psInfo->bPortRts;
                }
            }
            else if ((ret1 != SAL_RET_SUCCESS) || (ret2 != SAL_RET_SUCCESS))
            {
                ret = -1;
            }
            else
            {
                ret = 0;
            }
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static sint32 UART_SetPortConfig
(
    uint8                               ucCh,
    uint32                              uiPort
)
{
    uint32  idx;
    sint32  ret = 0;
    static const UartBoardPort_t board_serial[UART_PORT_TBL_SIZE] =
    {
        { 0UL,  GPIO_GPA(28UL),   GPIO_GPA(29UL),   TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(1UL),   GPIO_PERICH_CH0 },  // CTL_0, CH_0
        { 1UL,  GPIO_GPC(16UL),   GPIO_GPC(17UL),   GPIO_GPC(18UL),   GPIO_GPC(19UL),   GPIO_FUNC(2UL),   GPIO_PERICH_CH1 },  // CTL_0, CH_1

        { 2UL,  GPIO_GPB(8UL),    GPIO_GPB(9UL),    GPIO_GPB(10UL),   GPIO_GPB(11UL),   GPIO_FUNC(1UL),   GPIO_PERICH_CH0 },  // CTL_1, CH_0
        { 3UL,  GPIO_GPA(6UL),    GPIO_GPA(7UL),    GPIO_GPA(8UL),    GPIO_GPA(9UL),    GPIO_FUNC(2UL),   GPIO_PERICH_CH1 },  // CTL_1, CH_1

        { 4UL,  GPIO_GPB(25UL),   GPIO_GPB(26UL),   GPIO_GPB(27UL),   GPIO_GPB(28UL),   GPIO_FUNC(1UL),   GPIO_PERICH_CH0 },  // CTL_2, CH_0
        { 5UL,  GPIO_GPC(0UL),    GPIO_GPC(1UL),    GPIO_GPC(2UL),    GPIO_GPC(3UL),    GPIO_FUNC(2UL),   GPIO_PERICH_CH1 },  // CTL_2, CH_1

        { 6UL,  GPIO_GPA(16UL),   GPIO_GPA(17UL),   GPIO_GPA(18UL),   GPIO_GPA(19UL),   GPIO_FUNC(3UL),   GPIO_MFIO_CH0 },    // CTL_3, CH_0
        { 7UL,  GPIO_GPB(0UL),    GPIO_GPB(1UL),    GPIO_GPB(2UL),    GPIO_GPB(3UL),    GPIO_FUNC(3UL),   GPIO_MFIO_CH1 },    // CTL_3, CH_1
        { 8UL,  GPIO_GPC(4UL),    GPIO_GPC(5UL),    GPIO_GPC(6UL),    GPIO_GPC(7UL),    GPIO_FUNC(3UL),   GPIO_MFIO_CH2 },    // CTL_3, CH_2
        { 9UL,  GPIO_GPK(11UL),   GPIO_GPK(12UL),   GPIO_GPK(13UL),   GPIO_GPK(14UL),   GPIO_FUNC(3UL),   GPIO_MFIO_CH3 },    // CTL_3, CH_3

        { 10UL, GPIO_GPA(20UL),   GPIO_GPA(21UL),   GPIO_GPA(22UL),   GPIO_GPA(23UL),   GPIO_FUNC(3UL),   GPIO_MFIO_CH0 },    // CTL_4, CH_0
        { 11UL, GPIO_GPB(4UL),    GPIO_GPB(5UL),    GPIO_GPB(6UL),    GPIO_GPB(7UL),    GPIO_FUNC(3UL),   GPIO_MFIO_CH1 },    // CTL_4, CH_1
        { 12UL, GPIO_GPC(8UL),    GPIO_GPC(9UL),    GPIO_GPC(10UL),   GPIO_GPC(11UL),   GPIO_FUNC(3UL),   GPIO_MFIO_CH2 },    // CTL_4, CH_2

        { 13UL, GPIO_GPA(24UL),   GPIO_GPA(25UL),   GPIO_GPA(26UL),   GPIO_GPA(27UL),   GPIO_FUNC(3UL),   GPIO_MFIO_CH0 },    // CTL_5, CH_0
        { 14UL, GPIO_GPB(8UL),    GPIO_GPB(9UL),    GPIO_GPB(10UL),   GPIO_GPB(11UL),   GPIO_FUNC(3UL),   GPIO_MFIO_CH1 },    // CTL_5, CH_1
        { 15UL, GPIO_GPC(12UL),   GPIO_GPC(13UL),   GPIO_GPC(14UL),   GPIO_GPC(15UL),   GPIO_FUNC(3UL),   GPIO_MFIO_CH2 },    // CTL_5, CH_2
    };

    if ((uiPort < UART_PORT_CFG_MAX) && (ucCh < UART_CH_MAX))
    {
        for (idx = 0UL ; idx < UART_PORT_CFG_MAX ; idx++)
        {
            if (board_serial[idx].bPortCfg == uiPort)
            {
                ret = UART_SetGpio(ucCh, &board_serial[idx]);
                break;
            }
        }
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0U, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }

    return ret;
}

static SALRetCode_t UART_ClearGpio
(
    uint8                               ucCh
)
{
    uint32          gpio_Tx = 0;
    uint32          gpio_Rx = 0;
    uint32          gpio_Cts = 0;
    uint32          gpio_Rts = 0;
    SALRetCode_t    ret, ret1, ret2;

    ret = SAL_RET_SUCCESS;
    ret1 = SAL_RET_SUCCESS;
    ret2 = SAL_RET_SUCCESS;

    if(ucCh >= UART_CH_MAX)
    {
        ret = SAL_RET_FAILED;
    }
    else
    {
        gpio_Tx = uart[ucCh].sPort.bPortTx;
        gpio_Rx = uart[ucCh].sPort.bPortRx;

        /* Reset gpio */
        ret1 = GPIO_Config(gpio_Tx, GPIO_FUNC(0UL));
        ret2 = GPIO_Config(gpio_Rx, GPIO_FUNC(0UL));

        if((ret1 != SAL_RET_SUCCESS) || (ret2 != SAL_RET_SUCCESS))
        {
            ret = SAL_RET_FAILED;
        }
        else
        {
            if (uart[ucCh].sCtsRts == ON)
            {
                gpio_Cts = uart[ucCh].sPort.bPortCts;
                gpio_Rts = uart[ucCh].sPort.bPortRts;

                ret1 = GPIO_Config(gpio_Cts, GPIO_FUNC(0UL));
                ret2 = GPIO_Config(gpio_Rts, GPIO_FUNC(0UL));

                if ((ret1 != SAL_RET_SUCCESS) || (ret2 != SAL_RET_SUCCESS))
                {
                    ret = SAL_RET_FAILED;
                }
            }
        }

        if ((ret != SAL_RET_FAILED) && (ucCh >= UART_CH3))
        {
            /* Reset MFIO Configuration */
            if (ucCh == UART_CH3)
            {
                ret = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL0, GPIO_MFIO_DISABLE, GPIO_MFIO_CFG_CH_SEL0, GPIO_MFIO_CH0);
            }
            else if (ucCh == UART_CH4)
            {
                ret = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL1, GPIO_MFIO_DISABLE, GPIO_MFIO_CFG_CH_SEL1, GPIO_MFIO_CH0);
            }
            else if (ucCh == UART_CH5)
            {
                ret = GPIO_MfioCfg(GPIO_MFIO_CFG_PERI_SEL2, GPIO_MFIO_DISABLE, GPIO_MFIO_CFG_CH_SEL2, GPIO_MFIO_CH0);
            }
            else
            {
                // Nothing to do
            }
        }
    }

    return ret;
}

static sint32 UART_SetChannelConfig
(
    UartParam_t                         * pUartCfg
)
{
    uint8   ucCh;
    uint8   ucWordLength = (uint8)pUartCfg->sWordLength;
    uint32  cr_data = 0;
    uint32  lcr_data = 0;
    sint32  ret;
    sint32  iClkBusId;
    sint32  iClkPeriId;

    ucCh = pUartCfg->sCh;
    /* Enable the UART controller peri clock */
    iClkBusId   = (sint32)CLOCK_IOBUS_UART0 + (sint32)ucCh;
    (void)CLOCK_SetIobusPwdn(iClkBusId, SALDisabled);
    iClkPeriId  = (sint32)CLOCK_PERI_UART0 + (sint32)ucCh;
    ret         = CLOCK_SetPeriRate(iClkPeriId, UART_DEBUG_CLK);
    (void)CLOCK_EnablePeri(iClkPeriId);

    if (ret == 0)
    {
        (void)UART_SetBaudRate(ucCh, pUartCfg->sBaudrate);

        // line control setting
        // Word Length
        ucWordLength &= 0x3U;
        pUartCfg->sWordLength = (UartWordLen_t)ucWordLength;
        lcr_data |= UART_LCRH_WLEN((uint32)pUartCfg->sWordLength);

        // Enables FIFOs
        if (pUartCfg->sFIFO == ENABLE_FIFO)
        {
            lcr_data |= UART_LCRH_FEN;
        }

        // Two Stop Bits
        if (pUartCfg->s2StopBit == ON)
        {
            lcr_data |= UART_LCRH_STP2;
        }

        // Parity Enable
        switch (pUartCfg->sParity)
        {
            case PARITY_SPACE:
                lcr_data &= ~(UART_LCRH_PEN);
                break;
            case PARITY_EVEN:
                lcr_data |= ((UART_LCRH_PEN | UART_LCRH_EPS) & ~(UART_LCRH_SPS));
                break;
            case PARITY_ODD:
                lcr_data |= ((UART_LCRH_PEN & ~(UART_LCRH_EPS)) & ~(UART_LCRH_SPS));
                break;
            case PARITY_MARK:
                lcr_data |= ((UART_LCRH_PEN & ~(UART_LCRH_EPS)) | UART_LCRH_SPS);
                break;
            default:
                (void)SAL_DbgReportError(SAL_DRVID_UART, 0U, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
                break;
        }

        UART_RegWrite(ucCh, UART_REG_LCRH, lcr_data);

            // control register setting
            cr_data = UART_CR_EN;
            cr_data |= UART_CR_TXE;
            cr_data |= UART_CR_RXE;

            if (uart[ucCh].sCtsRts != 0UL)
            { //brace
                cr_data |= (UART_CR_RTSEN | UART_CR_CTSEN);
            }

        UART_RegWrite(ucCh, UART_REG_CR, cr_data);
    }

    return ret;
}

static sint32 UART_SetBaudRate
(
    uint8                               ucCh,
    uint32                              uiBaud
) // (uint32 => sint32)return type mismatched
{
    uint32  u32_div;
    uint32  mod;
    uint32  brd_i;
    uint32  brd_f;
    uint32  pclk;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        // Read the peri clock
        pclk = CLOCK_GetPeriRate((sint32)CLOCK_PERI_UART0 + (sint32)ucCh);

        if (pclk == 0UL)
        {
            ret = -1;
        }
        else
        {
            // calculate integer baud rate divisor
            u32_div = 16UL * uiBaud;
            brd_i   = pclk / u32_div;
            UART_RegWrite(ucCh, UART_REG_IBRD, brd_i);

            // calculate faction baud rate divisor
            // NOTICE : fraction maybe need sampling
            uiBaud  &= 0xFFFFFFU;
            mod     = (pclk % (16UL * uiBaud)) & 0xFFFFFFU;
            u32_div = ((((1UL << 3UL) * 16UL) * mod) / (16UL * uiBaud));
            brd_f   = u32_div / 2UL;
            UART_RegWrite(ucCh, UART_REG_FBRD, brd_f);
            ret = (sint32)SAL_RET_SUCCESS;
        }
    }
    return ret;
}

static void UART_StatusInit
(
    uint8                               ucCh
)
{
    uart[ucCh].sIsProbed                = OFF;
    uart[ucCh].sBase                    = UART_GET_BASE(ucCh);
    uart[ucCh].sCh                      = ucCh;
    uart[ucCh].sOpMode                  = UART_POLLING_MODE;
    uart[ucCh].sCtsRts                  = 0;
    uart[ucCh].sWordLength              = WORD_LEN_5;
    uart[ucCh].s2StopBit                = 0;
    uart[ucCh].sParity                  = PARITY_SPACE;

    /* Interupt mode init */
    uart[ucCh].sTxIntr.iXmitBuf         = NULL;
    uart[ucCh].sTxIntr.iHead            = -1;
    uart[ucCh].sTxIntr.iTail            = -1;
    uart[ucCh].sTxIntr.iSize            = 0;
    uart[ucCh].sRxIntr.iXmitBuf         = NULL;
    uart[ucCh].sRxIntr.iHead            = -1;
    uart[ucCh].sRxIntr.iTail            = -1;
    uart[ucCh].sRxIntr.iSize            = 0;

    /* DMA mode init */
    uart[ucCh].sTxDma.iCon              = 0xFF;
    uart[ucCh].sTxDma.iCh               = 0xFF;
    uart[ucCh].sTxDma.iSrcAddr          = NULL;
    uart[ucCh].sTxDma.iDestAddr         = NULL;
    uart[ucCh].sTxDma.iBufSize          = GDMA_BUFF_SIZE;
    uart[ucCh].sTxDma.iTransSize        = 0;
    uart[ucCh].sRxDma.iCon              = 0xFF;
    uart[ucCh].sRxDma.iCh               = 0xFF;
    uart[ucCh].sRxDma.iSrcAddr          = NULL;
    uart[ucCh].sRxDma.iDestAddr         = NULL;
    uart[ucCh].sRxDma.iBufSize          = GDMA_BUFF_SIZE;
    uart[ucCh].sRxDma.iTransSize        = 0;
}

static sint32 UART_Probe
(
    UartParam_t                         * pUartCfg
)
{
    uint8   ucCh;
    sint32  ret = -1;

    ucCh = pUartCfg->sCh;

    if ((ucCh < UART_CH_MAX) && (uart[ucCh].sIsProbed == OFF))
    {
        uart[ucCh].sOpMode      = pUartCfg->sMode;
        uart[ucCh].sCtsRts      = pUartCfg->sCtsRts;

        // Set port config
        ret = UART_SetPortConfig(ucCh, pUartCfg->sPortCfg);

        if (ret != -1)
        {
            ret = UART_SetChannelConfig(pUartCfg);

            if (ret != -1)
            {
                if (uart[ucCh].sOpMode == UART_INTR_MODE) // Configure for interrupt mode
                {
                    UART_InterruptProbe(ucCh);
                    UART_EnableInterrupt(ucCh, pUartCfg->sPriority, pUartCfg->sFIFO, pUartCfg->sFnCallback);
                }
                else if (uart[ucCh].sOpMode == UART_DMA_MODE) // Configure for DMA mode
                {
                    UART_DmaProbe(ucCh);
                    UART_EnableInterrupt(ucCh, pUartCfg->sPriority, pUartCfg->sFIFO, pUartCfg->sFnCallback);
                }
                else
                {
                    // UART_POLLING_MODE
                }

                uart[ucCh].sIsProbed = ON;
            }
        }
    }

    return ret;
}

static sint32 UART_Rx
(
    uint8                               ucCh
)
{
    uint32  status;
    uint32  max_count;
    uint32  data;
    uint8 * buf;
    sint32  ret;

    max_count   = UART_BUFF_SIZE;
    buf         = NULL;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        buf = uart[ucCh].sRxIntr.iXmitBuf;

        while (max_count > 0UL)
        {
            status = UART_RegRead(ucCh, UART_REG_FR);

            if ((status & UART_FR_RXFE) != 0UL)
            {
                break;
            }

            data = UART_RegRead(ucCh, UART_REG_DR);

            buf[uart[ucCh].sRxIntr.iHead] = (uint8)(data & 0xFFUL);

            if (uart[ucCh].sRxIntr.iHead < 0xFFFF)
            {
                uart[ucCh].sRxIntr.iHead++;
            }

            if (uart[ucCh].sRxIntr.iHead >= uart[ucCh].sRxIntr.iSize)
            {
                uart[ucCh].sRxIntr.iHead = 0;
            }

            max_count--;
        }

        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static sint32 UART_TxChar
(
    uint8                               ucCh,
    uint8                               cChar
)
{
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_TXFF) != 0UL)
        {
            ret = -1;
        }
        else
        {
            UART_RegWrite(ucCh, UART_REG_DR, cChar);
            ret = (sint32)SAL_RET_SUCCESS;
        }
    }

    return ret;
}

static sint32 UART_Tx
(
    uint8                               ucCh
)
{
    const uint8 * buf;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        buf = uart[ucCh].sTxIntr.iXmitBuf;

        // xmit buffer is empty
        if (uart[ucCh].sTxIntr.iHead == uart[ucCh].sTxIntr.iTail)
        {
            UART_RegWrite(ucCh, UART_REG_ICR, UART_INT_TXIS);
        }
        else
        {
            do
            {
                if (UART_TxChar(ucCh, buf[(uint32)uart[ucCh].sTxIntr.iTail]) != 0)
                {
                    break;
                }

                uart[ucCh].sTxIntr.iTail++;

                if (uart[ucCh].sTxIntr.iTail >= uart[ucCh].sTxIntr.iSize)
                {
                    uart[ucCh].sTxIntr.iTail = 0;
                }
            } while (uart[ucCh].sTxIntr.iHead != uart[ucCh].sTxIntr.iTail);
        }

        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static void UART_DmaRxIrq
(
    uint8                               ucCh
)
{
    uint32  dmacr;

    if (ucCh < UART_CH_MAX)
    {
        dmacr   = UART_RegRead(ucCh, UART_REG_DMACR);
        dmacr   &= ~UART_DMACR_RXDMAE;
        UART_RegWrite(ucCh, UART_REG_DMACR, dmacr);

        UART_RegWrite(ucCh, UART_REG_ICR, UART_INT_OEIS | UART_INT_BEIS | UART_INT_PEIS | UART_INT_FEIS);
        (void)UART_DmaRxTriggerDma(ucCh);
    }
}

static void UART_EnableInterrupt
(
    uint8                               ucCh,
    uint32                              uiPriority,
    uint8                               ucFIFO,
    GICIsrFunc                          fnCallback
)
{
    uint32  i;
    uint32  im = 0UL;

    if (ucCh < UART_CH_MAX)
    {
        (void)GIC_IntVectSet((uint32)GIC_UART0 + ucCh, uiPriority, GIC_INT_TYPE_LEVEL_HIGH, fnCallback, &(uart[ucCh]));
        (void)GIC_IntSrcEn((uint32)GIC_UART0 + ucCh);

        UART_RegWrite(ucCh, UART_REG_ICR, UART_INT_RXIS | UART_INT_TXIS | UART_INT_RTIS);

        for (i = 0UL ; i < (UART_RX_FIFO_SIZE * 2UL) ; ++i)
        {
            if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFF) != 0UL)
            {
                break;
            }

            (void)UART_RegRead(ucCh, UART_REG_DR);
        }

        if (ucFIFO == ENABLE_FIFO)
        {
            im = UART_INT_RTIS;
        }

        im |= UART_INT_RXIS;

        UART_RegWrite(ucCh, UART_REG_IMSC, im);
    }
}

static void UART_DisableInterrupt
(
    uint8                               ucCh
)
{
    if (ucCh < UART_CH_MAX)
    {
        (void)GIC_IntVectSet((uint32)GIC_UART0 + ucCh, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, NULL_PTR, NULL_PTR);
        (void)GIC_IntSrcDis((uint32)GIC_UART0 + ucCh);

        UART_RegWrite(ucCh, UART_REG_ICR, 0x7FF);
    }
}

static void UART_InterruptTxProbe
(
    uint8                               ucCh
)
{
    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sTxIntr.iXmitBuf = uart_buff[ucCh][UART_MODE_TX];
        uart[ucCh].sTxIntr.iSize    = (sint32)UART_BUFF_SIZE;
        uart[ucCh].sTxIntr.iHead    = 0;
        uart[ucCh].sTxIntr.iTail    = 0;
    }
}

static void UART_InterruptRxProbe
(
    uint8                               ucCh
)
{
    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sRxIntr.iXmitBuf = uart_buff[ucCh][UART_MODE_RX];
        uart[ucCh].sRxIntr.iSize    = (sint32)UART_BUFF_SIZE;
        uart[ucCh].sRxIntr.iHead    = 0;
        uart[ucCh].sRxIntr.iTail    = 0;
    }
}

static void UART_InterruptProbe
(
    uint8                               ucCh
)
{
    UART_InterruptTxProbe(ucCh);
    UART_InterruptRxProbe(ucCh);
}

static sint32 UART_InterruptWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    uint32  imsc;
    sint32  ret;

    if (pucBuf != NULL_PTR)
    {
        // copy user buffer to tx buffer
        for (i = 0 ; i < uiSize ; i++)
        {
            uart[ucCh].sTxIntr.iXmitBuf[uart[ucCh].sTxIntr.iHead] = pucBuf[i];
            if (uart[ucCh].sTxIntr.iHead < (sint32)UART_BUFF_SIZE)
            {
                uart[ucCh].sTxIntr.iHead++;
            }

            if(uart[ucCh].sTxIntr.iHead >= uart[ucCh].sTxIntr.iSize)
            {
                uart[ucCh].sTxIntr.iHead = 0;
            }
        }

        ret = UART_Tx(ucCh);

        if (ret == (sint32)SAL_RET_SUCCESS)
        {
            imsc = UART_RegRead(ucCh, UART_REG_IMSC);
            imsc |= UART_INT_TXIS;
            UART_RegWrite(ucCh, UART_REG_IMSC, imsc);
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static sint32 UART_InterruptRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
)
{
    sint32  cnt;
    sint32  i;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        if (uart[ucCh].sRxIntr.iHead > uart[ucCh].sRxIntr.iTail)
        {
            cnt = uart[ucCh].sRxIntr.iHead - uart[ucCh].sRxIntr.iTail;
        }
        else if (uart[ucCh].sRxIntr.iHead < uart[ucCh].sRxIntr.iTail)
        {
            cnt = uart[ucCh].sRxIntr.iSize - uart[ucCh].sRxIntr.iTail;
        }
        else
        {
            cnt = 0;
        }

        uiSize &= 0xFFFFU;
        if (cnt > (sint32)uiSize)
        {
            cnt = (sint32)uiSize;
        }

        // copy rx buffer to user buffer
        for (i = 0 ; i < cnt ; i++)
        {
            pucBuf[i] = uart[ucCh].sRxIntr.iXmitBuf[uart[ucCh].sRxIntr.iTail];
            uart[ucCh].sRxIntr.iTail++;

            if (uart[ucCh].sRxIntr.iTail >= uart[ucCh].sRxIntr.iSize)
            {
                uart[ucCh].sRxIntr.iTail = 0;
            }
        }

        ret = cnt;
    }

    return ret;
}

static void UART_DmaTxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
)
{
    uint32 regData;

    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sTxDma.iCon          = ucCh;
        uart[ucCh].sTxDma.iCh           = (uint32)GDMA_PERI_TX;
        uart[ucCh].sTxDma.iSrcAddr      = (uint8 *)(puiAddr);
        uart[ucCh].sTxDma.iDestAddr     = (uint8 *)(uart[ucCh].sBase);
        uart[ucCh].sTxDma.iBufSize      = GDMA_BUFF_SIZE;
        uart[ucCh].sTxDma.iTransSize    = 0;

        (void)GDMA_Init(&uart[ucCh].sTxDma, GIC_PRIORITY_NO_MEAN);

        // Enable Transmit DMA
        regData = (UART_RegRead(ucCh, UART_REG_DMACR) | UART_DMACR_DMAONERR | UART_DMACR_TXDMAE);
        UART_RegWrite(ucCh, UART_REG_DMACR, regData);

        GDMA_SetFlowControl(&uart[ucCh].sTxDma, GDMA_FLOW_TYPE_M2P);
        GDMA_SetAddrIncrement(&uart[ucCh].sTxDma, GDMA_NO_INC, GDMA_INC);
        GDMA_SetBurstSize(&uart[ucCh].sTxDma, GDMA_BURST_SIZE_1, GDMA_BURST_SIZE_1);

        GDMA_SetPeri(&uart[ucCh].sTxDma, (uint8)GDMA_PERI_REQ_PORT_UART1_TX, 0U);

        (void)UART_DmaTxEnable(ucCh, uart[ucCh].sTxDma.iBufSize, (const GDMAInformation_t *)&uart[ucCh].sTxDma);
    }
}

static void UART_DmaRxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
)
{
    uint32 regData;

    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sRxDma.iCon          = ucCh;
        uart[ucCh].sRxDma.iCh           = (uint32)GDMA_PERI_RX;
        uart[ucCh].sRxDma.iSrcAddr      = (uint8 *)(uart[ucCh].sBase);
        uart[ucCh].sRxDma.iDestAddr     = (uint8 *)(puiAddr);
        uart[ucCh].sRxDma.iBufSize      = GDMA_BUFF_SIZE;
        uart[ucCh].sRxDma.iTransSize    = 0UL;
        uart[ucCh].sRxDma.fpIsrCallbackForComplete    = UART_DmaRxISR;

        (void)GDMA_Init(&uart[ucCh].sRxDma, GIC_PRIORITY_NO_MEAN);

        // Enable Receive DMA
        regData = (UART_RegRead(ucCh, UART_REG_DMACR) | UART_DMACR_DMAONERR | UART_DMACR_RXDMAE);
        UART_RegWrite(ucCh, UART_REG_DMACR, regData);

        GDMA_SetFlowControl(&uart[ucCh].sRxDma, GDMA_FLOW_TYPE_P2M);
        GDMA_SetAddrIncrement(&uart[ucCh].sRxDma, GDMA_INC, GDMA_NO_INC);
        GDMA_SetBurstSize(&uart[ucCh].sRxDma, GDMA_BURST_SIZE_1, GDMA_BURST_SIZE_1);

        GDMA_SetPeri(&uart[ucCh].sRxDma, 0U, (uint8)GDMA_PERI_REQ_PORT_UART1_RX);

        (void)UART_DmaRxEnable(ucCh, uart[ucCh].sRxDma.iBufSize, (const GDMAInformation_t *)&uart[ucCh].sRxDma);
    }
}

static void UART_DmaProbe
(
    uint8                               ucCh
)
{
    uint32          uiDmaRxAddr;
    uint32          uiDmaTxAddr;
    static uint32 * dma_tx_buf;
    static uint32 * dma_rx_buf;

    if (ucCh < UART_CH_MAX)
    {
        uiDmaRxAddr = MPU_GetDMABaseAddress() + GDMA_ADDRESS_UNIT_CH_RX(ucCh);
        uiDmaTxAddr = MPU_GetDMABaseAddress() + GDMA_ADDRESS_UNIT_CH_TX(ucCh);

        (void) SAL_MemCopy(&dma_rx_buf, &uiDmaRxAddr, sizeof(uint32 *));
        (void) SAL_MemCopy(&dma_tx_buf, &uiDmaTxAddr, sizeof(uint32 *));

        if ((dma_tx_buf != NULL_PTR) && (dma_rx_buf != NULL_PTR))
        {
            (void)SAL_MemSet((void *)dma_rx_buf, 0, GDMA_BUFF_SIZE);
            (void)SAL_MemSet((void *)dma_tx_buf, 0, GDMA_BUFF_SIZE);

            UART_DmaTxProbe(ucCh, dma_tx_buf);
            UART_DmaRxProbe(ucCh, dma_rx_buf);
        }
        else
        {
            (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_NOT_SUPPORT, __FUNCTION__);
        }
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

static sint32 UART_DmaTxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const GDMAInformation_t *           psDmacon
)
{
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        uart[ucCh].sTxDma.iSrcAddr  = (uint8 *)(psDmacon->iSrcAddr);
        uart[ucCh].sTxDma.iDestAddr = (uint8 *)(psDmacon->iDestAddr);
        GDMA_SetSrcAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iSrcAddr));
        GDMA_SetDestAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iDestAddr));
        GDMA_InterruptEnable(&uart[ucCh].sTxDma);
        GDMA_SetTransferWidth(&uart[ucCh].sTxDma, GDMA_TRANSFER_SIZE_BYTE, GDMA_TRANSFER_SIZE_BYTE);
        GDMA_SetTransferSize(&uart[ucCh].sTxDma, uiSize);
        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static sint32 UART_DmaRxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const GDMAInformation_t *           psDmacon
)
{
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        uart[ucCh].sRxDma.iSrcAddr  = (uint8 *)(psDmacon->iSrcAddr);
        uart[ucCh].sRxDma.iDestAddr = (uint8 *)(psDmacon->iDestAddr);

        GDMA_SetSrcAddr(&uart[ucCh].sRxDma, (uint32)(uart[ucCh].sRxDma.iSrcAddr));
        GDMA_SetDestAddr(&uart[ucCh].sRxDma, (uint32)(uart[ucCh].sRxDma.iDestAddr));
        GDMA_InterruptEnable(&uart[ucCh].sRxDma);
        GDMA_SetTransferWidth(&uart[ucCh].sRxDma, GDMA_TRANSFER_SIZE_BYTE, GDMA_TRANSFER_SIZE_BYTE);
        GDMA_SetTransferSize(&uart[ucCh].sRxDma, uiSize);

        // Run DMA
        GDMA_ChannelEnable(&uart[ucCh].sRxDma);
        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static sint32 UART_DmaWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        if (uiSize > GDMA_BUFF_SIZE)
        {
            ret = -1;
        }
        else
        {
            for (i = 0; i < uiSize; i++)
            {
                uart[ucCh].sTxDma.iSrcAddr[i] = pucBuf[i];
            }

            GDMA_SetSrcAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iSrcAddr));
            GDMA_SetDestAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iDestAddr));
            GDMA_InterruptEnable(&uart[ucCh].sTxDma);
            GDMA_SetTransferWidth(&uart[ucCh].sTxDma, GDMA_TRANSFER_SIZE_BYTE, GDMA_TRANSFER_SIZE_BYTE);
            GDMA_SetTransferSize(&uart[ucCh].sTxDma, uiSize);

            // Run DMA
            GDMA_ChannelEnable(&uart[ucCh].sTxDma);
            ret = (sint32)SAL_RET_SUCCESS;
        }
    }

    return ret;
}

static sint32 UART_DmaRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
)
{
    uint32  cnt;
    uint32  post_cnt;
    uint32  i;
    uint32  num_brige;
    uint32  prev_buf;
    uint32  c_size;
    uint32  u_size;
    sint32  ret;

    cnt         = 0;
    post_cnt    = 0;
    num_brige   = 0;
    prev_buf    = 0;
    c_size      = 0;
    u_size      = uiSize;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        c_size = GDMA_GetTransferSize(&uart[ucCh].sRxDma);

        if (uart[ucCh].sRxDma.iTransSize != c_size)
        {
            if (uart[ucCh].sRxDma.iTransSize > c_size)
            {
                cnt         = (uart[ucCh].sRxDma.iTransSize - c_size) & 0xFFFFU;
                prev_buf    = (uart[ucCh].sRxDma.iBufSize - uart[ucCh].sRxDma.iTransSize) & 0xFFFFU;

                if (u_size >= cnt)
                {
                    for (i = 0 ; i < cnt ; i++)
                    {
                        pucBuf[i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf+i]);
                    }

                    uart[ucCh].sRxDma.iTransSize = c_size;
                }
                else //else if(u_size < cnt)
                {
                    for (i = 0 ; i < u_size ; i++)
                    {
                        pucBuf[i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf+i]);
                    }

                    uart[ucCh].sRxDma.iTransSize -=  u_size;
                    cnt = u_size;
                }
                ret = (sint32)cnt;
            }
            else if(c_size > uart[ucCh].sRxDma.iTransSize)
            {
                prev_buf    = (uart[ucCh].sRxDma.iBufSize - uart[ucCh].sRxDma.iTransSize) & 0xFFFFU;
                cnt         = (uart[ucCh].sRxDma.iBufSize - c_size) & 0xFFFFU;
                post_cnt    = (uart[ucCh].sRxDma.iTransSize) & 0xFFFFU;

                if (u_size > post_cnt)
                {
                    u_size  = u_size - post_cnt;

                    for (i= 0UL ; i < post_cnt ; i++)
                    {
                        pucBuf[i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf + i]);
                        num_brige=i+1UL;
                    }

                    if (u_size >= cnt)
                    {
                        for (i = 0UL ; i < cnt ; i++)
                        {
                            num_brige &= 0xFFFFU;
                            pucBuf[num_brige+i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[i]);
                        }

                        uart[ucCh].sRxDma.iTransSize = c_size;
                        cnt += post_cnt;
                    }
                    else // else if (u_size < cnt)
                    {
                        for (i = 0UL ; i < u_size ; i++)
                        {
                            num_brige &= 0xFFFFU;
                            pucBuf[num_brige+i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[i]);
                        }

                        uart[ucCh].sRxDma.iTransSize =  uart[ucCh].sRxDma.iBufSize - u_size;
                        cnt = u_size + post_cnt;
                    }
                }
                else
                {
                    for (i= 0UL ; i < u_size ; i++)
                    {
                        pucBuf[i]   = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf + i]);
                        // num_brige   = i + 1UL; // not used
                    }

                    uart[ucCh].sRxDma.iTransSize -= u_size;
                    cnt = u_size;
                }

                ret = (sint32)cnt;
            }
            else
            {
                ret = 0; // empty statement
            }
        }
        else
        {
            ret = 0; // empty statement
        }
    }

    return ret;
}

static sint32 UART_DmaRxTriggerDma
(
    uint8                               ucCh
)
{
    uint32  dmacr;
    uint32  im;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        dmacr = UART_RegRead(ucCh, UART_REG_DMACR);
        dmacr |= UART_DMACR_RXDMAE;
        UART_RegWrite(ucCh, UART_REG_DMACR, dmacr);

        im = UART_RegRead(ucCh, UART_REG_IMSC);
        im |= ~UART_INT_RXIS;
        UART_RegWrite(ucCh, UART_REG_IMSC, im);

        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static void UART_DmaRxISR(void * pArg)
{
    GDMAInformation_t * dmacon = NULL_PTR;

    (void)SAL_MemCopy(&dmacon, &pArg, sizeof(GDMAInformation_t *));

    if (dmacon != NULL_PTR)
    {
        GDMA_SetDestAddr(dmacon, (uint32)dmacon->iDestAddr);
        GDMA_InterruptEnable(dmacon);
        GDMA_SetTransferSize(dmacon, dmacon->iBufSize);

        GDMA_ChannelEnable(dmacon);
    }
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_UART == 1 )


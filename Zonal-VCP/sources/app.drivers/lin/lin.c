// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : lin.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_LIN == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <lin.h>
#include <lin_reg.h>
#include <reg_phys.h>
#include <gpio.h>
#include <pmio.h>

//#include <gdma.h>
#include <uart_drv.h>
#include <uart.h>
#include <bsp.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/* LIN_READ_LENGTH = break field(1) + sync field(1) + pid(1) + data size(8) + checksum(1) */
#define LIN_READ_LENGTH                 (12U)
#define LIN_MAX_BAUDRATE                (20000U)

/* Lin Header */
#define LIN_HEADER_LENGTH               (3U)
#define LIN_SYNC_BREAK                  (0x00U)
#define LIN_SYNC_FIELD                  (0x55U)

/* UART Config for using as Lin communication*/

#define LIN_MODE                        (UART_POLLING_MODE)
#define LIN_RTSCTS_USE                  (0U)

#define LIN_RESPONSE_WAIT               (5U)
LINResponseIsrFunc                      gpLinResFunc;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static sint8 LIN_TransceiverWakeup
(
    LINConfig_t                         sLinCfg,
    boolean                             bStatus
);

static sint8 LIN_SendHeader
(
    LINConfig_t                         sLinCfg,
    uint8                               ucPid
);

static sint32 LIN_PollingRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
);

static void delay1us
(
    uint32                              uiNum
);

static void LIN_ISR
(
    void *                              pArg
);


/*
***************************************************************************************************
*                                          delay1us
*
* @param        uiNum [in] ucFid is frame identifier
* @return       void
*
* Notes
*
***************************************************************************************************
*/

static void delay1us
(
    uint32                              uiNum
)
{
    uint32  i;
    uint32  val;

    val = uiNum * 500UL;
    for (i = 0UL; i < val; i++)
    {
         BSP_NOP_DELAY();
    }
}

/*
***************************************************************************************************
*                                          LIN_CalcProtectId
*
* @param        ucFid [in] ucFid is frame identifier
* @param        *pucPid [in] pucPid is protected identifier
* @return       sint8 return [out] return LIN_OK or LIN_ERROR
*
* Notes
*         Frame identifier consists of Six bits, Values in the range 0 to 63 can be used.
*
*         A protected identifier field consists of two sub-fields; the frame identifier and the parity.
*         Bits 0 to 5 are the frame identifier and bits 6 and 7 are the parity.
*
***************************************************************************************************
*/

sint8 LIN_CalcProtectId
(
    uint8                               ucFid,
    uint8 *                             pucPid
)
{
    sint8 cRet;
    uint8 ucP0;
    uint8 ucP1;

    cRet    = LIN_OK;
    ucP0    = 0;
    ucP1    = 0;

    if (ucFid > 63UL)
    {
        LIN_E("frame id value should be from 0 to 63. current frame id is [%d] \n", ucFid);
        cRet = LIN_ERROR;

        return cRet;
    }

    if( pucPid == NULL_PTR)
    {
        LIN_E("Pointer Parameter is NULL\n");
        cRet = LIN_ERROR;

        return cRet;
    }

    /* P0 = D0 + D1 + D2 + D4 */
    ucP0 = (ucFid & 0x01U) + ((ucFid >> 1U) & 0x01U) + ((ucFid >> 2U) & 0x01U) + ((ucFid >> 4U) & 0x01U);
    ucP0 = ucP0 & 0x01U;

    /* P1 = D1 + D3 + D4 + D5 */
    ucP1 = ((ucFid >> 1U) & 0x01U) + ((ucFid >> 3U) & 0x01U) + ((ucFid >> 4U) & 0x01U) + ((ucFid >> 5U) & 0x01U);
    ucP1 = ~(ucP1 & 0x01U);

    *pucPid = (uint8)(((uint32)ucP1 << 7U) | ((uint32)ucP0 << 6U) | (uint32)ucFid);

    LIN_D("Frame identifier [0x%02X], Protected identifier [0x%02X] \n", ucFid , *pucPid);

    return cRet;
}

/*
***************************************************************************************************
*                                          LIN_CalcFrameId
*
* @param        ucPid [in] ucPid is protected identifier
* @param        *pucFid [out] pucFid is frame identifier,
* @return       sint8 return [out] return LIN_OK or LIN_ERROR
*
* Notes
*
*
***************************************************************************************************
*/

sint8 LIN_CalcFrameId
(
    uint8                               ucPid,
    uint8 *                             pucFid
)
{
    sint8 cRet;
    cRet    = LIN_OK;

    if( pucFid == NULL_PTR)
    {
        LIN_E("Pointer Parameter is NULL\n");
        cRet = LIN_ERROR;

        return cRet;
    }

    *pucFid = ucPid & (uint8)0x3F;

    if (*pucFid > 63UL)
    {
        LIN_E("frame id value should be from 0 to 63. current frame id is [%d] \n", *pucFid);
        cRet = LIN_ERROR;

        return cRet;
    }

    LIN_D("Frame identifier [0x%02X], Protected identifier [0x%02X] \n", *pucFid , ucPid);

    return cRet;
}




/*
***************************************************************************************************
*                                          LIN_CalClassicChecksum
*
* @param        pucData [in] Data
* @param        ucSize [in] Size of data
* @return       uint8 [out] Classic checksum
*
* Notes
*         Checksum calculation over the data bytes only is called classic checksum and
*         it is used for the master request frame, slave response frame and
*         communication with LIN 1.x slaves.
*         Frame identifiers 60 (0x3C) to 61 (0x3D) shall always use classic checksum.
*
***************************************************************************************************
*/

uint8 LIN_CalClassicChecksum
(
    uint8 *                             pucData,
    uint8                               ucSize
)
{
    uint8   ucI;
    uint8   ucChecksum;
    uint32  uhwTemp;

    ucI         = 0;
    ucChecksum  = LIN_ERROR;
    uhwTemp     = 0;

    if((pucData != NULL_PTR) && (ucSize != 0))
    {
        for ( ; ucI < ucSize ; ucI++)
        {
            uhwTemp += pucData[ucI];
            if (uhwTemp > 0xFFU)
            {
                uhwTemp &= 0x00FFU;
                uhwTemp += 0x0001U;
            }
        }

        ucChecksum = (uint8)(~uhwTemp);

        LIN_D("ucChecksum : 0x%02x", ucChecksum);
    }
    else
    {
        LIN_E("Receive Wrong Parameter");
    }

    return ucChecksum;
}


/*
***************************************************************************************************
*                                          LIN_CalEnhancedChecksum
*
* @param        ucPid [in] protected identifier
* @param        pucData [in] send data
* @param        ucSize [in] size of send data
* @return       uint8 [out] Enhanced checksum
*
* Notes
*         The checksum model used in the LIN specification versions starting from LIN 2.0
*         for all frames,except the diagnostic frames.
*         The enhanced checksum includes the protected identifier and the data bytes.
*
***************************************************************************************************
*/

uint8 LIN_CalEnhancedChecksum
(
    uint8                               ucPid,
    uint8 *                             pucData,
    uint8                               ucSize
)
{
    uint8   ucI;
    uint8   ucChecksum;
    uint32  uhwTemp;

    ucI         = 0;
    ucChecksum  = LIN_ERROR;
    uhwTemp     = ucPid;

    if((pucData != NULL_PTR) && (ucSize != 0))
    {
        for ( ; ucI < ucSize ; ucI++)
        {
            uhwTemp += pucData[ucI];
            if (uhwTemp > 0xFFU)
            {
                uhwTemp &= 0x00FFU;
                uhwTemp += 0x0001U;
            }
        }

        ucChecksum = (uint8)(~uhwTemp);

        LIN_D("ucChecksum : 0x%02x", ucChecksum);
    }
    else
    {
        LIN_E("Receive Wrong Parameter");
    }

    return ucChecksum;
}


/*
***************************************************************************************************
*                                          LIN_SendHeader
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @param        ucPid [in] protected identifier
*
* Notes
*         The header starts at the falling edge of the break field and
*         ends after the end of the stop bit of the protected identifier (PID) field.
*
***************************************************************************************************
*/

static sint8 LIN_SendHeader
(
    LINConfig_t                         sLinCfg,
    uint8                               ucPid
)
{
    sint8   ret;
    uint32  ui_delay;
    uint8   ui_SyncField;

    LIN_D("");

    ui_SyncField    = LIN_SYNC_FIELD;

    /* Send Sync Break*/
#ifdef __GNU_C__
    ui_delay = (uint32)(((float)1/(float)sLinCfg.cfgBaud) * 10000 * 14);
#else
    ui_delay = (uint32)(((float)1/(float)sLinCfg.cfgBaud) * 10000 * 14 * 9);
#endif

    SAL_CoreCriticalEnter();
    UART_SetLineControlReg(sLinCfg.cfgLinCh, UART_LCRH_BRK, SALEnabled);
    (void)delay1us(ui_delay);
    UART_SetLineControlReg(sLinCfg.cfgLinCh, UART_LCRH_BRK, SALDisabled);

    SAL_CoreCriticalExit();

    /* Send Sync Field */
    ret = UART_Write(sLinCfg.cfgLinCh, &ui_SyncField, 1);

    /* Send Idetifier */
    ret = UART_Write(sLinCfg.cfgLinCh, (const uint8 *)&ucPid, 1);

    return ret;
}

sint8 LIN_ReadHeader
(
    LINConfig_t                         sLinCfg,
    uint8 *                             pucPid
)
{
    uint8 i;
    sint8 cGetErr;
    sint8 cReadSize;
    uint8 ucRxData;
    uint8 pucRxData[LIN_HEADER_LENGTH] = {0,};
    LINStatus_t status_ret = LIN_ERROR;

    for(i=0; i < LIN_HEADER_LENGTH; i++)
    {
        if(sLinCfg.cfgMode == UART_POLLING_MODE)
        {
             cGetErr     = LIN_OK;
             ucRxData    = (uint8)UART_GetData(sLinCfg.cfgLinCh, (sint32)5000, (sint8 *)&cGetErr);
             if (cGetErr == LIN_ERROR)
             {
                  LIN_E("There is NO Rx Data[i]", i);
                  status_ret = LIN_NO_DATA_INBUS;
                  break;
             }
        }
        else    //UART_INTR_MODE
        {
            cReadSize = UART_Read(sLinCfg.cfgLinCh, &ucRxData, 1);
            if (cReadSize <= 0)
            {
                 LIN_E("There is NO Rx Data[i]", i);
                 status_ret = LIN_NO_DATA_INBUS;
                 break;
            }

        }

        if(ucRxData == LIN_SYNC_BREAK)
        {
            i = 0;
            SAL_MemSet((void*)pucRxData, 0, LIN_HEADER_LENGTH);
        }
        else
        {
            pucRxData[i] = ucRxData;
        }
   }

    /* Clarify Header */
    if(i == LIN_HEADER_LENGTH)
    {
        if((pucRxData[0] == LIN_SYNC_BREAK) && (pucRxData[1] == LIN_SYNC_FIELD))
        {
            *pucPid    = pucRxData[2];
            status_ret = LIN_NO_RESPONSE;
        }
        else
        {
            LIN_E("LIN HEADER ERROR : 0x%02x, 0x%02x\n", pucRxData[0], pucRxData[1]);
            status_ret = LIN_HEADER_ERROR;
        }

    }

    return status_ret;
}

static sint32 LIN_PollingRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
)
{
    uint8 i;
    sint8 ret;
    uint32 uiRxData;


    ret     = LIN_OK;

    for ( i = 0 ; i < uiSize ; i++)
    {

        uiRxData    = UART_GetData(ucCh, (sint32)5000, (sint8 *)&ret); //LIN_POLLING_MODE
        if (ret == LIN_ERROR)
        {
            LIN_E("Fail Read Lin Data\n");
            break;
        }

        LIN_D("i:%d, uiRxData:0x%x\n", i, uiRxData);
        pucBuf[i] = (uint8)uiRxData;
    }

    if( i > 0 )
    {
        ret = (sint8) i;
    }

    return ret;
}

sint8 LIN_ReadResponse
(
    LINConfig_t                         sLinCfg,
    LINData_t *                         psLinRxData
)
{

    sint8 cStatus = 0;
    uint8 ucFid = 0;
    uint8 pucRxData[LIN_MAX_DATA_SIZE+1U] = {0,};
    uint8 ucWaitCnt = 0;
    uint8 ucReadSize = 0;
    uint8 ucRemainData = LIN_MAX_DATA_SIZE+1U;

    LINStatus_t status_ret = LIN_ERROR;
    LINData_t *psLinData = psLinRxData;

    if(psLinRxData == NULL)
    {
        LIN_D("Rx Data Buffer is NULL");
        status_ret = LIN_ERROR;
        return status_ret;
    }

    if(psLinRxData->ldDLC != 0 )
    {
        ucRemainData = psLinRxData->ldDLC + 1U;
    }

    while(1)
    {
        /* Read data , checksum */
        if(sLinCfg.cfgMode == UART_POLLING_MODE)
        {
            cStatus = LIN_PollingRead(sLinCfg.cfgLinCh, &pucRxData[ucReadSize], ucRemainData);

        }
        else // UART_INTR_MODE
        {
            cStatus = UART_Read(sLinCfg.cfgLinCh, &pucRxData[ucReadSize], ucRemainData);
        }

        if(cStatus > 0 )
        {
            ucReadSize += (uint8)cStatus;
            ucRemainData -= (uint8)cStatus;
        }

        if(ucReadSize == psLinRxData->ldDLC + 1U)
        {
            break;
        }

        if(ucWaitCnt > LIN_RESPONSE_WAIT)
        {
            LIN_E("Can NOT receive proper data");
            break;
        }

        ucWaitCnt++;
        (void)delay1us(10);                /* 1ms */
    }


    if (ucReadSize != 0)
    {
        uint8 ucCalChecksum;

        psLinData->ldSize   = ucReadSize - 1U;  /* Remove Checksum */

        /* Clarify Response */
        (void)SAL_MemCopy(psLinData->ldData, pucRxData, psLinData->ldSize);
        psLinData->ldCSum = pucRxData[psLinData->ldSize];

        /* Inspect Checksum */
        (void)LIN_CalcFrameId(psLinData->ldPid, &ucFid);

        if ((ucFid == LIN_DIAGNOSIC_ID1) || (ucFid == LIN_DIAGNOSIC_ID2))
        {
            ucCalChecksum = LIN_CalClassicChecksum(psLinData->ldData, psLinData->ldSize);
        }
        else
        {
            ucCalChecksum = LIN_CalEnhancedChecksum(psLinData->ldPid, psLinData->ldData, psLinData->ldSize);
        }

        if (psLinData->ldCSum != ucCalChecksum)
        {
            LIN_E("LIN Checksum Error ucRxChecksum : 0x%02x, ucCalChecksum : 0x%02x", psLinData->ldCSum, ucCalChecksum);
            status_ret = LIN_CSUM_ERROR;
        }
        else
        {
            status_ret = LIN_OK;
        }
    }
    else
    {
        LIN_E("There is NO Rx Data");
        status_ret = LIN_NO_DATA_INBUS;
    }

    return status_ret;
}

sint8 LIN_FlushRxData
(
    LINConfig_t                         sLinCfg
)
{
    sint8 cRxSize;
    uint8 ucData[UART_BUFF_SIZE];

    cRxSize = UART_Read(sLinCfg.cfgLinCh, ucData, UART_BUFF_SIZE);
    LIN_D("Flush Rx Data [size : %d, data : 0x%02X]\n", cRxSize, ucData);
    return cRxSize;
}

/*
***************************************************************************************************
*                                          LIN_Init
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @return       sint8 [out] LIN_OK is Success. Other value means fail
*
* Notes
*         Lin communication use UART.
*         Wake up Lin Transceiver setting Lin Sleep Pin as high.
*
***************************************************************************************************
*/

sint8 LIN_Init
(
    LINConfig_t                         sLinCfg
)
{
    UartParam_t     sUartCfg;
    sint8   cRet;

    cRet = LIN_OK;

    LIN_D("");

    /* Check valid baudrate */
    if (sLinCfg.cfgBaud > LIN_MAX_BAUDRATE)
    {
        LIN_E("Allow for bit rates up to 20 kBit/sec, current bit rates [%d]bit/s", sLinCfg.cfgBaud);

        return LIN_ERROR;
    }
    else
    {
        sUartCfg.sCh            = sLinCfg.cfgLinCh;
        sUartCfg.sPriority      = sLinCfg.cfgGicPriority;
        sUartCfg.sBaudrate      = sLinCfg.cfgBaud;
        sUartCfg.sMode          = sLinCfg.cfgMode;
        sUartCfg.sCtsRts        = LIN_RTSCTS_USE;
        sUartCfg.sPortCfg       = sLinCfg.cfgPortSel;
        sUartCfg.sWordLength    = WORD_LEN_8;
        sUartCfg.sFIFO          = ENABLE_FIFO;
        sUartCfg.s2StopBit      = TWO_STOP_BIT_OFF;
        sUartCfg.sParity        = PARITY_SPACE;
        sUartCfg.sFnCallback    = NULL_PTR;

        //UART_DecideGpioPort(sLinCfg.uiLinCh, sLinCfg.uiPortSel);
        if(sLinCfg.cfgMode == UART_POLLING_MODE)
        {
            cRet = (sint8)UART_Open(&sUartCfg);
        }
        else if(sLinCfg.cfgMode == UART_INTR_MODE)
        {
            if(sLinCfg.cfgLinMode == LIN_MASTER)
            {
                sUartCfg.sFnCallback    = (GICIsrFunc)&UART_ISR;
                cRet = (sint8)UART_Open(&sUartCfg);
            }
            else
            {
                sUartCfg.sFIFO          = DISABLE_FIFO;
                sUartCfg.sFnCallback    = (GICIsrFunc)&LIN_ISR;
                cRet = (sint8)UART_Open(&sUartCfg);
            }
        }
        else if(sLinCfg.cfgMode == UART_DMA_MODE)
        {
            sUartCfg.sFnCallback    = (GICIsrFunc)&UART_ISR;
            cRet = (sint8)UART_Open(&sUartCfg);
        }

        if (cRet != LIN_OK)
        {
            LIN_E("Fail Init Uart Port for LIN [%d]", cRet);

            return cRet;
        }

        /* If GPIO_GPK11,12 use as lin communication port, GPIO_GPK9,10 need to set as normal port */
        if (sLinCfg.cfgPortSel == LIN_SEL_PMIO_PORT)
        {
            PMIO_SetGpk( PMIO_GPK(12)|PMIO_GPK(11) ); //uart set
        }

        /* Set GPIO port for controlling SLP_N */
        cRet = (sint8)GPIO_Config(sLinCfg.cfgLinSlpPin, (uint32)(GPIO_FUNC(0U) | GPIO_OUTPUT));

        if (cRet != LIN_OK)
        {
            LIN_E("Fail to set LIN_SLP pin as GPIO_OUTPUT");

            return cRet;
        }

        /* Wake up lin transceiver */
        cRet = LIN_TransceiverWakeup(sLinCfg, SALEnabled);
    }

    return cRet;
}


/*
***************************************************************************************************
*                                          LIN_MasterTx
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @param        ucId [in] ucId is frame identifier
* @param        pucData [in] Data for Send
* @param        ucSize [in] Size of Send Data
* @return       sint8 [out] LIN_OK is Success. Other value means fail.
*
* Notes
*
***************************************************************************************************
*/

sint8 LIN_MasterTx
(
    LINConfig_t                         sLinCfg,
    LINData_t                           *psLinTxData
)
{
    sint8 cRet;

    cRet    = LIN_OK;

    /* Send Message Header */
    cRet = LIN_SendHeader(sLinCfg, psLinTxData->ldPid);

    /* Send Data Field */
    cRet = UART_Write(sLinCfg.cfgLinCh, (const uint8 *)psLinTxData->ldData, psLinTxData->ldSize);

    /* Send Checksum */
    cRet = UART_Write(sLinCfg.cfgLinCh, (const uint8 *)(&psLinTxData->ldCSum), 1U);

    return cRet;
}

sint8 LIN_MasterRx
(
    LINConfig_t                         sLinCfg,
    LINData_t                           *psLinTxData
)
{

    sint8 cRet;
    cRet = LIN_OK;

    /* Send Message Header */
    cRet = LIN_SendHeader(sLinCfg, psLinTxData->ldPid);

    return cRet;
}

LINStatus_t LIN_GetStatus
(
    LINConfig_t                         sLinCfg,
    LINData_t                           *psLinRxData
)
{

    LINStatus_t uiStatus = LIN_OK;
    LINData_t *psLinData = psLinRxData;

    do
    {
        uiStatus = LIN_ReadHeader(sLinCfg, &(psLinData->ldPid));

        if(uiStatus == LIN_NO_RESPONSE)
        {
            uiStatus = LIN_ReadResponse(sLinCfg, psLinData);
        }
    }
    while (uiStatus == LIN_NO_DATA_INBUS);

    return uiStatus;
}


/*
***************************************************************************************************
*                                          LIN_TransceiverWakeup
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @param        bStatus [in] SALEnabled : Set Lin Transceiver Wake up
*                            SALDisabled : Set Lin Transceiver Sleep
* @return       sint8 [out] return Read Data Size. If fail to read data, return LIN_ERROR.
*
* Notes
*
***************************************************************************************************
*/

sint8 LIN_TransceiverWakeup
(
    LINConfig_t                         sLinCfg,
    boolean                             bStatus
)
{
    sint8 cRet;

    cRet = LIN_OK;
    cRet = (sint8)GPIO_Set(sLinCfg.cfgLinSlpPin, bStatus);

    if(cRet != LIN_OK)
    {
        LIN_E("Fail to control LIN transceiver Status [%d]", bStatus);
    }

    return cRet;
}


/*
***************************************************************************************************
*                                          LIN_RegisterCB
*
*
***************************************************************************************************
*/
void LIN_RegisterCB
(
    LINResponseIsrFunc                 pLinResFunc
)
{
    if(pLinResFunc != NULL)
    {
        gpLinResFunc = pLinResFunc;
    }
}

/*
***************************************************************************************************
*                                          LIN_ISR
*
*
***************************************************************************************************
*/

void LIN_ISR
(
    void *                              pArg
)
{

    UartStatus_t *  uartStat;

    uint32          status;
    uint32          uiData;
    uint8 *         buf;
    uint32          UartBaseAddr;

    static uint8    cnt;
    static uint8    Receive_Header_Flag;


    uartStat    = pArg;


    if (uartStat != NULL_PTR)
    {

        UartBaseAddr = uartStat->sBase;
        buf = uartStat->sRxIntr.iXmitBuf;

        status = SAL_ReadReg(UartBaseAddr + UART_REG_MIS);

        if((status & (UART_INT_RTIS | UART_INT_RXIS )) != 0UL)
        {
           SAL_WriteReg(status & ~(UART_INT_RXIS | UART_INT_RTIS | UART_INT_TXIS ), UartBaseAddr + UART_REG_ICR);

           while((SAL_ReadReg(UartBaseAddr + UART_REG_FR) & UART_FR_RXFE) != UART_FR_RXFE)      //Receive FIFO NOT empty
           {
                uiData = (SAL_ReadReg(UartBaseAddr + UART_REG_DR) & 0xFFUL );

                buf[uartStat->sRxIntr.iHead] = uiData;
                uartStat->sRxIntr.iHead++;
                if(uartStat->sRxIntr.iHead >= uartStat->sRxIntr.iSize)
                {
                    uartStat->sRxIntr.iHead = 0;
                }

                /* Clarify Lin Header*/
                if(Receive_Header_Flag == TRUE)
                {
                    gpLinResFunc(uartStat, uiData);
                    Receive_Header_Flag = FALSE;
                }

                if( uiData == 0x00U )
                {
                    cnt = 0;
                }
                else
                {
                    cnt++;
                    if((cnt == 1) && (uiData == 0x55U))
                   {
                        Receive_Header_Flag = TRUE;
                   }
               }
           };

           SAL_WriteReg(UART_INT_RXIS | UART_INT_RTIS , UartBaseAddr + UART_REG_ICR);

       }
       else if((status & UART_INT_BEIS) != 0UL)
       {
           LIN_D("UART_INT_BEIS");
           SAL_WriteReg(UART_INT_BEIS , UartBaseAddr + UART_REG_ICR);
       }
       else
       {
           SAL_WriteReg(status & ~(UART_INT_RXIS | UART_INT_RTIS | UART_INT_TXIS ), UartBaseAddr + UART_REG_ICR);
       }
    }
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_LIN == 1 )


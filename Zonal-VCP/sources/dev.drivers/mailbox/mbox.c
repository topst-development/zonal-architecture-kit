// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : mbox.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_MBOX == 1 )

#include <sal_com.h>
#include <sal_internal.h>
#include <mbox.h>
#include <debug.h>

/******************************************************************************
 * Function name:  MBOX_Init
 * Description:    Initialize MBOX driver
 * Parameter:      None
 * Return value:   None
 * Remarks:
 * Requirements:
 ******************************************************************************/
void MBOX_Init
(
    void
)
{
    volatile uint32 * addr;

    /* Flush cmd & data FIFO */
    addr = MBOX_CTR;
    *addr = (*addr | MBOX_CTR_D_FLUSH | MBOX_CTR_FLUSH);

    /* Set OWN_TMN to inform the other processor  */
    addr = MBOX_TMN_STS;
    *addr = (*addr | MBOX_SET_OWN_TMN);
}


/******************************************************************************
 * Function name:  MBOX_DevSendSss
 * Description:    Send a MBOX message
 * Parameter:      =>[input] MBOX intialization
 *                 =>[input] Data for CMD FIFO
 *                 =>[input] Data for DATA FIFO
 *                 =>[input] Size of DATA FIFO
 * Return value:   MBOX_SUCCESS if sending a message is successful
 * Remarks:
 * Requirements:
 ******************************************************************************/
sint32 MBOX_DevSendSss
(
    uint8 ucDev,
    const uint32 *puiCmd,
    const uint32 *puiData,
    uint32 uiLen
)
{
    volatile uint32 *addr;
    volatile uint32 data;
    uint32 i;
    uint32 timeout;
    sint32 ret = MBOX_SUCCESS;


    /* Check parameters */
    if((ucDev != MBOX_INIT) || (puiCmd == NULL_PTR) || ((puiData != NULL_PTR) && (uiLen > MBOX_MSG_DATA_MAX)))
    {
        ret = MBOX_ERR_INVALID_PARAM;
    }
    else
    {
        /* Wait until transmit cmd FIFO is empty */
        addr = MBOX_CMD_STS;
        if((*addr & MBOX_CMD_STS_TXC_E) == 0u)
        {
            ret = MBOX_ERR_DEV_BUSY;
        }
        /* Wait until transmit data FIFO is empty */
        else
        {
            addr = MBOX_TXD_STS;
            if((*addr & MBOX_TXD_STS_E) == 0u)
            {
                ret = MBOX_ERR_DEV_BUSY;
            }
        }

        if(ret == MBOX_SUCCESS)
        {
            /* Disable transmit FIFO output */
            addr = MBOX_CTR;
            *addr = (*addr & ~MBOX_CTR_OEN);

             /* Flush transmit FIFO  output */
            *addr = (*addr | MBOX_CTR_D_FLUSH | MBOX_CTR_FLUSH);
            while (*addr & (MBOX_CTR_D_FLUSH | MBOX_CTR_FLUSH));

            /* Put data to data FIFO */
            if (puiData != NULL_PTR) {
                for (i = 0; i < uiLen; i++) {
                    addr = MBOX_TXD;
                    *addr = puiData[i];
                }

                /* Timeout: 2800 ms */
                timeout = MBOX_TIMEOUT_COUNT;
                do {
                    data = *MBOX_TXD_STS;
                    timeout--;
                    if(timeout == 0U) {
                        ret = MBOX_ERR_TIMEOUT;
                        break;
                    }
                } while (MBOX_GET_TXD_COUNT(data) != i);
            }

            /* Put data to cmd FIFO */
            for (i = 0; i < MBOX_MSG_CMD_COUNT; i++) {
                *MBOX_TXC(i) = puiCmd[i];
            }

            /* Timeout: 3200 ms */
            timeout = MBOX_TIMEOUT_COUNT;
            do {
                data = *MBOX_CMD_STS;

                timeout--;
                if(timeout == 0U) {
                    ret = MBOX_ERR_TIMEOUT;
                    break;
                }
            } while (MBOX_GET_TXC_COUNT(data) != MBOX_MSG_CMD_COUNT);

            /* Enable transmit FIFO output */
            addr = MBOX_CTR;
            *addr = (*addr | MBOX_CTR_OEN);
        }
    }

	return ret;
}

/******************************************************************************
 * Function name:  MBOX_DevRecvSss
 * Description:    Receive a MBOX message
 * Parameter:      =>[input] MBOX intialization
 *                 =>[output] Data for CMD FIFO
 *                 =>[output] Data for DATA FIFO
 * Return value:   MBOX_SUCCESS if receiving a message is successful
 * Remarks:
 * Requirements:
 ******************************************************************************/
sint32 MBOX_DevRecvSss
(
    uint8 ucDev,
    uint32 uiTimeout,
    uint32 *puiCmd,
    uint32 *puiData
)
{
    volatile uint32 data;
    uint32 cnt;
    uint32 i;
    uint32 timeout;
    sint32 ret = MBOX_SUCCESS;

    if ((ucDev != MBOX_INIT) || (puiCmd == NULL_PTR))
    {
        ret = MBOX_ERR_INVALID_PARAM;
    }
    else
    {
        /* Read a cmd FIFO and check if the size is 8 */

        /* 200 ns per run
        Calculate time as uiTimeout */
        timeout = (uiTimeout * MBOX_TIMEOUT_ADJUSTMENET_VALUE);
        do {
            data = *MBOX_CMD_STS;

            timeout--;
            if(timeout == 0U) {
                ret = MBOX_ERR_TIMEOUT;
                break;
            }
        } while ((data & MBOX_CMD_STS_RXC_E) != 0u);

        /* Timeout: 2800 ms */
        timeout = MBOX_TIMEOUT_COUNT;
        do {
            cnt = MBOX_GET_RXC_COUNT(data);

            timeout--;
            if(timeout == 0U) {
                ret = MBOX_ERR_TIMEOUT;
                break;
            }
        } while (cnt != MBOX_MSG_CMD_COUNT);

        /* Get data to data FIFO */
        if(puiData != NULL_PTR) {
            data = *MBOX_RXD_STS;
            cnt = MBOX_GET_RXD_COUNT(data);

            for (i = 0; i < cnt; i++) {
                data = *MBOX_RXD;
                puiData[i] = data;
            }
        }

        /* Get data to cmd FIFO */
        for (i = 0; i < MBOX_MSG_CMD_COUNT; i++) {
            data = *MBOX_RXC(i);
            puiCmd[i] = data;
        }
    }

    /* Flush Maiilbox FIFO */
    if (ret == MBOX_ERR_TIMEOUT) {
        MBOX_Flush();
    }

	return ret;
}

/******************************************************************************
 * Function name:  MBOX_Flush
 * Description:    Flush MBOX CMD & DATA FIFO
 * Parameter:      None
 * Return value:   None
 * Remarks:
 * Requirements:
 ******************************************************************************/
void MBOX_Flush
(
    void
)
{
    volatile uint32 *addr;

    /* Flush cmd & data FIFO */
    addr = MBOX_CTR;
    *addr = (*addr | MBOX_CTR_D_FLUSH | MBOX_CTR_FLUSH);
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_MBOX == 1 )


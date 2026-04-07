// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : midf.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_MIDF == 1 )

#include <sal_internal.h>
#include "midf_dev.h"
#include "midf.h"
#include "debug.h"

#if (DEBUG_ENABLE)
    #define MIDF_LOGD(fmt, args...)     {LOGD(DBG_TAG_MIDF, fmt, ## args)}
    #define MIDF_LOGE(fmt, args...)     {LOGE(DBG_TAG_MIDF, fmt, ## args)}
#else
    #define MIDF_LOGD(fmt, args...)
    #define MIDF_LOGE(fmt, args...)
#endif

/**
 * Filter Register Descriptions
 */
#define MIDF_BASE_L(ch)                 (MIDF_FILTER_BASE + 0x100UL + ((ch) * 0x20UL))
#define MIDF_BASE_H(ch)                 (MIDF_FILTER_BASE + 0x104UL + ((ch) * 0x20UL))
#define MIDF_TOP_L(ch)                  (MIDF_FILTER_BASE + 0x108UL + ((ch) * 0x20UL))
#define MIDF_TOP_H(ch)                  (MIDF_FILTER_BASE + 0x10CUL + ((ch) * 0x20UL))
#define MIDF_ATTRIBUTES(ch)             (MIDF_FILTER_BASE + 0x110UL + ((ch) * 0x20UL))
#define MIDF_ID_ACCESS(ch)              (MIDF_FILTER_BASE + 0x114UL + ((ch) * 0x20UL))

/**
 * Other Definition
 */
#define MIDF_MAX_CH                     (8UL)
#define MIDF_TYPE_ALL                   (MIDF_TYPE_READ|MIDF_TYPE_WRITE)
#define MIDF_ACCESS_DEFAULT             (0x10001UL << MIDF_ID_CR5)    /* CR5 can access whole area */

/**
 * Static Variables
 */
static uint8 ucMidfFilterInitialized    = 0;

/**
 * Initialize MID Filter
 */
/*
***************************************************************************************************
*                                          MIDF_FilterInitialize
*
* Register MID Filter
*
* @param    None
* @return   None
*
* Notes
*
***************************************************************************************************
*/

void MIDF_FilterInitialize
(
    void
)
{
#ifdef MIDF_NEW
    MIDF_TZC400_t *         pMIDCtl;
    MIDFConfigRegister_t *  pMIDCfg;
    uint32                  ch;
    uint32                  access;

    access = 0;

    if (ucMidfFilterInitialized == 0)
    {
        pMIDCtl = (MIDF_TZC400_t *)(MIDF_FILTER_BASE);
        pMIDCfg = (MIDFConfigRegister_t *)MIDF_FILTER_CFG_BASE;

        /* configure Master IDs */
        if ( pMIDCfg != NULL_PTR)
        {
            pMIDCfg->uMID_XS.nReg       = (uint32)((MIDF_ID_CR5  << 20) | (MIDF_ID_CR5  << 16) | (MIDF_ID_AP   << 4 ) | (MIDF_ID_AP   << 0));
            pMIDCfg->uMID_HS2.nReg      = (uint32)((MIDF_ID_JTAG << 20) | (MIDF_ID_JTAG << 16) | (MIDF_ID_DMA1 << 12) | (MIDF_ID_DMA1 << 8) | \
                                                   (MIDF_ID_DMA0 << 4 ) | (MIDF_ID_DMA0 << 0));
            pMIDCfg->uMID_HS3.nReg      = (uint32)((MIDF_ID_CAN2 << 20) | (MIDF_ID_CAN2 << 16) | (MIDF_ID_CAN1 << 12) | (MIDF_ID_CAN1 << 8) | \
                                                   (MIDF_ID_CAN0 << 4 ) | (MIDF_ID_CAN0 << 0));
            pMIDCfg->uMID_HS4.nReg[0]   = (uint32)((uint32)(MIDF_ID_GPSB3_4_5 << 28) | (MIDF_ID_GPSB3_4_5 << 24) | (MIDF_ID_GPSB2     << 20) | (MIDF_ID_GPSB2     << 16) | \
                                                           (MIDF_ID_GPSB1     << 12) | (MIDF_ID_GPSB1     << 8 ) | (MIDF_ID_GPSB0     << 4 ) | (MIDF_ID_GPSB0     << 0));
            pMIDCfg->uMID_HS4.nReg[1]   = (uint32)((uint32)(MIDF_ID_GPSB3_4_5 << 12) | (MIDF_ID_GPSB3_4_5 << 8 ) | (MIDF_ID_GPSB3_4_5 << 4 ) | (MIDF_ID_GPSB3_4_5 << 0));
            pMIDCfg->uMID_HS5.nReg[0]   = (uint32)((uint32)(MIDF_ID_UART3_4_5 << 28) | (MIDF_ID_UART3_4_5 << 24) | (MIDF_ID_UART2     << 20) | (MIDF_ID_UART2     << 16) | \
                                                           (MIDF_ID_UART1     << 12) | (MIDF_ID_UART1     << 8 ) | (MIDF_ID_UART0     << 4 ) | (MIDF_ID_UART0     << 0));
            pMIDCfg->uMID_HS5.nReg[1]   = (uint32)((uint32)(MIDF_ID_UART3_4_5 << 12) | (MIDF_ID_UART3_4_5 << 8 ) | (MIDF_ID_UART3_4_5 << 4 ) | (MIDF_ID_UART3_4_5 << 0));
        }

        /* Initialize memory region filter */
        if ( pMIDCtl != NULL_PTR)
        {
            /* clear all region except region 0*/
            ch  = MIDF_MAX_CH;

            while(ch != 0)
            {
                if ( pMIDCtl != NULL_PTR)
                {
                    pMIDCtl->uREGION_CTRL[ch].nReg[0] = 0x00000000U; /*BASE LOW*/
                    pMIDCtl->uREGION_CTRL[ch].nReg[1] = 0x00000000U; /*BASE HIGH*/
                    pMIDCtl->uREGION_CTRL[ch].nReg[2] = 0x00000000U; /*TOP LOW*/
                    pMIDCtl->uREGION_CTRL[ch].nReg[3] = 0x00000000U; /*TOP HIGH*/
                    pMIDCtl->uREGION_CTRL[ch].nReg[4] = 0x00000000U; /*ATTRIBUTE */
                    pMIDCtl->uREGION_CTRL[ch].nReg[5] = 0x00000000U; /*ID ACCESS */
                }

                ch--;
            }

            /* set region0 by default config */
            pMIDCtl->uREGION_CTRL[0].nReg[0] = 0x00000000U; /*BASE LOW*/
            pMIDCtl->uREGION_CTRL[0].nReg[1] = 0x00000000U; /*BASE HIGH*/
            pMIDCtl->uREGION_CTRL[0].nReg[2] = 0xFFFFFFFFU; /*TOP LOW*/
            pMIDCtl->uREGION_CTRL[0].nReg[3] = 0x00000000U; /*TOP HIGH*/

            /*pMIDCtl->uREGION_CTRL[0].nReg[4] = 0x1; *//*ATTRIBUTE : filter_en */
            access = (1<<MIDF_ID_CR5) | (1<<MIDF_ID_JTAG);
            access |= (access<<16);
            pMIDCtl->uREGION_CTRL[0].nReg[5] = access; /*ID ACCESS */
            pMIDCtl->uREGION_CTRL[0].bReg.filter0_en = 0x1;
        }

        ucMidfFilterInitialized = 1;
    }
#else
    uint32  ch;
    uint32  access;
    uint32  regBase;
    uint32  config;

    if (ucMidfFilterInitialized == 0UL)
    {
        /* clear all region */
        ch  = MIDF_MAX_CH;

        while(ch!=0UL)
        {
            regBase = MIDF_ATTRIBUTES(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_BASE_L(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_BASE_H(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_TOP_L(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_TOP_H(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_ID_ACCESS(ch);
            SAL_WriteReg(0x0UL, regBase);
            ch--;
        }

        /* configure Master IDs */
        regBase = MIDF_FILTER_CFG_BASE;
        config  = (uint32)((MIDF_ID_CR5 << (uint32)20UL) | ((uint32)MIDF_ID_CR5 << (uint32)16UL) | ((uint32)MIDF_ID_AP << (uint32)4UL) | ((uint32)MIDF_ID_AP << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x04UL;
        config  = (uint32)(((uint32)MIDF_ID_JTAG << (uint32)20UL) | ((uint32)MIDF_ID_JTAG<<(uint32)16UL) | \
                ((uint32)MIDF_ID_DMA1 << (uint32)12UL) | ((uint32)MIDF_ID_DMA1 << (uint32)8UL) | ((uint32)MIDF_ID_DMA0 << (uint32)4UL) | ((uint32)MIDF_ID_DMA0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x08UL;
        config  = (uint32)(((uint32)MIDF_ID_CAN2 << (uint32)20UL) | ((uint32)MIDF_ID_CAN2<<(uint32)16UL) | \
                ((uint32)MIDF_ID_CAN1 << (uint32)12UL) | ((uint32)MIDF_ID_CAN1 << (uint32)8UL) | ((uint32)MIDF_ID_CAN0 << (uint32)4UL) | ((uint32)MIDF_ID_CAN0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x0CUL;
        config  = (uint32)(((uint32)MIDF_ID_GPSB3_4_5 << (uint32)28UL) | ((uint32)MIDF_ID_GPSB3_4_5 << (uint32)24UL) | ((uint32)MIDF_ID_GPSB2 << (uint32)20UL) | ((uint32)MIDF_ID_GPSB2 << (uint32)16UL) | \
                ((uint32)MIDF_ID_GPSB1 << (uint32)12UL) | ((uint32)MIDF_ID_GPSB1 << (uint32)8UL) | ((uint32)MIDF_ID_GPSB0 << (uint32)4UL) | ((uint32)MIDF_ID_GPSB0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x10UL;
        config  = (uint32)(((uint32)MIDF_ID_GPSB3_4_5 << (uint32)28UL) | ((uint32)MIDF_ID_GPSB3_4_5 << (uint32)24UL) | ((uint32)MIDF_ID_UART2 << (uint32)20UL) | ((uint32)MIDF_ID_UART2 << (uint32)16UL) | \
                ((uint32)MIDF_ID_UART1 << (uint32)12UL) | ((uint32)MIDF_ID_UART1 << (uint32)8UL) | ((uint32)MIDF_ID_UART0 << (uint32)4UL) | ((uint32)MIDF_ID_UART0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        /* set region0 */
        regBase = MIDF_BASE_L(0UL);
        SAL_WriteReg(0x0UL, regBase);
        regBase = MIDF_BASE_H(0UL);
        SAL_WriteReg(0UL, regBase);
        regBase = MIDF_TOP_L(0UL);
        SAL_WriteReg(0xFFFFFFFFU, regBase);
        regBase = MIDF_TOP_H(0UL);
        SAL_WriteReg(0UL, regBase);
        access = (1UL<<MIDF_ID_CR5) | (1UL<<MIDF_ID_JTAG);
        access |= (access<<16);
        regBase = MIDF_ID_ACCESS(0UL);
        SAL_WriteReg(access, regBase);
        regBase = MIDF_ATTRIBUTES(0UL);
        SAL_WriteReg(1UL, regBase);

        ucMidfFilterInitialized = 1;
    }
#endif
}

/*
***************************************************************************************************
*                                          MIDF_RegisterFilterRegion
*
* Register MID Filter
*
* @param    Base address of memory buffer for 'id'. it should be 4kb aligned
* @param    Size of memory buffer for 'id'. it should be 4kb aligned
* @param    id [in] Master id.
* @param    type [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/

int32 MIDF_RegisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
)
{
#ifdef MIDF_NEW
    uint32          attr;
    uint32          access;
    uint32          ch;
    uint32          top;
    uint32          found;
    int32           result;
    MIDF_TZC400_t * pMIDCtl;

    result  = MIDF_SUCCESS;
    pMIDCtl = (MIDF_TZC400_t *)(MIDF_FILTER_BASE);

    if ( ucMidfFilterInitialized == 0 )
    {
        result = MIDF_ERROR_NOT_INITIALIZE;
    }

    if (uiID > MIDF_ID_UART3_4_5)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (((uiMemoryBase&(MIDF_REGION_MIN_SIZE-1)) != 0) || ((uiSize&(MIDF_REGION_MIN_SIZE-1)) != 0))
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    top = uiMemoryBase + (uiSize - 1);

    if (uiMemoryBase >= top)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    found = 0;

    if ((result == MIDF_SUCCESS) && (pMIDCtl != NULL_PTR) )
    {
        ch = MIDF_MAX_CH;

        while (ch!=0)
        {
            /* checking empty region */
            if (pMIDCtl->uREGION_CTRL[ch].bReg.filter0_en == 0)
            {
                found = ch;
            }

            /* checking overlaps of range */
            if ((pMIDCtl->uREGION_CTRL[ch].nReg[2] > uiMemoryBase ) /*TOP LOW*/
                && (pMIDCtl->uREGION_CTRL[ch].nReg[0]  < top)) /*BASE LOW*/
            {
                result = MIDF_ERROR_BAD_RANGES;
                break;
            }

            ch--;
        }

        if (found == 0 )
        {
            result = MIDF_ERROR_NO_RESOURCE;
        }
    }

    if (result == MIDF_SUCCESS)
    {
        attr    = 0;
        access  = 0x00000000;

        if ((uiType&MIDF_TYPE_READ) != 0)
        {
            attr = 1;
            access |= 1U<<(uiID&0xF);
        }

        if ((uiType&MIDF_TYPE_WRITE) != 0)
        {
            attr = 1;
            access |= 1U<<((uiID&0xF) + 16);
        }

        // configure memory id filter
        pMIDCtl->uREGION_CTRL[found].bReg.filter0_en    = 0x0;
        pMIDCtl->uREGION_CTRL[found].nReg[0]            = uiMemoryBase; /*BASE LOW*/
        pMIDCtl->uREGION_CTRL[found].nReg[2]            = top; /*TOP LOW*/
        access |= (MIDF_ACCESS_DEFAULT);
        pMIDCtl->uREGION_CTRL[found].nReg[5]            = access; /*ID ACCESS */
        pMIDCtl->uREGION_CTRL[found].bReg.filter0_en    = attr;
    }
#else
    uint32  attr;
    uint32  access;
    uint32  ch;
    uint32  top;
    uint32  found;
    uint32  regBase;
    uint32  config;
    uint32  regBase1;
    uint32  config1;
    int32   result;

    result = MIDF_SUCCESS;

    if ( ucMidfFilterInitialized == 0UL )
    {
        result = MIDF_ERROR_NOT_INITIALIZE;
    }

    if (uiID > MIDF_ID_UART3_4_5)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (((uiMemoryBase&(MIDF_REGION_MIN_SIZE-1UL)) != 0UL) || ((uiSize&(MIDF_REGION_MIN_SIZE-1UL)) != 0UL))
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    top = uiMemoryBase + (uiSize - 1UL);

    if (uiMemoryBase >= top)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    found = 0;

    if (result == MIDF_SUCCESS)
    {
        ch = MIDF_MAX_CH;

        while (ch!=0UL)
        {
            /* checking empty region */
            regBase = MIDF_ATTRIBUTES(ch);
            config  = SAL_ReadReg(regBase);

            if ((config & (uint32)0x1UL) == 0UL)
            {
                found = ch;
            }

            /* checking overlaps of range */
            regBase     = MIDF_TOP_L(ch);
            config      = SAL_ReadReg(regBase);
            regBase1    = MIDF_BASE_L(ch);
            config1     = SAL_ReadReg(regBase1);

            if ((config > uiMemoryBase) && (config1 < top))
            {
                result = MIDF_ERROR_BAD_RANGES;
                break;
            }

            ch--;
        }

        if (found == 0UL )
        {
            result = MIDF_ERROR_NO_RESOURCE;
        }
    }

    if (result == MIDF_SUCCESS)
    {
        attr    = 0;
        access  = 0x00000000;

        if ((uiType&MIDF_TYPE_READ) != 0UL)
        {
            attr    = 1;
            access  |= (uint32)1UL << (uint32)(uiID & 0xFUL);
        }

        if ((uiType&MIDF_TYPE_WRITE) != 0UL)
        {
            attr    = 1;
            access  |= (uint32)1UL << (uint32)((uiID & 0xFUL) + 16UL);
        }

        regBase = MIDF_ATTRIBUTES(found);
        SAL_WriteReg(0, regBase);
        regBase = MIDF_BASE_L(found);
        SAL_WriteReg(uiMemoryBase, regBase);
        regBase = MIDF_BASE_H(found);
        SAL_WriteReg(0, regBase);
        regBase = MIDF_TOP_L(found);
        SAL_WriteReg(top, regBase);
        regBase = MIDF_TOP_H(found);
        SAL_WriteReg(0, regBase);
        regBase = MIDF_ID_ACCESS(found);
        SAL_WriteReg(access | MIDF_ACCESS_DEFAULT, regBase);
        regBase = MIDF_ATTRIBUTES(found);
        SAL_WriteReg(attr, regBase);
    }
#endif

    return result;
}

/**
 * Unregister MID Filter
 * @param base[in] Base address of memory buffer for 'id'.
 * @param size[in] Size of memory buffer for 'id'.
 * @param id[in] Master id.
 * @param type[in] Read or Write type of memory buffer for 'id'
 * @result MID Result
 */

 /*
***************************************************************************************************
*                                          MIDF_UnregisterFilterRegion
*
* Unregister MID Filter
*
* @param    base [in] Base address of memory buffer for 'id'.
* @param    size [in] Size of memory buffer for 'id'.
* @param    id [in] Master id.
* @param    type [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/

int32 MIDF_UnregisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
)
{
#ifdef MIDF_NEW
    uint32          ch;
    uint32          top;
    uint32          access;
    int32           result;
    MIDF_TZC400_t * pMIDCtl;

    result  = MIDF_SUCCESS;
    pMIDCtl = (MIDF_TZC400_t *)(MIDF_FILTER_BASE);

    if (!ucMidfFilterInitialized)
    {
        result = MIDF_ERROR_NOT_INITIALIZE;
    }

    if (uiID > MIDF_ID_UART3_4_5)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (((uiMemoryBase&(MIDF_REGION_MIN_SIZE - 1)) != 0) || ((uiSize&(MIDF_REGION_MIN_SIZE - 1)) != 0))
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    top = uiMemoryBase + (uiSize - 1);

    if (uiMemoryBase >= top)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if ((result == MIDF_SUCCESS) && (pMIDCtl != NULL_PTR))
    {
        /* find matched region */
        ch = MIDF_MAX_CH;

        while(ch!=0)
        {
            if ((pMIDCtl->uREGION_CTRL[ch].nReg[2] == top ) /*TOP LOW*/
                && (pMIDCtl->uREGION_CTRL[ch].nReg[0]  == uiMemoryBase)) /*BASE LOW*/
            {
                access = MIDF_ACCESS_DEFAULT;

                if ((uiType&MIDF_TYPE_READ) != 0)
                {
                    access |= 1U<<(uiID&0xF);
                }

                if ((uiType&MIDF_TYPE_WRITE) != 0)
                {
                    access |= 1U<<((uiID&0xF) + 16);
                }

                if (pMIDCtl->uREGION_CTRL[ch].nReg[5] != access)
                {
                    result = MIDF_ERROR_BAD_PARAMETERS;
                    break;
                }

                if ((pMIDCtl->uREGION_CTRL[ch].nReg[1] != 0x0) /*BASE HIGH 64bit bus*/
                    && (pMIDCtl->uREGION_CTRL[ch].nReg[3] != 0x0)) /*TOP HIGH 64bit bus*/
                {
                    result = MIDF_ERROR_BAD_RANGES;
                    break;
                }

                pMIDCtl->uREGION_CTRL[ch].bReg.filter0_en   = 0x0;
                pMIDCtl->uREGION_CTRL[ch].nReg[0]           = 0x00000000U; /*BASE LOW*/
                pMIDCtl->uREGION_CTRL[ch].nReg[2]           = 0x00000000U; /*TOP LOW*/
                pMIDCtl->uREGION_CTRL[ch].nReg[5]           = 0x00000000U; /*ID ACCESS */
                break;
            }
            ch--;
        }

        if (ch == 0)
        {
            result = MIDF_ERROR_NOT_FOUND;
        }
    }

#else
    uint32  ch ;
    uint32  top;
    uint32  access;
    uint32  regBase;
    uint32  config;
    uint32  regBase1;
    uint32  config1;
    int32   result;

    result  = MIDF_SUCCESS;

    if (ucMidfFilterInitialized == 0UL)
    {
        result = MIDF_ERROR_NOT_INITIALIZE;
    }

    if (uiID > MIDF_ID_UART3_4_5)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (((uiMemoryBase & (MIDF_REGION_MIN_SIZE - 1UL)) != 0UL) || ((uiSize & (MIDF_REGION_MIN_SIZE - 1UL)) != 0UL))
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    top = uiMemoryBase + (uiSize - 1UL);

    if (uiMemoryBase >= top)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (result == MIDF_SUCCESS)
    {
        /* find matched region */
        ch  = MIDF_MAX_CH;

        while(ch!=0UL)
        {
            regBase     = MIDF_BASE_L(ch);
            config      = SAL_ReadReg(regBase);
            regBase1    = MIDF_TOP_L(ch);
            config1     =  SAL_ReadReg(regBase1);

            if ((config == uiMemoryBase) /*BASE LOW*/
                &&(config1 == top)) /*TOP LOW*/
            {
                access = MIDF_ACCESS_DEFAULT;

                if ((uiType&MIDF_TYPE_READ) != 0UL)
                {
                    access |= (uint32)1UL<<(uint32)(uiID&0xFUL);
                }

                if ((uiType&MIDF_TYPE_WRITE) != 0UL)
                {
                    access |= (uint32)1UL<<(uint32)((uiID&0xFUL) + 16UL);
                }

                regBase = MIDF_ID_ACCESS(ch);
                config  = SAL_ReadReg(regBase);

                if (config != access)
                {
                    result = MIDF_ERROR_BAD_PARAMETERS;
                    break;
                }

                regBase = MIDF_ATTRIBUTES(ch);
                SAL_WriteReg(0, regBase);
                regBase = MIDF_BASE_L(ch);
                SAL_WriteReg(0, regBase);
                regBase = MIDF_TOP_L(ch);
                SAL_WriteReg(0, regBase);
                regBase = MIDF_ID_ACCESS(ch);
                SAL_WriteReg(0, regBase);
                break;
            }

            ch--;
        }

        if (ch == 0UL)
        {
            result = MIDF_ERROR_NOT_FOUND;
        }
    }

#endif

    return result;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_MIDF == 1 )


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : mbox_phy.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/
#include <sal_com.h>
#include "gic.h"
#include "mbox.h"
#include "mbox_phy.h"
#include "mbox_dev.h"

MBOXPinfo_t mboxInfo[MBOX_CH_MAX] =
{
    {(MBOXReg) MBOX_HW3_CR5NS_BASE, GIC_MBOX_SLV_TX, GIC_MBOX_MST_TX, 0, 0, 0, {0, }, NULL, {0, }},
};

static uint32       opened = 0;
static MBOXQueue_t  queue[MBOX_CH_MAX][MBOX_MAX_MULTI_OPEN] = {0, };
static MBOXDvice_t  dlist[MBOX_CH_MAX][MBOX_MAX_MULTI_OPEN] = {0, };

int32 MBOX_PhyReady
(
    MBOXCh_t                            siCh,
    uint32                              ucOpen
)
{
    int32   ret;
    uint32  idx;

    ret = MBOX_SUCCESS;

    if ((siCh > (MBOXCh_t) MBOX_CH_INVAL) && (siCh < MBOX_CH_MAX))
    {
        if (((opened + ucOpen) <= (MBOX_MAX_MULTI_OPEN * ((uint32) MBOX_CH_MAX))) && (ucOpen > 0U) && (ucOpen <= ((uint32) MBOX_MAX_MULTI_OPEN)))
        {
            mboxInfo[siCh].mbOcnt = ucOpen;
            mboxInfo[siCh].mbDlst = &dlist[siCh][0];

            for (idx = 0 ; idx < ucOpen ; idx++)
            {
                mboxInfo[siCh].mbDlst[idx].dvChnl = (MBOXCh_t) MBOX_CH_INVAL;
                mboxInfo[siCh].mbDlst[idx].dvIden = -1;
                mboxInfo[siCh].mbDlst[idx].dvFlag =  0;
                mboxInfo[siCh].mbDlst[idx].dvQueu = &queue[siCh][idx];
            }

            opened += ucOpen;
        }
        else
        {
            ret = MBOX_ERR_MULTI_OPEN;
        }
    }
    else
    {
        ret = MBOX_ERR_ARGUMENT;
    }

    return ret;
}


void MBOX_PhyBitSetCtrl
(
    MBOXCh_t                            siCh,
    uint32                              uiValue
)
{
    MBOX_BITCLR(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG, uiValue);
    MBOX_BITSET(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG, uiValue);
}

void MBOX_PhyBitClearCtrl
(
    MBOXCh_t                            siCh,
    uint32                              uiValue
)
{
    MBOX_BITCLR(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG, uiValue);
}

uint8 MBOX_PhyIsIlevelSet
(
    MBOXCh_t                            siCh
)
{
    return (uint8)(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG & MBOX_BITS02);
}

uint8 MBOX_PhyIsIenSet
(
    MBOXCh_t                            siCh
)
{
    return (uint8)(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG >> MBOX_OFFSET_CTL_IEN) & MBOX_BIT(0U);
}

uint8 MBOX_PhyIsOenSet
(
    MBOXCh_t                            siCh
)
{
    return (uint8)(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG >> MBOX_OFFSET_CTL_OEN) & MBOX_BIT(0U);
}

uint8 MBOX_PhyIsFlushedCmd
(
    MBOXCh_t                            siCh
)
{
    return (uint8)(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG >> MBOX_OFFSET_CTL_CFLUSH) & MBOX_BIT(0U);
}

uint8 MBOX_PhyIsFlushedData
(
    MBOXCh_t                            siCh
)
{
    return (uint8)(mboxInfo[siCh].mbBase->mboxCtlCmd.nREG >> MBOX_OFFSET_CTL_DFLUSH) & MBOX_BIT(0U);
}

uint8 MBOX_PhyCountScmd
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxCtlStr.nREG >> MBOX_OFFSET_CMD_SCOUNT) & MBOX_BITS04);
}

uint8 MBOX_PhyFullScmd
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxCtlStr.nREG >> MBOX_OFFSET_CMD_SFUL) & MBOX_BIT(0U));
}

uint8 MBOX_PhyEmptyScmd
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxCtlStr.nREG >> MBOX_OFFSET_CMD_SEMP) & MBOX_BIT(0U));
}

uint8 MBOX_PhyCountMcmd
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxCtlStr.nREG >> MBOX_OFFSET_CMD_MCOUNT) & MBOX_BITS04);
}

uint8 MBOX_PhyFullMcmd
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxCtlStr.nREG >> MBOX_OFFSET_CMD_MFUL) & MBOX_BIT(0U));
}

uint8 MBOX_PhyEmptyMcmd
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxCtlStr.nREG & MBOX_BIT(0)));
}

uint8 MBOX_PhyEmptyMdata
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxTxStr.nREG >> MBOX_OFFSET_DAT_MEMP) & MBOX_BIT(0U));
}

uint8 MBOX_PhyFullMdata
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxTxStr.nREG >> MBOX_OFFSET_DAT_MFUL) & MBOX_BIT(0U));
}

uint16 MBOX_PhyCountMdata
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxTxStr.nREG & MBOX_BITS16));
}

uint8 MBOX_PhyEmptySdata
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxRxStr.nREG >> MBOX_OFFSET_DAT_SEMP) & MBOX_BIT(0U));
}

uint8 MBOX_PhyFullSdata
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxRxStr.nREG >> MBOX_OFFSET_DAT_SFUL) & MBOX_BIT(0U));
}

uint16 MBOX_PhyCountSdata
(
    MBOXCh_t                            siCh
)
{
    return (uint8)((mboxInfo[siCh].mbBase->mboxRxStr.nREG & MBOX_BITS16));
}

int32 MBOX_PhyWriteCmd
(
    MBOXCh_t                            siCh,
    uint32 *                            puiCmd
)
{
    int32 ret;

    ret = MBOX_SUCCESS;

    if(MBOX_PHY_NULLCHK(mboxInfo[siCh].mbBase))
    {
        if (MBOX_PHY_NULLCHK(puiCmd))
        {
            mboxInfo[siCh].mbBase->mboxCmdTx0.nREG = puiCmd[0];
            mboxInfo[siCh].mbBase->mboxCmdTx1.nREG = puiCmd[1];
            mboxInfo[siCh].mbBase->mboxCmdTx2.nREG = puiCmd[2];
            mboxInfo[siCh].mbBase->mboxCmdTx3.nREG = puiCmd[3];
            mboxInfo[siCh].mbBase->mboxCmdTx4.nREG = puiCmd[4];
            mboxInfo[siCh].mbBase->mboxCmdTx5.nREG = puiCmd[5];
            mboxInfo[siCh].mbBase->mboxCmdTx6.nREG = puiCmd[6];
            mboxInfo[siCh].mbBase->mboxCmdTx7.nREG = puiCmd[7];
        }
        else
        {
            ret = MBOX_ERR_ARGUMENT;
        }
    }
    else
    {
        ret = MBOX_ERR_NOT_INITIALIZE;
    }

    return ret;
}

uint32 MBOX_PhyReadCmd
(
    MBOXCh_t                            siCh
)
{
    uint16  scount;
    uint32  ret;

    ret = 0UL;

    if(MBOX_PHY_NULLCHK(mboxInfo[siCh].mbBase))
    {
        MBOX_PhyFunCall(scount, MBOX_PhyCountScmd(siCh));

        if (MBOX_PHY_VALIDCL(scount))
        {
            mboxInfo[siCh].mbMesg.mmCmnd[0] = mboxInfo[siCh].mbBase->mboxCmdRx0.nREG;
            mboxInfo[siCh].mbMesg.mmCmnd[1] = mboxInfo[siCh].mbBase->mboxCmdRx1.nREG;
            mboxInfo[siCh].mbMesg.mmCmnd[2] = mboxInfo[siCh].mbBase->mboxCmdRx2.nREG;
            mboxInfo[siCh].mbMesg.mmCmnd[3] = mboxInfo[siCh].mbBase->mboxCmdRx3.nREG;
            mboxInfo[siCh].mbMesg.mmCmnd[4] = mboxInfo[siCh].mbBase->mboxCmdRx4.nREG;
            mboxInfo[siCh].mbMesg.mmCmnd[5] = mboxInfo[siCh].mbBase->mboxCmdRx5.nREG;
            mboxInfo[siCh].mbMesg.mmCmnd[6] = mboxInfo[siCh].mbBase->mboxCmdRx6.nREG;
            mboxInfo[siCh].mbMesg.mmCmnd[7] = mboxInfo[siCh].mbBase->mboxCmdRx7.nREG;

            ret = ((uint32) scount) & 0x0000FFFFUL;
        }
    }

    return ret;

}

int32 MBOX_PhyWriteData
(
    MBOXCh_t                            siCh,
    uint32 *                            puiData,
    uint32                              uiLen
)
{
    uint32  i;
    int32   ret;

    ret = MBOX_SUCCESS;

    if(MBOX_PHY_NULLCHK(mboxInfo[siCh].mbBase))
    {
        if (MBOX_PHY_NULLCHK(puiData) && (uiLen <= MBOX_DATA_FIFO_SIZE))
        {
            for (i = 0 ; i < uiLen ; i++)
            {
                mboxInfo[siCh].mbBase->mboxTxFifo.nREG = puiData[i];
            }
        }
        else
        {
            ret = MBOX_ERR_ARGUMENT;
        }
    }
    else
    {
        ret = MBOX_ERR_NOT_INITIALIZE;
    }

    return ret;
}

uint32 MBOX_PhyReadData
(
    MBOXCh_t    siCh
)
{
    uint16  i;
    uint16  scount;
    uint32  ret;

    ret = 0UL;

    if(MBOX_PHY_NULLCHK(mboxInfo[siCh].mbBase))
    {
        MBOX_PhyFunCall(scount, MBOX_PhyCountSdata(siCh));

        if (MBOX_PHY_VALIDDL(scount))
        {
            for (i = 0 ; i < scount ; i++)
            {
                mboxInfo[siCh].mbMesg.mmData[i] = mboxInfo[siCh].mbBase->mboxRxFifo.nREG;
            }

            ret = ((uint32) scount) & 0x0000FFFFUL;
        }
    }

    return ret;
}

int32 MBOX_PhySetOwnTmnSts
(
    MBOXCh_t                            siCh,
    uint32                              uiValue
)
{
    int32 ret;

    ret = MBOX_SUCCESS;

    if(MBOX_PHY_NULLCHK(mboxInfo[siCh].mbBase))
    {
        mboxInfo[siCh].mbBase->mboxCtlSts.nREG = (mboxInfo[siCh].mbBase->mboxCtlSts.nREG & ~MBOX_BIT(0U)) | (uiValue & MBOX_BIT(0U));
    }
    else
    {
        ret = MBOX_ERR_NOT_INITIALIZE;
    }

    return ret;
}

int32 MBOX_PhyGetOwnTmnSts
(
    MBOXCh_t                            siCh,
    uint32 *                            uiValue
)
{
    int32 ret;

    ret = MBOX_SUCCESS;

    if(MBOX_PHY_NULLCHK(mboxInfo[siCh].mbBase))
    {
        *uiValue = (mboxInfo[siCh].mbBase->mboxCtlSts.nREG & MBOX_BIT(0U));
    }
    else
    {
        ret = MBOX_ERR_NOT_INITIALIZE;
    }

    return ret;
}

int32 MBOX_PhyGetOppTmnSts
(
    MBOXCh_t                            siCh,
    uint32 *                            uiValue
)
{
    int32 ret;

    ret = MBOX_SUCCESS;

    if(MBOX_PHY_NULLCHK(mboxInfo[siCh].mbBase))
    {
        *uiValue = (mboxInfo[siCh].mbBase->mboxCtlSts.nREG >> 16U)  & MBOX_BIT(0U);
    }
    else
    {
        ret = MBOX_ERR_NOT_INITIALIZE;
    }

    return ret;
}


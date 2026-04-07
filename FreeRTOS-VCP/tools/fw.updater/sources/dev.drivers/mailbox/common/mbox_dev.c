// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : mbox_dev.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/
#include <sal_com.h>
#include <string.h>
#include "gic.h"
#include "mbox.h"
#include "mbox_phy.h"
#include "mbox_dev.h"

static void MBOX_DevRegisterIrq
(
    MBOXCh_t                            siCh,
    uint32                              uiNo,
    GICIsrFunc                          fnIsr
);

static MBOXIsrFunc_t isrFunct[MBOX_CH_MAX] =
{
    {MBOX_DevIsrReceivedHsmNonSecure, MBOX_DevIsrWriteConfirmHsmNonSecure},
};

static MBOXHashMap_t devIdMap[MBOX_CH_MAX][MBOX_MAX_MULTI_OPEN];

 /* ====================================================================== *
  *  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or       *
  *  code or tables extracted from it, as desired without restriction.     *
  *                                                                        *
  *  First, the polynomial itself and its table of feedback terms.  The    *
  *  polynomial is                                                         *
  *  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0   *
  *                                                                        *
  *  Note that we take it "backwards" and put the highest-order term in    *
  *  the lowest-order bit.  The X^32 term is "implied"; the LSB is the     *
  *  X^31 term, etc.  The X^0 term (usually shown as "+1") results in      *
  *  the MSB being 1.                                                      *
  *                                                                        *
  *  Note that the usual hardware shift register implementation, which     *
  *  is what we're using (we're merely optimizing it by doing eight-bit    *
  *  chunks at a time) shifts bits into the lowest-order term.  In our     *
  *  implementation, that means shifting towards the right.  Why do we     *
  *  do it this way?  Because the calculated CRC must be transmitted in    *
  *  order from highest-order term to lowest-order term.  UARTs transmit   *
  *  characters in order from LSB to MSB.  By storing the CRC this way,    *
  *  we hand it to the UART in the order low-byte to high-byte; the UART   *
  *  sends each low-bit to hight-bit; and the result is transmission bit   *
  *  by bit from highest- to lowest-order term without requiring any bit   *
  *  shuffling on our part.  Reception works similarly.                    *
  *                                                                        *
  *  The feedback terms table consists of 256, 32-bit entries.  Notes:     *
  *                                                                        *
  *      The table can be generated at runtime if desired; code to do so   *
  *      is shown later.  It might not be obvious, but the feedback        *
  *      terms simply represent the results of eight shift/xor opera-      *
  *      tions for all combinations of data and CRC register values.       *
  *                                                                        *
  *      The values must be right-shifted by eight bits by the "updcrc"    *
  *      logic; the shift must be unsigned (bring in zeroes).  On some     *
  *      hardware you could probably optimize the shift in assembler by    *
  *      using byte-swap instructions.                                     *
  *      polynomial $edb88320                                              *
  *                                                                        *
  * ====================================================================== */
static uint32 MBOXCrc32Tab[] =
{
      0x00000000U, 0x77073096U, 0xee0e612cU, 0x990951baU, 0x076dc419U,
      0x706af48fU, 0xe963a535U, 0x9e6495a3U, 0x0edb8832U, 0x79dcb8a4U,
      0xe0d5e91eU, 0x97d2d988U, 0x09b64c2bU, 0x7eb17cbdU, 0xe7b82d07U,
      0x90bf1d91U, 0x1db71064U, 0x6ab020f2U, 0xf3b97148U, 0x84be41deU,
      0x1adad47dU, 0x6ddde4ebU, 0xf4d4b551U, 0x83d385c7U, 0x136c9856U,
      0x646ba8c0U, 0xfd62f97aU, 0x8a65c9ecU, 0x14015c4fU, 0x63066cd9U,
      0xfa0f3d63U, 0x8d080df5U, 0x3b6e20c8U, 0x4c69105eU, 0xd56041e4U,
      0xa2677172U, 0x3c03e4d1U, 0x4b04d447U, 0xd20d85fdU, 0xa50ab56bU,
      0x35b5a8faU, 0x42b2986cU, 0xdbbbc9d6U, 0xacbcf940U, 0x32d86ce3U,
      0x45df5c75U, 0xdcd60dcfU, 0xabd13d59U, 0x26d930acU, 0x51de003aU,
      0xc8d75180U, 0xbfd06116U, 0x21b4f4b5U, 0x56b3c423U, 0xcfba9599U,
      0xb8bda50fU, 0x2802b89eU, 0x5f058808U, 0xc60cd9b2U, 0xb10be924U,
      0x2f6f7c87U, 0x58684c11U, 0xc1611dabU, 0xb6662d3dU, 0x76dc4190U,
      0x01db7106U, 0x98d220bcU, 0xefd5102aU, 0x71b18589U, 0x06b6b51fU,
      0x9fbfe4a5U, 0xe8b8d433U, 0x7807c9a2U, 0x0f00f934U, 0x9609a88eU,
      0xe10e9818U, 0x7f6a0dbbU, 0x086d3d2dU, 0x91646c97U, 0xe6635c01U,
      0x6b6b51f4U, 0x1c6c6162U, 0x856530d8U, 0xf262004eU, 0x6c0695edU,
      0x1b01a57bU, 0x8208f4c1U, 0xf50fc457U, 0x65b0d9c6U, 0x12b7e950U,
      0x8bbeb8eaU, 0xfcb9887cU, 0x62dd1ddfU, 0x15da2d49U, 0x8cd37cf3U,
      0xfbd44c65U, 0x4db26158U, 0x3ab551ceU, 0xa3bc0074U, 0xd4bb30e2U,
      0x4adfa541U, 0x3dd895d7U, 0xa4d1c46dU, 0xd3d6f4fbU, 0x4369e96aU,
      0x346ed9fcU, 0xad678846U, 0xda60b8d0U, 0x44042d73U, 0x33031de5U,
      0xaa0a4c5fU, 0xdd0d7cc9U, 0x5005713cU, 0x270241aaU, 0xbe0b1010U,
      0xc90c2086U, 0x5768b525U, 0x206f85b3U, 0xb966d409U, 0xce61e49fU,
      0x5edef90eU, 0x29d9c998U, 0xb0d09822U, 0xc7d7a8b4U, 0x59b33d17U,
      0x2eb40d81U, 0xb7bd5c3bU, 0xc0ba6cadU, 0xedb88320U, 0x9abfb3b6U,
      0x03b6e20cU, 0x74b1d29aU, 0xead54739U, 0x9dd277afU, 0x04db2615U,
      0x73dc1683U, 0xe3630b12U, 0x94643b84U, 0x0d6d6a3eU, 0x7a6a5aa8U,
      0xe40ecf0bU, 0x9309ff9dU, 0x0a00ae27U, 0x7d079eb1U, 0xf00f9344U,
      0x8708a3d2U, 0x1e01f268U, 0x6906c2feU, 0xf762575dU, 0x806567cbU,
      0x196c3671U, 0x6e6b06e7U, 0xfed41b76U, 0x89d32be0U, 0x10da7a5aU,
      0x67dd4accU, 0xf9b9df6fU, 0x8ebeeff9U, 0x17b7be43U, 0x60b08ed5U,
      0xd6d6a3e8U, 0xa1d1937eU, 0x38d8c2c4U, 0x4fdff252U, 0xd1bb67f1U,
      0xa6bc5767U, 0x3fb506ddU, 0x48b2364bU, 0xd80d2bdaU, 0xaf0a1b4cU,
      0x36034af6U, 0x41047a60U, 0xdf60efc3U, 0xa867df55U, 0x316e8eefU,
      0x4669be79U, 0xcb61b38cU, 0xbc66831aU, 0x256fd2a0U, 0x5268e236U,
      0xcc0c7795U, 0xbb0b4703U, 0x220216b9U, 0x5505262fU, 0xc5ba3bbeU,
      0xb2bd0b28U, 0x2bb45a92U, 0x5cb36a04U, 0xc2d7ffa7U, 0xb5d0cf31U,
      0x2cd99e8bU, 0x5bdeae1dU, 0x9b64c2b0U, 0xec63f226U, 0x756aa39cU,
      0x026d930aU, 0x9c0906a9U, 0xeb0e363fU, 0x72076785U, 0x05005713U,
      0x95bf4a82U, 0xe2b87a14U, 0x7bb12baeU, 0x0cb61b38U, 0x92d28e9bU,
      0xe5d5be0dU, 0x7cdcefb7U, 0x0bdbdf21U, 0x86d3d2d4U, 0xf1d4e242U,
      0x68ddb3f8U, 0x1fda836eU, 0x81be16cdU, 0xf6b9265bU, 0x6fb077e1U,
      0x18b74777U, 0x88085ae6U, 0xff0f6a70U, 0x66063bcaU, 0x11010b5cU,
      0x8f659effU, 0xf862ae69U, 0x616bffd3U, 0x166ccf45U, 0xa00ae278U,
      0xd70dd2eeU, 0x4e048354U, 0x3903b3c2U, 0xa7672661U, 0xd06016f7U,
      0x4969474dU, 0x3e6e77dbU, 0xaed16a4aU, 0xd9d65adcU, 0x40df0b66U,
      0x37d83bf0U, 0xa9bcae53U, 0xdebb9ec5U, 0x47b2cf7fU, 0x30b5ffe9U,
      0xbdbdf21cU, 0xcabac28aU, 0x53b39330U, 0x24b4a3a6U, 0xbad03605U,
      0xcdd70693U, 0x54de5729U, 0x23d967bfU, 0xb3667a2eU, 0xc4614ab8U,
      0x5d681b02U, 0x2a6f2b94U, 0xb40bbe37U, 0xc30c8ea1U, 0x5a05df1bU,
      0x2d02ef8dL
};

/*
***************************************************************************************************
*                                      <MBOX_DevInit>
*
* This function initializes Mailbox hardware.
*
* @param    siCh  [in] A Mailbox channel to be initialized.
* @param    uiIlv [in] An interrupt Level for received command fifo.
* @param    ucOpn [in] The number of virtual devices per Mailbox channel.
* @param    pFunc [in] A pointer of callbacker functions.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes
*
***************************************************************************************************
*/

int32 MBOX_DevInit
(
    MBOXCh_t                            siCh,
    uint32                              uiIlv,
    uint32                              ucOpn,
    MBOXCallback_t *                    pFunc
)
{
    int32 ret;

    ret = MBOX_SUCCESS;
    MBOX_DevFunCall(ret, MBOX_PhyReady(siCh, ucOpn));

    if (ret == MBOX_SUCCESS)
    {
        MBOX_PhyBitSetCtrl(siCh, MBOX_CMD_FLUSH_BIT|MBOX_DAT_FLUSH_BIT);
        MBOX_PhyBitSetCtrl(siCh, MBOX_RD_IEN_BIT|uiIlv);
        MBOX_DevRegisterIrq(siCh, mboxInfo[siCh].mbRirq, isrFunct[siCh].cbRecev);
        (void) MBOX_PhySetOwnTmnSts(siCh, MBOX_ENABLE);

        if (MBOX_DEV_NULLCHK(pFunc))
        {
            mboxInfo[siCh].mbOsfn.osInit = pFunc->osInit;
            mboxInfo[siCh].mbOsfn.wrLock = pFunc->wrLock;
            mboxInfo[siCh].mbOsfn.wrUnlk = pFunc->wrUnlk;
            mboxInfo[siCh].mbOsfn.rsInit = pFunc->rsInit;
            mboxInfo[siCh].mbOsfn.rdWake = pFunc->rdWake;
            mboxInfo[siCh].mbOsfn.rdWait = pFunc->rdWait;
            mboxInfo[siCh].mbOsfn.wsInit = pFunc->wsInit;
            mboxInfo[siCh].mbOsfn.wrWake = pFunc->wrWake;
            mboxInfo[siCh].mbOsfn.wrWclr = pFunc->wrWclr;
            mboxInfo[siCh].mbOsfn.wrWait = pFunc->wrWait;
            MBOX_DevPtrCallVoid(mboxInfo[siCh].mbOsfn.osInit, (uint32 *) &mboxInfo[siCh].mbLock)

            if (MBOX_DEV_NULLCHK(mboxInfo[siCh].mbOsfn.wsInit))
            {
                mboxInfo[siCh].mbOsfn.wsInit((uint32 *) &mboxInfo[siCh].mbSiid);
                MBOX_DevRegisterIrq(siCh, mboxInfo[siCh].mbWirq, isrFunct[siCh].cbWrite);
                MBOX_PhyBitSetCtrl(siCh, MBOX_WR_IEN_BIT);
            }
        }
        else
        {
            MBOX_D("callback is null\n");
        }
    }
    else
    {
        MBOX_E("mbox_phy_ready fail [%d]\n", ret);
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevOpen>
*
* This function creates virtual device for specific mailbox channel.
*
* @param    siCh      [in] A Mailbox channel to be opened.
* @param    pcDevName [in] A virtual device name for specific Mailbox channel.
* @param    ucFlag    [in] An option flags (reserved).
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes
*
***************************************************************************************************
*/

MBOXDvice_t * MBOX_DevOpen
(
    MBOXCh_t                            siCh,
    int8 *                              pcDevName,
    uint8                               ucFlag
)
{
    int32           devId;
    int32           ret;
    MBOXDvice_t *   mboxDev;

    devId   = -1;
    ret     = MBOX_SUCCESS;
    mboxDev = NULL;

    if (MBOX_DEV_VALIDCH(siCh))
    {
        MBOX_DevCallJmpOnErr(ret, mboxInfo[siCh].mbOsfn.wrLock, mboxInfo[siCh].mbLock)
        MBOX_DevFunCall(devId, MBOX_DevNew(siCh, (uint8 *) pcDevName));

        if (MBOX_DEV_NULLCHK(pcDevName) && MBOX_DEV_VALIDID(devId))
        {
            if (MBOX_DEV_NULLCHK(mboxInfo[siCh].mbDlst) && MBOX_DEV_NULLCHK(mboxInfo[siCh].mbDlst[devId].dvQueu))
            {
                mboxInfo[siCh].mbDlst[devId].dvChnl         = siCh;
                mboxInfo[siCh].mbDlst[devId].dvIden         = devId;
                mboxInfo[siCh].mbDlst[devId].dvFlag         = ucFlag;
                mboxInfo[siCh].mbDlst[devId].dvQueu->qiFrnt = -1;
                mboxInfo[siCh].mbDlst[devId].dvQueu->qiRear = -1;
                mboxInfo[siCh].mbDlst[devId].dvName[0]      = (uint8) pcDevName[0];
                mboxInfo[siCh].mbDlst[devId].dvName[1]      = (uint8) pcDevName[1];
                mboxInfo[siCh].mbDlst[devId].dvName[2]      = (uint8) pcDevName[2];
                mboxInfo[siCh].mbDlst[devId].dvName[3]      = (uint8) pcDevName[3];
                mboxInfo[siCh].mbDlst[devId].dvName[4]      = (uint8) pcDevName[4];
                mboxInfo[siCh].mbDlst[devId].dvName[5]      = (uint8) pcDevName[5];
                mboxInfo[siCh].mbDlst[devId].dvName[6]      = (uint8) pcDevName[6];
                mboxDev = &mboxInfo[siCh].mbDlst[devId];
                MBOX_DevPtrCall(ret, mboxInfo[siCh].mbOsfn.rsInit, (uint32 *) &mboxInfo[siCh].mbDlst[devId].dvSiid)
            }
            else
            {
                MBOX_E("open fail siCh[%d] not initialized\n", siCh);
            }
        }
        else
        {
            MBOX_E("open fail siCh[%d] dev[%s] devId[%d]\n", siCh, pcDevName, (sint32) devId);
        }

        MBOX_DevCallJmpOnErr(ret, mboxInfo[siCh].mbOsfn.wrUnlk, mboxInfo[siCh].mbLock)
    }
    else
    {
        MBOX_E("open fail siCh[%d] dev[%d] invalid param\n", (sint32) siCh, (sint32) devId);
    }

err:
    return mboxDev;
}

/*
***************************************************************************************************
*                                      <MBOX_DevSend>
*
* This function sends 8 word command and 128 word data to Application Processor.
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiCmd  [in] A pointer of 8 word command (fixed length).
* @param    puiData [in] A pointer of 128 word data.
* @param    uiLen   [in] A data length of puiData within 128 word.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevSend
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData,
    uint32                              uiLen
)
{
    int32   ret;
    uint32  scmd[8] = {0, };

    ret = MBOX_SUCCESS;

    if (MBOX_DEV_NULLCHK(pDev) && MBOX_DEV_NULLCHK(puiData) && MBOX_DEV_VALIDDL(uiLen))
    {
        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrLock, mboxInfo[pDev->dvChnl].mbLock)

        if (MBOX_DEV_VALIDID(pDev->dvIden) && MBOX_DEV_VALIDCH(pDev->dvChnl))
        {
            if ((MBOX_PhyEmptyMdata(pDev->dvChnl) & MBOX_PhyEmptyMcmd(pDev->dvChnl)) == (uint8) 0x01)
            {
                if (MBOX_PhyWriteData(pDev->dvChnl, puiData, uiLen) == MBOX_SUCCESS)
                {
                    MBOX_DevMakeCtlPacket(scmd, puiCmd, pDev->dvName);
                    MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWclr, mboxInfo[pDev->dvChnl].mbSiid)
                    MBOX_PhyBitClearCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                    MBOX_DevFunCall(ret, MBOX_PhyWriteCmd(pDev->dvChnl, scmd));
                    MBOX_PhyBitSetCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                    MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWait, mboxInfo[pDev->dvChnl].mbSiid)
                }
                else
                {
                    ret = MBOX_ERR_WRITE;
                    MBOX_E("mbox phy write err[%d]\n", ret);
                }
            }
            else
            {
                ret =  MBOX_ERR_FIFO_FULL;
                MBOX_E("no space for uiLen[%d]\n", uiLen);
            }
        }
        else
        {
            ret = MBOX_ERR_DEV_NOT_OPENED;
            MBOX_E("devId %d, ch %d \n", pDev->dvIden, pDev->dvChnl);
        }
        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrUnlk, mboxInfo[pDev->dvChnl].mbLock)
    }
    else
    {
        ret = MBOX_ERR_ARGUMENT;
        MBOX_E("send fail pDev[0x%08X] puiData[0x%08X] uiLen[%d] invalid param\n", pDev, puiData, (sint32) uiLen);
    }

err:
    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevSendSss>
*
* This function sends 8 word command and 128 word data to Hsm (only for SSS Mailbox).
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiCmd  [in] A pointer of 8 word command (fixed length).
* @param    puiData [in] A pointer of 128 word data.
* @param    uiLen   [in] A data length of puiData within 128 word.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevSendSss
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData,
    uint32 uiLen
)
{
    int32  ret;
    uint32 scmd[8]  = {0, };

    ret = MBOX_SUCCESS;

    if (MBOX_DEV_NULLCHK(pDev) && MBOX_DEV_NULLCHK(puiData) && MBOX_DEV_NULLCHK(puiCmd))
    {
        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrLock, mboxInfo[pDev->dvChnl].mbLock)

        if (MBOX_DEV_VALIDID(pDev->dvIden) && MBOX_DEV_VALIDCH(pDev->dvChnl))
        {
            if ((MBOX_PhyEmptyMdata(pDev->dvChnl) & MBOX_PhyEmptyMcmd(pDev->dvChnl)) == (uint8) 0x01)
            {
                if (MBOX_PhyWriteData(pDev->dvChnl, puiData, uiLen) == MBOX_SUCCESS)
                {
                    MBOX_DevMakeCtlPacketSss(scmd, puiCmd, pDev->dvName);
                    MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWclr, mboxInfo[pDev->dvChnl].mbSiid)
                    MBOX_PhyBitClearCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                    MBOX_DevFunCall(ret, MBOX_PhyWriteCmd(pDev->dvChnl, scmd));
                    MBOX_PhyBitSetCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                    MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWait, mboxInfo[pDev->dvChnl].mbSiid)
                }
                else
                {
                    ret = MBOX_ERR_WRITE;
                    MBOX_E("mbox phy write err[%d]\n", ret);
                }
            }
            else
            {
                ret =  MBOX_ERR_FIFO_FULL;
                MBOX_E("no space for uiLen[%d]\n", uiLen);
            }
        }
        else
        {
            ret = MBOX_ERR_DEV_NOT_OPENED;
            MBOX_E("devId %d, ch %d \n", pDev->dvIden, pDev->dvChnl);
        }

        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrUnlk, mboxInfo[pDev->dvChnl].mbLock)
    }
    else
    {
        ret = MBOX_ERR_ARGUMENT;
        MBOX_E("send fail dev[0x%08X] puiData[0x%08X] uiLen[%d] invalid param\n", pDev, puiData, (sint32) uiLen);
    }

err:
    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevSendCmd>
*
* This function sends 8 word command to Application Processor.
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiCmd  [in] A pointer of 8 word command (fixed length).
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevSendCmd
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd
)
{
    int32   ret;
    uint32  scmd[8] = {0, };

    ret  = MBOX_SUCCESS;

    if (MBOX_DEV_NULLCHK(pDev) && MBOX_DEV_NULLCHK(puiCmd))
    {
        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrLock, mboxInfo[pDev->dvChnl].mbLock)

        if (MBOX_DEV_VALIDID(pDev->dvIden) && MBOX_DEV_VALIDCH(pDev->dvChnl))
        {
            if (MBOX_PhyEmptyMcmd(pDev->dvChnl) == (uint8) 0x01)
            {
                MBOX_DevMakeCtlPacket(scmd, puiCmd, pDev->dvName);
                MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWclr, mboxInfo[pDev->dvChnl].mbSiid)
                MBOX_PhyBitClearCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                MBOX_DevFunCall(ret, MBOX_PhyWriteCmd(pDev->dvChnl, scmd));
                MBOX_PhyBitSetCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWait, mboxInfo[pDev->dvChnl].mbSiid)
            }
            else
            {
                ret =  MBOX_ERR_FIFO_FULL;
                MBOX_E("open fail tx cmd fifo full\n");
            }
        }
        else
        {
            ret = MBOX_ERR_DEV_NOT_OPENED;
            MBOX_E("devId %d, ch %d \n", pDev->dvIden, pDev->dvChnl);
        }

        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrUnlk, mboxInfo[pDev->dvChnl].mbLock)
    }
    else
    {
        ret = MBOX_ERR_ARGUMENT;
        MBOX_E("send fail dev[0x%08X] puiCmd[0x%08X] invalid param\n", pDev, puiCmd);
    }

err:
    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevSendData>
*
* This function sends 8 word command and 128 word data to Application Processor.
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiData [in] A pointer of 128 word data.
* @param    uiLen   [in] A data length of puiData within 128 word.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevSendData
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiData,
    uint32                              uiLen
)
{
    int32   ret;
    uint32  scmd[8] = {0, };

    ret = MBOX_SUCCESS;

    if (MBOX_DEV_NULLCHK(pDev) && MBOX_DEV_NULLCHK(puiData) && MBOX_DEV_VALIDDL(uiLen))
    {
        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrLock, mboxInfo[pDev->dvChnl].mbLock)

        if (MBOX_DEV_VALIDID(pDev->dvIden) && MBOX_DEV_VALIDCH(pDev->dvChnl))
        {
            if ((MBOX_PhyEmptyMdata(pDev->dvChnl) & MBOX_PhyEmptyMcmd(pDev->dvChnl)) == (uint8) 0x01)
            {
                if (MBOX_PhyWriteData(pDev->dvChnl, puiData, uiLen) == MBOX_SUCCESS)
                {
                    MBOX_DevMakeCtlPacket(scmd, NULL, pDev->dvName);
                    MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWclr, mboxInfo[pDev->dvChnl].mbSiid)
                    MBOX_PhyBitClearCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                    ret = MBOX_PhyWriteCmd(pDev->dvChnl, scmd);
                    MBOX_PhyBitSetCtrl(pDev->dvChnl, MBOX_OEN_BIT);
                    MBOX_DevPtrCall(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrWait, mboxInfo[pDev->dvChnl].mbSiid)
                }
                else
                {
                    ret = MBOX_ERR_WRITE;
                    MBOX_E("mbox phy write err[%d]\n", ret);
                }
            }
            else
            {
                ret =  MBOX_ERR_FIFO_FULL;
                MBOX_E("[E] %s : no space for uiLen[%d]\n", uiLen);
            }
        }
        else
        {
            ret = MBOX_ERR_DEV_NOT_OPENED;
            MBOX_E("devId %d, ch %d \n", pDev->dvIden, pDev->dvChnl);
        }
        MBOX_DevCallJmpOnErr(ret, mboxInfo[pDev->dvChnl].mbOsfn.wrUnlk, mboxInfo[pDev->dvChnl].mbLock)
    }
    else
    {
        ret = MBOX_ERR_ARGUMENT;
        MBOX_E("send fail dev[0x%08X] puiData[0x%08X] uiLen[%d] invalid param\n", pDev, puiData, (sint32) uiLen);
    }

err:
    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevRecv>
*
* This function receives 8 word command and 128 word data from Application Processor.
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiCmd  [in] A pointer of 128 word data.
* @param    puiData [in] A data length of puiData within 128 word.
* @return   < 0 on error, == 0 if only 8 word command, > 0 data length within 128 word.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevRecv
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData
)
{
    int32 ret;

    ret = (int32) MBOX_DEV_GET_AGAIN;

    if (MBOX_DEV_NULLCHK(pDev) && (MBOX_DEV_NULLCHK(puiCmd) || MBOX_DEV_NULLCHK(puiData)))
    {
        if (MBOX_DEV_VALIDID(pDev->dvIden) && MBOX_DEV_VALIDCH(pDev->dvChnl))
        {
            while (ret == (int32) MBOX_DEV_GET_AGAIN)
            {
                MBOX_DevCallBreakOnErr(ret, (int32) MBOX_DevQget(pDev->dvChnl, pDev->dvIden, pDev->dvQueu, puiCmd, puiData), (int32) MBOX_DEV_GET_AGAIN)
                MBOX_DevCallBreakOnNonBlock(ret, mboxInfo[pDev->dvChnl].mbOsfn.rdWait, pDev->dvSiid, MBOX_SUCCESS)
            }
        }
        else
        {
            ret = MBOX_ERR_DEV_NOT_OPENED;
        }
    }
    else
    {
        ret = MBOX_ERR_ARGUMENT;
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevRecvSss>
*
* This function receives 8 word command and 128 word data from Hsm (only for SSS Mailbox).
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiCmd  [in] A pointer of 128 word data.
* @param    puiData [in] A data length of puiData within 128 word.
* @return   < 0 on error, == 0 if only 8 word command, > 0 data length within 128 word.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevRecvSss
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData
)
{
    int32 ret;

    ret = (int32) MBOX_DEV_GET_AGAIN;

    if (MBOX_DEV_NULLCHK(pDev) && (MBOX_DEV_NULLCHK(puiCmd) || MBOX_DEV_NULLCHK(puiData)))
    {
        if (MBOX_DEV_VALIDID(pDev->dvIden) && MBOX_DEV_VALIDCH(pDev->dvChnl))
        {
            while (ret == (int32) MBOX_DEV_GET_AGAIN)
            {
                MBOX_DevCallBreakOnErr(ret, (int32) MBOX_DevQgetSss(pDev->dvChnl, pDev->dvIden, pDev->dvQueu, puiCmd, puiData), (int32) MBOX_DEV_GET_AGAIN)
                MBOX_DevCallBreakOnNonBlock(ret, mboxInfo[pDev->dvChnl].mbOsfn.rdWait, pDev->dvSiid, MBOX_SUCCESS)
            }
        }
        else
        {
            ret = MBOX_ERR_DEV_NOT_OPENED;
        }
    }
    else
    {
        ret = MBOX_ERR_ARGUMENT;
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevRecvCmd>
*
* This function receives 8 word command and 128 word data from Application Processor.
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiCmd  [in] A pointer of 128 word data.
* @return   < 0 on error, == 0 if only 8 word command.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevRecvCmd
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd
)
{
    return MBOX_DevRecv(pDev, puiCmd, NULL);
}

/*
***************************************************************************************************
*                                      <MBOX_DevRecvData>
*
* This function receives 8 word command and 128 word data from Application Processor.
*
* @param    pDev    [in] A virtual device opened, which data must be sent.
* @param    puiData [in] A data length of puiData within 128 word.
* @return   < 0 on error, > 0 data length within 128 word.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

int32 MBOX_DevRecvData
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiData
)
{
    return MBOX_DevRecv(pDev, NULL, puiData);
}


/*
***************************************************************************************************
*                                      <MBOX_DevMakeCtlPacket>
*
* This function constructs control command to use virtaul device.
*
* @param    puiCtl    [in] A pointer of 8 word target command to be sent.
* @param    puiCmd    [in] A pointer of 8 word source commend to be copied.
* @param    ucpName   [in] A pointer of virtual device name.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void MBOX_DevMakeCtlPacket
(
    uint32 *                            puiCtl,
    uint32 *                            puiCmd,
    uint8 *                             ucpName
)
{
    uint8 * scmd;

    scmd    = (uint8 *) puiCtl;
    MBOX_DevByteCpy(&scmd[2],  &ucpName[4], 1);
    MBOX_DevByteCpy(&scmd[3],  &ucpName[5], 1);
    MBOX_DevByteCpy(&scmd[28], &ucpName[0], 1);
    MBOX_DevByteCpy(&scmd[29], &ucpName[1], 1);
    MBOX_DevByteCpy(&scmd[30], &ucpName[2], 1);
    MBOX_DevByteCpy(&scmd[31], &ucpName[3], 1);

    if (MBOX_DEV_NULLCHK(puiCmd))
    {
        MBOX_DevWordCpy(&puiCtl[1],  puiCmd, 6);
    }
}

/*
***************************************************************************************************
*                                      <MBOX_DevMakeCtlPacket>
*
* This function constructs control command to use virtaul device for Hsm (only for SSS Mailbox).
*
* @param    puiCtl    [in] A pointer of 8 word target command to be sent.
* @param    puiCmd    [in] A pointer of 8 word source commend to be copied.
* @param    ucpName   [in] A pointer of virtual device name.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void MBOX_DevMakeCtlPacketSss
(
    uint32 *                            puiCtl,
    uint32 *                            puiCmd,
    uint8 *                             ucpName
)
{
    uint8 * scmd;
    uint8 * pcmd;

    scmd    = (uint8 *) puiCtl;
    pcmd    = (uint8 *) puiCmd;
    MBOX_DevByteCpy(&scmd[0],  &pcmd[0], 1);
    MBOX_DevByteCpy(&scmd[1],  &pcmd[1], 1);
    MBOX_DevByteCpy(&scmd[2],  &ucpName[4], 1);
    MBOX_DevByteCpy(&scmd[3],  &ucpName[5], 1);
    MBOX_DevByteCpy(&scmd[28], &ucpName[0], 1);
    MBOX_DevByteCpy(&scmd[29], &ucpName[1], 1);
    MBOX_DevByteCpy(&scmd[30], &ucpName[2], 1);
    MBOX_DevByteCpy(&scmd[31], &ucpName[3], 1);
    MBOX_DevWordCpy(&puiCtl[1], &puiCmd[1], 6);
}

/*
***************************************************************************************************
*                                  <MBOX_DevIsrReceivedHsmNonSecure>
*
* This function is called by IRQ interrupt from Applecation Processor Hsm Non-Secure Channel.
*
* @param    arg     [in] not used in this function
* @return
*
* Notes
*
***************************************************************************************************
*/

static void MBOX_DevIsrReceivedHsmNonSecure
(
    void *                              pArg
)
{
    (void) pArg;

    if (MBOX_DevDeploySwbuf(MBOX_CH_HSM_NONSECURE, MBOX_CCNT(MBOX_CH_HSM_NONSECURE)) < 0)
    {
        MBOX_E("deploy data fail\n");
        // send NAK --> with reason :  full, no such device
    }
}

/*
***************************************************************************************************
*                                  <MBOX_DevIsrWriteConfirmHsmNonSecure>
*
* This function is called by IRQ interrupt from Applecation Processor Hsm Non-Secure Channel.
*
* @param    arg     [in] not used in this function
* @return
*
* Notes
*
***************************************************************************************************
*/

static void MBOX_DevIsrWriteConfirmHsmNonSecure
(
    void *                              pArg
)
{
    (void) pArg;
    MBOX_DevPtrCallVoid(mboxInfo[MBOX_CH_HSM_NONSECURE].mbOsfn.wrWake, mboxInfo[MBOX_CH_HSM_NONSECURE].mbSiid)
    MBOX_PhyBitSetCtrl(MBOX_CH_HSM_NONSECURE, MBOX_WR_ICLR_BIT);
}

/*
***************************************************************************************************
*                                      <MBOX_DevDeploySwbuf>
*
* This function puts received data into software buffer.
*
* @param    siCh     [in] A Mailbox channel that data has been transmitted.
* @param    uiLen    [in] A length of received data.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes
*
***************************************************************************************************
*/

static int32 MBOX_DevDeploySwbuf
(
    MBOXCh_t                            siCh,
    uint32                              uiLen
)
{
    int32 ret;
    int32 devId;

    ret     = MBOX_SUCCESS;
    devId   = -1;

    if (MBOX_DEV_NULLCHK(mboxInfo[siCh].mbDlst) && MBOX_DEV_VALIDCL(uiLen))
    {
        MBOX_DevFunCall(devId, MBOX_DevFind(siCh, &mboxInfo[siCh].mbMesg));

        if (MBOX_DEV_VALIDID(devId))
        {
            MBOX_DevFunCall(ret, MBOX_DevQput(siCh, devId, mboxInfo[siCh].mbDlst[devId].dvQueu, MBOX_DCNT(siCh)));
            MBOX_DevPtrCallVoid(mboxInfo[siCh].mbOsfn.rdWake, mboxInfo[siCh].mbDlst[devId].dvSiid)
        }
        else
        {
            ret = MBOX_ERR_DEV_NOT_FOUND;
            MBOX_E("no such device dev[%d]\n", (sint32) devId);
        }
    }
    else
    {
        ret = MBOX_ERR_NOT_INITIALIZE;
        MBOX_E("dev no initialized uiLen[%u]\n", uiLen);
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevQput>
*
* This function puts received data into software queue.
*
* @param    siCh     [in] A Mailbox channel that data has been transmitted.
* @param    siDevId  [in] A virtual device identifier to be opened.
* @param    pQueue   [in] A pointer of queue for data to be stored.
* @param    uiLene   [in] A length of data received.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes
*
***************************************************************************************************
*/
static int32 MBOX_DevQput
(
    MBOXCh_t                            siCh,
    int32                               siDevId,
    MBOXQueue_t *                       pQueue,
    uint32                              uiLen
)
{
    int32 ret;

    ret = MBOX_SUCCESS;

    if (MBOX_DEV_VALIDID(siDevId) && MBOX_DEV_NULLCHK(pQueue))
    {
        if (((pQueue->qiRear + 1) % MBOX_MAX_QUEUE_SIZE) == pQueue->qiFrnt)
        {
            ret = MBOX_ERR_QUEUE_FULL;
            MBOX_E("full for ch[%d] siDevId[%d]\n", (int32) siCh, siDevId);
        }
        else
        {
            pQueue->qiRear = ((pQueue->qiRear + 1) % MBOX_MAX_QUEUE_SIZE);
            pQueue->qiData[pQueue->qiRear].mmDlen = 0;
            MBOX_DevWordCpy(pQueue->qiData[pQueue->qiRear].mmCmnd, mboxInfo[siCh].mbMesg.mmCmnd, MBOX_CMD_FIFO_SIZE);

            if (MBOX_DEV_VALIDDL(uiLen))
            {
                pQueue->qiData[pQueue->qiRear].mmDlen = uiLen;
                MBOX_DevWordCpy(pQueue->qiData[pQueue->qiRear].mmData, mboxInfo[siCh].mbMesg.mmData, uiLen);
            }
        }
    }
    else
    {
        ret = MBOX_ERR_NO_SUCH_DEV;
        MBOX_E("no such device siDev[%d]\n", (sint32) siDevId);
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevQget>
*
* This function gets received data form software queue.
*
* @param    siCh     [in] A Mailbox channel that data has been transmitted.
* @param    siDevId  [in] A virtual device identifier to be opened.
* @param    puiCmd   [in] A pointer of 8 word command to store received command.
* @param    puiData  [in] A pointer of 128 word data to store received data.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

static uint32 MBOX_DevQget
(
    MBOXCh_t                            siCh,
    int32                               siDevId,
    MBOXQueue_t *                       pQueue,
    uint32 *                            puiCmd,
    uint32 *                            puiData
)
{
    uint32 ret;

    (void) siCh;
    ret = MBOX_DEV_GET_AGAIN;

    if (MBOX_DEV_NULLCHK(pQueue) && (MBOX_DEV_NULLCHK(puiCmd) || MBOX_DEV_NULLCHK(puiData)))
    {
        if (pQueue->qiFrnt == pQueue->qiRear)
        {
            MBOX_D("empty for ch[%d] siDevId[%d]\n", (sint32) siCh, siDevId);
        }
        else
        {
            pQueue->qiFrnt = ((pQueue->qiFrnt + 1) % MBOX_MAX_QUEUE_SIZE);
            MBOX_DevWordCpy(puiCmd, &pQueue->qiData[pQueue->qiFrnt].mmCmnd[1], (MBOX_CMD_FIFO_SIZE - 2UL));

            if (MBOX_DEV_VALIDDL(pQueue->qiData[pQueue->qiFrnt].mmDlen))
            {
                MBOX_DevWordCpy(puiData, pQueue->qiData[pQueue->qiFrnt].mmData, pQueue->qiData[pQueue->qiFrnt].mmDlen);
            }

            ret = pQueue->qiData[pQueue->qiFrnt].mmDlen;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevQget>
*
* This function gets received data form software queue with Hsm (only for SSS mailbox).
*
* @param    siCh     [in] A Mailbox channel that data has been transmitted.
* @param    siDevId  [in] A virtual device identifier to be opened.
* @param    puiCmd   [in] A pointer of 8 word command to store received command.
* @param    puiData  [in] A pointer of 128 word data to store received data.
* @return   < 0 on error, otherwise MBOX_SUCCESS.
*
* Notes 1word = 4byte.
*
***************************************************************************************************
*/

static uint32 MBOX_DevQgetSss
(
    MBOXCh_t                            siCh,
    int32                               siDevId,
    MBOXQueue_t *                       pQueue,
    uint32 *                            puiCmd,
    uint32 *                            puiData
)
{
    uint32 ret;

    (void) siCh;
    ret = MBOX_DEV_GET_AGAIN;

    if (MBOX_DEV_NULLCHK(pQueue) && (MBOX_DEV_NULLCHK(puiCmd) || MBOX_DEV_NULLCHK(puiData)))
    {
        if (pQueue->qiFrnt == pQueue->qiRear)
        {
            MBOX_D("empty for ch[%d] siDevId[%d] \n", (sint32) siCh, siDevId);
        }
        else
        {
            pQueue->qiFrnt = ((pQueue->qiFrnt + 1) % MBOX_MAX_QUEUE_SIZE);
            MBOX_DevWordCpy(puiCmd, &pQueue->qiData[pQueue->qiFrnt].mmCmnd[0], (MBOX_CMD_FIFO_SIZE));

            if (MBOX_DEV_VALIDDL(pQueue->qiData[pQueue->qiFrnt].mmDlen))
            {
                MBOX_DevWordCpy(puiData, pQueue->qiData[pQueue->qiFrnt].mmData, pQueue->qiData[pQueue->qiFrnt].mmDlen);
            }

            ret = pQueue->qiData[pQueue->qiFrnt].mmDlen;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevRegisterIrq>
*
* This function enables IRQ and registers callback function to be called by IRQ.
*
* @param    siCh     [in] A Mailbox channel that data has been transmitted.
* @param    uiNo     [in] A interrupt number to call interrupt service routine.
* @param    fnIsr    [in] A pointer of interrupt service routine.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void MBOX_DevRegisterIrq
(
    MBOXCh_t                            siCh,
    uint32                              uiNo,
    GICIsrFunc                          fnIsr
)
{
    (void) siCh;
    (void) GIC_IntVectSet(uiNo, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, fnIsr, (void *) 0);
    (void) GIC_IntSrcEn(uiNo);
}

/*
***************************************************************************************************
*                                      <MBOX_DevIoctl>
*
* This function writes/reads Mailbox registers.
*
* @param    pDev     [in] A pointer of virtual device opened.
* @param    uiNo     [in] A command identifier.
* @param    puiValue [in] A pointer of read/written data from/to registers.
* @return   < 0  on error, otherwise MBOX_SUCCESS.
*
* Notes
*
***************************************************************************************************
*/

int32 MBOX_DevIoctl
(
    MBOXDvice_t *                       pDev,
    uint32                              uiCmd,
    uint32 *                            puiValue
)
{
    int32 ret;

    ret = MBOX_SUCCESS;

    if (MBOX_DEV_NULLCHK(pDev) && MBOX_DEV_NULLCHK(puiValue) && MBOX_DEV_VALIDCH(pDev->dvChnl))
    {
        switch (uiCmd)
        {
            case MBOX_IO_CMD_SET_CTRL_REG :
            {
                MBOX_PhyBitSetCtrl(pDev->dvChnl, *puiValue);
                break;
            }

            case MBOX_IO_CMD_CLR_CTRL_REG :
            {
                MBOX_PhyBitClearCtrl(pDev->dvChnl, *puiValue);
                break;
            }

            case MBOX_IO_CMD_GET_ILVL_BIT :
            {
                *puiValue = (uint32) MBOX_PhyIsIlevelSet(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_INEN_BIT :
            {
                *puiValue = (uint32) MBOX_PhyIsIenSet(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_WOEN_BIT :
            {
                *puiValue = (uint32) MBOX_PhyIsOenSet(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_CFLS_BIT :
            {
                *puiValue = (uint32) MBOX_PhyIsFlushedCmd(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_DFLS_BIT :
            {
                *puiValue = (uint32) MBOX_PhyIsFlushedData(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_RCNT_CMD :
            {
                *puiValue = (uint32) MBOX_PhyCountScmd(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_RFUL_CMD :
            {
                *puiValue = (uint32) MBOX_PhyFullScmd(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_REMT_CMD :
            {
                *puiValue = (uint32) MBOX_PhyEmptyScmd(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_WCNT_CMD :
            {
                *puiValue = (uint32) MBOX_PhyCountMcmd(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_WFUL_CMD :
            {
                *puiValue = (uint32) MBOX_PhyFullMcmd(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_WEMT_CMD :
            {
                *puiValue = (uint32) MBOX_PhyEmptyMcmd(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_RCNT_DAT :
            {
                *puiValue = (uint32) MBOX_PhyCountSdata(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_RFUL_DAT :
            {
                *puiValue = (uint32) MBOX_PhyFullSdata(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_REMT_DAT :
            {
                *puiValue = (uint32) MBOX_PhyEmptySdata(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_WCNT_DAT :
            {
                *puiValue = (uint32) MBOX_PhyCountMdata(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_WFUL_DAT :
            {
                *puiValue = (uint32) MBOX_PhyFullMdata(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_WEMT_DAT :
            {
                *puiValue = (uint32) MBOX_PhyEmptyMdata(pDev->dvChnl);
                break;
            }

            case MBOX_IO_CMD_GET_OWN_STS :
            {
                ret = MBOX_PhyGetOwnTmnSts(pDev->dvChnl, puiValue);
                break;
            }

            case MBOX_IO_CMD_GET_OPP_STS :
            {
                ret = MBOX_PhyGetOppTmnSts(pDev->dvChnl, puiValue);
                break;
            }

            default :
            {
                MBOX_E("invalid ioctl uiCmd[%u]\n", uiCmd);
                ret = MBOX_ERR_IOCTL;
                // print err : invalid ioctl cmd
                break;
            }
        }
    }
    else
    {
        MBOX_E("invalid param dev[0x%08X]\n", pDev);
        ret = MBOX_ERR_ARGUMENT;
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevWordCpy>
*
* This function copies words from source to target.
*
* @param    puiTarget   [in] A pointer of target.
* @param    puiSource   [in] A pointer of source.
* @param    uiLen       [in] A length of words to be copied.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void MBOX_DevWordCpy
(
    uint32 *                            puiTarget,
    uint32 *                            puiSource,
    uint32                              uiLen
)
{
    uint32 idx;

    if (MBOX_DEV_NULLCHK(puiTarget) && MBOX_DEV_NULLCHK(puiSource))
    {
        for (idx = 0 ; idx < uiLen ; idx++)
        {
            puiTarget[idx]  = puiSource[idx];
        }
    }
    else
    {
        MBOX_E("invalid param target[0x%08X] source[0x%08X]\n", puiTarget, puiSource);
    }
}

/*
***************************************************************************************************
*                                      <MBOX_DevByteCpy>
*
* This function copies bytes from source to target.
*
* @param    puiTarget   [in] A pointer of target.
* @param    puiSource   [in] A pointer of source.
* @param    uiLen       [in] A length of bytes to be copied.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void MBOX_DevByteCpy
(
    uint8 *                             puiTarget,
    uint8 *                             puiSource,
    uint32                              uiLen
)
{
    uint32 idx;

    if (MBOX_DEV_NULLCHK(puiTarget) && MBOX_DEV_NULLCHK(puiSource))
    {
        for (idx = 0 ; idx < uiLen ; idx++)
        {
            puiTarget[idx] = puiSource[idx];
        }
    }
    else
    {
        MBOX_E("invalid param target[0x%08X] source[0x%08X]\n", puiTarget, puiSource);
    }

}

/*
***************************************************************************************************
*                                      <MBOX_DevFind>
*
* This function writes/reads Mailbox registers.
*
* @param    siCh        [in] A Mailbox channel, which data is arrived.
* @param    pMsg        [in] A pointer of received data.
* @return
*
* Notes
*
***************************************************************************************************
*/

static int32 MBOX_DevFind
(
    MBOXCh_t                            siCh,
    MBOXMsage_t *                       pMsg
)
{
    uint8 key[MBOX_MAX_KEY_SIZE + 1];

    key[0]  = (uint8) ((pMsg->mmCmnd[7] & 0x000000FFUL) >> 0UL);
    key[1]  = (uint8) ((pMsg->mmCmnd[7] & 0x0000FF00UL) >> 8UL);
    key[2]  = (uint8) ((pMsg->mmCmnd[7] & 0x00FF0000UL) >> 16UL);
    key[3]  = (uint8) ((pMsg->mmCmnd[7] & 0xFF000000UL) >> 24UL);
    key[4]  = (uint8) ((pMsg->mmCmnd[0] & 0x00FF0000UL) >> 16UL);
    key[5]  = (uint8) ((pMsg->mmCmnd[0] & 0xFF000000UL) >> 24UL);

    return MBOX_DevMapGet(siCh, (int8 *) key);
}

/*
***************************************************************************************************
*                                      <MBOX_DevNew>
*
* This function allocates memory for virtual device.
*
* @param    siCh        [in] A Mailbox channel to be used.
* @param    pucDevName  [in] A pointer of device name.
* @return
*
* Notes
*
***************************************************************************************************
*/

static int32 MBOX_DevNew
(
    MBOXCh_t                            siCh,
    uint8 *                             pucDevName
)
{
    uint32  idx;
    int32   ret;

    ret = MBOX_DEV_ERR_FULL;
    MBOX_DevFunCall(ret, MBOX_DevMapGet(siCh, (int8 *) pucDevName));

    if (ret < 0)
    {
        for (idx = 0 ; idx < MBOX_MAX_MULTI_OPEN ; idx++)
        {
            if (mboxInfo[siCh].mbDlst[idx].dvIden < 0)
            {
                MBOX_DevFunCall(ret, MBOX_DevMapPut(siCh, pucDevName, (int32) idx));
                break;
            }
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevMapGet>
*
* This function finds device opened from hash map.
*
* @param    siCh        [in] A Mailbox channel to be used.
* @param    pucDevName  [in] A pointer of device name.
* @return
*
* Notes: Open source
* There are no restriction to use this logic according to http:\\github.com\petewarden\c_hashmap
*
***************************************************************************************************
*/

static int32 MBOX_DevMapGet
(
    MBOXCh_t                            siCh,
    int8 *                              pucDevName
)
{
    int32   ret;
    uint32  key;
    uint32  index;

    ret = -1;
    key = 0UL;

    if (MBOX_DEV_VALIDCH(siCh))
    {
        if (MBOX_DEV_NULLCHK(pucDevName))
        {
            key = MBOX_DevCrc32((uint8 *) pucDevName, MBOX_MAX_KEY_SIZE);
            /* Robert Jenkins' 32 bit Mix Function */
            key += (key << 12UL);
            key ^= (key >> 22UL);
            key += (key << 4UL);
            key ^= (key >> 9UL);
            key += (key << 10UL);
            key ^= (key >> 2UL);
            key += (key << 7UL);
            key ^= (key >> 12UL);
            /* Knuth's Multiplicative Method */
            key = (key >> 3UL) * 2654435761UL;
            key = key % MBOX_MAX_MULTI_OPEN;

            for (index = 0 ; index < MBOX_MAX_MULTI_OPEN ; index++)
            {
                if ((devIdMap[siCh][key].hmInuse == 1) &&
                    (pucDevName[0] == (int8) devIdMap[siCh][key].hmSrkey[0]) &&
                    (pucDevName[1] == (int8) devIdMap[siCh][key].hmSrkey[1]) &&
                    (pucDevName[2] == (int8) devIdMap[siCh][key].hmSrkey[2]) &&
                    (pucDevName[3] == (int8) devIdMap[siCh][key].hmSrkey[3]) &&
                    (pucDevName[4] == (int8) devIdMap[siCh][key].hmSrkey[4]) &&
                    (pucDevName[5] == (int8) devIdMap[siCh][key].hmSrkey[5]))
                {
                    ret = devIdMap[siCh][key].hmValue;
                    break;
                }
                else
                {
                    key = (key + 1UL) % MBOX_MAX_MULTI_OPEN;
                    ret = MBOX_DEV_ERR_KEY_NOTINUSED;
                }
            }
        }
        else
        {
            ret = MBOX_DEV_ERR_INVALID_NAME;
        }
    }
    else
    {
        ret = MBOX_DEV_ERR_INVALID_CH;
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevMapPut>
*
* This function stores device opened into hash map.
*
* @param    siCh        [in] A Mailbox channel to be used.
* @param    pucDevName  [in] A pointer of device name.
* @param    siIdx         [in] A device number to be opened.
* @return
*
* Notes: Open source
* There are no restriction to use this logic according to http:\\github.com\petewarden\c_hashmap
*
***************************************************************************************************
*/

static int32 MBOX_DevMapPut
(
    MBOXCh_t                            siCh,
    uint8 *                             pucDevName,
    int32                               siIdx
)
{
    int32   ret;
    uint32  key;
    uint32  index;

    ret = -1;
    key = 0UL;

    if (MBOX_DEV_VALIDCH(siCh))
    {
        if (MBOX_DEV_NULLCHK(pucDevName))
        {
            key = MBOX_DevCrc32((uint8 *) pucDevName, MBOX_MAX_KEY_SIZE);
            /* Robert Jenkins' 32 bit Mix Function */
            key += (key << 12UL);
            key ^= (key >> 22UL);
            key += (key << 4UL);
            key ^= (key >> 9UL);
            key += (key << 10UL);
            key ^= (key >> 2UL);
            key += (key << 7UL);
            key ^= (key >> 12UL);
            /* Knuth's Multiplicative Method */
            key = (key >> 3U) * 2654435761UL;
            key = key % MBOX_MAX_MULTI_OPEN;

            for (index = 0 ; index < MBOX_MAX_MULTI_OPEN ; index++)
            {
                if (devIdMap[siCh][key].hmInuse == 0)
                {
                    devIdMap[siCh][key].hmInuse     = 1;
                    devIdMap[siCh][key].hmValue     = siIdx;
                    devIdMap[siCh][key].hmSrkey[0]  = pucDevName[0];
                    devIdMap[siCh][key].hmSrkey[1]  = pucDevName[1];
                    devIdMap[siCh][key].hmSrkey[2]  = pucDevName[2];
                    devIdMap[siCh][key].hmSrkey[3]  = pucDevName[3];
                    devIdMap[siCh][key].hmSrkey[4]  = pucDevName[4];
                    devIdMap[siCh][key].hmSrkey[5]  = pucDevName[5];
                    ret = devIdMap[siCh][key].hmValue;
                    break;
                }
                else
                {
                    key = (key + 1UL) % MBOX_MAX_MULTI_OPEN;
                    ret = MBOX_DEV_ERR_HASH_MAP_FULL;
                }
            }
        }
        else
        {
            ret = MBOX_DEV_ERR_INVALID_NAME;
        }
    }
    else
    {
        ret = MBOX_DEV_ERR_INVALID_CH;
    }

    return ret;
}

/*
***************************************************************************************************
*                                      <MBOX_DevCrc32>
*
* This function creates crc32 code for hash map.
*
* @param    puc         [in] A pointer of device name to be hashed.
* @param    uiLen       [in] A length of device name.
* @return
*
* Notes: Open source
* There are no restriction to use this logic according to http:\\github.com\petewarden\c_hashmap
*
***************************************************************************************************
*/

static uint32 MBOX_DevCrc32
(
    const uint8 *                       puc,
    uint32                              uiLen
)
{
    uint32  i;
    uint32  crc32val;

    crc32val = 0;

    for (i = 0 ;  i < uiLen ;  i ++)
    {
        crc32val = MBOXCrc32Tab[(crc32val ^ puc[i]) & 0xffU] ^ (crc32val >> 8U);
    }

    return crc32val;
}


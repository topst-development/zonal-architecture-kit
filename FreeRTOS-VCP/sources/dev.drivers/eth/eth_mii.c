// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eth_mii.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_ETH == 1 )

#include <eth.h>
#include <eth_mii.h>

/*************************************************************************************************/
/*                                      LOCAL VARIABLES                                          */
/*************************************************************************************************/

ETHDev_t *                              pmac;

/*
***************************************************************************************************
*                                      FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void delay1us
(
   uint32                              uiUs
);

static uint32 PHY_BusyCheck
(
    void
);

static void PHY_SwReset
(
    void
);

static uint16 PHY_LinkCheck
(
    void
);

static uint32 PHY_GetLinkStatusRealTime
(
    void
);

static uint32 PHY_GetLinkStatusLatched
(
    void
);

static void PHY_RegisterDump
(
    void
);

static void PHY_Config
(
    void
);

static void PHY_ConfigLoopback
(
    void
);

/*
***************************************************************************************************
*                                          delay1us
* Function to generate delay.
*
* @param    [In] uiUs  :  time value
* @return   None
* Notes
***************************************************************************************************
*/

static void delay1us
(
   uint32                              uiUs
)
{
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
*                                          PHY_BusyCheck
* Function to check phy state.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static uint32 PHY_BusyCheck
(
    void
)
{
    uint32 count = 0;

    while(count < 1000)
    {
        if((SAL_ReadReg(ETH_MAC_BASE + pmac->dMiiInfo.mAddr) & MII_BUSY) == 1)
        {
            count++;

            if(count == 1000)
            {
                return 1;
            }
        }
        else
        {
            break;
        }
    }

    return 0;
}

/*
***************************************************************************************************
*                                          PHY_MdioRead
* Function to communicate between MAC and PHY.
*
* @param    [In] uiReg : phy register address
* @return   read data
* Notes
*       phy read / write data : 16bit
*       Marvell phy mdio interface between GMAC : clause 22 to 45
*
***************************************************************************************************
*/

static uint32 PHY_MdioRead
(
    uint32                              uiReg
)
{
    uint32                              uiMiiAddress;
    uint32                              uiMiiData;
    uint32                              uiData;
    uint32                              uiValue;
    uint32                              uiRet;

    uiMiiAddress = pmac->dMiiInfo.mAddr;
    uiMiiData = pmac->dMiiInfo.mData;
    uiData = 0;
    uiValue = MII_BUSY;
    uiRet = 0;

    uiValue |= (MRVL_PHY_ADDR << pmac->dMiiInfo.mAddrShift) & pmac->dMiiInfo.mAddrMask;
    uiValue |= (uiReg << pmac->dMiiInfo.mRegShift) & pmac->dMiiInfo.mRegMask;
    uiValue |= (pmac->dMiiInfo.mCsr << pmac->dMiiInfo.mCsrShift) & pmac->dMiiInfo.mCsrMask;

    // gmac4
    uiValue |= MII_GMAC4_READ;

    uiRet = PHY_BusyCheck();

    if(uiRet != 0)
    {
        mcu_printf("[%s] MDIO BUSY CHECK FAIL. \n ", __func__);
    }

    SAL_WriteReg(uiValue, ETH_MAC_BASE + uiMiiAddress);

    uiRet = PHY_BusyCheck();

    if(uiRet != 0)
    {
        mcu_printf("[%s] MDIO BUSY CHECK FAIL. \n ", __func__);
    }

    /* Read the data from the MII data register */
    uiData = SAL_ReadReg(ETH_MAC_BASE + uiMiiData);

    return uiData;
}

/*
***************************************************************************************************
*                                          PHY_MdioWrite
* Function to communicate between MAC and PHY.
*
* @param    [In] uiReg : phy register address
* @param    [In] uiData : data
* @return   result
* Notes
*       phy read / write data : 16bit
*       Marvell phy mdio interface between GMAC : clause 22 to 45
*
***************************************************************************************************
*/

static uint32 PHY_MdioWrite
(
    uint32                              uiReg,
    uint16                              uiData
)
{
    uint32                              uiMiiAddress;
    uint32                              uiMiiData;
    uint32                              uiValue;
    uint32                              uiRet;

    uiMiiAddress = pmac->dMiiInfo.mAddr;
    uiMiiData = pmac->dMiiInfo.mData;
    uiValue = MII_BUSY;
    uiRet = 0;

    uiValue |= (MRVL_PHY_ADDR << pmac->dMiiInfo.mAddrShift)
                & pmac->dMiiInfo.mAddrMask;
    uiValue |= (uiReg << pmac->dMiiInfo.mRegShift) & pmac->dMiiInfo.mRegMask;
    uiValue |= (pmac->dMiiInfo.mCsr << pmac->dMiiInfo.mCsrShift)
                & pmac->dMiiInfo.mCsrMask;

    // for gmac4
    uiValue |= MII_GMAC4_WRITE;

    /* Wait until any existing MII operation is complete */
    uiRet = PHY_BusyCheck();

    if(uiRet != 0)
    {
        mcu_printf("[%s] MDIO BUSY CHECK FAIL. \n ", __func__);
    }

    /* Set the MII address register to write */
    SAL_WriteReg(uiData, ETH_MAC_BASE + uiMiiData);
    SAL_WriteReg(uiValue, ETH_MAC_BASE + uiMiiAddress);

    uiRet = PHY_BusyCheck();

    if(uiRet != 0)
    {
        mcu_printf("[%s] MDIO BUSY CHECK FAIL. \n ", __func__);
    }

    return uiRet;
}

/*
***************************************************************************************************
*                                          PHY_Read
* Function to read phy register.
*
* @param    [In] uiAddr : phy register device address
* @param    [In] uiReg : phy register address
* @return
* Notes
*       phy read / write data : 16bit
*       Marvell phy mdio interface between GMAC : clause 22 to 45
*
***************************************************************************************************
*/

uint32 PHY_Read
(
    uint32                              uiAddr,
    uint32                              uiReg
)
{
    uint32                              uiRet;

    uiRet = 0;

    PHY_MdioWrite(MMD_ACCESS_CTRL,((OPERATION_ADDR << OPERATION_BIT_SHIFT) | uiAddr));
    PHY_MdioWrite(MMD_ACCESS_ADDR_DATA, uiReg);
    PHY_MdioWrite(MMD_ACCESS_CTRL,((OPERATION_RW << OPERATION_BIT_SHIFT) | uiAddr));

    uiRet = PHY_MdioRead(MMD_ACCESS_ADDR_DATA);

    return uiRet;
}

/*
***************************************************************************************************
*                                          PHY_Write
* Function to read phy register.
*
* @param    [In] uiAddr : phy register device address
* @param    [In] uiReg : phy register address
* @param    [In] usData : Data
* @return
* Notes
*       phy read / write data : 16bit
*       Marvell phy mdio interface between GMAC : clause 22 to 45
*
***************************************************************************************************
*/

uint32 PHY_Write
(
    uint32                              uiAddr,
    uint32                              uiReg,
    uint16                              usData
)
{
    PHY_MdioWrite(MMD_ACCESS_CTRL,(OPERATION_ADDR + uiAddr));
    PHY_MdioWrite(MMD_ACCESS_ADDR_DATA, uiReg);
    // PHY_MdioWrite(MMD_ACCESS_CTRL,(OPERATION_RW + dev_addr));
    PHY_MdioWrite(MMD_ACCESS_CTRL,((OPERATION_RW << OPERATION_BIT_SHIFT) | uiAddr));
    PHY_MdioWrite(MMD_ACCESS_ADDR_DATA, usData);

    return 0;
}

/*
***************************************************************************************************
*                                          PHY_SwReset
* Function to do SW PHY reset
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void PHY_SwReset
(
    void
)
{
    uint32                              uiReg;

    uiReg = PHY_Read(1, 0x0000);
    uiReg |= (1 << 11);

    PHY_Write(1, 0x0000, uiReg);
    PHY_Write(3, 0xFFE4, 0x000C);

    delay1us(1000);

    PHY_Write(3, 0xFFE4, 0x06B6);
    uiReg &= (~(1 << 11));
    PHY_Write(1, 0x0000, uiReg);

    delay1us(1000);

    PHY_Write(3, 0xFC47, 0x0030);
    PHY_Write(3, 0xFC47, 0x0031);
    PHY_Write(3, 0xFC47, 0x0030);
    PHY_Write(3, 0xFC47, 0x0000);
    PHY_Write(3, 0xFC47, 0x0001);
    PHY_Write(3, 0xFC47, 0x0000);
    PHY_Write(3, 0x0900, 0x8000);
    PHY_Write(1, 0x0900, 0x0000);
    PHY_Write(3, 0xFFE4, 0x000C);
}

/*
***************************************************************************************************
*                                          PHY_LinkCheck
* Function to check link status.
*
* @param    None
* @return   Link status
* Notes
*
***************************************************************************************************
*/

static uint16 PHY_LinkCheck
(
    void
)
{
    uint16                              usRetData1;
    uint16                              usRetData2;
    uint16                              usRet;

    // fake read(?)
    PHY_Read(3, 0x0901);

    usRetData1 = PHY_Read(3, 0x0901);
    usRetData2 = PHY_Read(7, 0x8001);

    mcu_printf("retData1 : %x , retData2 : %x \n" , usRetData1, usRetData2);

    delay1us(1000);

    usRet = (0x0 != (usRetData1 & 0x0004))  && (0x0 != (usRetData2 & 0x3000));

    return usRet;
}

/*
***************************************************************************************************
*                                          PHY_GetLinkStatusRealTime
* Function to check link status. (real time status)
*
* @param    None
* @return   Link status
* Notes
*
***************************************************************************************************
*/

static uint32 PHY_GetLinkStatusRealTime
(
    void
)
{
    uint16                              uiRet;

    uiRet = 0;

    // fake read(?)
    PHY_Read(3, 0x0901);

    uiRet = PHY_Read(3, 0x0901);

    return (0x0 != (uiRet & 0x0004));
}

/*
***************************************************************************************************
*                                          PHY_GetLinkStatusLatched
* Function to check link status. (latched)
*
* @param    None
* @return   Link status
* Notes
*
***************************************************************************************************
*/

static uint32 PHY_GetLinkStatusLatched
(
    void
)
{
    uint16                              uiRet;

    // fake read(?)
    PHY_Read(7, 0x0201);

    uiRet = PHY_Read(7, 0x0201);

    if(uiRet != 0xffff)
    {
        return (0x0 != (uiRet & 0x0004));
    }
    else
    {
        return 0;
    }
}

/*
***************************************************************************************************
*                                          PHY_SetMasterSlave
* Function to set phy master/slave mode.
*
* @param    [In] uiMaster : Master 1 or Slave 0
* @return   None
* Notes
*
***************************************************************************************************
*/

void PHY_SetMasterSlave
(
    uint32 uiMaster
)
{
    uint16                              uiRet;

    uiRet = PHY_Read(1, 0x0834);

    if(uiMaster)
    {
        uiRet |= 0x4000;
    }
    else
    {
        uiRet &= 0xBFFF;
    }

    PHY_Write(1, 0x0834, uiRet);
    PHY_Write(7, 0x8032, 0x0064);
    PHY_Write(7, 0x8031, 0x0A01);
    PHY_Write(7, 0x8031, 0x0C01);
}

/*
***************************************************************************************************
*                                          PHY_GetMasterSlave
* Function to get phy master/slave mode.
*
* @param    None
* @return   Master 1 or Slave 0
* Notes
*
***************************************************************************************************
*/

uint32 PHY_GetMasterSlave
(
    void
)
{
    uint32                              uiRet2;
    uint32                              uiRet;

    // fake read(?)
    PHY_Read(3, 0x0901);

    uiRet2 = PHY_Read(7, 0x8001);

    uiRet = (0x0 != (uiRet2 & 0x4000));

    return uiRet;
}

/*
***************************************************************************************************
*                                          PHY_ReadStatus
* Function to get link status and set operating info.
*
* @param    None
* @return   0
* Notes
*
***************************************************************************************************
*/

uint32 PHY_ReadStatus
(
    void
)
{
    // Update the link, but return if there
    // was an error
#ifdef ETH_FIXED_LINK
    pmac->dLinkInfo.lLink = 1;
#else
    pmac->dLinkInfo.lLink = PHY_GetLinkStatusLatched();
#endif
    pmac->dLinkInfo.lCurSpeed = SPEED_1000;
    pmac->dLinkInfo.lCurDuplex = DUPLEX_FULL;

    return 0;
}

/*
***************************************************************************************************
*                                          PHY_Config
* Function to set phy configuration value and get phy device id.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void PHY_Config
(
    void
)
{
    uint32                              uiPhyId;

    uiPhyId = PHY_Read(1, 0x0003);

    mcu_printf("[Ethernet PHY] q2110 phy master(1)/slave(0) : %x\n", PHY_GetMasterSlave());
    mcu_printf("[Ethernet PHY] phy id : %08x\n", uiPhyId);

    if((uiPhyId & 0xfff0) != (MRVL_Q2110_ID & 0xfff0)) // ignore minor version
    {
        mcu_printf("Invaild PHY ID : 0x%x (expected : 0x%x)\n", uiPhyId, MRVL_Q2110_ID);
    }

    return;
}

/*
***************************************************************************************************
*                                          PHY_ConfigLoopback
* Function to set phy configuration value and get phy device id.
*
* @param    None
* @return   None
* Notes
*
***************************************************************************************************
*/

static void PHY_ConfigLoopback
(
    void
)
{
    uint32                              uiPhyId;
    uint32                              uiReg;

    // phy loopback mode
    uiReg = PHY_Read(1, 0x0000);
    uiReg |= 0x1;
    PHY_Write(1, 0x0000, uiReg);

    uiPhyId = PHY_Read(1, 0x0003);

    mcu_printf("[Ethernet PHY] q2110 phy master(1)/slave(0) : %x\n", PHY_GetMasterSlave());
    mcu_printf("[Ethernet PHY] phy id : %08x\n", uiPhyId);

    if((uiPhyId & 0xfff0) != (MRVL_Q2110_ID & 0xfff0)) // ignore minor version
    {
        mcu_printf("Invaild PHY ID : 0x%x (expected : 0x%x)\n",uiPhyId, MRVL_Q2110_ID);
    }

    return;

}

/*
***************************************************************************************************
*                                          PHY_Init
* Function to intialize phy device.
*
* @param    [In] pMac : pointer of mac device handle struct.
* @return   None
* Notes
*
***************************************************************************************************
*/

void PHY_Init
(
    ETHDev_t *                          pMac
)
{
    uint32                              uiReg;
    uint32                              uiMode;

    uiReg = 0;
    pmac = pMac;

    (void)PHY_Write(1, 0x0900, 0x4000);
    (void)PHY_Write(7, 0x0200, 0x0000);

    uiReg = PHY_Read(1, 0x0834);
    uiReg = ((uiReg & 0xFFF0) | 0x0001);

    uiMode = PHY_GetMasterSlave();
    //uiMode = 1; // forced master
    //uiMode = 0; // forced slave

    if (uiMode == 0x1)
    {
        // master
        uiReg &= (~(1 << 14));
        uiReg |= (1 << 14);
    }
    else
    {
        // slave
        uiReg &= (~(1 << 14));
    }

    PHY_Write(1, 0x0834, uiReg);

    PHY_Write(3, 0xFFE4, 0x07B5);
    PHY_Write(3, 0xFFE4, 0x06B6);

    delay1us(5000);

    PHY_Write(3, 0xFFDE, 0x402F);
    PHY_Write(3, 0xFE2A, 0x3C3D);
    PHY_Write(3, 0xFE34, 0x4040);
    PHY_Write(3, 0xFE4B, 0x9337);
    PHY_Write(3, 0xFE2A, 0x3C1D);
    PHY_Write(3, 0xFE34, 0x0040);
    PHY_Write(3, 0xFE0F, 0x0000);
    PHY_Write(3, 0xFC00, 0x01C0);
    PHY_Write(3, 0xFC17, 0x0425);
    PHY_Write(3, 0xFC94, 0x5470);
    PHY_Write(3, 0xFC95, 0x0055);
    PHY_Write(3, 0xFC19, 0x08d8);
    PHY_Write(3, 0xFC1A, 0x0110);
    PHY_Write(3, 0xFC1B, 0x0a10);
    PHY_Write(3, 0xFC3A, 0x2725);
    PHY_Write(3, 0xFC61, 0x2627);
    PHY_Write(3, 0xFC3B, 0x1612);
    PHY_Write(3, 0xFC62, 0x1C12);
    PHY_Write(3, 0xFC9D, 0x6367);
    PHY_Write(3, 0xFC9E, 0x8060);
    PHY_Write(3, 0xFC00, 0x01C8);
    PHY_Write(3, 0x8000, 0x0000);
    PHY_Write(3, 0x8016, 0x0011);
    PHY_Write(3, 0xFDA3, 0x1800);
    PHY_Write(3, 0xFE02, 0x00C0);
    PHY_Write(3, 0xFFDB, 0x0010);
    PHY_Write(3, 0xFFF3, 0x0020);
    PHY_Write(3, 0xFE40, 0x00A6);
    PHY_Write(3, 0xFE60, 0x0000);
    PHY_Write(3, 0xFE2A, 0x3C3D);
    PHY_Write(3, 0xFE4B, 0x9334);
    PHY_Write(3, 0xFC10, 0xF600);
    PHY_Write(3, 0xFC11, 0x073D);
    PHY_Write(3, 0xFC12, 0x000D);
    PHY_Write(3, 0xFC13, 0x0010);

    PHY_Write(7, 0x8032, 0x0064);
    PHY_Write(7, 0x8031, 0x0A01);
    PHY_Write(7, 0x8031, 0x0C01);
    PHY_Write(3, 0x800C, 0x0000);
    PHY_Write(7, 0x8032, 0x0002);
    PHY_Write(7, 0x8031, 0x0A1B);
    PHY_Write(7, 0x8031, 0x0C1B);
    PHY_Write(7, 0x8032, 0x0003);
    PHY_Write(7, 0x8031, 0x0A1C);
    PHY_Write(7, 0x8031, 0x0C1C);
    PHY_Write(3, 0xFE04, 0x0008);

    PHY_SwReset();

    delay1us(1000);

    PHY_Config();

}

/*
***************************************************************************************************
*                                          PHY_InitLoopback
* Function to intialize phy device for loopback test.
* @param    [In] pMac : pointer of mac device handle struct.
* @return   None
* Notes
*
***************************************************************************************************
*/

void PHY_InitLoopback
(
    ETHDev_t *                          pMac
)
{
    uint32                              uiReg;
    uint32                              uiMode;

    uiReg = 0;
    pmac = pMac;

    (void)PHY_Write(1, 0x0900, 0x4000);
    (void)PHY_Write(7, 0x0200, 0x0000);

    uiReg = PHY_Read(1, 0x0834);
    uiReg = ((uiReg & 0xFFF0) | 0x0001);

    uiMode = PHY_GetMasterSlave();
    //uiMode = 1; // forced master
    //uiMode = 0; // forced slave

    if (uiMode == 0x1)
    {
        // master
        uiReg &= (~(1 << 14));
        uiReg |= (1 << 14);
    }
    else
    {
        // slave
        uiReg &= (~(1 << 14));
    }

    PHY_Write(1, 0x0834, uiReg);

    PHY_Write(3, 0xFFE4, 0x07B5);
    PHY_Write(3, 0xFFE4, 0x06B6);

    delay1us(5000);

    PHY_Write(3, 0xFFDE, 0x402F);
    PHY_Write(3, 0xFE2A, 0x3C3D);
    PHY_Write(3, 0xFE34, 0x4040);
    PHY_Write(3, 0xFE4B, 0x9337);
    PHY_Write(3, 0xFE2A, 0x3C1D);
    PHY_Write(3, 0xFE34, 0x0040);
    PHY_Write(3, 0xFE0F, 0x0000);
    PHY_Write(3, 0xFC00, 0x01C0);
    PHY_Write(3, 0xFC17, 0x0425);
    PHY_Write(3, 0xFC94, 0x5470);
    PHY_Write(3, 0xFC95, 0x0055);
    PHY_Write(3, 0xFC19, 0x08d8);
    PHY_Write(3, 0xFC1A, 0x0110);
    PHY_Write(3, 0xFC1B, 0x0a10);
    PHY_Write(3, 0xFC3A, 0x2725);
    PHY_Write(3, 0xFC61, 0x2627);
    PHY_Write(3, 0xFC3B, 0x1612);
    PHY_Write(3, 0xFC62, 0x1C12);
    PHY_Write(3, 0xFC9D, 0x6367);
    PHY_Write(3, 0xFC9E, 0x8060);
    PHY_Write(3, 0xFC00, 0x01C8);
    PHY_Write(3, 0x8000, 0x0000);
    PHY_Write(3, 0x8016, 0x0011);
    PHY_Write(3, 0xFDA3, 0x1800);
    PHY_Write(3, 0xFE02, 0x00C0);
    PHY_Write(3, 0xFFDB, 0x0010);
    PHY_Write(3, 0xFFF3, 0x0020);
    PHY_Write(3, 0xFE40, 0x00A6);
    PHY_Write(3, 0xFE60, 0x0000);
    PHY_Write(3, 0xFE2A, 0x3C3D);
    PHY_Write(3, 0xFE4B, 0x9334);
    PHY_Write(3, 0xFC10, 0xF600);
    PHY_Write(3, 0xFC11, 0x073D);
    PHY_Write(3, 0xFC12, 0x000D);
    PHY_Write(3, 0xFC13, 0x0010);

    PHY_Write(7, 0x8032, 0x0064);
    PHY_Write(7, 0x8031, 0x0A01);
    PHY_Write(7, 0x8031, 0x0C01);
    PHY_Write(3, 0x800C, 0x0000);
    PHY_Write(7, 0x8032, 0x0002);
    PHY_Write(7, 0x8031, 0x0A1B);
    PHY_Write(7, 0x8031, 0x0C1B);
    PHY_Write(7, 0x8032, 0x0003);
    PHY_Write(7, 0x8031, 0x0A1C);
    PHY_Write(7, 0x8031, 0x0C1C);
    PHY_Write(3, 0xFE04, 0x0008);

    PHY_SwReset();

    delay1us(1000);

    PHY_ConfigLoopback();
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_ETH == 1 )


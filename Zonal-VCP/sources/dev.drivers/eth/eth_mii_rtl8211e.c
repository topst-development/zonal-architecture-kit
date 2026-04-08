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

#include <eth_mii_rtl8211e.h>

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

    uiValue |= (RTL_PHY_ADDR << pmac->dMiiInfo.mAddrShift) & pmac->dMiiInfo.mAddrMask;
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

    uiValue |= (RTL_PHY_ADDR << pmac->dMiiInfo.mAddrShift)
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
    uint32                              uiReg
)
{
    uint32                              uiRet;

    uiRet = 0;

    uiRet = PHY_MdioRead(uiReg);

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
    uint32                              uiReg,
    uint16                              usData
)
{
    PHY_MdioWrite(uiReg, usData);

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
    // SW reset
    PHY_Write(RTL8211E_BMCR, RTL8211E_RESET_MASK);
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
    uint16                              usRetData;
    uint16                              usRet;

    // Read BMSR register. offset 0x01
    usRetData = PHY_Read(RTL8211E_BMSR);
    // Check Link status (Bit 2)
    if((usRetData & RTL8211E_LINK_STATUS_MASK) > 0)
    {
        usRet = 1;
    }
    else
    {
        usRet = 0;
    }

    return usRet;
}

static uint16 PHY_SpeedCheck
(
    void
)
{
    uint16                              usRetData;
    uint16                              usRet;

    // Read PHYSR register. offset 0x11
    usRetData = PHY_Read(RTL8211E_PHYSR);

    // Check Link status (Bit 15:14)
    usRetData = (usRetData & RTL8211E_SPEED_STATUS_MASK) >> 14;

    switch(usRetData)
    {
        case 0:
            usRet = SPEED_10;
            break;
        case 1:
            usRet = SPEED_100;
            break;
        case 2:
            usRet = SPEED_1000;
            break;
        default:
            usRet = 0;
            break;
    }

    return usRet;
}

static uint16 PHY_DuplexCheck
(
    void
)
{
    uint16                              usRetData;
    uint16                              usRet;

    // Read PHYSR register. offset 0x11
    usRetData = PHY_Read(RTL8211E_PHYSR);

    // Check Link status (Bit 13)
    usRetData = (usRetData & RTL8211E_DUPLEX_STATUS_MASK);

    if(usRetData > 0)
    {
        usRet = DUPLEX_FULL;
    }
    else
    {
        usRet = DUPLEX_HALF;
    }

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
    return 0;
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
    // Do nothing.
    return 0;
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
    // Do nothing.
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
    // Do nothing.
    // Use default master/slave settings.
    return 0;
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

    pmac->dLinkInfo.lLink = PHY_LinkCheck();
    pmac->dLinkInfo.lCurSpeed = PHY_SpeedCheck();
    pmac->dLinkInfo.lCurDuplex = PHY_DuplexCheck();

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
    uint32                              uiReg;

    // Read PHYID1 register. offset 0x02
    uiReg = PHY_Read(0x0002);
    uiPhyId = (uiReg << 16);

    // Read PHYID2 register. offset 0x03
    uiReg = PHY_Read(0x0003);
    uiPhyId = uiPhyId + uiReg;

    mcu_printf("[Ethernet PHY] RTL8211E phy id : %08x\n", uiPhyId);

    if((uiPhyId & 0xfff0) != (RTL8211E_ID & 0xfff0)) // ignore minor version
    {
        mcu_printf("Invaild PHY ID : 0x%x (expected : 0x%x)\n", uiPhyId, RTL8211E_ID);
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

    // Read BMCR regster. offset 0x00
    uiReg = PHY_Read(0x0000);
    uiReg |= (0x1 << 14); // set loopback mode enable bit.
    PHY_Write(0x0000, uiReg);

    // Read PHYID1 register. offset 0x02
    uiReg = PHY_Read(0x0002);
    uiPhyId = (uiReg << 16);

    // Read PHYID2 register. offset 0x03
    uiReg = PHY_Read(0x0003);
    uiPhyId = uiPhyId + uiReg;

    mcu_printf("[Ethernet PHY] RTL8211E phy id : %08x\n", uiPhyId);

    if((uiPhyId & 0xfff0) != (RTL8211E_ID & 0xfff0)) // ignore minor version
    {
        mcu_printf("Invaild PHY ID : 0x%x (expected : 0x%x)\n", uiPhyId, RTL8211E_ID);
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
    pmac = pMac;

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
    pmac = pMac;

    PHY_SwReset();

    delay1us(1000);

    PHY_ConfigLoopback();
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_ETH == 1 )


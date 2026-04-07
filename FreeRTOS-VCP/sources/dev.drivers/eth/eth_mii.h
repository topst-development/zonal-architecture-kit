// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eth_mii.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_ETH_MII_HEADER
#define MCU_BSP_ETH_MII_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_ETH == 1 )

#define MII_BUSY                        0x1
#define MII_WRITE                       0x2

/* GMAC4 defines */
#define MII_GMAC4_GOC_SHIFT             2
#define MII_GMAC4_WRITE                 (1 << MII_GMAC4_GOC_SHIFT)
#define MII_GMAC4_READ                  (3 << MII_GMAC4_GOC_SHIFT)

#define MMD_ACCESS_CTRL                 (13)
#define MMD_ACCESS_ADDR_DATA            (14)

#define OPERATION_BIT_SHIFT             (14)
#define OPERATION_ADDR                  (0x0)
#define OPERATION_RW                    (0x1)

#define MRVL_Q212X_LPSD_FEATURE_ENABLE  0
#define MRVL_Q2110_ID                   0x00000980
#define MRVL_PHY_ADDR                   0x1

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
);

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
);

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
    uint32                              uiMaster
);

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
);

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
);

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
);

/*
***************************************************************************************************
*                                          PHY_InitLoopback
* Function to intialize phy device for loopback test.
* @param    [In] pMac : pointer of mac device handle struct.
* @return
* Notes
*
***************************************************************************************************
*/

void PHY_InitLoopback
(
    ETHDev_t *                          pMac
);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_ETH == 1 )

#endif  // MCU_BSP_ETH_MII_HEADER


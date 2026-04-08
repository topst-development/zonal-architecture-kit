// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : ssm.h
*
*   Copyright (c) Telechips Inc.
*
*   Description : Header file of System Safety Mechnism Driver
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_SSM_HEADER
#define MCU_BSP_SSM_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )

#include <sal_internal.h>

#define SSM_SOFT_FAULT_INJECTION_TEST // Test feature for Soft Fault Injection

#define SSM_FEATURE_ECC
//#define SSM_FEATURE_ISO //TCC70xx doesn't support bus isolation

#ifdef SSM_FEATURE_ECC
typedef  void (*ECC_FAULT_HANDLER)(uint32, uint32);

typedef enum ECCTypeID
{
    ECC_SRAM0 = 0x00000000UL,
    ECC_SFMC = 0x00000001UL,
    ECC_MBOX = 0x00000002UL,
    ECC_HSM = 0x00000003UL
} ECCTypeID_t;

typedef enum SRAMStatusType
{
    SRAM_CB_REGISTERED          = 0x000001U,
//  Reserved [1-3]
    SRAM_ECC_ERR_REQ_ENABLED    = 0x000010U,
//  Reserved [5-7]
    SRAM_0_LANE0_FAULT          = 0x000100U,
    SRAM_0_LANE1_FAULT          = 0x000200U,
    SRAM_0_LANE2_FAULT          = 0x000400U,
    SRAM_0_LANE3_FAULT          = 0x000800U,
    SRAM_0_LANE4_FAULT          = 0x001000U,
    SRAM_0_LANE5_FAULT          = 0x002000U,
    SRAM_0_LANE6_FAULT          = 0x004000U,
    SRAM_0_LANE7_FAULT          = 0x008000U,
    SRAM_1_LANE0_FAULT          = 0x010000U,
    SRAM_1_LANE1_FAULT          = 0x020000U,
    SRAM_1_LANE2_FAULT          = 0x040000U,
    SRAM_1_LANE3_FAULT          = 0x080000U,
    SRAM_1_LANE4_FAULT          = 0x100000U,
    SRAM_1_LANE5_FAULT          = 0x200000U,
    SRAM_1_LANE6_FAULT          = 0x400000U,
    SRAM_1_LANE7_FAULT          = 0x800000U
} SRAMStatusType_t;
#endif

#ifdef SSM_FEATURE_ISO
#define BUS_FAULT_INJ_FMU                       (0x0U)          // Fault Injection : 0x1U,  Normal operation : 0x0U

typedef enum BUSTypeIds
{
    BUS_AXI_MST_WCH_ISOLATION                   = 0x0U,
    BUS_AXI_MST_RCH_ISOLATION                   = 0x1U,
    BUS_AXI_SLV_WCH_ISOLATION                   = 0x2U,
    BUS_AXI_SLV_RCH_ISOLATION                   = 0x3U,
    BUS_AXI_SLV_RES_TIMER                       = 0x4U,
    BUS_AHB_MST_ISOLATION                       = 0x5U,
    BUS_TYPE_SIZE                               = 0x6U,

} BUSTypeId_t;

typedef enum BUSStatusType
{
    BUS_STS_CB_AXI_MST_WCH_REGISTERED           = 0x000001U,
    BUS_STS_CB_AXI_MST_RCH_REGISTERED           = 0x000002U,
    BUS_STS_CB_AXI_SLV_WCH_REGISTERED           = 0x000004U,
    BUS_STS_CB_AXI_SLV_RCH_REGISTERED           = 0x000008U,
    BUS_STS_CB_AXI_SLV_RES_TIMER_REGISTERED     = 0x000010U,
    BUS_STS_AXI_MST_WCH_ENABLED                 = 0x000100U,
    BUS_STS_AXI_MST_RCH_ENABLED                 = 0x000200U,
    BUS_STS_AXI_SLV_WCH_ENABLED                 = 0x000400U,
    BUS_STS_AXI_SLV_RCH_ENABLED                 = 0x000800U,
    BUS_STS_AXI_SLV_RES_TIMER_ENABLED           = 0x001000U,
    BUS_STS_AHB_SLV_ENABLED                     = 0x002000U,
    BUS_STS_AXI_MST_WCH_FAULT                   = 0x010000U,
    BUS_STS_AXI_MST_RCH_FAULT                   = 0x020000U,
    BUS_STS_AXI_SLV_WCH_FAULT                   = 0x040000U,
    BUS_STS_AXI_SLV_RCH_FAULT                   = 0x080000U,
    BUS_STS_AXI_SLV_RES_TIMER_FAULT             = 0x100000U,

} BUSStatusType_t;

enum
{
    BUS_ERR_ALREADY_DONE                        = (uint32)SAL_DRVID_ISO + 1UL,
    BUS_ERR_ENABLE                              = (uint32)SAL_DRVID_ISO + 2UL,
    BUS_ERR_DISABLE                             = (uint32)SAL_DRVID_ISO + 3UL,
    BUS_ERR_INVALID_PARAM                       = (uint32)SAL_DRVID_ISO + 4UL,
    BUS_ERR_SET_PW                              = (uint32)SAL_DRVID_ISO + 5UL,
    BUS_ERR_SET_TIME                            = (uint32)SAL_DRVID_ISO + 6UL,
    BUS_ERR_OUTOFRANGE_TIME                     = (uint32)SAL_DRVID_ISO + 7UL,
    BUS_ERR_GET_NEW_HANDLER_ID                  = (uint32)SAL_DRVID_ISO + 8UL,
    BUS_ERR_INVALID_ID                          = (uint32)SAL_DRVID_ISO + 101UL,
    BUS_ERR_FULL_SLOT                           = (uint32)SAL_DRVID_ISO + 102UL

};// Tcc_Bus_Ret;

#define BUS_API_COMMON_INDEX                    (0x0)
#define BUS_API_ENABLE                          (BUS_API_COMMON_INDEX + 0)
#define BUS_API_DISABLE                         (BUS_API_COMMON_INDEX + 1)
#define BUS_API_SET_SLAVE_RESTIME               (BUS_API_COMMON_INDEX + 2)
#define BUS_API_REG_FAULT_HANDLER               (BUS_API_COMMON_INDEX + 3)
#define BUS_API_UNREG_FAULT_HANDLER             (BUS_API_COMMON_INDEX + 4)
#define BUS_API_GET_STATUS                      (BUS_API_COMMON_INDEX + 5)
#define BUS_API_CONFIG_PW                       (BUS_API_COMMON_INDEX + 6)

typedef void                                    (* BUS_FAULT_HANDLER)(BUSTypeId_t uiId, BUSStatusType_t uiStatus);
#endif

/*
***************************************************************************************************
*                                         FUNCTION DECLEARATION
***************************************************************************************************
*/
#ifdef SSM_FEATURE_ECC

/*
*********************************************************************************************************
*                                          SSM_EccEnable
*
* Enable ECC encoder / decoder with SECDED to detect the fault of the SRAM memory itself
* when writing or reading data to the SRAM
*
* @param        ECCTypeID_t
* @return       0 : Success, -1 : Already enabled, -2 : Fail to set
*
* Notes
*
*********************************************************************************************************
*/
int32 SSM_EccEnable
(
    ECCTypeID_t uiEccType,
    FMUSeverityLevelType_t uiFmuSeverityLevel
);


/*
*********************************************************************************************************
*                                          SSM_EccDisable
*
* Disable ECC encoder /  encoder with SECDED
*
* @param        None
* @return       0 : Success, -1 : Already disabled, -2 : Fail to set
*
* Notes
*
*********************************************************************************************************
*/
int32 SSM_EccDisable
(
    ECCTypeID_t uiEccType
);

/*
*********************************************************************************************************
*                                          SSM_SoftFautlCheckEnable
*
* Enable Soft Fault Check Safety Mechanism
* If this safety mechanism is enabled, the Soft Fault detector compares the Mission Register value
* and the duplicated Mission Register value every clock cycle. If the two values
* are different, the Fault Request Handler informs the Fault Management Unit
* (FMU) that a soft fault has occurred.
*
* @param        uiFmuSeverityLevel : FMU_SVL_LOW, FMU_SVL_MID, FMU_SVL_HIGH
* @return       0 : Success, -1 : Already enabled, -2 : Fail to set
*
* Notes
*
*********************************************************************************************************
*/
int32 SSM_SoftFaultCheckEnable
(
    FMUSeverityLevelType_t uiFmuSeverityLevel
);


/*
*********************************************************************************************************
*                                          SSM_SoftFautlCheckDisable
*
* Disable Soft Fault Check Safety Mechanism
*
* @param        void
* @return       0 : Success, -1 : Already enabled, -2 : Fail to set
*
* Notes
*
*********************************************************************************************************
*/
int16 SSM_SoftFautlCheckDisable
(
    void
);

#endif


#ifdef SSM_FEATURE_ISO
/*
*********************************************************************************************************
*                                          SSM_BusIsolationEnable
*
*
*
* @param        BUSTypeId_t : type, uint32 : tennanosec
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusIsolationEnable
(
    BUSTypeId_t                         uiType,
    uint32                              uiTennanosec
);

/*
*********************************************************************************************************
*                                          SSM_BusIsolationDisable
*
*
*
* @param        BUSTypeId_t : type
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusIsolationDisable
(
    BUSTypeId_t                         uiType
);


/*
*********************************************************************************************************
*                                          SSM_BusRegisterFaultHandler
*
* This interface is to register the handler to inform external controller of error interrupt.
*
* @param        typeId : is to distinguish the kind of AXI isolation
*               handlerId : 0 when registering the handler for the first time or the previously issued ID(1~10)
*                           to update the handler function.
*               faultHandler : handler function pointer
* @return        1~10 : Handler ID, 101 : BUS_ERR_INVALID_ID, 102 : BUS_ERR_FULL_SLOT
*
* Notes
*
*********************************************************************************************************
*/
int16 SSM_BusRegisterFaultHandler
(
    BUSTypeId_t                         uiType,
    uint16                              uwHandlerId,
    BUS_FAULT_HANDLER                   fnFaultHandler
);

/*
*********************************************************************************************************
*                                          SSM_BusUnregisterFaultHandler
*
* This interface is to unregister the handler to inform external controller of error interrupt.
*
* @param        typeId : is to distinguish the kind of AXI isolation
*               handlerId : the previously issued ID(1~10) to unregister the handler function.
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusUnregisterFaultHandler
(
    BUSTypeId_t                         uiType,
    uint16                              uwHandlerId
);

/*
*********************************************************************************************************
*                                          SSM_BusSetAxiSlvResTime
*
* Set-up the value of AXI Outgoing Transaction Response Time
*
* @param        resTime : new response time (10ns ~ 1s : Bus clock(300mhz) time per tick(about 3ns) * resTime )
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusSetAxiSlvResTime
(
    uint32                              uiTennanosec
);

/*
*********************************************************************************************************
*                                          SSM_BusGetStatus
*
* This interface is to return the current status of Bus Isolation.
*
* @param        uwHandlerId : Issued ID of fault handler or 0 if the ID hasn't been issued
* @return       Bit [0] : Status of master write channel isolation callback function
*                   0 : not registered, 1: registered
*               Bit [1] : Status of master read channel isolation callback function
*                   0 : not registered, 1: registered
*               Bit [2] : Status of slave write channel isolation callback function
*                   0 : not registered, 1: registered
*               Bit [3] : Status of slave read channel isolation callback function
*                   0 : not registered, 1: registered
*               Bit [4] : Status of slave response timer callback function
*                   0 : not registered, 1: registered
*               Bit [5-7] : Reserved
*               Bit [8] : Status of AXI master write channel isolation
*                   0 : disabled, 1 : enabled
*               Bit [9] : Status of AXI master read channel isolation
*                   0 : disabled, 1 : enabled
*               Bit [10] : Status of AXI slave write channel isolation
*                   0 : disabled, 1 : enabled
*               Bit [11] : Status of AXI slave read channel isolation
*                   0 : disabled, 1 : enabled
*               Bit [12] : Status of AXI slave response timer
*                   0 : disabled, 1 : enabled
*               Bit [13] : Status of AHB slave isolation
*                   0 : disabled, 1 : enabled
*               Bit [14-15] : Reserved
*               Bit [16] : Status of detection for fault of master write channel
*                   0 : not detected, 1 : detected
*               Bit [17] : Status of detection for fault of master read channel
*                   0 : not detected, 1 : detected
*               Bit [18] : Status of detection for fault of slave write channel
*                   0 : not detected, 1 : detected
*               Bit [19] : Status of detection for fault of slave read channel
*                   0 : not detected, 1 : detected
*               Bit [20] : Status of detection for fault of slave response timer
*                   0 : not detected, 1 : detected
*
* Notes
*
*********************************************************************************************************
*/
static uint32 SSM_BusGetStatus
(
    uint16                              uwHandlerId
);

#endif

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )

#endif  // MCU_BSP_SSM_HEADER


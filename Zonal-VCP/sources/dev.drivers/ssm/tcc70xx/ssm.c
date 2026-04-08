// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : ssm.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : System Safety Mechnism Driver
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )

#include "fmu.h"
#include "ssm.h"
#include "clock.h"
#include "clock_dev.h"
#include <gic.h>
#include <debug.h>
#include "ssm_dev.h"

#if (DEBUG_ENABLE)
    #define SSM_LOGD(fmt, args...)     {LOGD(DBG_TAG_SSM, fmt, ## args)}
    #define SSM_LOGE(fmt, args...)     {LOGE(DBG_TAG_SSM, fmt, ## args)}
#else
    #define SSM_LOGD(fmt, args...)
    #define SSM_LOGE(fmt, args...)
#endif
/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define SSM_SF_CFG_PW                  (0x5AFEACE5U)
#define SSM_SF_CFG_WR_PW               (0xA0F261C0U)
#define SSM_SF_FAULT_CHK_EN               (0xA0F261F0U)
#define SSM_SF_CTRL_STS           (0xA0F261F4U)

#ifdef SSM_FEATURE_ECC
#define SSM_ECC_CFG_PW                  (0x5AFEACE5U)
#define SSM_ECC_CFG_WR_PW               (0xA0F26048U)

#define SSM_ECC_SRAM0_ERR_REQ_EN        (0xA0F26000U)
#define SSM_ECC_SRAM0_ERR_STS           (0xA0F26004U)
#define SSM_ECC_SRAM0_ERR_ADDR           (0xA0F26008U)

#define SSM_ECC_SFMC_ERR_REQ_EN        (0xA0F2600CU)
#define SSM_ECC_SFMC_ERR_STS           (0xA0F26010U)
#define SSM_ECC_SFMC_ERR_ADDR           (0xA0F26014U)

#define SSM_ECC_MBOX_ERR_REQ_EN        (0xA0F26018U)
#define SSM_ECC_MBOX_ERR_STS           (0xA0F2601CU)
#define SSM_ECC_MBOX_ERR_ADDR           (0xA0F26020U)

#define SSM_ECC_HSM_ERR_REQ_EN        (0xA0F26024U)
#define SSM_ECC_HSM_ERR_STS           (0xA0F26028U)
#define SSM_ECC_HSM_ERR_ADDR           (0xA0F2602CU)

#define SSM_ECC_MAX_HANDLERS            (10)

#define SSM_WRITE_VERIFY( tgt, compare) (SAL_ReadReg(tgt) == (compare))

typedef struct SSMECCHandlerItem
{
    ECC_FAULT_HANDLER                   ehiHandler;
    uint8                               ehiRegistered;

} SSMECCHandlerItem_t;
#endif

// AXI BUS Isolation
#ifdef SSM_FEATURE_ISO
/* Register Map */
#ifndef TCC_SM_BASE
#define TCC_SM_BASE                     (0x1B921000U)
#endif

#ifndef TCC_DSE_BASE
#define TCC_DSE_BASE                    (0x1B938000U)
#endif

#define BUS_CFG_PW                      (0x5AFEACE5U)
#define BUS_CFG_WR_PW                   (0xA4U)//0xA0U

#define BUS_AXI_MST_ISO_CFG             (0x80U)
#define BUS_AXI_MST_WCH_ISO_STS         (0x84U)
#define BUS_AXI_MST_RCH_ISO_STS         (0x88U)
#define BUS_AXI_SLV_ISO_CFG             (0x8CU)
#define BUS_AXI_SLV_TIMER_CFG           (0x90U)
#define BUS_AXI_SLV_WCH_ISO_STS         (0x94U)
#define BUS_AXI_SLV_RCH_ISO_STS         (0x98U)
#define BUS_AXI_SLV_TIMER_STS           (0x9CU)
#define BUS_AHB_SLV_ISO                 (0xA0U)

#define DSE_CFG_WR_PW                   (0x10U)
#define DSE_IRQ_EN                      (0x08U)
#define DSE_DEF_SLV_CFG                 (0x0CU)

/* Configuration Value */
#define BUS_AXI_TEST_FMU_EN             (BUS_FAULT_INJ_FMU << 24)
#define BUS_AXI_RCH_ISO_EN              (0x3U << 8)
#define BUS_AXI_WCH_ISO_EN              (0x3U)
#define BUS_AXI_RES_TIMER_EN            (0x1U << 16)
#define BUS_AHB_ISO_EN                  (0x1U)
#define BUS_AHB_ISO_DIS                 (0x0U)

#define BUS_ISOLATED                    (0x14U)
#define BUS_FAULT_DETECTED              (0x20U)

#define BUS_FMU_SEV_LV                  (FMU_SVL_MID) //FMU_SVL_LOW,FMU_SVL_HIGH

#define DSE_IH_EN                       (0x1U)
#define DSE_IRQ_AHB_EN                  (0x1U << 24)

#define BUS_MAX_HANDLERS                (10)

#define BUS_CHECK_ENABLED( confreg, stsreg, enval )                  \
    (((SAL_ReadReg(confreg) & (enval)) > 0) && ((SAL_ReadReg(stsreg) & BUS_ISOLATED) > 0U))

#define BUS_CHECK_DISABLED( confreg, stsreg, enval )                 \
    (((SAL_ReadReg(confreg) & (enval)) == 0) && ((SAL_ReadReg(stsreg) & BUS_ISOLATED) == 0U))

#define BUS_CHECK_TIMER_ENABLED                                                         \
    (((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) & BUS_AXI_RES_TIMER_EN) > 0U)     \
      && (SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_TIMER_CFG) > 0U)                        \
      && ((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_TIMER_STS) & 0x1U) == 0U))

#define BUS_CHECK_TIMER_DISABLED                                                        \
    (((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) & BUS_AXI_RES_TIMER_EN) == 0U)    \
      && ((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_TIMER_STS) & 0x1U) == 1U))

#define BUS_PASS(item,code)     \
        (SSM_LOGD("06. BUS Isolation Configuration (with Memory Sub System) And Verification, "#item " [ PASS -- " #code " ]\n"))

#define BUS_FAIL(item,code)     \
        (SSM_LOGD("06. BUS Isolation Configuration (with Memory Sub System) And Verification, "#item " [ FAIL -- " #code " ]\n"))

typedef struct BUSHandlerItem
{
    BUS_FAULT_HANDLER                   hiHandler;
    boolean                             hiRegistered;

} BUSHandlerItem_t;

typedef struct BUSHandlerTableItem
{
    BUSHandlerItem_t                    htiHandlerTable[BUS_MAX_HANDLERS];
    uint8                               hitHandlerCount;
} BUSHandleTable_t;
#endif

static void SSM_EccFmuHandlerForSram0
(
    void *                              pAarg
);



/*
***************************************************************************************************
                                             LOCAL VARIABLES
***************************************************************************************************
*/
static uint32                            gSfEnabled = FALSE;

#ifdef SSM_FEATURE_ECC
static uint32                            gEccEnabled = FALSE;
#endif

// AXI BUS Isolation
#ifdef SSM_FEATURE_ISO
static BUSHandleTable_t                 Bus_Handlers[BUS_TYPE_SIZE];
#endif

/*
***************************************************************************************************
                                         LOCAL FUNCTION PROTOTYPES
***************************************************************************************************
*/

#ifdef SSM_FEATURE_ECC
static void SSM_EccFmuHandlerForSram0
(
    void *                              pAarg
);

static void SSM_EccFmuHandlerForSfmc
(
    void *                              pAarg
);

static void SSM_EccFmuHandlerForMbox
(
    void *                              pAarg
);

static void SSM_EccFmuHandlerForHsm
(
    void *                              pAarg
);

#endif

// AXI BUS Isolation
#ifdef SSM_FEATURE_ISO
static void SSM_BusFmuHandler(BUSTypeId_t uiId);
static void SSM_BusMstWchFmuHandler(void * pAarg);
static void SSM_BusMstRchFmuHandler(void * pAarg);
static void SSM_BusSlvWchFmuHandler(void * pAarg);
static void SSM_BusSlvRchFmuHandler(void * pAarg);
static void SSM_BusSlvResTmrFmuHandler(void * pAarg);
static SALRetCode_t SSM_BusPasswordWrite(void);
static int16 SSM_BusGetNewID(BUSTypeId_t uiTypeId);
static SALRetCode_t SSM_BusEnableAxiMstWchIsolation(void);
static SALRetCode_t SSM_BusEnableAxiMstRchIsolation(void);
static SALRetCode_t SSM_BusDisableAxiMstWchIsolation(void);
static SALRetCode_t SSM_BusDisableAxiMstRchIsolation(void);
static SALRetCode_t SSM_BusEnableAxiSlvWchIsolation(void);
static SALRetCode_t SSM_BusEnableAxiSlvRchIsolation(void);
static SALRetCode_t SSM_BusDisableAxiSlvWchIsolation(void);
static SALRetCode_t SSM_BusDisableAxiSlvRchIsolation(void);
static SALRetCode_t SSM_BusEnableAxiSlvResTimer(uint32 uiTennanosec);
static SALRetCode_t SSM_BusDisableAxiSlvResTimer(void);
static SALRetCode_t SSM_BusEnableAhbMstIsolation(void);
static SALRetCode_t SSM_BusDisableAhbMstIsolation(void);
#endif

/*
***************************************************************************************************
*                                         FUNCTION DECLEARATION
***************************************************************************************************
*/
#ifdef SSM_FEATURE_ECC

/*
*********************************************************************************************************
*                                          SSM_EccFmuHandlerForSram0
*
* FMU error hander for sram0 ECC.
*
* @param        pAarg : void *
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*
*
* This interface is to return the current status of SRAM ECC
*
* @param        uwHandlerId : Issued ID of fault handler or 0 if the ID hasn't been issued
* @return       Bit [0] : Status of callback function
*                   0 : not registered, 1: registered
*               Bit [1-3] : Reserved
*               Bit [4] : SRAM ECC error request
*                   0 : disabled, 1 : enabled
*               Bit [5-7] : Reserved
*               Bit [8] : status of detection for SRAM0 ECC byte lane 0 error
*                   0 : not detected, 1 : detected
*               Bit [9] : status of detection for SRAM0 ECC byte lane 1 error
*                   0 : not detected, 1 : detected
*               Bit [10] : status of detection for SRAM0 ECC byte lane 2 error
*                   0 : not detected, 1 : detected
*               Bit [11] : status of detection for SRAM0 ECC byte lane 3 error
*                   0 : not detected, 1 : detected
*               Bit [12] : status of detection for SRAM0 ECC byte lane 4 error
*                   0 : not detected, 1 : detected
*               Bit [13] : status of detection for SRAM0 ECC byte lane 5 error
*                   0 : not detected, 1 : detected
*               Bit [14] : status of detection for SRAM0 ECC byte lane 6 error
*                   0 : not detected, 1 : detected
*               Bit [15] : status of detection for SRAM0 ECC byte lane 7 error
*                   0 : not detected, 1 : detected
*               Bit [16] : status of detection for SRAM1 ECC byte lane 0 error
*                   0 : not detected, 1 : detected
*               Bit [17] : status of detection for SRAM1 ECC byte lane 1 error
*                   0 : not detected, 1 : detected
*               Bit [18] : status of detection for SRAM1 ECC byte lane 2 error
*                   0 : not detected, 1 : detected
*               Bit [19] : status of detection for SRAM1 ECC byte lane 3 error
*                   0 : not detected, 1 : detected
*               Bit [20] : status of detection for SRAM1 ECC byte lane 4 error
*                   0 : not detected, 1 : detected
*               Bit [21] : status of detection for SRAM1 ECC byte lane 5 error
*                   0 : not detected, 1 : detected
*               Bit [22] : status of detection for SRAM1 ECC byte lane 6 error
*                   0 : not detected, 1 : detected
*               Bit [23] : status of detection for SRAM1 ECC byte lane 7 error
*                   0 : not detected, 1 : detected
*
* Notes
*
*********************************************************************************************************
*/

static void SSM_EccFmuHandlerForSram0(void * pAarg)
{
    uint16      index;
    uint32      ret = 0U;
    (void)pAarg;

    for (index = 0U; index < 8U; index++)
    {
        if (((SAL_ReadReg(SSM_ECC_SRAM0_ERR_STS) >> (index << 2U)) & 0xFU) > 0U)
        {
            ret |= (uint32)SRAM_ECC_ERR_REQ_ENABLED;
            ret |= ((uint32)SRAM_0_LANE0_FAULT << index);
        }
    }
    //status clear
    SAL_WriteReg(SAL_ReadReg(SSM_ECC_SRAM0_ERR_STS), SSM_ECC_SRAM0_ERR_STS);

    (void)FMU_IsrClr(FMU_ID_SRAM0_ECC);
    SSM_LOGD("[SSM] SRAM ECC Fault IRQ Handler Called Fault Address : 0x%x\n", *(uint32 *)(SSM_ECC_SRAM0_ERR_ADDR));
}

/*
*********************************************************************************************************
*                                          SSM_EccFmuHandlerForSfmc
*
* FMU error hander for Sfmc ECC.
*
* @param        pAarg : void *
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_EccFmuHandlerForSfmc(void * pAarg)
{
    (void)pAarg;

    SSM_LOGD("[SSM] SFMC ECC Fault IRQ Handler Called Fault Status : 0x%x\n", *(uint32 *)(SSM_ECC_SFMC_ERR_STS));
    SAL_WriteReg(SAL_ReadReg(SSM_ECC_SFMC_ERR_STS), SSM_ECC_SFMC_ERR_STS);

    (void)FMU_IsrClr(FMU_ID_SFMC_ECC);
    SSM_LOGD("[SSM] SFMC ECC Fault IRQ Handler Called Fault Address : 0x%x\n", *(uint32 *)(SSM_ECC_SFMC_ERR_ADDR));

}

/*
*********************************************************************************************************
*                                          SSM_EccFmuHandlerForMbox
*
* FMU error hander for Mbox ECC.
*
* @param        pAarg : void *
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_EccFmuHandlerForMbox(void * pAarg)
{
    (void)pAarg;
    SSM_LOGD("[SSM] MBOX ECC Fault IRQ Handler Called Fault Status : 0x%x\n", *(uint32 *)(SSM_ECC_MBOX_ERR_STS));
    SAL_WriteReg(SAL_ReadReg(SSM_ECC_MBOX_ERR_STS), SSM_ECC_MBOX_ERR_STS);

    (void)FMU_IsrClr(FMU_ID_MBOX0_S_ECC);
    SSM_LOGD("[SSM] MBOX ECC Fault IRQ Handler Called Fault Address : 0x%x\n", *(uint32 *)(SSM_ECC_MBOX_ERR_ADDR));
}

/*
*********************************************************************************************************
*                                          SSM_EccFmuHandlerForHsm
*
* FMU error hander for Hsm ECC.
*
* @param        pAarg : void *
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_EccFmuHandlerForHsm(void * pAarg)
{
    (void)pAarg;
    SSM_LOGD("[SSM] HSM ECC Fault IRQ Handler Called Fault Status : 0x%x\n", *(uint32 *)(SSM_ECC_HSM_ERR_STS));
    SAL_WriteReg(SAL_ReadReg(SSM_ECC_HSM_ERR_STS), SSM_ECC_HSM_ERR_STS);

    (void)FMU_IsrClr(FMU_ID_HSM_ECC);
    SSM_LOGD("[SSM] HSM ECC Fault IRQ Handler Called Fault Address : 0x%x\n", *(uint32 *)(SSM_ECC_HSM_ERR_ADDR));

}

/*
*********************************************************************************************************
*                                          SSM_ECCEnable
*
* Enable ECC encoder / decoder with SECDED to detect the fault of the SRAM memory itself
* when writing or reading data to the SRAM
*
* @param        ECCTypeID_t : one of the ECCTypeID_t enumeration.
* @param        uiFmuSeverityLevel : FMU_SVL_LOW, FMU_SVL_MID, FMU_SVL_HIGH
* @return       0 : Success, -1 : Already enabled, -2 : Fail to set
*
* Notes
*
*********************************************************************************************************
*/

int32 SSM_EccEnable( ECCTypeID_t uiEccType, FMUSeverityLevelType_t uiFmuSeverityLevel )
{
    int32 retVal = -2;
    FMUFaultid_t uiFmuFaultId = (FMUFaultid_t)0;

    if ( (uiEccType <= ECC_HSM) && (uiFmuSeverityLevel < FMU_SVL_REV) )
    {
        if(((gEccEnabled>>(uint32)uiEccType) & 0x1U) == 0x0U)
        {
             switch (uiEccType)
             {
                  case ECC_SRAM0:
                     uiFmuFaultId = FMU_ID_SRAM0_ECC;
                     (void)FMU_IsrHandler((FMUFaultid_t)uiFmuFaultId, (FMUSeverityLevelType_t)uiFmuSeverityLevel, (FMUIntFnctPtr)&SSM_EccFmuHandlerForSram0, NULL_PTR);
                     SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                     SAL_WriteReg(0xFFFFFFFFU, SSM_ECC_SRAM0_ERR_REQ_EN);
                     if (SSM_WRITE_VERIFY(SSM_ECC_SRAM0_ERR_REQ_EN, 0xFFFFFFFFU))
                     {
                         retVal = 0;
                     }
                     break;
                  case ECC_SFMC:
                     uiFmuFaultId = FMU_ID_SFMC_ECC;
                     (void)FMU_IsrHandler((FMUFaultid_t)uiFmuFaultId, (FMUSeverityLevelType_t)uiFmuSeverityLevel, (FMUIntFnctPtr)&SSM_EccFmuHandlerForSfmc, NULL_PTR);
                     SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                     SAL_WriteReg(0x000000FU, SSM_ECC_SFMC_ERR_REQ_EN);
                     if (SSM_WRITE_VERIFY(SSM_ECC_SFMC_ERR_REQ_EN, 0x000000FU))
                     {
                         retVal = 0;
                     }
                     break;
                  case ECC_MBOX:
                     uiFmuFaultId = FMU_ID_MBOX0_S_ECC;
                     (void)FMU_IsrHandler((FMUFaultid_t)uiFmuFaultId, (FMUSeverityLevelType_t)uiFmuSeverityLevel, (FMUIntFnctPtr)&SSM_EccFmuHandlerForMbox, NULL_PTR);
                     SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                     SAL_WriteReg(0x0000F0FU, SSM_ECC_MBOX_ERR_REQ_EN);
                     if (SSM_WRITE_VERIFY(SSM_ECC_MBOX_ERR_REQ_EN, 0x0000F0FU))
                     {
                         retVal = 0;
                     }
                     break;
                  case ECC_HSM:
                     uiFmuFaultId = FMU_ID_HSM_ECC;
                     (void)FMU_IsrHandler((FMUFaultid_t)uiFmuFaultId, (FMUSeverityLevelType_t)uiFmuSeverityLevel, (FMUIntFnctPtr)&SSM_EccFmuHandlerForHsm, NULL_PTR);
                     SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                     SAL_WriteReg(0x000000FU, SSM_ECC_MBOX_ERR_REQ_EN);
                     if (SSM_WRITE_VERIFY(SSM_ECC_MBOX_ERR_REQ_EN, 0x000000FU))
                     {
                         retVal = 0;
                     }
                     break;
                  default:
                     break;
             }
             // fmu enable
             if (FMU_Set((FMUFaultid_t)uiFmuFaultId) == FMU_OK)
             {
                 retVal = 0;
             }
        }
        else
        {
              retVal = -1;
        }

        if (retVal == 0)
        {
            gEccEnabled |= ((uint32)0x1 << (uint32)uiEccType);
        }
    }

    return retVal;
}

/*
*********************************************************************************************************
*                                          SSM_EccDisable
*
* Disable ECC encoder /  encoder with SECDED
*
* @param        uiEccType : one of the ECCTypeID_t enumeration.
* @return       0 : Success, -1 : Already disabled, -2 : Fail to set
*
* Notes
*
*********************************************************************************************************
*/
int32 SSM_EccDisable(ECCTypeID_t uiEccType)
{
    int32 retVal = -2;
    FMUFaultid_t uiFmuFaultId = (FMUFaultid_t)0;

    if ( uiEccType <= ECC_HSM )
    {
        if(((gEccEnabled>>(uint32)uiEccType) & 0x1U) == 0x1U)
        {
            switch (uiEccType)
            {
                 case ECC_SRAM0:
                    uiFmuFaultId = FMU_ID_SRAM0_ECC;
                    SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                    SAL_WriteReg(0x0U, SSM_ECC_SRAM0_ERR_REQ_EN);
                    if (SSM_WRITE_VERIFY(SSM_ECC_SRAM0_ERR_REQ_EN, 0x0000000U))
                    {
                        retVal = 0;
                    }
                    break;
                 case ECC_SFMC:
                    uiFmuFaultId = FMU_ID_SFMC_ECC;
                    SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                    SAL_WriteReg(0x0U, SSM_ECC_SFMC_ERR_REQ_EN);
                    if (SSM_WRITE_VERIFY(SSM_ECC_SFMC_ERR_REQ_EN, 0x0000000U))
                    {
                        retVal = 0;
                    }
                    break;
                 case ECC_MBOX:
                    uiFmuFaultId = FMU_ID_MBOX0_S_ECC;
                    SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                    SAL_WriteReg(0x0U, SSM_ECC_MBOX_ERR_REQ_EN);
                    if (SSM_WRITE_VERIFY(SSM_ECC_MBOX_ERR_REQ_EN, 0x0000000U))
                    {
                        retVal = 0;
                    }
                    break;
                 case ECC_HSM:
                    uiFmuFaultId = FMU_ID_HSM_ECC;
                    SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
                    SAL_WriteReg(0x0U, SSM_ECC_MBOX_ERR_REQ_EN);
                    if (SSM_WRITE_VERIFY(SSM_ECC_MBOX_ERR_REQ_EN, 0x0000000U))
                    {
                        retVal = 0;
                    }
                    break;
                 default:
                    break;
            }

            if (FMU_IsrClr((FMUFaultid_t)uiFmuFaultId) == FMU_OK)
            {
                retVal = 0;
            }
        }
        else
        {
            retVal = -1;
        }

        if (retVal == 0)
        {
            gEccEnabled &= ~(0x1U <<(uint32)uiEccType);
        }
    }

    return retVal;

}
#endif

/*
*********************************************************************************************************
*                                          SSM_SfFmuHandler
*
* FMU error hander for Soft Fault.
*
* @param        pAarg : void *
* @return       None
*
* Notes
*
*********************************************************************************************************
*/

static void SSM_SfFmuHandler(void * pAarg)
{
    (void)pAarg;

    SSM_LOGD("[SSM] SF Fault Handler Called Fault Status : 0x%x\n", *(uint32 *)(SSM_SF_CTRL_STS));
    #ifdef SSM_SOFT_FAULT_INJECTION_TEST
    SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
    SAL_WriteReg(SAL_ReadReg(0xA0F26000), 0xA0F26000);
    #endif
    //status clear
    SAL_WriteReg(SAL_ReadReg(SSM_SF_CTRL_STS), SSM_SF_CTRL_STS);

    (void)FMU_IsrClr(FMU_ID_SYS_SM_CFG);
}

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

int32 SSM_SoftFaultCheckEnable( FMUSeverityLevelType_t uiFmuSeverityLevel  )
{
    int32 retVal = -2;
    uint32 uiFcEn = 0;

    if ( gSfEnabled == FALSE )
    {
        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SYS_SM_CFG, (FMUSeverityLevelType_t)uiFmuSeverityLevel, (FMUIntFnctPtr)&SSM_SfFmuHandler, NULL_PTR);
        SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
        uiFcEn = SAL_ReadReg(SSM_SF_FAULT_CHK_EN);
        uiFcEn |= 0x7U; //SOFT_FAULT_CHK_EN[0:2]
        SAL_WriteReg(uiFcEn, SSM_SF_FAULT_CHK_EN);
        if (SSM_WRITE_VERIFY(SSM_SF_FAULT_CHK_EN, uiFcEn))
        {
         retVal = 0;
        }
        // fmu enable
        if (FMU_Set((FMUFaultid_t)FMU_ID_SYS_SM_CFG) == FMU_OK)
        {
            retVal = 0;
        }
    }
    else
    {
         retVal = -1;
    }

    if (retVal == 0)
    {
         gSfEnabled = TRUE;
    }

    return retVal;
}

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
int16 SSM_SoftFautlCheckDisable(void)
{
    int16 retVal = -2;
    uint32 uiFcEn = 0;

    if( gSfEnabled == TRUE )
    {
        SAL_WriteReg(SSM_ECC_CFG_PW, SSM_ECC_CFG_WR_PW);
        uiFcEn = SAL_ReadReg(SSM_SF_FAULT_CHK_EN);
        uiFcEn &= ~0x7U; //SOFT_FAULT_CHK_EN[0:2]
        SAL_WriteReg(uiFcEn, SSM_SF_FAULT_CHK_EN);

        if (SSM_WRITE_VERIFY(SSM_SF_FAULT_CHK_EN, uiFcEn))
        {
            retVal = 0;
        }

        if (FMU_IsrClr((FMUFaultid_t)FMU_ID_SYS_SM_CFG) == FMU_OK)
        {
            retVal = 0;
        }
    }
    else
    {
            retVal = -1;
    }

    if (retVal == 0)
    {
            gEccEnabled = FALSE;
    }

    return retVal;
}

#ifdef SSM_FEATURE_ISO
/*
*********************************************************************************************************
*                                          SSM_BusFmuHandler
*
* @param        id: BUSTypeId_t
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_BusFmuHandler
(
    BUSTypeId_t uiId
)
{
    uint16 index = 0U;
    if (Bus_Handlers[uiId].hitHandlerCount > 0U)
    {

        for (; index < BUS_MAX_HANDLERS; index++)
        {
            if (Bus_Handlers[uiId].htiHandlerTable[index].hiRegistered == TRUE)
            {
                (*(Bus_Handlers[uiId].htiHandlerTable[index].hiHandler))((uint32)uiId, SSM_BusGetStatus(index + 1U));
            }
        }
    }
}

/*
*********************************************************************************************************
*                                          SSM_BusMstWchFmuHandler
*
* @param        arg : void *
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_BusMstWchFmuHandler
(
    void * pAarg
)
{
    SSM_BusFmuHandler(BUS_AXI_MST_WCH_ISOLATION);
    (void)FMU_IsrClr(FMU_ID_X2X_MST1);
}

/*
*********************************************************************************************************
*                                          SSM_BusMstRchFmuHandler
*
* @param        None
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_BusMstRchFmuHandler
(
    void * pAarg
)
{
    SSM_BusFmuHandler(BUS_AXI_MST_RCH_ISOLATION);
    (void)FMU_IsrClr(FMU_ID_X2X_MST0);
}

/*
*********************************************************************************************************
*                                          SSM_BusSlvWchFmuHandler
*
* @param        None
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_BusSlvWchFmuHandler
(
    void * pAarg
)
{
    SSM_BusFmuHandler(BUS_AXI_SLV_WCH_ISOLATION);
    (void)FMU_IsrClr(FMU_ID_X2X_SLV1);
}

/*
*********************************************************************************************************
*                                          SSM_BusSlvRchFmuHandler
*
* @param        None
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_BusSlvRchFmuHandler
(
    void * pAarg
)
{
    SSM_BusFmuHandler(BUS_AXI_SLV_RCH_ISOLATION);
    (void)FMU_IsrClr(FMU_ID_X2X_SLV0);
}

/*
*********************************************************************************************************
*                                          SSM_BusSlvResTmrFmuHandler
*
* @param        None
* @return       None
*
* Notes
*
*********************************************************************************************************
*/
static void SSM_BusSlvResTmrFmuHandler
(
    void * pAarg
)
{
    SSM_BusFmuHandler(BUS_AXI_SLV_RES_TIMER);
    (void)FMU_IsrClr(FMU_ID_X2X_SLV2);
}

/*
*********************************************************************************************************
*                                          SSM_BusPasswordWrite
*
* This interface is first called before  AXI Isolation's configuration is set.
* It will be available to change the status of configuration by writing a predefined password.
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
static SALRetCode_t SSM_BusPasswordWrite(void)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (SAL_ReadReg(TCC_SM_BASE + BUS_CFG_WR_PW) != 0x1U)
    {
        SAL_WriteReg(BUS_CFG_PW, TCC_SM_BASE + BUS_CFG_WR_PW);

        if (SAL_ReadReg(TCC_SM_BASE + BUS_CFG_WR_PW) == 0x1U)
        {;}
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_CONFIG_PW,
                                     BUS_ERR_SET_PW,
                                     __FUNCTION__);
        }
    }
    else
    {
        SAL_DbgReportError(SAL_DRVID_ISO,
                           BUS_API_CONFIG_PW,
                           BUS_ERR_ALREADY_DONE,
                           __FUNCTION__);
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusGetNewID
*
* This function is to issue new ID to register a fault handler.
*
* @param        typeId : is to distinguish the kind of AXI isolation
* @return       -1 : Fail to issue new ID, 1~10 : Issued ID
*
* Notes
*
*********************************************************************************************************
*/
static int16 SSM_BusGetNewID
(
    BUSTypeId_t     uiTypeId
)
{
    int16           id = -1;
    int16           index = 0;

    if (Bus_Handlers[uiTypeId].hitHandlerCount < BUS_MAX_HANDLERS)
    {
        for (index = 0/* for C2143 on VC9.0 */; index < BUS_MAX_HANDLERS; index++)
        {
            if (Bus_Handlers[uiTypeId].htiHandlerTable[index].hiRegistered == FALSE)
            {
                id = index + 1;
                break;
            }
        }
    }

    return id;
}

/*
*********************************************************************************************************
*                                          SSM_BusEnableAxiMstWchIsolation
*
* Enable AXI Incoming Write Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusEnableAxiMstWchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_MST_WCH_ISO_STS,
                          BUS_AXI_WCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_ENABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Enabling AXI Master W Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        (void)FMU_IsrHandler(FMU_ID_X2X_MST1, BUS_FMU_SEV_LV, (FMUIntFnctPtr)&SSM_BusMstWchFmuHandler, NULL_PTR);
        ret = (FMU_Set(FMU_ID_X2X_MST1) == FMU_OK) ? SAL_RET_SUCCESS : SAL_RET_FAILED;

        if (ret == SAL_RET_SUCCESS)
        {
            if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
            {
                SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG) |(BUS_AXI_WCH_ISO_EN | BUS_AXI_TEST_FMU_EN),
                       TCC_SM_BASE + BUS_AXI_MST_ISO_CFG);

                // Unnecessary code(BUS_AXI_MST_WCH_ISO_STS, 0) in Controller Tester Tool ( Wired ), TCC_BUS_FAIL_ENABLE
                if (!BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                                       TCC_SM_BASE + BUS_AXI_MST_WCH_ISO_STS,
                                       BUS_AXI_WCH_ISO_EN))
                {
                    ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                             BUS_API_ENABLE,
                                             BUS_ERR_ENABLE,
                                             __FUNCTION__);

                    BUS_FAIL(Enabling AXI Master W Isolation, BUS_ERR_ENABLE);
                }
                else
                {
                    BUS_PASS(Enabling AXI Master W Isolation, BUS_OK);
                }
            }
            else
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_ENABLE,
                                         BUS_ERR_SET_PW,
                                         __FUNCTION__);

                BUS_FAIL(Enabling AXI Master W Isolation, BUS_ERR_SET_PW);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_ENABLE,
                                     BUS_ERR_ENABLE,
                                     __FUNCTION__);

            BUS_FAIL(Enabling AXI Master W Isolation, BUS_ERR_ENABLE);
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusEnableAxiMstRchIsolation
*
* Enable AXI Incoming Read Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusEnableAxiMstRchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_MST_RCH_ISO_STS,
                          BUS_AXI_RCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_ENABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Enabling AXI Master R Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        (void)FMU_IsrHandler(FMU_ID_X2X_MST0, BUS_FMU_SEV_LV, (FMUIntFnctPtr)&SSM_BusMstRchFmuHandler, NULL_PTR);
        ret = (FMU_Set(FMU_ID_X2X_MST0) == FMU_OK) ? SAL_RET_SUCCESS : SAL_RET_FAILED;

        if (ret == SAL_RET_SUCCESS)
        {
            if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
            {
                SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG) | (BUS_AXI_RCH_ISO_EN | BUS_AXI_TEST_FMU_EN),
                       TCC_SM_BASE + BUS_AXI_MST_ISO_CFG);

                // Unnecessary code(BUS_AXI_MST_RCH_ISO_STS, 0) in Controller Tester Tool ( Wired ), TCC_BUS_FAIL_ENABLE
                if (!BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                                       TCC_SM_BASE + BUS_AXI_MST_RCH_ISO_STS,
                                       BUS_AXI_RCH_ISO_EN))
                {
                    ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                             BUS_API_ENABLE,
                                             BUS_ERR_ENABLE,
                                             __FUNCTION__);

                    BUS_FAIL(Enabling AXI Master R Isolation, BUS_ERR_ENABLE);
                }
                else
                {
                    BUS_PASS(Enabling AXI Master R Isolation, BUS_OK);
                }
            }
            else
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_ENABLE,
                                         BUS_ERR_SET_PW,
                                         __FUNCTION__);

                BUS_FAIL(Enabling AXI Master R Isolation, BUS_ERR_SET_PW);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_ENABLE,
                                     BUS_ERR_ENABLE,
                                     __FUNCTION__);

            BUS_FAIL(Enabling AXI Master R Isolation, BUS_ERR_ENABLE);
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusDisableAxiMstWchIsolation
*
* Disable AXI Incoming Write  Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusDisableAxiMstWchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_FAILED;

    if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                           TCC_SM_BASE + BUS_AXI_MST_WCH_ISO_STS,
                           BUS_AXI_WCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_DISABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Disabling AXI Master W Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG) & (~BUS_AXI_WCH_ISO_EN),
                   TCC_SM_BASE + BUS_AXI_MST_ISO_CFG);

            if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                                   TCC_SM_BASE + BUS_AXI_MST_WCH_ISO_STS,
                                   BUS_AXI_WCH_ISO_EN))
            {
                ret = SAL_RET_SUCCESS;
            }
            else
            {
                // Unnecessary code(BUS_AXI_MST_WCH_ISO_STS) in Controller Tester Tool (wired), BUS_ERR_DISABLE
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Master W Isolation, BUS_ERR_DISABLE);
            }
        }

        if (ret == SAL_RET_SUCCESS)
        {
            if (FMU_IsrClr(FMU_ID_X2X_MST1) != FMU_OK)
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Master W Isolation, BUS_ERR_DISABLE);
            }
            else
            {
                BUS_PASS(Disabling AXI Master W Isolation, BUS_OK);
            }
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusDisableAxiMstRchIsolation
*
* Disable AXI Incoming Read Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusDisableAxiMstRchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_FAILED;

    if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                            TCC_SM_BASE + BUS_AXI_MST_RCH_ISO_STS,
                            BUS_AXI_RCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_DISABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Disabling AXI Master R Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG) & (~BUS_AXI_RCH_ISO_EN),
                   TCC_SM_BASE + BUS_AXI_MST_ISO_CFG);

            if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                                   TCC_SM_BASE + BUS_AXI_MST_RCH_ISO_STS,
                                   BUS_AXI_RCH_ISO_EN))
            {
                ret = SAL_RET_SUCCESS;
            }
            else
            {
                // Unnecessary code(BUS_AXI_MST_RCH_ISO_STS) in Controller Tester Tool (wired), BUS_ERR_DISABLE
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Master R Isolation, BUS_ERR_DISABLE);
            }
        }

        if (ret == SAL_RET_SUCCESS)
        {
            if (FMU_IsrClr(FMU_ID_X2X_MST0) != FMU_OK)
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Master R Isolation, BUS_ERR_DISABLE);
            }
            else
            {
                BUS_PASS(Disabling AXI Master R Isolation, BUS_OK);
            }
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusEnableAxiSlvWchIsolation
*
* Enable AXI Outgoing Write Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusEnableAxiSlvWchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_SLV_WCH_ISO_STS,
                          BUS_AXI_WCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_ENABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Enabling AXI Slave W Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        (void)FMU_IsrHandler(FMU_ID_X2X_SLV1, BUS_FMU_SEV_LV, (FMUIntFnctPtr)&SSM_BusSlvWchFmuHandler, NULL_PTR);
        ret = (FMU_Set(FMU_ID_X2X_SLV1) == FMU_OK) ? SAL_RET_SUCCESS : SAL_RET_FAILED;

        if (ret == SAL_RET_SUCCESS)
        {
            if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
            {
                SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) |(BUS_AXI_WCH_ISO_EN | BUS_AXI_TEST_FMU_EN),
                       TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG);

                // Unnecessary code(BUS_AXI_SLV_WCH_ISO_STS, 0) in Controller Tester Tool ( Wired ), TCC_BUS_FAIL_ENABLE
                if (!BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                                       TCC_SM_BASE + BUS_AXI_SLV_WCH_ISO_STS,
                                       BUS_AXI_WCH_ISO_EN))
                {
                    ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                             BUS_API_ENABLE,
                                             BUS_ERR_ENABLE,
                                             __FUNCTION__);

                    BUS_FAIL(Enabling AXI Slave W Isolation, BUS_ERR_ENABLE);
                }
                else
                {
                    BUS_PASS(Enabling AXI Slave W Isolation, BUS_OK);
                }
            }
            else
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_ENABLE,
                                         BUS_ERR_SET_PW,
                                         __FUNCTION__);

                BUS_FAIL(Enabling AXI Slave W Isolation, BUS_ERR_SET_PW);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_ENABLE,
                                     BUS_ERR_ENABLE,
                                     __FUNCTION__);

            BUS_FAIL(Enabling AXI Slave W Isolation, BUS_ERR_ENABLE);
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusEnableAxiSlvRchIsolation
*
* Enable AXI Outgoing Read Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusEnableAxiSlvRchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_SLV_RCH_ISO_STS,
                          BUS_AXI_RCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_ENABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Enabling AXI Slave R Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {

        (void)FMU_IsrHandler(FMU_ID_X2X_SLV0, BUS_FMU_SEV_LV, (FMUIntFnctPtr)&SSM_BusSlvRchFmuHandler, NULL_PTR);
        ret = (FMU_Set(FMU_ID_X2X_SLV0) == FMU_OK) ? SAL_RET_SUCCESS : SAL_RET_FAILED;

        if (ret == SAL_RET_SUCCESS)
        {
            if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
            {
                SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) |(BUS_AXI_RCH_ISO_EN | BUS_AXI_TEST_FMU_EN),
                       TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG);

                // Unnecessary code(BUS_AXI_SLV_RCH_ISO_STS, 0) in Controller Tester Tool ( Wired ), TCC_BUS_FAIL_ENABLE
                if (!BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                                       TCC_SM_BASE + BUS_AXI_SLV_RCH_ISO_STS,
                                       BUS_AXI_RCH_ISO_EN))
                {
                    ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                             BUS_API_ENABLE,
                                             BUS_ERR_ENABLE,
                                             __FUNCTION__);

                    BUS_FAIL(Enabling AXI Slave R Isolation, BUS_ERR_ENABLE);
                }
                else
                {
                    BUS_PASS(Enabling AXI Slave R Isolation, BUS_OK);
                }
            }
            else
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_ENABLE,
                                         BUS_ERR_SET_PW,
                                         __FUNCTION__);

                BUS_FAIL(Enabling AXI Slave R Isolation, BUS_ERR_SET_PW);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_ENABLE,
                                     BUS_ERR_ENABLE,
                                     __FUNCTION__);

            BUS_FAIL(Enabling AXI Slave R Isolation, BUS_ERR_ENABLE);
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusDisableAxiSlvWchIsolation
*
* Disable AXI Outgoing Write  Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusDisableAxiSlvWchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_FAILED;

    if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                           TCC_SM_BASE + BUS_AXI_SLV_WCH_ISO_STS,
                           BUS_AXI_WCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_DISABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Disabling AXI Slave W Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) & (~BUS_AXI_WCH_ISO_EN),
                   TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG);

            if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                                   TCC_SM_BASE + BUS_AXI_SLV_WCH_ISO_STS,
                                   BUS_AXI_WCH_ISO_EN))
            {
                ret = SAL_RET_SUCCESS;
            }
            else
            {
                // Unnecessary code(BUS_AXI_SLV_WCH_ISO_STS) in Controller Tester Tool (wired), BUS_ERR_DISABLE
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Slave W Isolation, BUS_ERR_DISABLE);
            }
        }

        if (ret == SAL_RET_SUCCESS)
        {
            if (FMU_IsrClr(FMU_ID_X2X_SLV1) != FMU_OK)
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Slave W Isolation, BUS_ERR_DISABLE);
            }
            else
            {
                BUS_PASS(Disabling AXI Slave W Isolation, BUS_OK);
            }
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusDisableAxiSlvRchIsolation
*
* Disable AXI Outgoing Read Channel Isolation
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusDisableAxiSlvRchIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_FAILED;

    if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                           TCC_SM_BASE + BUS_AXI_SLV_RCH_ISO_STS,
                           BUS_AXI_RCH_ISO_EN))
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_DISABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Disabling AXI Slave R Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) & (~BUS_AXI_RCH_ISO_EN),
                   TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG);

            if (BUS_CHECK_DISABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                                   TCC_SM_BASE + BUS_AXI_SLV_RCH_ISO_STS,
                                   BUS_AXI_RCH_ISO_EN))
            {
                ret = SAL_RET_SUCCESS;
            }
            else
            {
                // Unnecessary code(BUS_AXI_SLV_RCH_ISO_STS) in Controller Tester Tool (wired), BUS_ERR_DISABLE
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Slave R Isolation, BUS_ERR_DISABLE);
            }
        }

        if (ret == SAL_RET_SUCCESS)
        {
            if (FMU_IsrClr(FMU_ID_X2X_SLV0) != FMU_OK)
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AXI Slave R Isolation, BUS_ERR_DISABLE);
            }
            else
            {
                BUS_PASS(Disabling AXI Slave R Isolation, BUS_OK);
            }
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusEnableAxiSlvResTimer
*
* Enable AXI Outgoing Transaction Response Timer
*
* @param        tennanosec : new response time (10ns ~ 1s : Bus clock time per tick(about 3ns) * resTime )
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusEnableAxiSlvResTimer
(
    uint32          uiTennanosec
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (BUS_CHECK_TIMER_ENABLED)
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_ENABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Enabling Slave Response Timer, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        (void)FMU_IsrHandler(FMU_ID_X2X_SLV2, BUS_FMU_SEV_LV, (FMUIntFnctPtr)&SSM_BusSlvResTmrFmuHandler, NULL_PTR);
        ret = (FMU_Set(FMU_ID_X2X_SLV2) == FMU_OK) ? SAL_RET_SUCCESS : SAL_RET_FAILED;

        if (ret == SAL_RET_SUCCESS)
        {
            ret = SSM_BusSetAxiSlvResTime(uiTennanosec);
        }

        if (ret == SAL_RET_SUCCESS)
        {
            if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
            {
                SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) |(BUS_AXI_RES_TIMER_EN | BUS_AXI_TEST_FMU_EN),
                       TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG);

                // Unnecessary code(BUS_AXI_SLV_TIMER_STS, 0) in Controller Tester Tool ( Wired ), TCC_BUS_FAIL_ENABLE
                if (!BUS_CHECK_TIMER_ENABLED)
                {
                    ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                             BUS_API_ENABLE,
                                             BUS_ERR_ENABLE,
                                             __FUNCTION__);

                    BUS_FAIL(Enabling Slave Response Timer, BUS_ERR_ENABLE);
                }
                else
                {
                    BUS_PASS(Enabling Slave Response Timer, BUS_OK);
                }
            }
            else
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_ENABLE,
                                         BUS_ERR_SET_PW,
                                         __FUNCTION__);

                BUS_FAIL(Enabling Slave Response Timer, BUS_ERR_SET_PW);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_ENABLE,
                                     BUS_ERR_ENABLE,
                                     __FUNCTION__);

            BUS_FAIL(Enabling Slave Response Timer, BUS_ERR_ENABLE);
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusDisableAxiSlvResTimer
*
* Disable AXI Outgoing Transaction Response Timer
*
* @param        None
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusDisableAxiSlvResTimer(void)
{
    SALRetCode_t    ret = SAL_RET_FAILED;

    if (BUS_CHECK_TIMER_DISABLED)
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_DISABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Disabling Slave Response Timer, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            SAL_WriteReg(SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG) & (~BUS_AXI_RES_TIMER_EN),
                   TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG);

            if (BUS_CHECK_TIMER_DISABLED)
            {
                ret = SAL_RET_SUCCESS;
            }
            else
            {
                // Unnecessary code(BUS_AXI_SLV_TIMER_STS) in Controller Tester Tool (wired), BUS_ERR_DISABLE
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling Slave Response Timer, BUS_ERR_DISABLE);
            }
        }

        if (ret == SAL_RET_SUCCESS)
        {
            if (FMU_IsrClr(FMU_ID_X2X_SLV2) != FMU_OK)
            {
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling Slave Response Timer, BUS_ERR_DISABLE);
            }
            else
            {
                BUS_PASS(Disabling Slave Response Timer, BUS_OK);
            }
        }
    }

    return ret;
}

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
    uint32          uiTennanosec
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (100000000UL >= uiTennanosec)
    {
        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            uint32 value = (uiTennanosec * 10UL) / 3UL;

            SAL_WriteReg(value, TCC_SM_BASE + BUS_AXI_SLV_TIMER_CFG);

            if (SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_TIMER_CFG) == value)
            {
                BUS_PASS(Setting Slave Response Time, BUS_OK);
            }
            else
            {
                // Unnecessary code (BUS_ERR_SET_TIME) in Controller Tester Tool ( Wired )
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_SET_SLAVE_RESTIME,
                                         BUS_ERR_SET_TIME,
                                         __FUNCTION__);

                BUS_FAIL(Setting Slave Response Time, BUS_ERR_SET_TIME);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_SET_SLAVE_RESTIME,
                                     BUS_ERR_SET_TIME,
                                     __FUNCTION__);

            BUS_FAIL(Setting Slave Response Time, BUS_ERR_SET_TIME);
        }
    }
    else
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_SET_SLAVE_RESTIME,
                                 BUS_ERR_OUTOFRANGE_TIME,
                                 __FUNCTION__);

        BUS_FAIL(Setting Slave Response Time, BUS_ERR_OUTOFRANGE_TIME);
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusEnableAhbMstIsolation
*
*
*
* @param        void
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
static SALRetCode_t SSM_BusEnableAhbMstIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if (SAL_ReadReg(TCC_SM_BASE + BUS_AHB_SLV_ISO) == BUS_AHB_ISO_EN)
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_ENABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Enabling AHB Slave Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        SAL_WriteReg(DSE_IH_EN, TCC_DSE_BASE + DSE_IRQ_EN);
        SAL_WriteReg(SAL_ReadReg(TCC_DSE_BASE + DSE_DEF_SLV_CFG) | DSE_IRQ_AHB_EN, TCC_DSE_BASE + DSE_DEF_SLV_CFG);

        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            SAL_WriteReg(BUS_AHB_ISO_EN, TCC_SM_BASE + BUS_AHB_SLV_ISO);

            if (SAL_ReadReg(TCC_SM_BASE + BUS_AHB_SLV_ISO) != BUS_AHB_ISO_EN)
            {
                // Unnecessary code(BUS_AXI_MST_WCH_ISO_STS, 0) in Controller Tester Tool ( Wired ), TCC_BUS_FAIL_ENABLE
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_ENABLE,
                                         BUS_ERR_ENABLE,
                                         __FUNCTION__);

                BUS_FAIL(Enabling AHB Slave Isolation, BUS_ERR_ENABLE);
            }
            else
            {
                BUS_PASS(Enabling AHB Slave Isolation, BUS_OK);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_ENABLE,
                                     BUS_ERR_SET_PW,
                                     __FUNCTION__);

            BUS_FAIL(Enabling AHB Slave Isolation, BUS_ERR_SET_PW);
        }
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusDisableAhbMstIsolation
*
*
*
* @param        void
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
static SALRetCode_t SSM_BusDisableAhbMstIsolation(void)
{
    SALRetCode_t    ret = SAL_RET_FAILED;

    if (SAL_ReadReg(TCC_SM_BASE + BUS_AHB_SLV_ISO) == BUS_AHB_ISO_DIS)
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_DISABLE,
                                 BUS_ERR_ALREADY_DONE,
                                 __FUNCTION__);

        BUS_FAIL(Disabling AHB Slave Isolation, BUS_ERR_ALREADY_DONE);
    }
    else
    {
        SAL_WriteReg(SAL_ReadReg(TCC_DSE_BASE + DSE_DEF_SLV_CFG) & (~DSE_IRQ_AHB_EN), TCC_DSE_BASE + DSE_DEF_SLV_CFG);

        if (SSM_BusPasswordWrite() == SAL_RET_SUCCESS)
        {
            SAL_WriteReg(BUS_AHB_ISO_DIS, TCC_SM_BASE + BUS_AHB_SLV_ISO);

            if (SAL_ReadReg(TCC_SM_BASE + BUS_AHB_SLV_ISO) != BUS_AHB_ISO_DIS)
            {
                // Unnecessary code(BUS_AXI_MST_WCH_ISO_STS) in Controller Tester Tool (wired), BUS_ERR_DISABLE
                ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_DISABLE,
                                         BUS_ERR_DISABLE,
                                         __FUNCTION__);

                BUS_FAIL(Disabling AHB Slave Isolation, BUS_ERR_DISABLE);
            }
            else
            {
                BUS_PASS(Disabling AHB Slave Isolation, BUS_OK);
            }
        }
        else
        {
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_DISABLE,
                                     BUS_ERR_SET_PW,
                                     __FUNCTION__);

            BUS_FAIL(Disabling AHB Slave Isolation, BUS_ERR_SET_PW);
        }
    }

    return ret;
}

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
    BUSTypeId_t     uiType,
    uint32          uiTennanosec
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    switch (uiType)
    {
        case BUS_AXI_MST_WCH_ISOLATION :
            ret = SSM_BusEnableAxiMstWchIsolation();
            break;

        case BUS_AXI_MST_RCH_ISOLATION :
            ret = SSM_BusEnableAxiMstRchIsolation();
            break;

        case BUS_AXI_SLV_WCH_ISOLATION :
            ret = SSM_BusEnableAxiSlvWchIsolation();
            break;

        case BUS_AXI_SLV_RCH_ISOLATION :
            ret = SSM_BusEnableAxiSlvRchIsolation();
            break;

        case BUS_AXI_SLV_RES_TIMER :

        {
            ret = SSM_BusDisableAxiSlvWchIsolation();

            if (ret == SAL_RET_FAILED) break;

            ret = SSM_BusDisableAxiSlvRchIsolation();

            if (ret == SAL_RET_FAILED) break;

            ret = SSM_BusEnableAxiSlvResTimer(uiTennanosec);

            break;
        }

        case BUS_AHB_MST_ISOLATION :
            ret = SSM_BusEnableAhbMstIsolation();
            break;

        default :
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_ENABLE,
                                     BUS_ERR_INVALID_PARAM,
                                     __FUNCTION__);

            BUS_FAIL(Enabling Isolation, BUS_ERR_INVALID_PARAM);
            break;
    }

    return ret;
}

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
    BUSTypeId_t     uiType
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    switch (uiType)
    {
        case BUS_AXI_MST_WCH_ISOLATION :
            ret = SSM_BusDisableAxiMstWchIsolation();
            break;

        case BUS_AXI_MST_RCH_ISOLATION :
            ret = SSM_BusDisableAxiMstRchIsolation();
            break;

        case BUS_AXI_SLV_WCH_ISOLATION :
            ret = SSM_BusDisableAxiSlvWchIsolation();
            break;

        case BUS_AXI_SLV_RCH_ISOLATION :
            ret = SSM_BusDisableAxiSlvRchIsolation();
            break;

        case BUS_AXI_SLV_RES_TIMER :
            ret = SSM_BusDisableAxiSlvResTimer();
            break;

        case BUS_AHB_MST_ISOLATION :
            ret = SSM_BusDisableAhbMstIsolation();
            break;

        default :
            ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_DISABLE,
                                     BUS_ERR_INVALID_PARAM,
                                     __FUNCTION__);

            BUS_FAIL(Disabling Isolation, BUS_ERR_INVALID_PARAM);
            break;
    }

    return ret;
}

/*
*********************************************************************************************************
*                                          SSM_BusRegisterFaultHandler
*
* This interface is to register the handler to inform external controller of error interrupt.
*
* @param        typeId : is to distinguish the kind of AXI isolation
*               uwHandlerId : 0 when registering the handler for the first time or the previously issued ID(1~10)
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
    BUSTypeId_t         uiType,
    uint16              uwHandlerId,
    BUS_FAULT_HANDLER   fnFaultHandler
)
{
    int16               newId = 0;
    int16               retId = BUS_ERR_FULL_SLOT;

    if (Bus_Handlers[uiType].hitHandlerCount < BUS_MAX_HANDLERS)
    {
        if (uwHandlerId == 0U)
        {
            newId = SSM_BusGetNewID(uiType);

            if (newId > 0)
            {
                Bus_Handlers[uiType].htiHandlerTable[newId - 1].hiHandler       = fnFaultHandler;
                Bus_Handlers[uiType].htiHandlerTable[newId - 1].hiRegistered    = TRUE;

                Bus_Handlers[uiType].hitHandlerCount++;

                retId = newId;
                BUS_PASS(Handling Fault Interrupt, BUS_OK);
            }
            else
            {
                (void)SAL_DbgReportError(SAL_DRVID_ISO,
                                         BUS_API_REG_FAULT_HANDLER,
                                         BUS_ERR_GET_NEW_HANDLER_ID,
                                         __FUNCTION__);

                BUS_FAIL(Handling Fault Interrupt, Fail Issue Id);
            }
        }
        else if (uwHandlerId <= BUS_MAX_HANDLERS)
        {
            Bus_Handlers[uiType].htiHandlerTable[uwHandlerId - 1].hiHandler = fnFaultHandler;

            if (Bus_Handlers[uiType].htiHandlerTable[uwHandlerId - 1].hiRegistered == FALSE)
            {
                Bus_Handlers[uiType].htiHandlerTable[uwHandlerId - 1].hiRegistered = TRUE;
                Bus_Handlers[uiType].hitHandlerCount++;
            }

            retId = (int16)uwHandlerId;
            BUS_PASS(Handling Fault Interrupt, BUS_OK);
        }
        else
        {
            retId = BUS_ERR_INVALID_ID;
            (void)SAL_DbgReportError(SAL_DRVID_ISO,
                                     BUS_API_REG_FAULT_HANDLER,
                                     BUS_ERR_INVALID_ID,
                                     __FUNCTION__);

            BUS_FAIL(Handling Fault Interrupt, BUS_ERR_INVALID_ID);
        }
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_REG_FAULT_HANDLER,
                                 BUS_ERR_FULL_SLOT,
                                 __FUNCTION__);

        BUS_FAIL(Handling Fault Interrupt, BUS_ERR_FULL_SLOT);
    }

    return retId;
}

/*
*********************************************************************************************************
*                                          SSM_BusUnregisterFaultHandler
*
* This interface is to unregister the handler to inform external controller of error interrupt.
*
* @param        typeId : is to distinguish the kind of AXI isolation
*               uwHandlerId : the previously issued ID(1~10) to unregister the handler function.
* @return       Tcc_Bus_Ret
*
* Notes
*
*********************************************************************************************************
*/
SALRetCode_t SSM_BusUnregisterFaultHandler
(
    BUSTypeId_t     uiType,
    uint16          uwHandlerId
)
{
    SALRetCode_t    ret = SAL_RET_SUCCESS;

    if ((1U <= uwHandlerId) && (uwHandlerId <= BUS_MAX_HANDLERS))
    {
        Bus_Handlers[uiType].htiHandlerTable[uwHandlerId- 1].hiHandler = NULL_PTR;
        Bus_Handlers[uiType].htiHandlerTable[uwHandlerId- 1].hiRegistered = FALSE;

        Bus_Handlers[uiType].hitHandlerCount--;

        BUS_PASS(Handling Fault Interrupt, BUS_OK);
    }
    else
    {
        ret = SAL_DbgReportError(SAL_DRVID_ISO,
                                 BUS_API_UNREG_FAULT_HANDLER,
                                 BUS_ERR_INVALID_ID,
                                 __FUNCTION__);

        BUS_FAIL(Handling Fault Interrupt, BUS_ERR_INVALID_ID);
    }

    return ret;
}

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
    uint16          uwHandlerId
)
{
    uint32 ret = 0;
    uint16 index = 0;

    for (; index < BUS_TYPE_SIZE; index++)
    {
        if ((Bus_Handlers[index].hitHandlerCount > 0)
            && ((1 <= uwHandlerId) && (uwHandlerId <= BUS_MAX_HANDLERS))
            && (Bus_Handlers[index].htiHandlerTable[uwHandlerId - 1].hiRegistered == TRUE))
        {
            ret |= ((uint32)BUS_STS_CB_AXI_MST_WCH_REGISTERED << index);
        }
    }

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_MST_WCH_ISO_STS,
                          BUS_AXI_WCH_ISO_EN))
    {
        ret |= BUS_STS_AXI_MST_WCH_ENABLED;
    }

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_MST_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_MST_RCH_ISO_STS,
                          BUS_AXI_RCH_ISO_EN))
    {
        ret |= BUS_STS_AXI_MST_RCH_ENABLED;
    }

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_SLV_WCH_ISO_STS,
                          BUS_AXI_WCH_ISO_EN))
    {
        ret |= BUS_STS_AXI_SLV_WCH_ENABLED;
    }

    if (BUS_CHECK_ENABLED(TCC_SM_BASE + BUS_AXI_SLV_ISO_CFG,
                          TCC_SM_BASE + BUS_AXI_SLV_RCH_ISO_STS,
                          BUS_AXI_RCH_ISO_EN))
    {
        ret |= BUS_STS_AXI_SLV_RCH_ENABLED;
    }

    if (BUS_CHECK_TIMER_ENABLED)
    {
        ret |= BUS_STS_AXI_SLV_RES_TIMER_ENABLED;
    }

    if (SAL_ReadReg(TCC_SM_BASE + BUS_AHB_SLV_ISO) == BUS_AHB_ISO_EN)
    {
        ret |= BUS_STS_AHB_SLV_ENABLED;
    }

    if ((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_MST_WCH_ISO_STS) & BUS_FAULT_DETECTED) > 0)
    {
        ret |= BUS_STS_AXI_MST_WCH_FAULT;
    }

    if ((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_MST_RCH_ISO_STS) & BUS_FAULT_DETECTED) > 0)
    {
        ret |= BUS_STS_AXI_MST_RCH_FAULT;
    }

    if ((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_WCH_ISO_STS) & BUS_FAULT_DETECTED) > 0)
    {
        ret |= BUS_STS_AXI_SLV_WCH_FAULT;
    }

    if ((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_RCH_ISO_STS) & BUS_FAULT_DETECTED) > 0)
    {
        ret |= BUS_STS_AXI_SLV_RCH_FAULT;
    }

    if ((SAL_ReadReg(TCC_SM_BASE + BUS_AXI_SLV_TIMER_STS) & (BUS_FAULT_DETECTED >> 4)) > 0)
    {
        ret |= BUS_STS_AXI_SLV_RES_TIMER_FAULT;
    }

    return ret;
}
#endif

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )


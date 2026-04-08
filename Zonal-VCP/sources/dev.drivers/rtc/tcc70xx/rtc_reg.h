// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : rtc_reg.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_RTC_REG_HEADER
#define MCU_BSP_RTC_REG_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <sal_com.h>
#include <reg_phys.h>


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/* Common (Register function) ===============================================*/
#define RTC_REG_GET(o, m, a)           ( ((*(SALReg32 *)((uint32)a)) & (uint32)m ) >> (uint32)o )
#define RTC_REG_SET(x, o, m, a)        (*(SALReg32 *)((uint32)a)) = (((uint32)x << (uint32)o) & (uint32)m)
#define RTC_REG_APPEND(x, o, m, a)     ( (*(SALReg32 *)((uint32)a)) = ( ((*(SALReg32 *)((uint32)a)) & (~((uint32) m))) | ((uint32)x << o) ) )

/* Common (Bit Flags) ===============================================*/
#define RTC_VA_MASK_1                  (0x1UL)
#define RTC_VA_MASK_2                  (0x3UL)
#define RTC_VA_MASK_3                  (0x7UL)
#define RTC_VA_MASK_4                  (0xFUL)
#define RTC_VA_MASK_5                  (0x1FUL)
#define RTC_VA_MASK_6                  (0x3FUL)
#define RTC_VA_MASK_7                  (0x7FUL)
#define RTC_VA_MASK_8                  (0xFFUL)
#define RTC_VA_MASK_9                  (0x1FFUL)
#define RTC_VA_MASK_10                 (0x3FFUL)
#define RTC_VA_MASK_11                 (0x7FFUL)
#define RTC_VA_MASK_12                 (0xFFFUL)
#define RTC_VA_MASK_13                 (0x1FFFUL)
#define RTC_VA_MASK_14                 (0x3FFFUL)
#define RTC_VA_MASK_15                 (0x7FFFUL)
#define RTC_VA_MASK_16                 (0xFFFFUL)
#define RTC_VA_MASK_17                 (0x1FFFFUL)
#define RTC_VA_MASK_18                 (0x3FFFFUL)
#define RTC_VA_MASK_19                 (0x7FFFFUL)
#define RTC_VA_MASK_20                 (0xFFFFFUL)
#define RTC_VA_MASK_21                 (0x1FFFFFUL)
#define RTC_VA_MASK_22                 (0x3FFFFFUL)
#define RTC_VA_MASK_23                 (0x7FFFFFUL)
#define RTC_VA_MASK_24                 (0xFFFFFFUL)
#define RTC_VA_MASK_25                 (0x1FFFFFFUL)
#define RTC_VA_MASK_26                 (0x3FFFFFFUL)
#define RTC_VA_MASK_27                 (0x7FFFFFFUL)
#define RTC_VA_MASK_28                 (0xFFFFFFFUL)
#define RTC_VA_MASK_29                 (0x1FFFFFFFUL)
#define RTC_VA_MASK_30                 (0x3FFFFFFFUL)
#define RTC_VA_MASK_31                 (0x7FFFFFFFUL)
#define RTC_VA_MASK_32                 (0xFFFFFFFFUL)

/* REGISTER INFO (Map: RTC Address table) ==================================*/
#define RTC_ADDR_BASE                                       (MCU_BSP_RTC_BASE)
#define RTC_ADDR_RTCCON                                     (RTC_ADDR_BASE + 0x00UL)
#define RTC_ADDR_INTCON                                     (RTC_ADDR_BASE + 0x04UL)
#define RTC_ADDR_RTCALM                                     (RTC_ADDR_BASE + 0x08UL)
#define RTC_ADDR_ALMSEC                                     (RTC_ADDR_BASE + 0x0CUL)
#define RTC_ADDR_ALMMIN                                     (RTC_ADDR_BASE + 0x10UL)
#define RTC_ADDR_ALMHOUR                                    (RTC_ADDR_BASE + 0x14UL)
#define RTC_ADDR_ALMDATE                                    (RTC_ADDR_BASE + 0x18UL)
#define RTC_ADDR_ALMDAY                                     (RTC_ADDR_BASE + 0x1CUL)
#define RTC_ADDR_ALMMON                                     (RTC_ADDR_BASE + 0x20UL)
#define RTC_ADDR_ALMYEAR                                    (RTC_ADDR_BASE + 0x24UL)
#define RTC_ADDR_BCDSEC                                     (RTC_ADDR_BASE + 0x28UL)
#define RTC_ADDR_BCDMIN                                     (RTC_ADDR_BASE + 0x2CUL)
#define RTC_ADDR_BCDHOUR                                    (RTC_ADDR_BASE + 0x30UL)
#define RTC_ADDR_BCDDATE                                    (RTC_ADDR_BASE + 0x34UL)
#define RTC_ADDR_BCDDAY                                     (RTC_ADDR_BASE + 0x38UL)
#define RTC_ADDR_BCDMON                                     (RTC_ADDR_BASE + 0x3CUL)
#define RTC_ADDR_BCDYEAR                                    (RTC_ADDR_BASE + 0x40UL)
#define RTC_ADDR_RTCIM                                      (RTC_ADDR_BASE + 0x44UL)
#define RTC_ADDR_RTCPEND                                    (RTC_ADDR_BASE + 0x48UL)

/* REGISTER INFO (Map: RTC Register controller) ============================*/
#define RTC_REG_RTCCON                                      (*(SALReg32 *)(RTC_ADDR_RTCCON))
#define RTC_REG_INTCON                                      (*(SALReg32 *)(RTC_ADDR_INTCON))
#define RTC_REG_RTCALM                                      (*(SALReg32 *)(RTC_ADDR_RTCALM))
#define RTC_REG_ALMSEC                                      (*(SALReg32 *)(RTC_ADDR_ALMSEC))
#define RTC_REG_ALMMIN                                      (*(SALReg32 *)(RTC_ADDR_ALMMIN))
#define RTC_REG_ALMHOUR                                     (*(SALReg32 *)(RTC_ADDR_ALMHOUR))
#define RTC_REG_ALMDATE                                     (*(SALReg32 *)(RTC_ADDR_ALMDATE))
#define RTC_REG_ALMDAY                                      (*(SALReg32 *)(RTC_ADDR_ALMDAY))
#define RTC_REG_ALMMON                                      (*(SALReg32 *)(RTC_ADDR_ALMMON))
#define RTC_REG_ALMYEAR                                     (*(SALReg32 *)(RTC_ADDR_ALMYEAR))
#define RTC_REG_BCDSEC                                      (*(SALReg32 *)(RTC_ADDR_BCDSEC))
#define RTC_REG_BCDMIN                                      (*(SALReg32 *)(RTC_ADDR_BCDMIN))
#define RTC_REG_BCDHOUR                                     (*(SALReg32 *)(RTC_ADDR_BCDHOUR))
#define RTC_REG_BCDDATE                                     (*(SALReg32 *)(RTC_ADDR_BCDDATE))
#define RTC_REG_BCDDAY                                      (*(SALReg32 *)(RTC_ADDR_BCDDAY))
#define RTC_REG_BCDMON                                      (*(SALReg32 *)(RTC_ADDR_BCDMON))
#define RTC_REG_BCDYEAR                                     (*(SALReg32 *)(RTC_ADDR_BCDYEAR))
#define RTC_REG_RTCIM                                       (*(SALReg32 *)(RTC_ADDR_RTCIM))
#define RTC_REG_RTCPEND                                     (*(SALReg32 *)(RTC_ADDR_RTCPEND))

/* FIELD INFO (Common) ===============================================*/
#define RTC_ADDR_COMMON_FIELD_ZERO                          (0UL)
#define RTC_ADDR_COMMON_FIELD_FULL_MASK                     (RTC_VA_MASK_31)
#define RTC_ADDR_COMMON_FIELD_CLEAR_MASK                    (0UL)

/* FIELD INFO (Register: RTCCON) ===========================================*/
#define RTC_ADDR_RTCCON_FIELD_AIOUTEN                       (6UL)
#define RTC_ADDR_RTCCON_FIELD_AIOUTEN_MASK                  (RTC_VA_MASK_1 << RTC_ADDR_RTCCON_FIELD_AIOUTEN)
#define RTC_ADDR_RTCCON_FIELD_OSCEN                         (5UL)
#define RTC_ADDR_RTCCON_FIELD_OSCEN_MASK                    (RTC_VA_MASK_1 << RTC_ADDR_RTCCON_FIELD_OSCEN)
#define RTC_ADDR_RTCCON_FIELD_CLKRST                        (4UL)
#define RTC_ADDR_RTCCON_FIELD_CLKRST_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCCON_FIELD_CLKRST)
#define RTC_ADDR_RTCCON_FIELD_CNTSEL                        (3UL)
#define RTC_ADDR_RTCCON_FIELD_CNTSEL_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCCON_FIELD_CNTSEL)
#define RTC_ADDR_RTCCON_FIELD_CLKSEL                        (2UL)
#define RTC_ADDR_RTCCON_FIELD_CLKSEL_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCCON_FIELD_CLKSEL)
#define RTC_ADDR_RTCCON_FIELD_RTCWEN                        (1UL)
#define RTC_ADDR_RTCCON_FIELD_RTCWEN_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCCON_FIELD_RTCWEN)
#define RTC_ADDR_RTCCON_FIELD_STARTB                        (0UL)
#define RTC_ADDR_RTCCON_FIELD_STARTB_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCCON_FIELD_STARTB)

/* FIELD INFO (Register: INTCON) ===========================================*/
#define RTC_ADDR_INTCON_FIELD_PROT                          (15UL)
#define RTC_ADDR_INTCON_FIELD_PROT_MASK                     (RTC_VA_MASK_1 << RTC_ADDR_INTCON_FIELD_PROT)
#define RTC_ADDR_INTCON_FIELD_XDRV                          (12UL)
#define RTC_ADDR_INTCON_FIELD_XDRV_MASK                     (RTC_VA_MASK_2 << RTC_ADDR_INTCON_FIELD_XDRV)
#define RTC_ADDR_INTCON_FIELD_FSEL                          (8UL)
#define RTC_ADDR_INTCON_FIELD_FSEL_MASK                     (RTC_VA_MASK_3 << RTC_ADDR_INTCON_FIELD_FSEL)
#define RTC_ADDR_INTCON_FIELD_STATUS                        (1UL)
#define RTC_ADDR_INTCON_FIELD_STATUS_MASK                   (RTC_VA_MASK_2 << RTC_ADDR_INTCON_FIELD_STATUS)
#define RTC_ADDR_INTCON_FIELD_INTWREN                       (0UL)
#define RTC_ADDR_INTCON_FIELD_INTWREN_MASK                  (RTC_VA_MASK_1 << RTC_ADDR_INTCON_FIELD_INTWREN)

/* FIELD INFO (Register: RTCALM) ===========================================*/
#define RTC_ADDR_RTCALM_FIELD_ALMEN                         (7UL)
#define RTC_ADDR_RTCALM_FIELD_ALMEN_MASK                    (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_ALMEN)
#define RTC_ADDR_RTCALM_FIELD_YEAREN                        (6UL)
#define RTC_ADDR_RTCALM_FIELD_YEAREN_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_YEAREN)
#define RTC_ADDR_RTCALM_FIELD_MONEN                         (5UL)
#define RTC_ADDR_RTCALM_FIELD_MONEN_MASK                    (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_MONEN)
#define RTC_ADDR_RTCALM_FIELD_DAYEN                         (4UL)
#define RTC_ADDR_RTCALM_FIELD_DAYEN_MASK                    (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_DAYEN)
#define RTC_ADDR_RTCALM_FIELD_DATEEN                        (3UL)
#define RTC_ADDR_RTCALM_FIELD_DATEEN_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_DATEEN)
#define RTC_ADDR_RTCALM_FIELD_HOUREN                        (2UL)
#define RTC_ADDR_RTCALM_FIELD_HOUREN_MASK                   (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_HOUREN)
#define RTC_ADDR_RTCALM_FIELD_MINEN                         (1UL)
#define RTC_ADDR_RTCALM_FIELD_MINEN_MASK                    (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_MINEN)
#define RTC_ADDR_RTCALM_FIELD_SECEN                         (0UL)
#define RTC_ADDR_RTCALM_FIELD_SECEN_MASK                    (RTC_VA_MASK_1 << RTC_ADDR_RTCALM_FIELD_SECEN)

/* FIELD INFO (Register: ALMSEC) ===========================================*/
#define RTC_ADDR_ALMSEC_FIELD_SECDATA                       (0UL)
#define RTC_ADDR_ALMSEC_FIELD_SECDATA_MASK                  (RTC_VA_MASK_7 << RTC_ADDR_ALMSEC_FIELD_SECDATA)
#define RTC_ADDR_ALMSEC_FIELD_SECDATA_1                     (0UL)
#define RTC_ADDR_ALMSEC_FIELD_SECDATA_1_MASK                (RTC_VA_MASK_4 << RTC_ADDR_ALMSEC_FIELD_SECDATA_1)
#define RTC_ADDR_ALMSEC_FIELD_SECDATA_2                     (4UL)
#define RTC_ADDR_ALMSEC_FIELD_SECDATA_2_MASK                (RTC_VA_MASK_3 << RTC_ADDR_ALMSEC_FIELD_SECDATA_2)

/* FIELD INFO (Register: ALMMIN) ===========================================*/
#define RTC_ADDR_ALMMIN_FIELD_MINDATA                       (0UL)
#define RTC_ADDR_ALMMIN_FIELD_MINDATA_MASK                  (RTC_VA_MASK_7 << RTC_ADDR_ALMMIN_FIELD_MINDATA)
#define RTC_ADDR_ALMMIN_FIELD_MINDATA_1                     (0UL)
#define RTC_ADDR_ALMMIN_FIELD_MINDATA_1_MASK                (RTC_VA_MASK_4 << RTC_ADDR_ALMMIN_FIELD_MINDATA_1)
#define RTC_ADDR_ALMMIN_FIELD_MINDATA_2                     (4UL)
#define RTC_ADDR_ALMMIN_FIELD_MINDATA_2_MASK                (RTC_VA_MASK_3 << RTC_ADDR_ALMMIN_FIELD_MINDATA_2)

/* FIELD INFO (Register: ALMHOUR) ===========================================*/
#define RTC_ADDR_ALMHOUR_FIELD_HOURDATA                     (0UL)
#define RTC_ADDR_ALMHOUR_FIELD_HOURDATA_MASK                (RTC_VA_MASK_6 << RTC_ADDR_ALMHOUR_FIELD_HOURDATA)
#define RTC_ADDR_ALMHOUR_FIELD_HOURDATA_1                   (0UL)
#define RTC_ADDR_ALMHOUR_FIELD_HOURDATA_1_MASK              (RTC_VA_MASK_4 << RTC_ADDR_ALMHOUR_FIELD_HOURDATA_1)
#define RTC_ADDR_ALMHOUR_FIELD_HOURDATA_2                   (4UL)
#define RTC_ADDR_ALMHOUR_FIELD_HOURDATA_2_MASK              (RTC_VA_MASK_2 << RTC_ADDR_ALMHOUR_FIELD_HOURDATA_2)

/* FIELD INFO (Register: ALMDATE) ===========================================*/
#define RTC_ADDR_ALMDATE_FIELD_DATEDATA                     (0UL)
#define RTC_ADDR_ALMDATE_FIELD_DATEDATA_MASK                (RTC_VA_MASK_6 << RTC_ADDR_ALMDATE_FIELD_DATEDATA)
#define RTC_ADDR_ALMDATE_FIELD_DATEDATA_1                   (0UL)
#define RTC_ADDR_ALMDATE_FIELD_DATEDATA_1_MASK              (RTC_VA_MASK_4 << RTC_ADDR_ALMDATE_FIELD_DATEDATA_1)
#define RTC_ADDR_ALMDATE_FIELD_DATEDATA_2                   (4UL)
#define RTC_ADDR_ALMDATE_FIELD_DATEDATA_2_MASK              (RTC_VA_MASK_2 << RTC_ADDR_ALMDATE_FIELD_DATEDATA_2)

/* FIELD INFO (Register: ALMDAY) ===========================================*/
#define RTC_ADDR_ALMDAY_FIELD_DAYDATA                       (0UL)
#define RTC_ADDR_ALMDAY_FIELD_DAYDATA_MASK                  (RTC_VA_MASK_3 << RTC_ADDR_ALMDAY_FIELD_DAYDATA)

/* FIELD INFO (Register: ALMMON) ===========================================*/
#define RTC_ADDR_ALMMON_FIELD_MONDATA                       (0UL)
#define RTC_ADDR_ALMMON_FIELD_MONDATA_MASK                  (RTC_VA_MASK_5 << RTC_ADDR_ALMMON_FIELD_MONDATA)
#define RTC_ADDR_ALMMON_FIELD_MONDATA_1                     (0UL)
#define RTC_ADDR_ALMMON_FIELD_MONDATA_1_MASK                (RTC_VA_MASK_4 << RTC_ADDR_ALMMON_FIELD_MONDATA_1)
#define RTC_ADDR_ALMMON_FIELD_MONDATA_2                     (4UL)
#define RTC_ADDR_ALMMON_FIELD_MONDATA_2_MASK                (RTC_VA_MASK_1 << RTC_ADDR_ALMMON_FIELD_MONDATA_2)

/* FIELD INFO (Register: ALMYEAR) ===========================================*/
#define RTC_ADDR_ALMYEAR_FIELD_YEARDATA                     (0UL)
#define RTC_ADDR_ALMYEAR_FIELD_YEARDATA_MASK                (RTC_VA_MASK_16 << RTC_ADDR_ALMYEAR_FIELD_YEARDATA)
#define RTC_ADDR_ALMYEAR_FIELD_YEARDATA_1                   (0UL)
#define RTC_ADDR_ALMYEAR_FIELD_YEARDATA_1_MASK              (RTC_VA_MASK_8 << RTC_ADDR_ALMYEAR_FIELD_YEARDATA_1)
#define RTC_ADDR_ALMYEAR_FIELD_YEARDATA_2                   (8UL)
#define RTC_ADDR_ALMYEAR_FIELD_YEARDATA_2_MASK              (RTC_VA_MASK_8 << RTC_ADDR_ALMYEAR_FIELD_YEARDATA_2)

/* FIELD INFO (Register: BCDSEC) ===========================================*/
#define RTC_ADDR_BCDSEC_FIELD_SECDATA                       (0UL)
#define RTC_ADDR_BCDSEC_FIELD_SECDATA_MASK                  (RTC_VA_MASK_7 << RTC_ADDR_BCDSEC_FIELD_SECDATA)
#define RTC_ADDR_BCDSEC_FIELD_SECDATA_1                     (0UL)
#define RTC_ADDR_BCDSEC_FIELD_SECDATA_1_MASK                (RTC_VA_MASK_4 << RTC_ADDR_BCDSEC_FIELD_SECDATA_1)
#define RTC_ADDR_BCDSEC_FIELD_SECDATA_2                     (4UL)
#define RTC_ADDR_BCDSEC_FIELD_SECDATA_2_MASK                (RTC_VA_MASK_3 << RTC_ADDR_BCDSEC_FIELD_SECDATA_2)

/* FIELD INFO (Register: BCDMIN) ===========================================*/
#define RTC_ADDR_BCDMIN_FIELD_MINDATA                       (0UL)
#define RTC_ADDR_BCDMIN_FIELD_MINDATA_MASK                  (RTC_VA_MASK_7 << RTC_ADDR_BCDMIN_FIELD_MINDATA)
#define RTC_ADDR_BCDMIN_FIELD_MINDATA_1                     (0UL)
#define RTC_ADDR_BCDMIN_FIELD_MINDATA_1_MASK                (RTC_VA_MASK_4 << RTC_ADDR_BCDMIN_FIELD_MINDATA_1)
#define RTC_ADDR_BCDMIN_FIELD_MINDATA_2                     (4UL)
#define RTC_ADDR_BCDMIN_FIELD_MINDATA_2_MASK                (RTC_VA_MASK_3 << RTC_ADDR_BCDMIN_FIELD_MINDATA_2)

/* FIELD INFO (Register: BCDHOUR) ===========================================*/
#define RTC_ADDR_BCDHOUR_FIELD_HOURDATA                     (0UL)
#define RTC_ADDR_BCDHOUR_FIELD_HOURDATA_MASK                (RTC_VA_MASK_6 << RTC_ADDR_BCDHOUR_FIELD_HOURDATA)
#define RTC_ADDR_BCDHOUR_FIELD_HOURDATA_1                   (0UL)
#define RTC_ADDR_BCDHOUR_FIELD_HOURDATA_1_MASK              (RTC_VA_MASK_4 << RTC_ADDR_BCDHOUR_FIELD_HOURDATA_1)
#define RTC_ADDR_BCDHOUR_FIELD_HOURDATA_2                   (4UL)
#define RTC_ADDR_BCDHOUR_FIELD_HOURDATA_2_MASK              (RTC_VA_MASK_2 << RTC_ADDR_BCDHOUR_FIELD_HOURDATA_2)

/* FIELD INFO (Register: BCDDATE) ===========================================*/
#define RTC_ADDR_BCDDATE_FIELD_DATEDATA                     (0UL)
#define RTC_ADDR_BCDDATE_FIELD_DATEDATA_MASK                (RTC_VA_MASK_6 << RTC_ADDR_BCDDATE_FIELD_DATEDATA)
#define RTC_ADDR_BCDDATE_FIELD_DATEDATA_1                   (0UL)
#define RTC_ADDR_BCDDATE_FIELD_DATEDATA_1_MASK              (RTC_VA_MASK_4 << RTC_ADDR_BCDDATE_FIELD_DATEDATA_1)
#define RTC_ADDR_BCDDATE_FIELD_DATEDATA_2                   (4UL)
#define RTC_ADDR_BCDDATE_FIELD_DATEDATA_2_MASK              (RTC_VA_MASK_2 << RTC_ADDR_BCDDATE_FIELD_DATEDATA_2)

/* FIELD INFO (Register: BCDDAY) ===========================================*/
#define RTC_ADDR_BCDDAY_FIELD_DAYDATA                       (0UL)
#define RTC_ADDR_BCDDAY_FIELD_DAYDATA_MASK                  (RTC_VA_MASK_3 << RTC_ADDR_BCDDAY_FIELD_DAYDATA)

/* FIELD INFO (Register: BCDMON) ===========================================*/
#define RTC_ADDR_BCDMON_FIELD_MONDATA                       (0UL)
#define RTC_ADDR_BCDMON_FIELD_MONDATA_MASK                  (RTC_VA_MASK_5 << RTC_ADDR_BCDMON_FIELD_MONDATA)
#define RTC_ADDR_BCDMON_FIELD_MONDATA_1                     (0UL)
#define RTC_ADDR_BCDMON_FIELD_MONDATA_1_MASK                (RTC_VA_MASK_4 << RTC_ADDR_BCDMON_FIELD_MONDATA_1)
#define RTC_ADDR_BCDMON_FIELD_MONDATA_2                     (4UL)
#define RTC_ADDR_BCDMON_FIELD_MONDATA_2_MASK                (RTC_VA_MASK_1 << RTC_ADDR_BCDMON_FIELD_MONDATA_2)

/* FIELD INFO (Register: BCDYEAR) ===========================================*/
#define RTC_ADDR_BCDYEAR_FIELD_YEARDATA                     (0UL)
#define RTC_ADDR_BCDYEAR_FIELD_YEARDATA_MASK                (RTC_VA_MASK_16 << RTC_ADDR_BCDYEAR_FIELD_YEARDATA)
#define RTC_ADDR_BCDYEAR_FIELD_YEARDATA_1                   (0UL)
#define RTC_ADDR_BCDYEAR_FIELD_YEARDATA_1_MASK              (RTC_VA_MASK_8 << RTC_ADDR_BCDYEAR_FIELD_YEARDATA_1)
#define RTC_ADDR_BCDYEAR_FIELD_YEARDATA_2                   (8UL)
#define RTC_ADDR_BCDYEAR_FIELD_YEARDATA_2_MASK              (RTC_VA_MASK_8 << RTC_ADDR_BCDYEAR_FIELD_YEARDATA_2)

/* FIELD INFO (Register: RTCIM) ===========================================*/
#define RTC_ADDR_RTCIM_FIELD_PWDN                           (3UL)
#define RTC_ADDR_RTCIM_FIELD_PWDN_MASK                      (RTC_VA_MASK_1 << RTC_ADDR_RTCIM_FIELD_PWDN)
#define RTC_ADDR_RTCIM_FIELD_WKUPMODE                       (2UL)
#define RTC_ADDR_RTCIM_FIELD_WKUPMODE_MASK                  (RTC_VA_MASK_1 << RTC_ADDR_RTCIM_FIELD_WKUPMODE)
#define RTC_ADDR_RTCIM_FIELD_INTMODE                        (0UL)
#define RTC_ADDR_RTCIM_FIELD_INTMODE_MASK                   (RTC_VA_MASK_2 << RTC_ADDR_RTCIM_FIELD_INTMODE)

/* FIELD INFO (Register: RTCPEND) ===========================================*/
#define RTC_ADDR_RTCPEND_FIELD_PEND                         (0UL)
#define RTC_ADDR_RTCPEND_FIELD_PEND_MASK                    (RTC_VA_MASK_1 << RTC_ADDR_RTCPEND_FIELD_PEND)


/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

#endif  // ( MCU_BSP_SUPPORT_DRIVER_RTC == 1 )

#endif  // MCU_BSP_RTC_REG_HEADER


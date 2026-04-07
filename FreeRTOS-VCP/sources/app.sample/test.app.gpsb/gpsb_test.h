// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gpsb_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_GPSB_TEST_HEADER
#define MCU_BSP_GPSB_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_FMU value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU != 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_GPSB != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_GPSB value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_GPSB != 1 )

//#define PORT_TEST
/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <gpsb_reg.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define GPSB_TEST_NUM                   (8UL)
#define DMA_BUF_SIZE                    (512UL)
#define GPSB_TEST_BUF_SIZE              (8UL)
#define SDM_GPSB_MAX_WRITE_SIZE         (128UL)


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static SALRetCode_t GPSB_SafetyTestProtocolMonitor
(
    uint8                               ucCh,
    uint8                               ucBpw
);

static SALRetCode_t GPSB_SafetyTestStuckMonitor
(
    uint8                               ucMstCh
);

static SALRetCode_t GPSB_SafetyTestLoopback
(
    uint8                               ucMstCh,
    uint8                               ucSlvCh
);

static SALRetCode_t GPSB_SafetyTestWriteLock
(
    void
);

static SALRetCode_t GPSB_TestIpVerification
(
    void
);

static SALRetCode_t GPSB_TestSdm
(
    void
);

static SALRetCode_t GPSB_SdmExtTest
(
    void
);

static SALRetCode_t GPSB_SdmIntTest
(
    uint32                              uiSclk
);

static SALRetCode_t GPSB_InternalLoopbackTest
(
    uint8                               ucCh,
    uint8                               ucBpw,
    uint8                               ucXferMode,
    uint32                              uiSpeed
);

#ifdef PORT_TEST
SALRetCode_t GPSB_ExternalLoopbackTest
(
    uint8                               ucMstCh,
    uint8                               ucSlvCh,
    uint32                              uiMode,
    uint8                               ucMPortIndex,
    uint8                               ucSPortIndex
);

#else
static SALRetCode_t GPSB_ExternalLoopbackTest
(
    uint8                               ucMstCh,
    uint8                               ucSlvCh,
    uint32                              uiMode
);
#endif
static SALRetCode_t GPSB_ExtDevTest
(
    void
);

void GPSB_CreateSlaveTestTask
(
    void
);

void GPSB_Test
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )

#endif  // MCU_BSP_GPSB_TEST_HEADER


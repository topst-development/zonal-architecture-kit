// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eth_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_ETH != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_ETH value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_ETH != 1 )

#if ( MCU_BSP_SUPPORT_APP_IPERF != 1 )
    #error MCU_BSP_SUPPORT_APP_IPERF value must be 1.
#endif  // ( MCU_BSP_SUPPORT_APP_IPERF != 1 )

#include <eth.h>
#include <iperf_task.h>

void ETH_TestLoopback
(
    uint32                              uiMode
);

void ETH_TestInit
(
    uint32                              uiMode
);

void ETH_TestSend
(
    uint32                              uiIdx
);

void ETH_TestIPInit
(
    void
);

void ETH_TestIperf
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )


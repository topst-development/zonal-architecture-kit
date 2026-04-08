// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : cpu_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_CPU == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_WATCHDOG value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG != 1 )

extern void Disble_Cache
(
    void
);

extern void Enable_Cache
(
    void
);

extern void Calculate_CacheTime
(
    void
);

extern void Calculate_FPU
(
    void
);

extern void RemapMicomTest
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_CPU == 1 )


// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : echo_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_ECHO_TEST_HEADER
#define MCU_BSP_ECHO_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_SOCKET == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_ETH != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_ETH value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_ETH != 1 )

#define SIZEOFMACADDR                   (6)
#define SIZEOFIPV4ADDR                  (4)

#define ALLUSAGE                        (0x1F)
#define MACUSAGE                        (0x01)
#define IPV4USAGE                       (0x02)
#define NETMASKUSAGE                    (0x04)
#define GWUSAGE                         (0x08)
#define DNSUSAGE                        (0x10)
#define MSGUSAGE                        (0x20)

#define USAGE                                                              \
{                                                                          \
    mcu_printf("USGAE: socket ipup [mac] [ipv4] [netmask] [gw] [dns]\n");  \
    mcu_printf("USGAE: socket svr\n");  \
    mcu_printf("USGAE: socket cli [ipv4] [msg]\n");  \
}                                                                          \

#define USAGEIPUP(LV)                                                      \
{                                                                          \
    mcu_printf("USGAE: socket ipup [mac] [ipv4] [netmask] [gw] [dns]\n");  \
    if (MACUSAGE == (LV & MACUSAGE))                                       \
    {                                                                      \
        mcu_printf("[string:mac] AA:BB:CC:DD:EE:FF\n");                    \
    }                                                                      \
    if (IPV4USAGE == (LV & IPV4USAGE))                                     \
    {                                                                      \
        mcu_printf("[string:ipv4] XX.XX.XX.XX\n");                         \
    }                                                                      \
    if (NETMASKUSAGE == (LV & NETMASKUSAGE))                               \
    {                                                                      \
        mcu_printf("[string:netmask] XX.XX.XX.XX\n");                      \
    }                                                                      \
    if (GWUSAGE == (LV & GWUSAGE))                                         \
    {                                                                      \
        mcu_printf("[string:gw] XX.XX.XX.XX\n");                           \
    }                                                                      \
    if (DNSUSAGE == (LV & DNSUSAGE))                                       \
    {                                                                      \
        mcu_printf("[string:dns] XX.XX.XX.XX\n");                          \
    }                                                                      \
    goto err;                                                              \
}

#define USAGECLIENT(LV)                                                    \
{                                                                          \
    mcu_printf("USGAE: socket cli [ipv4] [msg]\n");                        \
    if (IPV4USAGE == (LV & IPV4USAGE))                                     \
    {                                                                      \
        mcu_printf("[string:ipv4] XX.XX.XX.XX\n");                         \
    }                                                                      \
    if (NETMASKUSAGE == (LV & MSGUSAGE))                                   \
    {                                                                      \
        mcu_printf("[string:msg] ABCDEF\n");                               \
    }                                                                      \
    goto err;                                                              \
}


void SOCKTEST_IPInit
(
    const uint8*                        pucMacAddr,
    const uint8*                        pucIpAddr
);

void SOCKTEST_EchoSend
(
    const uint8*                        pucIpAddr,
    const uint8*                        pucMsg
);

void SOCKTEST_EchoRecv
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SOCKET == 1 )

#endif  // MCU_BSP_ECHO_TEST_HEADER


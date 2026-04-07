// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : console.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )

#include <stdlib.h>
#include <string.h>

#include <uart.h>

#if ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )
    #include "wdt.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_WATCHDOG == 1 )

#include <console.h>
#include "debug.h"

#include <main.h>
#include <app_cfg.h>

#if ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )
    #include "pdm_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )
    #include "ictc_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )
    #include "lin_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )
    #include "i2c_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
    #include "can_demo.h"
#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )
    #include "audio_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )
    #include "fmu_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )

/* For Test Code(Verification Code) */
#if ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )
    #include <wdt_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )
    #include <timer_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )
    #include "gdma_test.h"
    #include "gdma_ip_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )
    #include "dse_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )
    #include "gic_test.h"
#endif  // M( CU_BSP_SUPPORT_TEST_APP_GIC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )
    #include <pmu_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )
    #include <gpio_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )
    #include "adc_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )
    #include "gpsb_test.h"
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )
    #include <hsm_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )

#if( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )
    #include <hsm_manager.h>
#endif  // ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )
    #include <rtc_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )
    #include <eflash_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )
    #include <uart_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
    #include "fmu.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )
    #include "ssm.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_PMIO == 1 )
    #include "pmio.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_PMIO == 1 )

#if ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )
    #include <spi_led.h>
#endif  // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

#if ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )
    #include "eth_test.h"
    #if ( RTL_8211E == 1 )
        #include "eth_mii_rtl8211e.h"
    #else
        #include "eth_mii.h"
    #endif
#endif  // ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )
    #include <spu_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )

//#define MIDF_SUPPORT //TBD
#ifdef MIDF_SUPPORT
#include <midf.h>
#endif

#ifdef IP_VERIFICATION_DEFINED_GDMA_MIDF_TOP_CRD
#include <gdma_ip_test.h>
#endif

#if ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )
    #include <sfmc_test.h>
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
    #include "fwupdate.h"
#elif ( MCU_BSP_SUPPORT_APP_FW_UPDATE_ECCP == 1 )
    #include "fwupdate.h"
#endif

#include <stdio.h>

#ifdef OS_FREERTOS
#ifdef ECHO_TEMP
#include "echo_test.h"
#endif
#endif

int32                                   siADCDebug = -1;
int32                                   siADCModule = 0;
/****************************************************************************************************/
/*                                       STATIC FUNCTIONS                                           */
/****************************************************************************************************/

static ConsoleCmdList_t *pGetConsoleCmdList
(
    uint32                              uiIndex
);

static void CSL_HelpList
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static uint32 CSL_StringToHex
(
    const int8                          *param
);

static void CSL_ReadMemory
(
    uint8                               ucArgc,
    void                                *pArgv[]
);

static void CSL_WriteMemory
(
    uint8                               ucArgc,
    void                                *pArgv[]
);

static void CSL_SetAliveMessage
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

#if ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )
static void CSL_DeviceI2c
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )
static void CSL_DeviceAudio
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )
static void CSL_DeviceSpu
(
    uint8                               ucArgc,
    void                                *pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )
static void CSL_DeviceUart
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
static void CSL_DeviceCan
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )
static void CSL_DeviceLin
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )

static void CSL_DeviceMbox
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

#if ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )
static void CSL_DevicePdm
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif

#if ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )
static void CSL_DeviceIctc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )
static void CSL_DeviceAdc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )
static void CSL_DeviceGpio
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )
static void CSL_DeviceTmr
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )
static void CSL_DeviceWdt
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )

static void CSL_DeviceMpu
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceFmu
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

#if ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )
static void CSL_DeviceGpsb
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )
static void CSL_DeviceGic
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PMIO == 1 )
static void CSL_DevicePmio
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )
static void CSL_DevicePmu
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )

static void CSL_DeviceCkc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )
static void CSL_DeviceGdma
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )
static void CSL_DeviceDse
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )

#if ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )
static void CSL_DeviceGmac
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )

#if MIDF_SUPPORT
static void CSL_DeviceMidf
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif

#if ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )
static void CSL_DeviceSsm
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )
static void CSL_DeviceHsm
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )
static void CSL_DeviceRtc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )

static void CSL_EnableLog
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceEFLASH
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

#if ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )
static void CSL_DeviceSFMC
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )

static void CSL_DeviceFWUD
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

#if ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )
static void CSL_DeviceOTP
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
static void CSL_SecureBootEnable
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif  // ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )

#ifdef ECHO_TEMP
static void CSL_DeviceSocket
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif

static sint32 CSL_EmptyHistoryList
(
    void
);

static uint32 CSL_PullHistoryList
(
    uint8 *                             pucCmdBuf,
    uint32                              uiOldCmdSize
);

static void CSL_PushHistoryList
(
    uint8 *                             pucBuf,
    uint32                              uiCmdSize
);

static void ConsoleTask
(
    void *                              pArg
);

static uint32 CSL_GetOneWordLength
(
    uint8 *                             puiCmdBuf,
    uint32                              uiCmdLength
);

static uint32 CSL_GetCommandLength
(
    const uint8 *                       puiCmdBuf
);

static void CSL_ExecuteCommand
(
    uint8 *                             puiCmdBuf,
    uint32                              uiCmdLength
);

static void CSL_LogCmdType01
(
    void *                              pArgv[]
);

static void CSL_LogCmdType02
(
    void *                              pArgv[]
);

static void CSL_LogCmdType03
(
    void *                              pArgv[]
);

static void CSL_UsageLogCmd
(
    uint8 ucInfo,
    uint8 ucHelp
);

static void CSL_ShowLogInfo
(
    void
);

/*
***************************************************************************************************
*                                        DEFINITIONS
***************************************************************************************************
*/
//{ CMD_ENABLE, (uint8 *)string you want,  will be executed function}

static ConsoleCmdList_t ConsoleCmdList[CSL_CMD_NUM_MAX] =
{
    { CMD_ENABLE,   (const uint8 *)"help",     CSL_HelpList},
    { CMD_ENABLE,   (const uint8 *)"md",       CSL_ReadMemory},
    { CMD_ENABLE,   (const uint8 *)"mm",       CSL_WriteMemory},
    { CMD_ENABLE,   (const uint8 *)"alive",    CSL_SetAliveMessage},

#if ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )
    { CMD_ENABLE,   (const uint8 *)"pdm",      CSL_DevicePdm},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )
    { CMD_ENABLE,   (const uint8 *)"ictc",     CSL_DeviceIctc},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )
    { CMD_ENABLE,   (const uint8 *)"audio",    CSL_DeviceAudio},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )
    { CMD_ENABLE,   (const uint8 *)"i2c",      CSL_DeviceI2c},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
    { CMD_ENABLE,   (const uint8 *)"can",      CSL_DeviceCan},  //10
#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )
    { CMD_ENABLE,   (const uint8 *)"fmu",      CSL_DeviceFmu},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )

    { CMD_ENABLE,   (const uint8 *)"uart",     CSL_DeviceUart},

#if ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )
    { CMD_ENABLE,   (const uint8 *)"lin",      CSL_DeviceLin},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )

    { CMD_ENABLE,   (const uint8 *)"mbox",     CSL_DeviceMbox},

#if ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )
    { CMD_ENABLE,   (const uint8 *)"adc",      CSL_DeviceAdc},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )
    { CMD_ENABLE,   (const uint8 *)"gpio",     CSL_DeviceGpio},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )
    { CMD_ENABLE,   (const uint8 *)"tmr",      CSL_DeviceTmr},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )
    { CMD_ENABLE,   (const uint8 *)"wdt",      CSL_DeviceWdt},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )

    { CMD_ENABLE,   (const uint8 *)"mpc",      CSL_DeviceMpu},

#if ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )
    { CMD_ENABLE,   (const uint8 *)"gpsb",     CSL_DeviceGpsb}, //20
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )
    { CMD_ENABLE,   (const uint8 *)"gic",      CSL_DeviceGic},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PMIO == 1 )
    { CMD_ENABLE,   (const uint8 *)"pmio",     CSL_DevicePmio},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )
    { CMD_ENABLE,   (const uint8 *)"pmu",      CSL_DevicePmu},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )

    { CMD_ENABLE,   (const uint8 *)"ckc",      CSL_DeviceCkc},

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )
    { CMD_ENABLE,   (const uint8 *)"gdma",     CSL_DeviceGdma},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )
    { CMD_ENABLE,   (const uint8 *)"dse",      CSL_DeviceDse},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )

#if ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )
    { CMD_ENABLE,   (const uint8 *)"gmac",     CSL_DeviceGmac},
#endif  // ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )
    { CMD_ENABLE,   (const uint8 *)"ssm",      CSL_DeviceSsm},
#endif  // ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )

#ifdef MIDF_SUPPORT
    { CMD_ENABLE,   (const uint8 *)"midf",     CSL_DeviceMidf},
#endif

#if ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )
    { CMD_ENABLE,   (const uint8 *)"hsm",      CSL_DeviceHsm},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )
    { CMD_ENABLE,   (const uint8 *)"rtc",      CSL_DeviceRtc},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )

    { CMD_ENABLE,   (const uint8 *)"log",      CSL_EnableLog}, //30

#if ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )
    { CMD_ENABLE,   (const uint8 *)"eflash",   CSL_DeviceEFLASH},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )
    { CMD_ENABLE,   (const uint8 *)"sfmc",     CSL_DeviceSFMC},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )

    { CMD_ENABLE,   (const uint8 *)"fwud",     CSL_DeviceFWUD},

#if ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )
    { CMD_ENABLE,   (const uint8 *)"spu",      CSL_DeviceSpu},
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )
    { CMD_ENABLE,   (const uint8 *)"otp",      CSL_DeviceOTP},
#endif  // ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
    { CMD_ENABLE,   (const uint8 *)"sb_en",    CSL_SecureBootEnable},
#endif  // ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )

#ifdef ECHO_TEMP
    { CMD_ENABLE,   (const uint8 *)"socket",   CSL_DeviceSocket},
#endif

    { CMD_DISABLE,  (const uint8 *)"",         NULL}
};

static ConsoleLogFunc_t ConsoleLogFunc[CSL_LOG_NUM_MAX] =
{
    {CSL_LogCmdType01},
    {CSL_LogCmdType02},
    {CSL_LogCmdType03}
};

#ifdef CONSOLE_HISTORY_ENABLE
    static CSLHistoryManager_t CSHistoryManager;
#endif

/*
***************************************************************************************************
*                                    FUNCTION PROTOTYPES
***************************************************************************************************
*/

static ConsoleCmdList_t *pGetConsoleCmdList(uint32 uiIndex)
{
    return &ConsoleCmdList[uiIndex];
}

static void CSL_HelpList(uint8 ucArgc, void *pArgv[])
{
    (void)  ucArgc;
    (void)  pArgv;

    UART_Write(UART_DEBUG_CH, (const uint8 *)"\r\n================================================", 50);
    UART_Write(UART_DEBUG_CH, (const uint8 *)"\r\n============== How to use command ==============", 50);
    UART_Write(UART_DEBUG_CH, (const uint8 *)"\r\n================================================", 50);

    UART_Write(UART_DEBUG_CH, (const uint8 *)"\r\n [command_string] [arg1] ... [arg10]", 38);
    UART_Write(UART_DEBUG_CH, (const uint8 *)"\n", 1);
    UART_Write(UART_DEBUG_CH, (const uint8 *)"\r\n 1. display alive message : [alive] [on/off]", 46);

    UART_Write(UART_DEBUG_CH, (const uint8 *)"\n", 1);
}

static void CSL_ReadMemory(uint8 ucArgc, void *pArgv[])
{
    uint32  addr;
    uint32  reg;

    if (pArgv != NULL_PTR)
    {
        if (pArgv[0] != NULL)
        {
            addr = CSL_StringToHex(pArgv[0]);
            if(addr % 4 == 0)
            {
                reg = SAL_ReadReg(addr);
                mcu_printf("\n          0x%x    ==>    0x%x\n", addr, reg);
            }
            else
            {
                mcu_printf("\n [md] [hex address is not 4 byte align] \n");
            }
        }
        else
        {
            mcu_printf("\n Wrong argument, argv(%s)\n", pArgv[0]);
            mcu_printf("\n [md] [hex address] \n");
        }
    }
}

static void CSL_WriteMemory(uint8 ucArgc, void *pArgv[])
{
    uint32  addr;
    uint32  val;
    uint32  reg;

    if (pArgv != NULL_PTR)
    {
        if (pArgv[0] != NULL)
        {
            addr = CSL_StringToHex(pArgv[0]);
            val = CSL_StringToHex(pArgv[1]);
            if(addr % 4 == 0)
            {
                SAL_WriteReg(val, addr);
                reg = SAL_ReadReg(addr);
                mcu_printf("\n          0x%x    ==>    0x%x\n", addr, reg);

            }
            else
            {
                mcu_printf("\n [md] [hex address is not 4 byte align] \n");
            }
        }
        else
        {
            mcu_printf("\n Wrong argument, argv(%s)\n", pArgv[1]);
            mcu_printf("\n [mm] [hex address] [value] \n");
        }
    }
}


static void CSL_SetAliveMessage(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 onbuf[2]    = "on";
    const uint8 offbuf[3]   = "off";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, onbuf, 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            gALiveMsgOnOff  = ON;
        }
        else if((SAL_StrNCmp(str, offbuf, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            gALiveMsgOnOff  = OFF;
        }
        else
        {
            mcu_printf("Wrong argument : on or off %s\n", pArgv[0]);
        }
    }
}

#if ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )
static void CSL_DevicePdm(uint8 ucArgc, void *pArgv[])
{
    uint32   ucMode;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        ucMode  = strtoul(pArgv[0], NULL_PTR, 16UL);
        PDM_SelectTestCase(ucMode);
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PDM == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )
static void CSL_DeviceIctc(uint8 ucArgc, void *pArgv[])
{
    uint32   ucMode;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        ucMode  = strtoul(pArgv[0], NULL_PTR, 10UL);
        ICTC_SelectTestCase(ucMode);
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ICTC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )
static void CSL_DeviceI2c(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            I2C_TestMain();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_I2C == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )
static void CSL_DeviceFmu(uint8 ucArgc, void *pArgv[])
{
    int32 ucMode;
    if (pArgv != NULL_PTR)
    {
        ucMode  = strtol(pArgv[0], NULL_PTR, 10UL);
        FMU_StartFmuTest(ucMode);
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_FMU == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )
static void CSL_DeviceAudio(uint8 ucArgc, void *pArgv[])
{
    AUDIO_SampleTest(ucArgc, pArgv);
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_AUDIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )
static void CSL_DeviceSpu(uint8 ucArgc, void *pArgv[])
{
    SPU_SelectTestCase(ucArgc, pArgv);
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SPU == 1 )

static uint32 CSL_StringToHex(const int8 *pucData)
{
    uint32 hex = 0;
    uint32 count = 0UL;
    uint32 i;

    if (pucData != NULL_PTR)
    {
        (void)SAL_StrLength(pucData, (SALSize *)&count);

        for (i = 0UL ; i < count; i++)
        {
            if((pucData[i] >= '0') && (pucData[i] <= '9'))
            {
                hex = (((hex * 16u) + pucData[i]) - '0');
            }
            else if ((pucData[i] >= 'A') && (pucData[i] <= 'F'))
            {
                hex = ((((hex * 16u) + pucData[i]) - 'A') + 10u);
            }
            else if ((pucData[i] >= 'a') && (pucData[i] <= 'f'))
            {
                hex = ((((hex * 16u) + pucData[i]) - 'a') + 10u);
            }
            else
            {
                ;
            }
        }
    }

    return hex;
}

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )
static void CSL_DeviceGdma(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4]    = "test";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if ((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GDMA_SampleForM2M();
        }
     else if ((SAL_StrNCmp(str, (const uint8 *)"verf", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            #ifdef GDMA_IP_TEST_CODE
            uint32 test_id;
            sint32 intRet;

            test_id = (uint32)strtoul(pArgv[1], NULL, 10);
            intRet = GDMA_IPTest(test_id);
            if ( intRet == 0)
            {
                mcu_printf("PASS \n");
            }
            else
            {
                mcu_printf("FAIL \n");
                }
            #else
            mcu_printf("Need to check GDMA_IP_TEST_CODE feature  \n");
            #endif
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )
static void CSL_DeviceUart(uint8 ucArgc, void *pArgv[])
{
    uint8 ucTxCh;
    uint8 uiPortCfg;
    const uint8 * pucBuf;
    uint32 len;

    if (pArgv != NULL_PTR)
    {
        if(ucArgc == 3)
        {
            ucTxCh = strtol(pArgv[0], NULL_PTR, 10UL);
            uiPortCfg = strtol(pArgv[1], NULL_PTR, 10UL);
            pucBuf = (const uint8 *)pArgv[2];
            (void)SAL_StrLength(pArgv[2], (SALSize *)&len);

            (void)UART_ConsoleTxTest(ucTxCh, uiPortCfg, pucBuf, len);
        }
        else
        {
            mcu_printf("\n USAGE: uart [ch] [portCfg] [data]\n");
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )

#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )
static void CSL_DeviceCan(uint8 ucArgc, void *pArgv[])
{
    CAN_DemoTest(ucArgc, pArgv);
}
#endif  // ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )
static void CSL_DeviceLin(uint8 ucArgc, void *pArgv[])
{
    LIN_SampleTest(ucArgc, pArgv);
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_LIN == 1 )

static void CSL_DeviceMbox(uint8 ucArgc, void *pArgv[])
{

}

#if ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )
static void CSL_DeviceAdc(uint8 ucArgc, void *pArgv[])
{
#if (ACFG_APP_KEY_EN == 0)
    int32 ucMode;
    if (pArgv != NULL_PTR)
    {
        ucMode  = strtol(pArgv[0], NULL_PTR, 10UL);
        ADC_StartAdcTest(ucMode);
    }
#else
    int32 ucMode;
    sint32 ret;
    if ((SAL_StrNCmp((const uint8 *) pArgv[0], (const uint8 *) "ch", 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
    {
        ucMode  = strtol(pArgv[1], NULL_PTR, 10UL);
        siADCDebug= ucMode;
    }
    else if ((SAL_StrNCmp((const uint8 *) pArgv[0], (const uint8 *) "mod", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
    {
        ucMode  = strtol(pArgv[1], NULL_PTR, 10UL);
        if ((ucMode == 0) || (ucMode == 1))
            siADCModule = ucMode;
    }
    else
    {
        mcu_printf("Please set ACFG_APP_KEY_EN = 0! or USAGE: adc [ch|mod] [num]\n");
    }
#endif
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )
static void CSL_DeviceGpio(uint8 ucArgc, void *pArgv[])
{
    int32   ucMode;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        ucMode  = strtol(pArgv[0], NULL_PTR, 16UL);
        GPIO_StartGpioTest(ucMode);
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )
static void CSL_DeviceTmr(uint8 ucArgc, void *pArgv[])
{
    int32       iMode;
    uint32      uiTime;
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    sint32      ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];
        iMode   = strtol(pArgv[1], NULL_PTR, 10UL);
        uiTime  = strtol(pArgv[2], NULL_PTR, 10UL);

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            TIMER_StartTimerTest(iMode, uiTime);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_TIMER == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )
static void CSL_DeviceWdt(uint8 ucArgc, void *pArgv[])
{
    int32       iMode;
    uint32      uiTime;
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    sint32      ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];
        iMode   = strtol(pArgv[1], NULL_PTR, 10UL);
        uiTime  = strtol(pArgv[2], NULL_PTR, 10UL);

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            WDT_StartTest(iMode, uiTime);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_WDT == 1 )

static void CSL_DeviceMpu(uint8 ucArgc, void *pArgv[])
{

}

#if ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )
static void CSL_DeviceGpsb(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    const uint8 ucSlaveTest[5] = "slave";

#if ( MCU_BSP_SUPPORT_APP_SPI_LED == 1)
    const uint8 ucLedTest[3] = "led";
    const uint8 ucLedOn[2] = "on";
    const uint8 ucLedOff[3] = "off";
#endif  // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GPSB_Test();
        }
        else if((SAL_StrNCmp(str, ucSlaveTest, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GPSB_CreateSlaveTestTask();
        }
#if ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )
        else if((SAL_StrNCmp(str, ucLedTest, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            str     = (const uint8 *)pArgv[1];
            if((SAL_StrNCmp(str, ucLedOn, 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                SPILED_StatusOn();
         }
            else if((SAL_StrNCmp(str, ucLedOff, 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                SPILED_StatusOff();
         }
         else
         {
                mcu_printf("\nWrong argument : %s\n", pArgv[0]);
         }
        }
#endif  // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GPSB == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )
static void CSL_DeviceGic(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4]    = "test";
    const uint8 ucBufTest1[7]   = "sgitest";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GIC_Test_SmInt();
        }
        else if((SAL_StrNCmp(str, ucBufTest1, 7, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GIC_Test_Sgi();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }

    return;
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GIC == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PMIO == 1 )
static void CSL_DevicePmio(uint8 ucArgc, void *pArgv[])
{
    const uint32 uiTestNum = 1UL;
    uint8 ucTest[1][50]    =
    {
        {"info   (boot info)"},
    };
    uint32 uiTestIdx = 0UL;
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        for(uiTestIdx = 0UL ; uiTestIdx < uiTestNum ; uiTestIdx++)
        {
            if(ucArgc == 0UL || ((ucArgc == 1UL) && (pArgv[0]==NULL)))
            {
                mcu_printf("pmio %s\n", ucTest[uiTestIdx]);
            }
            else
            {
                if ((SAL_StrNCmp(pArgv[0], ucTest[uiTestIdx], 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                {
                    switch(uiTestIdx)
                    {
                        case 0:
                            {
                                PMIO_DebugPrintPowerStatus();
                                break;
                            }

                        default:
                            mcu_printf("PMIO Unknown test num %d\n", uiTestIdx);
                            break;
                    }
                    break;
                }
            }
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMIO == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )
static void CSL_DevicePmu(uint8 ucArgc, void *pArgv[])
{
    const uint32 uiTestNum = 4UL;
    uint8 ucTest[4][8]    =
    {
        {"fmu"},
        {"isrst"},
        {"rst"},
        {"swrst"},
    };
    uint32 uiTestIdx = 0UL;
    uint32 uiTestVa = 0UL;
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        for(uiTestIdx = 0UL ; uiTestIdx < uiTestNum ; uiTestIdx++)
        {
            if(ucArgc == 0UL)
            {
                mcu_printf("PMU Cmd1: %s\n", ucTest[uiTestIdx]);
            }
            else
            {
                if ((SAL_StrNCmp(pArgv[0], ucTest[uiTestIdx], 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                {
                    switch(uiTestIdx)
                    {
                        case 0:
                            {
                                if(ucArgc == 1U)
                                {
                                    PMU_TEST_FmuMonitoringTest(0UL);
                                }
                                else
                                {
                                    uiTestVa = (uint32)strtoul(pArgv[1], NULL, 10UL);
                                    PMU_TEST_FmuMonitoringTest(uiTestVa);
                                }
                                break;
                            }
                        case 1:
                            {
                                PMU_TEST_GetRstStatus();
                                break;
                            }
                        case 2:
                            {
                                if(ucArgc == 1U)
                                {
                                    PMU_TEST_RstMonitoringTest(0UL);
                                }
                                else
                                {
                                    uiTestVa = (uint32)strtoul(pArgv[1], NULL, 10UL);
                                    PMU_TEST_RstMonitoringTest(uiTestVa);
                                }
                                break;
                            }
                        case 3:
                            {
                                if(ucArgc == 1U)
                                {
                                    PMU_TEST_RstReqTest(0UL);
                                }
                                else
                                {
                                    uiTestVa = (uint32)strtoul(pArgv[1], NULL, 10UL);
                                    PMU_TEST_RstReqTest(uiTestVa);

                                }
                                break;
                            }
                        default:
                            mcu_printf("PMU Unknown test num %d\n", uiTestIdx);
                            break;
                    }
                    break;
                }
            }
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_PMU == 1 )

static void CSL_DeviceCkc(uint8 ucArgc, void *pArgv[])
{

}

#if ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )
static void CSL_DeviceDse(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4]    = "test";
    const uint8 ucBufTest1[7]   = "dup";
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            DSE_Test_R5AccessFault();
        }
        else if((SAL_StrNCmp(str, ucBufTest1, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            DSE_Test_SoftFaultCheck();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }

    return;
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_DSE == 1 )

#if ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )
static void CSL_DeviceGmac(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    sint32 ret;
    const uint8 ucMacTest[5] = "maclb";
    const uint8 ucPhyTest[5] = "phylb";
    const uint8 ucInit[4] = "init";
    const uint8 ucSend[4] = "send";
    const uint8 ucIperf[5] = "iperf";
    const uint8 ucIPInit[6] = "ipinit";
    const uint8 ucCornertest[5] = "ctest";
#if ( RTL_8211E == 1 )
    const uint8 ucPread[5] = "pread";
    const uint8 ucPwrite[6] = "pwrite";
#endif

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucMacTest, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ETH_TestLoopback(ETH_MODE_MAC_LOOPBACK);
        }
        else if((SAL_StrNCmp(str, ucPhyTest, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ETH_TestLoopback(ETH_MODE_PHY_LOOPBACK);
        }
        else if((SAL_StrNCmp(str, ucInit, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ETH_TestInit(ETH_MODE_NORMAL);
        }
        else if((SAL_StrNCmp(str, ucSend, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ETH_TestSend(strtoul(pArgv[1], NULL, 10));
        }
        else if((SAL_StrNCmp(str, ucIperf, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ETH_TestIPInit();
            ETH_TestIperf();
        }
        else if((SAL_StrNCmp(str, ucIPInit, 6, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ETH_TestIPInit();
        }
        else if((SAL_StrNCmp(str, ucCornertest, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ETH_CornerSample_Test();
        }
#if ( RTL_8211E == 1)
        else if((SAL_StrNCmp(str, ucPread, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            uint32 addr = CSL_StringToHex(pArgv[1]);
            mcu_printf("PHY Read (0x%x) : 0x%x\n", addr, PHY_Read(addr));
        }
        else if((SAL_StrNCmp(str, ucPwrite, 6, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            uint32 addr = CSL_StringToHex(pArgv[1]);
            uint32 reg = CSL_StringToHex(pArgv[2]);

            PHY_Write(addr, reg);
            mcu_printf("PHY Read after write(0x%x) : 0x%x\n", addr, PHY_Read(addr));
        }
#endif
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }

}
#endif  // ( MCU_BSP_SUPPORT_APP_SAMPLE_ETH == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )
static void CSL_DeviceSsm(uint8 ucArgc, void *pArgv[])
{
    const uint8 *strCategory;
#ifdef CRD_TEST_ENABLE
    const uint8 *strAction;
#endif
    const uint8 ucBufTest[3] = "ecc";

    int32       ucMode;
    sint32      ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        strCategory = (const uint8 *)pArgv[0];
#ifdef CRD_TEST_ENABLE
        strAction   = (const uint8 *)pArgv[1];
#endif
        ucMode      = strtol(pArgv[2], NULL_PTR, 10UL);

        if ((SAL_StrNCmp(strCategory, (const uint8 *)ucBufTest, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            ucMode = strtol(pArgv[2], NULL_PTR, 10UL);

            if ( ECC_SRAM0 == ucMode )
            {
                //soft fault injection
                SSM_EccEnable(ECC_SRAM0, FMU_SVL_HIGH);
                SAL_WriteReg(0x5AFEACE5, 0xA0F26048);
                SAL_WriteReg(SAL_ReadReg(0xA0F26004), 0xA0F26004);
                //write lock
                SAL_WriteReg(0x5AFEACE5, 0xA0F26048);
                SAL_WriteReg(0x00000001, 0xA0F26038);
                //write lock
                SAL_WriteReg(0x5AFEACE5, 0xA0F26048);
                SAL_WriteReg(0x00000001, 0xA0F2603c);
                SAL_WriteReg(0xffffffff, 0x00000080);
            }
            else if ( ECC_SFMC == ucMode )
            {
                //soft fault injection
            }
            else if ( ECC_MBOX == ucMode )
            {
                //soft fault injection
            }
            else if ( ECC_HSM == ucMode )
            {
                //soft fault injection
            }
            else
            {
                ;
            }
        }
#ifdef CRD_TEST_ENABLE
        else if ((SAL_StrNCmp(strCategory, "crd", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            uint32 isAll =0;
            if ((SAL_StrNCmp(strAction, "all", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                isAll = 1;
            }
            // ucMode(Severity Level : 0 : Low, 1 : Mid, 2: High
            if ((isAll) || ((SAL_StrNCmp(strAction, "subsys", 6, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(0, ucMode);
            }
            if ((isAll) || ((SAL_StrNCmp(strAction, "cr5", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(1, ucMode);
            }
            if ((isAll) || ((SAL_StrNCmp(strAction, "mid", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(2, ucMode);
            }
            if ((isAll) || ((SAL_StrNCmp(strAction, "sm", 2, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(3, ucMode);
            }
        }
#endif
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_DRIVER_SSM == 1 )

#ifdef MIDF_SUPPORT
static void CSL_DeviceMidf(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    sint32 ret;
    uint32 uiMemoryBase = 0xC1000000UL; //SRAM1
    uint32 uiSize       = 0x1000UL; // 4KB
    uint32 uiID         = MIDF_ID_CAN0;
    uint32 uiType       =(MIDF_TYPE_READ|MIDF_TYPE_WRITE);

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, (const uint8 *)"init", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            MIDF_FilterInitialize();
        }
        else if((SAL_StrNCmp(str, (const uint8 *)"reg", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            MIDF_UnregisterFilterRegion(uiMemoryBase, uiSize, uiID, uiType);
        }
        else if ((SAL_StrNCmp(str, (const uint8 *)"del", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            MIDF_RegisterFilterRegion(uiMemoryBase, uiSize, uiID, uiType);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }

    return;
}
#endif

#if ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )
static void CSL_DeviceHsm(uint8 ucArgc, void *pArgv[])
{
    if (pArgv != NULL_PTR)
    {
        if (pArgv[0] != NULL)
        {
            HSM_Test((uint8*)pArgv[0], (uint8*)pArgv[1]);
        }
        else
        {
            (void)mcu_printf("\n Wrong argument, argv(%s)\n", pArgv[0]);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_HSM == 1 )

#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
static void CSL_SecureBootEnable(uint8 ucArgc, void *pArgv[])
{
    sint32 ret;

    ret = HSM_UpdateVerify(FWUD_STOR_TYPE_REMAP);
    if (ret != HSM_OK) {
        mcu_printf("verify failed(0x%x)\n", ret);
        return;
    }
    mcu_printf("SecureBoot Enable Success\n");

	PMU_ColdReset();
}
#endif  // ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )
static void CSL_DeviceRtc(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4]    = "time";
    const uint8 ucBufTest1[5]   = "alarm";
    uint32 uiVa = 0UL;
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 1, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            RTC_TEST_Time();
        }
        else if((SAL_StrNCmp(str, ucBufTest1, 1, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {

            uiVa = (uint32)strtoul(pArgv[1], NULL, 10);
            RTC_TEST_Alarm(uiVa);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_RTC == 1 )

static void CSL_EnableLog(uint8 ucArgc, void *pArgv[])
{
    if (pArgv != NULL_PTR)
    {
        if(DBG_VALID_ARGCOUNT(ucArgc))
        {
            ConsoleLogFunc[DBG_CNT_TO_IDX(ucArgc)].clFunc(pArgv);
        }
        else
        {
            CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
        }
    }
}

#if ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )
static void CSL_DeviceEFLASH(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucEccTest[3] = "ecc";
    const uint8 ucResetTest[5] = "reset";
    const uint8 ucintTest[3] = "int";
    const uint8 ucAccTest[3] = "acc";
    const uint8 ucDintTest[4] = "dint";
    const uint8 ucDEccTest[4] = "decc";
    const uint8 ucDAccTest[4] = "dacc";
    const uint8 ucDflashWriteTime[5] = "dtime";
    const uint8 ucSnorWriteTime[5] = "stime";

    sint32 ret;
    uint32 uiF = 0;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];
        uiF     = CSL_StringToHex(pArgv[1]);

        if((SAL_StrNCmp(str, ucEccTest, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_ECC(uiF);
        }
        else if((SAL_StrNCmp(str, ucResetTest, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_RESET(uiF);
        }
        else if((SAL_StrNCmp(str, ucintTest, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_Int_Test();
        }
        else if((SAL_StrNCmp(str, ucAccTest, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_PflashAccessControl(uiF);
        }
        else if((SAL_StrNCmp(str, ucDintTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_Int_DFLASHTest();
        }
        else if((SAL_StrNCmp(str, ucDEccTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_DflashEcc(uiF);
        }
        else if((SAL_StrNCmp(str, ucDAccTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_DflashAccessControl(uiF);
        }
        else if((SAL_StrNCmp(str, ucDflashWriteTime, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_flashEraseWrite(uiF);
        }
        else if((SAL_StrNCmp(str, ucSnorWriteTime, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            EFLASHTest_snorEraseWrite(uiF);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )

#if ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )
static void CSL_DeviceSFMC(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            SFMC_Test();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_TEST_APP_SFMC == 1 )

static void CSL_DeviceFWUD(uint8 ucArgc, void *pArgv[])
{
#if ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    const uint8 ucBufSync[4] = "sync";
    sint32 sRet;
    boolean bRet;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &sRet) == SAL_RET_SUCCESS) && (sRet == 0))
        {
            FWUD_Start();
        }
        else if((SAL_StrNCmp(str, ucBufSync, 4, &sRet) == SAL_RET_SUCCESS) && (sRet == 0))
        {
            bRet = FWUD_Sync();
            if(bRet == TRUE) {
                mcu_printf("\n Synchronize FW is succeeded \n");
            } else {
                mcu_printf("\n Synchronize FW is failed \n");
            }
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
#endif  // ( MCU_BSP_SUPPORT_APP_FW_UPDATE == 1 )
}

#if ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )
static void CSL_DeviceOTP(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufWrite[5] = "write";
    const uint8 ucBufRead[4] = "read";
    sint32 ret;
    uint32 addr;
    uint32 data[4] = {0};
    uint32 crc = 0;

    if (HSM_Init() != HSM_OK) {
        return;
    }

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufWrite, 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            if (pArgv[1] != NULL_PTR && pArgv[2] != NULL_PTR && pArgv[3] != NULL_PTR &&
                    pArgv[4] != NULL_PTR && pArgv[5] != NULL_PTR && ucArgc == 6) {
                addr = strtoul(pArgv[1], NULL, 16);
                data[0] = strtoul(pArgv[2], NULL, 16);
                data[1] = strtoul(pArgv[3], NULL, 16);
                data[2] = strtoul(pArgv[4], NULL, 16);
                data[3] = strtoul(pArgv[5], NULL, 16);
                ret = HSM_ProgramLDT(addr, data[0], data[1], data[2], data[3], &crc);
                if (ret != HSM_OK) {
                    mcu_printf("OTP Write failed(0x%x)\n", ret);
                }
                else {
                    mcu_printf("OTP Write success(0x%x)\n", ret);
                    mcu_printf("CRC of 128 bits Data : 0x%x\n", crc);
                }
            }
            else {
                mcu_printf("\nWrong argument num : %d\n", ucArgc);
                mcu_printf("usage : otp write [addr(HEX)] [data1(HEX)] [data2(HEX)] [data3(HEX)] [data4(HEX)]\n");
            }
        }
        else if((SAL_StrNCmp(str, ucBufRead, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            if (pArgv[1] != NULL_PTR && ucArgc == 2) {
                addr = strtoul(pArgv[1], NULL, 16);
                ret = HSM_ReadLDT(addr, &crc);
                if (ret != HSM_OK) {
                    mcu_printf("OTP Read failed\n");
                }
                else {
                    mcu_printf("CRC of 128 bits Data : 0x%x\n", crc);
                }
            }
            else {
                mcu_printf("\nWrong argument num : %d\n", ucArgc);
                mcu_printf("usage : otp read [addr(HEX)]\n");
            }
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
            mcu_printf("usage : otp write [addr(HEX)] [data1(HEX)] [data2(HEX)] [data3(HEX)] [data4(HEX)]\n");
            mcu_printf("      : otp read [addr(HEX)]\n");
        }
    }
}
#endif  // ( MCU_BSP_SUPPORT_DRIVER_HSM == 1 )

#ifdef ECHO_TEMP
static void CSL_DeviceSocket(uint8 ucArgc, void *pArgv[])
{
#ifdef OS_FREERTOS
    const uint8 *str;
    sint32 ret;
    if ((ucArgc > 0) && (pArgv != NULL_PTR))
    {
        str = (const uint8 *) pArgv[0];


        if((SAL_StrNCmp(str, (const uint8 *) "ipup", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            if (ucArgc == 3)
            {
                SOCKTEST_IPInit((const uint8 *) pArgv[1], (const uint8 *) pArgv[2]);
            }
            else
            {
                USAGEIPUP(ALLUSAGE)
            }
        }
        else if((SAL_StrNCmp(str, (const uint8 *) "cli", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            if (ucArgc == 3)
            {
                SOCKTEST_EchoSend((const uint8 *) pArgv[1], (const uint8 *) pArgv[2]);
            }
            else
            {
                USAGECLIENT(ALLUSAGE);
            }
        }
        else if((SAL_StrNCmp(str, (const uint8 *) "svr", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            SOCKTEST_EchoRecv();
        }
        else
        {
            USAGEIPUP(ALLUSAGE)
        }
    }
    else
    {
        USAGEIPUP(ALLUSAGE)
    }

err:
    mcu_printf("");
#endif
}
#endif


static void CSL_LogCmdType01(void *pArgv[])
{
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        if((SAL_StrNCmp(pArgv[0],  (const uint8 *)"help", DBG_HELP_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            CSL_UsageLogCmd(DBG_ENABLE, DBG_ENABLE);
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"on", DBG_ON_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = DBG_ENABLE_ALL;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"off", DBG_OFF_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = DBG_DISABLE_ALL;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"debug", DBG_LEVEL_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = (gDebugOption & ~DBG_LEVEL_POSITION) | DBG_LEVEL_DEBUG;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"error", DBG_LEVEL_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = (gDebugOption & ~DBG_LEVEL_POSITION) | DBG_LEVEL_ERROR;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"info", DBG_INFO_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            CSL_ShowLogInfo();
        }
        else
        {
            CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
        }
    }
}

static void CSL_LogCmdType02(void *pArgv[])
{
    uint32  tagIdx;
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        tagIdx = strtoul(pArgv[0], NULL_PTR, 10UL);

        if(DBG_VALID_TAGIDX(tagIdx))
        {
            if((SAL_StrNCmp(pArgv[1], (const uint8 *)"on", DBG_ON_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption | (DBG_TAG_BIT(tagIdx));
            }
            else if((SAL_StrNCmp(pArgv[1], (const uint8 *)"off", DBG_OFF_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption & ~(DBG_TAG_BIT(tagIdx));
            }
            else
            {
                CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
            }
        }
        else
        {
            CSL_UsageLogCmd(DBG_DISABLE, DBG_ENABLE);
        }
    }
}

static void CSL_LogCmdType03(void *pArgv[])
{
    uint32  tagIdx;
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        tagIdx = strtoul(pArgv[0], NULL_PTR, 10UL);

        if(DBG_VALID_TAGIDX(tagIdx))
        {
            if(DBG_EQUALS(SAL_StrNCmp(pArgv[1], (const uint8 *)"on", DBG_ON_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption | (DBG_TAG_BIT(tagIdx));
                if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"debug", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_DEBUG;
                }
                else if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"error", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_ERROR;
                }
                else
                {
                    CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
                }
            }
            else if(DBG_EQUALS(SAL_StrNCmp(pArgv[1], (const uint8 *)"off", DBG_OFF_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption & ~(DBG_TAG_BIT(tagIdx));

                if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"debug", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_DEBUG;
                }
                else if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"error", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_ERROR;
                }
                else
                {
                    CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
                }
            }
            else
            {
                CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
            }
        }
        else
        {
            CSL_UsageLogCmd(DBG_DISABLE, DBG_ENABLE);
        }
    }
}

static void CSL_UsageLogCmd(uint8 ucInfo, uint8 ucHelp)
{
    uint32 index;

    if(DBG_EQUALS(ucInfo, DBG_ENABLE))
    {
        mcu_printf("=== USAGE INFO ===\n");
        mcu_printf("log [Index] on/off debug/error\n");
        mcu_printf("log [Index] on/off\n");
        mcu_printf("log on/off\n");
        mcu_printf("log debug/error\n");
        mcu_printf("log info\n");
        mcu_printf("log help\n");
    }

    if(DBG_EQUALS(ucHelp, DBG_ENABLE))
    {
        mcu_printf("\n=== INDEX INFO ===\n");

        for(index = 0 ; index < DBG_MAX_TAGS ; index++)
        {
            mcu_printf("[%-5s] : [%2d]\n", dbgTags[index].dlTag, index);
        }
    }
}

static void CSL_ShowLogInfo(void)
{
    uint32 index;
    uint32 check = 0;
    uint32 cnt   = 0;

    mcu_printf("Enabled :");

    for(index = 0 ; index < (DBG_MAX_TAGS - 2UL) ; index++)
    {
        check = (gDebugOption >> index) & ~DBG_LEVEL_POSITION;

        if((check & DBG_CHECK_BIT) == DBG_CHECK_BIT)
        {
            mcu_printf(" %s(%2d) ", dbgTags[index].dlTag, index);
            cnt++;
        }
    }

    if(cnt == DBG_ZERO)
    {
        mcu_printf(" None ");
    }

    mcu_printf("\nLevel   :");
    check = (gDebugOption & DBG_LEVEL_POSITION);

    if(check == 0UL)
    {
        mcu_printf(" Error\n");
    }
    else
    {
        mcu_printf(" Debug\n");
    }
}

#ifdef CONSOLE_HISTORY_ENABLE

static CSLHistoryManager_t *pGetCSHistoryManager(void)
{
    return &CSHistoryManager;
}

static sint32 CSL_EmptyHistoryList(void)
{
    CSLHistoryManager_t *   pCSHistoryManager;
    uint32                  index;
    uint32                  flag    = 0;
    sint32                  ret     = -1;

    pCSHistoryManager = pGetCSHistoryManager();

    for(index = 0 ; index < CSL_HISTORY_COUNT ; index++)
    {
        flag = pCSHistoryManager->hmList[index].hcFlag;

        if(flag == OFF)
        {
            ret = (sint32)index;
            break;
        }
    }

    return ret;
}

static uint32 CSL_PullHistoryList(uint8 *pucCmdBuf, uint32 uiOldCmdSize)
{
    uint32                  num;
    uint32                  currPtr;
    uint32                  newCmdSize = 0;
    uint8                   listFlag;
    uint8               *   pHistoryBuf;
    CSLHistoryManager_t *   pCSHistoryManager;

    SAL_MemSet(pucCmdBuf, 0, CSL_CMD_BUFF_SIZE);
    pCSHistoryManager   = pGetCSHistoryManager();
    currPtr             = pCSHistoryManager->hmPtr;

    /* Remove current command in console */
    for(num = 0 ; num < uiOldCmdSize ; num ++)
    {
        UART_Write(UART_DEBUG_CH, (const uint8 *)"\b \b", 3);
    }

    for(num = 0 ; num < CSL_HISTORY_COUNT ; num++)
    {
        listFlag = pCSHistoryManager->hmList[currPtr].hcFlag;

        if(listFlag == ON)
        {
            pCSHistoryManager->hmPtr    = currPtr;
            pHistoryBuf = pCSHistoryManager->hmList[currPtr].hcBuf;
            newCmdSize  = pCSHistoryManager->hmList[currPtr].hcSize;

            UART_Write(UART_DEBUG_CH, (const uint8 *)pHistoryBuf, newCmdSize);
            SAL_MemCopy(pucCmdBuf, pHistoryBuf, newCmdSize);

            if(currPtr == 0UL)
            {
                currPtr = CSL_HISTORY_COUNT;
            }

            currPtr--;
            pCSHistoryManager->hmPtr = currPtr;
            break;
        }
        else
        {
            if(currPtr == 0UL)
            {
                currPtr = CSL_HISTORY_COUNT;
            }

            currPtr--;
        }
    }

    return newCmdSize;
}

static void CSL_PushHistoryList(uint8 *pucBuf, uint32 uiCmdSize)
{
    sint32                  index = 0;
    CSLHistoryManager_t *   pCSHistoryManager;
    uint8               *   pHistoryBuf;

    index = CSL_EmptyHistoryList();

    if(index < 0)
    {
        index = 0;
    }

    pCSHistoryManager   = pGetCSHistoryManager();
    pHistoryBuf         = pCSHistoryManager->hmList[index].hcBuf;

    if(pHistoryBuf != NULL)
    {
        SAL_MemSet(pHistoryBuf, 0, CSL_CMD_BUFF_SIZE);
        pCSHistoryManager->hmPtr                    = (uint32)index;
        pCSHistoryManager->hmList[index].hcFlag     = ON;
        pCSHistoryManager->hmList[index].hcIndex    = (uint8)index;
        pCSHistoryManager->hmList[index].hcSize     = uiCmdSize;
        SAL_MemCopy(pHistoryBuf, pucBuf, uiCmdSize);
    }
}
#endif


/*****************************************************************************/
/*                                 FUNCTIONS                                 */
/*****************************************************************************/

static void ConsoleTask(void *pArg)
{
    uint8   cmdBuffer[CSL_CMD_BUFF_SIZE + CSL_CMD_PROMPT_SIZE];
    uint32  cmdLength   = 0;
    uint8   cmdStatus   = (uint8)CSL_NOINPUT;
    sint8   getc_err;
    sint32  ret         =0;
    uint8   c           =0;
    uint32  idx;

    (void)pArg;

    for(;;)
    {
        ret = UART_GetChar(UART_DEBUG_CH, 0, (sint8 *)&getc_err);

        if(ret > 0)
        {
            c   = ((uint8)ret & 0xFFUL);

            switch(c)
            {
                case ARRIAGE_RETURN:
                case LINE_FEED:
                {
                    cmdStatus = (uint8)CSL_EXECUTE;
                    break;
                }

                case BACK_SPACE:
                case PUTTY_BS:
                {
                    if (cmdLength > 0UL)
                    {
                        cmdStatus = (uint8)CSL_INPUTING;
                        cmdLength--;
                        UART_Write(UART_DEBUG_CH, (const uint8 *)"\b \b", 3UL);
                    }

                    break;
                }

                case NAK_KEY:
                {

                    /* Remove current command in command buffer */
                    for(idx = 0UL; idx < cmdLength; idx ++)
                    {
                        UART_Write(UART_DEBUG_CH, (const uint8 *)"\b \b", 3UL);
                    }

                    cmdStatus   = (uint8)CSL_INPUTING;
                    cmdLength   = 0UL;
                    break;
                }

                case ESC_KEY:
                    break;

#ifdef CONSOLE_HISTORY_ENABLE
                case LBRACKET:
                {
                    cmdLength = CSL_PullHistoryList(cmdBuffer, cmdLength);
                    (void)UART_GetChar(UART_DEBUG_CH, 0, (sint8 *)&getc_err);
                    break;
                }
#endif
                default:
                {
                    if(cmdLength < (CSL_CMD_BUFF_SIZE-1UL))
                    {
                        cmdStatus               = (uint8)CSL_INPUTING;
                        cmdBuffer[cmdLength]    = c;
                        cmdLength++;
                        UART_Write(UART_DEBUG_CH, &c, 1UL);
                    }

                    break;
                }
            }

            if(cmdStatus == (uint8)CSL_EXECUTE)
            {
                cmdStatus = (uint8)CSL_NOINPUT;
                UART_Write(UART_DEBUG_CH, (const uint8 *)CRLF, 2UL);
                CSL_ExecuteCommand(cmdBuffer, cmdLength);
                UART_Write(UART_DEBUG_CH, (const uint8 *)CRLF, 2UL);
                UART_Write(UART_DEBUG_CH, (const uint8 *)"> ", 2UL);  //Prompt

                if(cmdLength != 0UL)
                {
#ifdef CONSOLE_HISTORY_ENABLE
                    CSL_PushHistoryList(cmdBuffer, cmdLength);
#endif
                    cmdLength   = 0UL;
                }
            }
        }
        else
        {
            (void)SAL_TaskSleep(50);
            //mcu_printf("ERROR NO ACK\n");
        }
    }
}

static uint32 CSL_GetOneWordLength(uint8 * puiCmdBuf, uint32 uiCmdLength)
{
    uint32  idx;
    uint8   key;

    for(idx = 0UL ; idx < uiCmdLength ; idx++)
    {
        key = *(puiCmdBuf+idx);

        if(key == SPACE_BAR)
        {
            break;
        }
    }

    return idx;
}

static uint32 CSL_GetCommandLength(const uint8 * puiCmdBuf)
{
    uint32  idx = 0UL;
    uint8   key;

    if (puiCmdBuf != NULL_PTR)
    {
        for( ; idx < CSL_CMD_BUFF_SIZE ; idx++)
        {
            key = *(puiCmdBuf+idx);

            if(key == NUL_KEY)
            {
                break;
            }
        }
    }

    return idx;
}


static void CSL_ExecuteCommand(uint8 * puiCmdBuf, uint32 uiCmdLength)
{
    uint32              idx;
    uint32              totalCnt        = 0;
    uint32              realCmdLength   = 0;
    uint32              paramLength     = 0;
    uint32              clNameLength;   // name length in command list
    ConsoleCmdList_t *  pConsoleCommand;
    uint8               ucArgc;
    void *              pArgv[CSL_ARGUMENT_MAX] = {NULL};
    uint32              curPosition     =0;
    sint32              ret;

    if (puiCmdBuf != NULL_PTR)
    {
        realCmdLength   = CSL_GetOneWordLength(puiCmdBuf, uiCmdLength);
        curPosition     = realCmdLength;

        if(uiCmdLength > 0UL)
        {
            for(ucArgc = 0UL ; ucArgc < CSL_ARGUMENT_MAX ; ucArgc++)
            {
                if(uiCmdLength <= curPosition)
                {
                    break;
                }
                else
                {
                    curPosition += CSL_SPACE_1COUNT;
                    paramLength = CSL_GetOneWordLength((puiCmdBuf+curPosition), uiCmdLength-curPosition);

                    if(paramLength != 0UL)
                    {
                        pArgv[ucArgc]   = (void *)puiCmdBuf+curPosition;
                        puiCmdBuf[curPosition+paramLength] = 0; // Exchange SPACE to NULL
                        curPosition     += paramLength;
                    }
                }
            }

            totalCnt = sizeof(ConsoleCmdList) / sizeof(ConsoleCmdList[0]);

            for(idx = 0UL ; idx < totalCnt ; idx++)
            {
                pConsoleCommand = pGetConsoleCmdList(idx);

                if(pConsoleCommand->clEnable == CMD_ENABLE)
                {
                    clNameLength    = CSL_GetCommandLength(pConsoleCommand->clName);

                    if(clNameLength == realCmdLength)
                    {
                        if((SAL_StrNCmp(pConsoleCommand->clName, puiCmdBuf, realCmdLength, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                        {
                            pConsoleCommand->clFunc(ucArgc, pArgv);
                        }
                    }
                }
            }
        }
    }
}

void CreateConsoleTask(void)
{
    static uint32   ConsoleTaskID = 0UL;
    static uint32   ConsoleTaskStk[CSL_TASK_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &ConsoleTaskID,
        (const uint8 *)"Console Task",
        (SALTaskFunc)&ConsoleTask,
        &ConsoleTaskStk[0],
        CSL_TASK_STK_SIZE,
        SAL_PRIO_CONSOLE,
        NULL
    );
}

#endif  // ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )


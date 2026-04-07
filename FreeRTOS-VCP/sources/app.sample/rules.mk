# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : ruls.mk
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

MCU_BSP_APP_SAMPLE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Main
include $(MCU_BSP_APP_SAMPLE_PATH)/app.base/rules.mk

# CAN Demo Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_CAN_DEMO), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.can.demo/rules.mk
endif

# Console Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_CONSOLE), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.console/rules.mk
endif

# FWUD Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.fw.update/rules.mk
else ifeq ($(MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE_ECCP), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.fw.update.eccp/rules.mk
endif

# Idle State
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_IDLE), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.idle/rules.mk
endif

# iPerf
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_IPERF), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.iperf/rules.mk
endif

# Key Demo Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_KEY_DEMO), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.key.demo/rules.mk
endif

# Key Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_KEY), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.key/rules.mk
endif

# LED On Demo Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_SPI_LED), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.spi.led/rules.mk
endif

# ADC Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_ADC), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.adc/rules.mk
endif

# Audio Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_AUDIO), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.audio/rules.mk
endif

# CAN Test Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_CAN), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.can/rules.mk
endif

# CPU Test Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_CPU), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.cpu/rules.mk
endif

# DSE Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_DSE), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.dse/rules.mk
endif

# EFLASH Test Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_EFLASH), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.eflash/rules.mk
endif

# FMU Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_FMU), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.fmu/rules.mk
endif

# GDMA Sample Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_GDMA), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gdma/rules.mk
endif

# GIC Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_GIC), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gic/rules.mk
endif

# GPIO Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_GPIO), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gpio/rules.mk
endif

# GPSB Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_GPSB), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gpsb/rules.mk
endif

# HSM Sample Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_HSM), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.hsm/rules.mk
endif

# I2C Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_I2C), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.i2c/rules.mk
endif

# ICTC Sample Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_ICTC), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.ictc/rules.mk
endif

# LIN Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_LIN), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.lin/rules.mk
endif

# PDM Sample Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_PDM), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.pdm/rules.mk
endif

# PMU Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_PMU), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.pmu/rules.mk
endif

# RTC Test Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_RTC), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.rtc/rules.mk
endif

# SFMC Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_SFMC), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.sfmc/rules.mk
endif

# SOCKET Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_SOCKET), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.socket/rules.mk
endif

# Timer Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_TIMER), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.timer/rules.mk
endif

# UART Verification  Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_UART), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.uart/rules.mk
endif

# Watchdog Verification Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_WDT), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.wdt/rules.mk
endif

# Writelock Test Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_WRITEBACK), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.writelock/rules.mk
endif

# Ethernet Application
ifeq ($(MCU_BSP_BUILD_FLASG_TEST_APP_ETH), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.eth/rules.mk
endif

# SPU Test Application
ifeq ($(MCU_BSP_BUILD_FLAGS_TEST_APP_SPU), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.spu/rules.mk
endif

# PWC Test Application.
ifeq ($(ACFG_APP_POWER_COMMUNICATION_EN), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.expwr.demo/rules.mk
endif

ifeq ($(APLT_APP_LINUX_SUPPORT_SPI_DEMO), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.spi.eccp/rules.mk
endif

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

MCU_BSP_DEV_DRIVERS_PATH := $(MCU_BSP_BUILD_CURDIR)

# Ananlog-to-Digital Converter
include $(MCU_BSP_DEV_DRIVERS_PATH)/adc/rules.mk

# Controller Area Network
include $(MCU_BSP_DEV_DRIVERS_PATH)/can/rules.mk

# CLOCK Control Unit
include $(MCU_BSP_DEV_DRIVERS_PATH)/clock/rules.mk

# Common
include $(MCU_BSP_DEV_DRIVERS_PATH)/common/rules.mk

# Default Slave Error
include $(MCU_BSP_DEV_DRIVERS_PATH)/dse/rules.mk

# Eflash
include $(MCU_BSP_DEV_DRIVERS_PATH)/eflash/rules.mk

# Ethernet
include $(MCU_BSP_DEV_DRIVERS_PATH)/eth/rules.mk

# Fault Management Unut
include $(MCU_BSP_DEV_DRIVERS_PATH)/fmu/rules.mk

# Direct Memory Access
include $(MCU_BSP_DEV_DRIVERS_PATH)/gdma/rules.mk

# General Interrupt Controller
include $(MCU_BSP_DEV_DRIVERS_PATH)/gic/rules.mk

# General Purpose I/O
include $(MCU_BSP_DEV_DRIVERS_PATH)/gpio/rules.mk

# Serial Peripheral Interface
include $(MCU_BSP_DEV_DRIVERS_PATH)/gpsb/rules.mk

# Inter-Integrated Circuit
include $(MCU_BSP_DEV_DRIVERS_PATH)/i2c/rules.mk

# Integrated Interchip Sound
include $(MCU_BSP_DEV_DRIVERS_PATH)/i2s/rules.mk

# Input Capture Time Counter
include $(MCU_BSP_DEV_DRIVERS_PATH)/ictc/rules.mk

# Mail-Box
include $(MCU_BSP_DEV_DRIVERS_PATH)/mailbox/rules.mk

# Memory ID Filter(TZC400)
include $(MCU_BSP_DEV_DRIVERS_PATH)/midf/rules.mk

# Memory Protection Unit
include $(MCU_BSP_DEV_DRIVERS_PATH)/mpu/rules.mk

# Preload
include $(MCU_BSP_DEV_DRIVERS_PATH)/preload/rules.mk

# Pulse Density Modulator
include $(MCU_BSP_DEV_DRIVERS_PATH)/pdm/rules.mk

# Power Management I/O
include $(MCU_BSP_DEV_DRIVERS_PATH)/pmio/rules.mk

# Power Management UNIT
include $(MCU_BSP_DEV_DRIVERS_PATH)/pmu/rules.mk

# RTC
include $(MCU_BSP_DEV_DRIVERS_PATH)/rtc/rules.mk

# SFMC
include $(MCU_BSP_DEV_DRIVERS_PATH)/sfmc/rules.mk

# SPU
include $(MCU_BSP_DEV_DRIVERS_PATH)/spu/rules.mk

# System Safety Mechanism
include $(MCU_BSP_DEV_DRIVERS_PATH)/ssm/rules.mk

# Timer
include $(MCU_BSP_DEV_DRIVERS_PATH)/timer/rules.mk

# Unuversal Asynchronous Receiver/Transimitter
include $(MCU_BSP_DEV_DRIVERS_PATH)/uart/rules.mk

# Watchdog
include $(MCU_BSP_DEV_DRIVERS_PATH)/watchdog/rules.mk


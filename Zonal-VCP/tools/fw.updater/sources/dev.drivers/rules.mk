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

# CLOCK Control Unit
include $(MCU_BSP_DEV_DRIVERS_PATH)/clock/rules.mk

# Common
include $(MCU_BSP_DEV_DRIVERS_PATH)/common/rules.mk

# Eflash
include $(MCU_BSP_DEV_DRIVERS_PATH)/eflash/rules.mk

# Direct Memory Access
include $(MCU_BSP_DEV_DRIVERS_PATH)/gdma/rules.mk

# General Interrupt Controller
include $(MCU_BSP_DEV_DRIVERS_PATH)/gic/rules.mk

# General Purpose I/O
include $(MCU_BSP_DEV_DRIVERS_PATH)/gpio/rules.mk

# Serial Peripheral Interface
include $(MCU_BSP_DEV_DRIVERS_PATH)/gpsb/rules.mk

# Mail-Box
include $(MCU_BSP_DEV_DRIVERS_PATH)/mailbox/rules.mk

# Memory Protection Unit
include $(MCU_BSP_DEV_DRIVERS_PATH)/mpu/rules.mk

#SFMC
include $(MCU_BSP_DEV_DRIVERS_PATH)/sfmc/rules.mk

# Unuversal Asynchronous Receiver/Transimitter
include $(MCU_BSP_DEV_DRIVERS_PATH)/uart/rules.mk


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

MCU_BSP_OS_FREERTOS_PORTABLE_PATH := $(MCU_BSP_BUILD_CURDIR)

# add rules file
include $(MCU_BSP_OS_FREERTOS_PORTABLE_PATH)/$(MCU_BSP_ARM_CORE_NAME)/rules.mk
include $(MCU_BSP_OS_FREERTOS_PORTABLE_PATH)/MemMang/rules.mk


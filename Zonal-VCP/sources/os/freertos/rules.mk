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

MCU_BSP_OS_FREERTOS_PATH := $(MCU_BSP_BUILD_CURDIR)

include $(MCU_BSP_OS_FREERTOS_PATH)/FreeRTOS-Kernel/rules.mk

ifeq ($(MCU_BSP_BUILD_TARGET_SUPPORT_DRIVER_ETHERNET), 1)
    include $(MCU_BSP_OS_FREERTOS_PATH)/FreeRTOS-Plus-TCP/rules.mk
endif


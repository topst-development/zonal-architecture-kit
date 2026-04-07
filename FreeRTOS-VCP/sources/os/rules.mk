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

MCU_BSP_OS_PATH := $(MCU_BSP_BUILD_CURDIR)

# uCOS
ifeq ($(MCU_BSP_TARGET_OS), ucos)
    include $(MCU_BSP_OS_PATH)/ucos/rules.mk
endif

# free RTOS
ifeq ($(MCU_BSP_TARGET_OS), freertos)
    include $(MCU_BSP_OS_PATH)/freertos/rules.mk
endif


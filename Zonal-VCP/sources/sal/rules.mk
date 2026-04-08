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

MCU_BSP_SAL_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_SAL_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_SAL_PATH)

# Sources
ASMSRCS += sal.S

SRCS += sal_api.c

ifeq ($(MCU_BSP_TARGET_OS), ucos)
    include $(MCU_BSP_SAL_PATH)/sal_ucos/rules.mk
endif

ifeq ($(MCU_BSP_TARGET_OS), freertos)
    include $(MCU_BSP_SAL_PATH)/sal_freertos/rules.mk
endif


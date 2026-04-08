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

MCU_BSP_OS_UCOS_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_OS_UCOS_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_OS_UCOS_PATH)

# Sources
SRCS += bsp_os.c

#uCOS
include $(MCU_BSP_OS_UCOS_PATH)/uC-CPU/rules.mk
include $(MCU_BSP_OS_UCOS_PATH)/uC-LIB/rules.mk
include $(MCU_BSP_OS_UCOS_PATH)/uCOS-III/rules.mk

# os configuration
include $(MCU_BSP_OS_UCOS_PATH)/uCOS/rules.mk


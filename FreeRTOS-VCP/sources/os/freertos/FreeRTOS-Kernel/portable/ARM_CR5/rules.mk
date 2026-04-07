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

MCU_BSP_OS_FREERTOS_PORTABLE_ARM_CR5_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_OS_FREERTOS_PORTABLE_ARM_CR5_PATH)
VPATH += $(MCU_BSP_OS_FREERTOS_PORTABLE_ARM_CR5_PATH)/GCC

# Includes
INCLUDES += -I$(MCU_BSP_OS_FREERTOS_PORTABLE_ARM_CR5_PATH)

# Sources
ASMSRCS += portASM.S
ASMSRCS += freertos.S

SRCS += port.c


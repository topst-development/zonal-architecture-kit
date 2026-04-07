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

MCU_BSP_OS_FREERTOS_PORTABLE_MEMMANG_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_OS_FREERTOS_PORTABLE_MEMMANG_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_OS_FREERTOS_PORTABLE_MEMMANG_PATH)

# Sources
SRCS += heap_5.c


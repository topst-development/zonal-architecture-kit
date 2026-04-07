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

MCU_BSP_OS_FREERTOS_KERNEL_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_OS_FREERTOS_KERNEL_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_OS_FREERTOS_KERNEL_PATH)
INCLUDES += -I$(MCU_BSP_OS_FREERTOS_KERNEL_PATH)/include

# Sources
SRCS += tasks.c
SRCS += list.c
SRCS += queue.c
SRCS += event_groups.c
SRCS += timers.c
SRCS += stream_buffer.c

include $(MCU_BSP_OS_FREERTOS_KERNEL_PATH)/portable/rules.mk


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

MCU_BSP_APP_SAMPLE_IDLE_TASK_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_IDLE=1

# Include
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_IDLE_TASK_PATH)

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_IDLE_TASK_PATH)

# Sources
SRCS += idle.c


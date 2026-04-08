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

MCU_BSP_APP_SAMPLE_POWER_APP_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DAPLT_LINUX_SUPPORT_POWER_CTRL=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_POWER_APP_PATH)

# Include
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_POWER_APP_PATH)

# Sources
SRCS += power_app.c
SRCS += power_com.c


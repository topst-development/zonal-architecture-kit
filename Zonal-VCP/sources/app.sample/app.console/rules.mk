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

MCU_BSP_APP_SAMPLE_CONSOLE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_CONSOLE=1
COMMON_FLAGS += -DCONSOLE_HISTORY_ENABLE
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_PMIO=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_CONSOLE_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_CONSOLE_PATH)

# Sources
SRCS += console.c


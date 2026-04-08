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

MCU_BSP_APP_IPERF_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_IPERF=1

# Paths
VPATH += $(MCU_BSP_APP_IPERF_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_IPERF_PATH)

# Sources
SRCS += iperf_task_v3_0d.c


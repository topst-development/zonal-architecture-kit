# SPDX-License-Identifier: Apache-2.0

##################################################################################################
#
#   FileName : ruls.mk
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

MCU_BSP_APP_SAMPLE_FW_UPDATE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_FW_UPDATE_ECCP=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_FW_UPDATE_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_FW_UPDATE_PATH)

# Sources
SRCS += fwupdate.c


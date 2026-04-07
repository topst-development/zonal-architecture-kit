# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : rules.mk
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

MCU_BSP_DEV_DRIVERS_DSE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_DEV_DRIVERS_DSE_PATH)
VPATH += $(MCU_BSP_DEV_DRIVERS_DSE_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Includes
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_DSE_PATH)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_DSE_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Sources
SRCS += dse.c


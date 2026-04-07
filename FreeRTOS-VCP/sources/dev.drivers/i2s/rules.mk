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

MCU_BSP_DEV_DRIVERS_I2S_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_DEV_DRIVERS_I2S_PATH)
VPATH += $(MCU_BSP_DEV_DRIVERS_I2S_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Includes
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_I2S_PATH)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_I2S_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Sources
SRCS += i2s.c
#SRCS += i2s_reg.c


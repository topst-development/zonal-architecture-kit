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

MCU_BSP_DEV_DRIVERS_ETH_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_DEV_DRIVERS_ETH_PATH)
VPATH += $(MCU_BSP_DEV_DRIVERS_ETH_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Includes
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_ETH_PATH)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_ETH_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Sources
SRCS += eth.c
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_REALTEK),1)
SRCS += eth_mii_rtl8211e.c
else
SRCS += eth_mii.c
endif


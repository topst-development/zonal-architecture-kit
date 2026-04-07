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

MCU_BSP_DEV_DRIVERS_MBOX_PATH := $(MCU_BSP_BUILD_CURDIR)
MCU_BSP_DEV_DRIVERS_MBOX_COMMON_PATH := common

# Paths
VPATH += $(MCU_BSP_DEV_DRIVERS_MBOX_PATH)
VPATH += $(MCU_BSP_DEV_DRIVERS_MBOX_PATH)$(MCU_BSP_CHIPSET_FAMILY_NAME)
VPATH += $(MCU_BSP_DEV_DRIVERS_MBOX_PATH)$(MCU_BSP_DEV_DRIVERS_MBOX_COMMON_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_MBOX_PATH)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_MBOX_PATH)$(MCU_BSP_CHIPSET_FAMILY_NAME)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_MBOX_PATH)$(MCU_BSP_DEV_DRIVERS_MBOX_COMMON_PATH)

# Sources
SRCS += mbox_phy.c
SRCS += mbox_dev.c
SRCS += mbox.c


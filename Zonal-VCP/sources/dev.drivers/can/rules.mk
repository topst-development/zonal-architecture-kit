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

MCU_BSP_DEV_DRIVERS_CAN_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_DEV_DRIVERS_CAN_PATH)
VPATH += $(MCU_BSP_DEV_DRIVERS_CAN_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Includes
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_CAN_PATH)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_CAN_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Sources
SRCS += can.c
SRCS += can_drv.c
SRCS += can_msg.c
SRCS += can_par.c
SRCS += can_porting.c


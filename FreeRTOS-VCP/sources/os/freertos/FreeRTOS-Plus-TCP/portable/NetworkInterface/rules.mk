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

MCU_BSP_FREERTOS_PLUST_TCP_PORTABLE_NETWORK_INTERFACE_PATH := $(MCU_BSP_BUILD_CURDIR)

VPATH += $(MCU_BSP_FREERTOS_PLUST_TCP_PORTABLE_NETWORK_INTERFACE_PATH)/board_family

INCLUDE += -I$(MCU_BSP_FREERTOS_PLUST_TCP_PORTABLE_NETWORK_INTERFACE_PATH)/include

SRCS += NetworkInterface.c


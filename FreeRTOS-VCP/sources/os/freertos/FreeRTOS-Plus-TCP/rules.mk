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

MCU_BSP_OS_FREERTOS_PLUS_TCP_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_OS_FREERTOS_PLUS_TCP_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_OS_FREERTOS_PLUS_TCP_PATH)
INCLUDES += -I$(MCU_BSP_OS_FREERTOS_PLUS_TCP_PATH)/include

# Sources
SRCS += FreeRTOS_ARP.c
SRCS += FreeRTOS_DHCP.c
SRCS += FreeRTOS_DNS.c
SRCS += FreeRTOS_IP.c
SRCS += FreeRTOS_Sockets.c
SRCS += FreeRTOS_Stream_Buffer.c
SRCS += FreeRTOS_TCP_IP.c
SRCS += FreeRTOS_TCP_WIN.c
SRCS += FreeRTOS_UDP_IP.c
SRCS += FreeRTOS_Port.c

include $(MCU_BSP_OS_FREERTOS_PLUS_TCP_PATH)/portable/rules.mk


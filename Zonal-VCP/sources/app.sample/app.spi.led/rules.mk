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

MCU_BSP_APP_SAMPLE_SPI_LED_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_SPI_LED=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_SPI_LED_PATH)

# Include
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_SPI_LED_PATH)

# Sources
SRCS += spi_led.c


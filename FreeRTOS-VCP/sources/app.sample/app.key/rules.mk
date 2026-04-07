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

MCU_BSP_APP_SAMPLE_KEY_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_KEY=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_KEY_TEST_PATH)

# Include
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_KEY_TEST_PATH)

# Sources
SRCS += key_adc.c
SRCS += key_gpio.c
#SRCS += key_i2c.c
SRCS += key.c


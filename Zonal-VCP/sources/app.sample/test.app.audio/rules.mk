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

MCU_BSP_APP_SAMPLE_AUDIO_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_AUDIO=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_AUDIO_DEMO=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_AUDIO_TEST_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_AUDIO_TEST_PATH)

# Sources
SRCS += audio_test.c
SRCS += audio_chip_veri.c
#SRCS += wm8731.c
SRCS += wm8904.c


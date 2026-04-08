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

MCU_BSP_APP_SAMPLE_SFMC_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flasgs
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_SFMC=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_SFMC_TEST_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_SFMC_TEST_PATH)

# Sources
SRCS += sfmc_test.c


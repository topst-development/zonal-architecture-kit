# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : rules.mk
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

MCU_BSP_APP_SAMPLE_ECCP_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_ECCP_TEST_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_ECCP_TEST_PATH)

# Sources
SRCS += eccp_test.c


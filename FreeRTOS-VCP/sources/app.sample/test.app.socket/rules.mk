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

MCU_BSP_APP_SAMPLE_SOCKET_TEST_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_TEST_APP_SOCKET=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_SOCKET_TEST_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_SOCKET_TEST_PATH)

# Sources
SRCS += echo_test.c


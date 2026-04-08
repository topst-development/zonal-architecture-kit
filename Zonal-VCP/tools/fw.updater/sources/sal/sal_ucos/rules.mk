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

MCU_BSP_SAL_UCOS_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_SAL_UCOS_PATH)

# Includes
INCLUDES += -I$(MCU_BSP_SAL_UCOS_PATH)

# Sources
SRCS += sal_ucos_impl.c


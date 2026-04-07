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

MCU_BSP_APP_SAMPLE_SPI_ECCP_PATH := $(MCU_BSP_BUILD_CURDIR)

#Flags
COMMON_FLAGS += -DAPLT_LINUX_SUPPORT_SPI_DEMO=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_SPI_ECCP_PATH)

# Include
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_SPI_ECCP_PATH)

# Sources
SRCS += spi_eccp.c


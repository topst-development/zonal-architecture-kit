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

MCU_BSP_APP_DRIVERS_HSM_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags

# Paths
VPATH += $(MCU_BSP_APP_DRIVERS_HSM_PATH)$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Includes
INCLUDES += -I$(MCU_BSP_APP_DRIVERS_HSM_PATH)$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Sources
#SRCS += HSMManagerA53.c
#SRCS += CryptoDataA53.c
SRCS += hsm_manager.c


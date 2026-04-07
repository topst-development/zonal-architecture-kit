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

MCU_BSP_APP_DRIVERS_PATH := $(MCU_BSP_BUILD_CURDIR)

# HSM Manager
include $(MCU_BSP_APP_DRIVERS_PATH)/hsm/rules.mk


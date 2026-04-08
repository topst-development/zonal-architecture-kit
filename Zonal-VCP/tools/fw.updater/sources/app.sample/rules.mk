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

MCU_BSP_APP_SAMPLE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Main
include $(MCU_BSP_APP_SAMPLE_PATH)/app.base/rules.mk

# FWUD Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.fw.update/rules.mk
else ifeq ($(MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE_ECCP), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.fw.update.eccp/rules.mk
endif

# External Core Communication Protocol Sample Application
ifeq ($(MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE_ECCP), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.spi.eccp/rules.mk
endif

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

MCU_BSP_PATH := $(MCU_BSP_BUILD_CURDIR)

# Application Sample
include $(MCU_BSP_PATH)/app.sample/rules.mk

# Application Drivers
include $(MCU_BSP_PATH)/app.drivers/rules.mk

# MCU Core
include $(MCU_BSP_PATH)/core/rules.mk

# Device Drivers
include $(MCU_BSP_PATH)/dev.drivers/rules.mk

# OS
include $(MCU_BSP_PATH)/os/rules.mk

# SAL, System Abstraction Layer
include $(MCU_BSP_PATH)/sal/rules.mk


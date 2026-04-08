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

MCU_BSP_DEV_DRIVERS_RTC_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
# CONFIGURATION for RTC FUNCTION ===================================================
#    RTC_CONF_DEBUG_ALONE   : [Def] Print always about errors and debugs log.
#                                    (No use a LOG_D/E console.)
COMMON_FLAGS += -DRTC_CONF_DEBUG_ALONE
# =================================================== CONFIGURATION for RTC FUNCTION

# Paths
VPATH += $(MCU_BSP_DEV_DRIVERS_RTC_PATH)
VPATH += $(MCU_BSP_DEV_DRIVERS_RTC_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Includes
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_RTC_PATH)
INCLUDES += -I$(MCU_BSP_DEV_DRIVERS_RTC_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)

# Sources
SRCS += rtc_dev.c


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

MCU_BSP_CORE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Paths
VPATH += $(MCU_BSP_CORE_PATH)
VPATH += $(MCU_BSP_CORE_PATH)/GCC

# Includes
INCLUDES += -I$(MCU_BSP_CORE_PATH)

# Sources
ASMSRCS += startup.S
ASMSRCS += vector.S
ASMSRCS += vfp_init.S
ASMSRCS += arm_a.S


# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : Makefile
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

.PHONY: all check_eula

all: check_eula

check_eula:
	@if [ ! -f ../../../.eula_accepted ]; then \
		echo "You must accept the EULA before building."; \
		echo "Please go to {build_dir}/topst-vcp/"; \
		echo "Please run: {build_dir}/topst-vcp/easy_setup_vcp-g.sh"; \
		exit 1; \
	fi
build:
	@echo "Building..."
	make

# Find the local dir of the make file
MCU_BSP_BUILD_CURDIR                ?= $(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
MCU_BSP_BUID_DATE                   ?= $(shell date +%Y%m%d%H%M)

TOPDIR                              ?= ./../../..
TOPDIR_SRC                          ?= ./../../../sources

# Paths for Build
MCU_BSP_TOP_PATH                    ?= $(TOPDIR)
MCU_BSP_BUILD_PATH                  ?= $(MCU_BSP_TOP_PATH)/build
MCU_BSP_BUILD_TCC70xx_PATH          ?= $(MCU_BSP_BUILD_PATH)/tcc70xx
MCU_BSP_BUILD_TCC70xx_GCC_PATH      ?= $(MCU_BSP_BUILD_TCC70xx_PATH)/gcc
MCU_BSP_BUILD_TCC70XX_MAKE_UTILITY  ?= $(MCU_BSP_BUILD_TCC70xx_PATH)/make_utility
MCU_BSP_DOCUMENTS_PATH              ?= $(MCU_BSP_TOP_PATH)/documents
MCU_BSP_SCRIPTS_PATH                ?= $(MCU_BSP_TOP_PATH)/scripts
MCU_BSP_SOURCES_PATH                ?= $(MCU_BSP_TOP_PATH)/sources
MCU_BSP_SOURCES_APP_DRIVERS_PATH    ?= $(MCU_BSP_SOURCES_PATH)/app.drivers
MCU_BSP_SOURCES_APP_SAMPLE_PATH     ?= $(MCU_BSP_SOURCES_PATH)/app.sample
MCU_BSP_SOURCES_CORE_PATH           ?= $(MCU_BSP_SOURCES_PATH)/core
MCU_BSP_SOURCES_DEV_DRIVERS_PATH    ?= $(MCU_BSP_SOURCES_PATH)/dev.drivers
MCU_BSP_SOURCES_OS_PATH             ?= $(MCU_BSP_SOURCES_PATH)/os
MCU_BSP_SOURCES_SAL_PATH            ?= $(MCU_BSP_SOURCES_PATH)/sal
MCU_BSP_TOOLS_PATH                  ?= $(MCU_BSP_TOP_PATH)/tools


# Build Flags
MCU_BSP_BUILD_FLAGS_ENV_HOST_WINDOW         ?= 0
MCU_BSP_BUILD_FLAGS_TARGET_CHIPSET_TCC7022  ?= 0
MCU_BSP_BUILD_FLAGS_TARGET_OS_FREERTOS      ?= 1
MCU_BSP_BUILD_FLAGS_TARGET_OS_UCOS          ?= 0
MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_NONE         ?= 0
MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_D16          ?= 1
MCU_BSP_BUILD_FLAGS_VERBOSE                 ?= 0
MCU_BSP_BUILD_FLAGS_DEBUG                   ?= 1
MCU_BSP_BUILD_FLAGS_PFLASH_BOOT             ?= 1
MCU_BSP_BUILD_FLAGS_SECURE_UPDATE           ?= 1
MCU_BSP_BUILD_FLAGS_VCP_MODULE_BOARD        ?= 1

# Device Driver Build Flags
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ADC       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_CAN       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_DSE       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_EFLASH    ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ETH       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_MARVELL   ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_REALTEK   ?= 0
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_FMU       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_GDMA      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_GPSB      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_I2C       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_I2S       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ICTC      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_MBOX      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_MIDF      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PRELOAD   ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PDM       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PMIO      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PMU       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_RTC       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SFMC      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SPU       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SSM       ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_UART      ?= 1
MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_WATCHDOG  ?= 1

# Application Driver Build Flags
MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_HSM  ?= 1
MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_LIN  ?= 1
MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_SWL  ?= 1

# Sample Application Build Flags
MCU_BSP_BUILD_FLAGS_APP_CAN_DEMO            ?= 1
MCU_BSP_BUILD_FLAGS_APP_CONSOLE             ?= 1
MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE           ?= 1
MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE_ECCP      ?= 0
MCU_BSP_BUILD_FLAGS_APP_IDLE                ?= 1
MCU_BSP_BUILD_FLAGS_APP_IPERF               ?= 1
MCU_BSP_BUILD_FLAGS_APP_KEY_DEMO            ?= 0
MCU_BSP_BUILD_FLAGS_APP_KEY                 ?= 0
MCU_BSP_BUILD_FLAGS_APP_SPI_LED             ?= 1

# Test Application Build Flags
MCU_BSP_BUILD_FLAGS_TEST_APP_ADC            ?= 0
MCU_BSP_BUILD_FLAGS_TEST_APP_AUDIO          ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_CAN            ?= 0
MCU_BSP_BUILD_FLAGS_TEST_APP_CPU            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_DSE            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_EFLASH         ?= 1
MCU_BSP_BUILD_FLASG_TEST_APP_ETH            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_FMU            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_GDMA           ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_GIC            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_GPIO           ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_GPSB           ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_HSM            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_I2C            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_ICTC           ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_LIN            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_PDM            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_PMU            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_RTC            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_SFMC           ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_SOCKET         ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_SPU            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_TIMER          ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_UART           ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_WDT            ?= 1
MCU_BSP_BUILD_FLAGS_TEST_APP_WRITEBACK      ?= 1

#Test for STR
ACFG_APP_POWER_COMMUNICATION_EN             ?= 1
ACFG_APP_POWER_EXT_AP_CTL_EN                ?= 1
APLT_APP_LINUX_SUPPORT_SPI_DEMO             ?= 1
# Build Environment
MCU_BSP_TOOLCHAIN_PATH   ?= /opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi
MCU_BSP_TOOLCHAIN_PREFIX ?= $(MCU_BSP_TOOLCHAIN_PATH)/bin/arm-eabi-
LIBCDIR   := $(MCU_BSP_TOOLCHAIN_PATH)/arm-eabi/libc/usr/lib
LIBGCCDIR := $(MCU_BSP_TOOLCHAIN_PATH)/lib/gcc/arm-eabi/7.2.1
LIBGCC    := -L$(LIBCDIR) -L$(LIBGCCDIR) -lc -lm -lgcc
AS        := $(MCU_BSP_TOOLCHAIN_PREFIX)as
CC        := $(MCU_BSP_TOOLCHAIN_PREFIX)gcc
LD        := $(MCU_BSP_TOOLCHAIN_PREFIX)ld
SIZE      := $(MCU_BSP_TOOLCHAIN_PREFIX)size
OBJCOPY   := $(MCU_BSP_TOOLCHAIN_PREFIX)objcopy
OBJDUMP   := $(MCU_BSP_TOOLCHAIN_PREFIX)objdump
INCLUDES  += -I$(MCU_BSP_BUILD_ROOT_PATH)
INCLUDES  += -Iinclude
INCLUDES  += -I$(LIBCDIR)/../include
INCLUDES  += -I$(LIBGCCDIR)/include

ifneq ($(MCU_BSP_BUILD_FLAGS_VERBOSE), 1)
    MCU_BSP_BUILD_QUIET ?= @
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_DEBUG), 1)
    MCU_BSP_BUILD_CONFIG ?= debug
else
    MCU_BSP_BUILD_CONFIG ?= release
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_TARGET_OS_FREERTOS), 1)
    MCU_BSP_TARGET_OS ?= freertos
else ifeq ($(MCU_BSP_BUILD_FLAGS_TARGET_OS_UCOS), 1)
    MCU_BSP_TARGET_OS ?= ucos
else
    $(error Please select corrent OS.)
endif

# TCC70xx VFP soupport only 16 mode, not support 32 mode
ifeq ($(MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_NONE), 1)
    MCU_BSP_OS_CPU_VFP ?= none
else ifeq ($(MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_D16), 1)
    MCU_BSP_OS_CPU_VFP ?= d16
endif

MCU_BSP_CHIPSET_NAME        ?= tcc70xx
MCU_BSP_ARM_CORE_NAME       ?= ARM_CR5
MCU_BSP_CHIPSET_FAMILY_NAME ?= tcc70xx
MCU_BSP_PROJECT_NAME        ?= boot

MCU_BSP_BUILD_ROOT_PATH     = $(MCU_BSP_BUILD_TCC70xx_GCC_PATH)/$(MCU_BSP_CHIPSET_NAME)-$(MCU_BSP_TARGET_OS)-$(MCU_BSP_BUILD_CONFIG)
MCU_BSP_BUILD_OBJS_PATH     = $(MCU_BSP_BUILD_ROOT_PATH)/obj
MCU_BSP_BUILD_IMG_OUT_PATH  = $(MCU_BSP_BUILD_TCC70xx_GCC_PATH)/output

# Path for save compile console output, which used for calculating each module's memory size
COMPILE_SCRIPT_SAVE_PATH    = $(MCU_BSP_BUILD_ROOT_PATH)/compile_console_output.log

ifeq ($(MCU_BSP_BUILD_FLAGS_TARGET_CHIPSET_TCC7022), 1)
    MCU_BSP_BUILD_TARGET_TCC7022       ?= 1
else
    MCU_BSP_BUILD_TARGET_TCC7025_35_45 ?= 1
endif

# Determines whether the ETHERNET driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ETH.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ETH), 1)
    ifeq ($(MCU_BSP_BUILD_TARGET_TCC7025_35_45), 1)
        MCU_BSP_BUILD_TARGET_SUPPORT_DRIVER_ETHERNET ?= 1
    else
        $(warning The ETHERNET driver is not supported. Check the value of the MCU_BSP_BUILD_FLAGS_TARGET_CHIPSET_TCC7025_35_45.)
    endif
endif

COMMON_FLAGS += -O0
COMMON_FLAGS += -finline
COMMON_FLAGS += -fno-builtin
COMMON_FLAGS += -fno-common
COMMON_FLAGS += -fno-gcse
COMMON_FLAGS += -fno-strict-aliasing
COMMON_FLAGS += -marm
COMMON_FLAGS += -mcpu=cortex-r5
COMMON_FLAGS += -mno-unaligned-access
COMMON_FLAGS += -nostdinc
COMMON_FLAGS += -g
COMMON_FLAGS += -gdwarf-2
COMMON_FLAGS += -pipe
COMMON_FLAGS += -D__GNU_C__

ifeq ($(MCU_BSP_OS_CPU_VFP), d16)
    COMMON_FLAGS += -mfpu=vfpv3-d16
    COMMON_FLAGS += -mfloat-abi=softfp
    COMMON_FLAGS += -DFPU_USE
    COMMON_FLAGS += -DFPU_D16
else ifeq ($(MCU_BSP_OS_CPU_VFP), none)
    COMMON_FLAGS += -mfloat-abi=soft
endif

ifeq ($(MCU_BSP_TARGET_OS), freertos)
    COMMON_FLAGS += -DOS_FREERTOS
else ifeq ($(MCU_BSP_TARGET_OS), ucos)
    COMMON_FLAGS += -DOS_UCOS
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PRELOAD), 1)
    COMMON_FLAGS += -DPRELOAD_ONRAM
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_PFLASH_BOOT), 1)
    COMMON_FLAGS += -DPFLASH_STANDALONE
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_SECURE_UPDATE), 1)
    COMMON_FLAGS += -DSECURE_UPDATE
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_DEBUG), 1)
    COMMON_FLAGS += -DDEBUG_ENABLE=1
else
    COMMON_FLAGS += -DDEBUG_ENABLE=0
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_VCP_MODULE_BOARD), 1)
    COMMON_FLAGS += -DVCP_MODULE_BOARD=1
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_REALTEK), 1)
    COMMON_FLAGS += -DRTL_8211E
endif

COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_CLOCK=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GIC=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GPIO=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_MPU=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_PMIO=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_TIMER=1

# Determines whether the ADC driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ADC.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ADC), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_ADC=1
endif

# Determines whether the CAN driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_CAN.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_CAN), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_CAN=1
endif

# Determines whether the DSE driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_DSE.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_DSE), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_DSE=1
endif

# Determines whether the EFLASH driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_EFLASH.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_EFLASH), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_EFLASH=1
endif

# Determines whether the ETHERNET driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ETH.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ETH), 1)
    ifeq ($(MCU_BSP_BUILD_TARGET_TCC7025_35_45), 1)
        COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_ETH=1
    endif
endif

# Determines whether the FMU driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_FMU.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_FMU), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_FMU=1
endif

# Determines whether the GDMA driver included or not based on the value of MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_GDMA.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_GDMA), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GDMA=1
endif

# Determines wheterh the GPSB driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_GPSB.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_GPSB), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GPSB=1
endif

# Determies whether the HSM driver included or not based on the value of the MCU_BSP_BUILD_TARGET_SUPPORT_DRIVER_UART.
ifeq ($(MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_HSM), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_HSM=1
endif

# Determines wheterh the I2C driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_I2C.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_I2C), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_I2C=1
endif

# Determines wheterh the I2S driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_I2S.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_I2S), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_I2S=1
endif

# Determines wheterh the ICTC driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ICTC.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_ICTC), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_ICTC=1
endif

# Determies whether the LIN driver included or not based on the values of the MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_LIN.
ifeq ($(MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_LIN), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_LIN=1
endif

# Determines wheterh the MAILBOX driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_MBOX.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_MBOX), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_MBOX=1
endif

# Determines wheterh the MIDF driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_MIDF.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_MIDF), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_MIDF=1
endif

# Determines wheterh the OnRAM driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PRELOAD.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PRELOAD), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_PRELOAD=1
endif

# Determines wheterh the PDM driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PDM.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PDM), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_PDM=1
endif

# Determines wheterh the PMU driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PMIO.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PMIO), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_PMIO=1
endif
# Determines wheterh the PMU driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PMU.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PMU), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_PMU=1
endif

# Determines wheterh the RTC driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_RTC.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_RTC), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_RTC=1
endif

# Determines wheterh the SFMC driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SFMC.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SFMC), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_SFMC=1
endif

# Determines wheterh the SPU driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SPU.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SPU), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_SPU=1
endif

# Determines wheterh the SSM driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SSM.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_SSM), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_SSM=1
endif

# Determies whether the SWL driver included or not based on the value of the MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_SWL.
ifeq ($(MCU_BSP_BUILD_FLAGS_APPLICATION_DRIVER_SWL), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_SWL=1
endif

# Determines whether the UART driver included or not based on the value of MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_UART.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_UART), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_UART=1
endif

# Determines whether the WATDOG driver included or not based on the values of the MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_WATCHDOG.
ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_WATCHDOG), 1)
    COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_WATCHDOG=1
endif

# Determines whether STR.
ifeq ($(ACFG_APP_POWER_COMMUNICATION_EN), 1)
    COMMON_FLAGS += -DACFG_APP_POWER_COMMUNICATION_EN=1
endif

# Determines whether STR.
ifeq ($(ACFG_APP_POWER_EXT_AP_CTL_EN), 1)
    COMMON_FLAGS += -DACFG_APP_POWER_EXT_AP_CTL_EN=1
endif

COMMON_WARNINGS += -W
COMMON_WARNINGS += -Wall
COMMON_WARNINGS += -Wempty-body

ifneq ($(MCU_BSP_BUILD_FLAGS_APP_IPERF), 1)
    COMMON_WARNINGS += -Werror
endif

COMMON_WARNINGS += -Wextra
COMMON_WARNINGS += -Wformat
COMMON_WARNINGS += -Wmissing-braces
COMMON_WARNINGS += -Wmissing-field-initializers
COMMON_WARNINGS += -Wmissing-prototypes
COMMON_WARNINGS += -Wno-cast-align
COMMON_WARNINGS += -Wno-trigraphs
COMMON_WARNINGS += -Wno-unused-function
COMMON_WARNINGS += -Wno-unused-parameter
COMMON_WARNINGS += -Wno-unused-result
COMMON_WARNINGS += -Wparentheses
COMMON_WARNINGS += -Wpointer-sign
COMMON_WARNINGS += -Wshadow
COMMON_WARNINGS += -Wsign-compare
COMMON_WARNINGS += -Wstrict-prototypes
COMMON_WARNINGS += -Wswitch
#COMMON_WARNINGS += -Wundef
COMMON_WARNINGS += -Wuninitialized
COMMON_WARNINGS += -Wunknown-pragmas
COMMON_WARNINGS += -Wunused-label
COMMON_WARNINGS += -Wunused-value
COMMON_WARNINGS += -Wunused-variable
CWARNS          += $(COMMON_WARNINGS)
CFLAGS          += $(INCLUDES) $(COMMON_FLAGS) $(CWARNS)
LDFLAGS         += --cref
LDFLAGS         += -Bstatic
LDFLAGS         += -nostdlib
LDFLAGS         += -nostartfiles
LDFLAGS         += -p
LDFLAGS         += -EL
LDFLAGS         += -Map $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME).map

ifeq ($(MCU_BSP_BUILD_FLAGS_PFLASH_BOOT), 1)
    ifeq ($(MCU_BSP_BUILD_TARGET_TCC7025_35_45), 1)
        ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PRELOAD), 1)
            LDFLAGS += -T linker_512_withPreload.ld
        else
            LDFLAGS += -T linker_512.ld
        endif
    else ifeq ($(MCU_BSP_BUILD_TARGET_TCC7022), 1)
        ifeq ($(MCU_BSP_BUILD_FLAGS_DEVICE_DRIVER_PRELOAD), 1)
            LDFLAGS += -T linker_256_withPreload.ld
        else
            LDFLAGS += -T linker_256.ld
        endif
    else
        $(error Build flags error. Need to check the Makefile.)
    endif
else
    LDFLAGS += -T linker_onRAM.ld
endif

OBJCOPYFLAGS = -O binary -R .note -R .comment -S
OBJS         = $(patsubst %.c,$(MCU_BSP_BUILD_OBJS_PATH)/%.o,$(SRCS)) $(patsubst %.S,$(MCU_BSP_BUILD_OBJS_PATH)/%.o,$(ASMSRCS))

# compile modules include
include $(MCU_BSP_SOURCES_PATH)/rules.mk

.PHONY : all
all: $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME).bin

$(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME).bin : $(OBJS)
	$(MCU_BSP_BUILD_QUIET)mkdir -p $(MCU_BSP_BUILD_ROOT_PATH)
	$(MCU_BSP_BUILD_QUIET)mkdir -p $(MCU_BSP_BUILD_IMG_OUT_PATH)
	$(MCU_BSP_BUILD_QUIET)$(LD) $(LDFLAGS) -o $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME) $(OBJS) $(LIBGCC)
	$(MCU_BSP_BUILD_QUIET)$(OBJCOPY) $(OBJCOPYFLAGS) $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME) $@
	@echo --------------Make TC Image Format-----------------------------
	$(MCU_BSP_BUILD_QUIET)chmod 755 mkimg.sh
	./mkimg.sh TOOL_PATH=$(MCU_BSP_TOOLS_PATH) INPUT_PATH=$(MCU_BSP_BUILD_ROOT_PATH) OUTPUT_PATH=$(MCU_BSP_BUILD_IMG_OUT_PATH) TARGET_ADDRESS=0x00000000 IMAGE_VERSION=0.0.0 ENV_HOST=$(MCU_BSP_BUILD_FLAGS_ENV_HOST_WINDOW)
	@echo ---------------------------------------------------------------
	$(MCU_BSP_BUILD_QUIET)$(SIZE) $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME)
	$(MCU_BSP_BUILD_QUIET)chmod 755 mkrom.sh
	./mkrom.sh BOARD_NAME=$(MCU_BSP_CONFIG_BOARD_NAME) OUTPUT_PATH=$(MCU_BSP_BUILD_IMG_OUT_PATH) ENV_HOST=$(MCU_BSP_BUILD_FLAGS_ENV_HOST_WINDOW) SECURE_UPDATE=$(MCU_BSP_BUILD_FLAGS_SECURE_UPDATE)
	@echo ---------------------------------------------------------------
	@echo  Finished ...

.PHONY : clean
clean:
	$(MCU_BSP_BUILD_QUIET)rm -rf $(MCU_BSP_BUILD_ROOT_PATH) $@
	$(MCU_BSP_BUILD_QUIET)rm -rf $(MCU_BSP_BUILD_IMG_OUT_PATH) $@

.PHONY : memsize
memsize:
	@echo --------------Make Section Memory Size Info Excel--------------
	$(MCU_BSP_BUILD_QUIET)chmod 755 mkmemsize.sh
	./mkmemsize.sh TOOL_PATH=$(MCU_BSP_TOOLS_PATH) OUTPUT_PATH=$(MCU_BSP_BUILD_IMG_OUT_PATH)
	@echo ---------------------------------------------------------------
	@echo  Finished ...


$(OBJS): | $(MCU_BSP_BUILD_OBJS_PATH)

$(MCU_BSP_BUILD_OBJS_PATH):
	$(MCU_BSP_BUILD_QUIET)mkdir -p $@

# General rules
$(MCU_BSP_BUILD_OBJS_PATH)/%.o : %.S
	@echo compile: $< | tee -a $(COMPILE_SCRIPT_SAVE_PATH)
	$(MCU_BSP_BUILD_QUIET)$(CC) -c $(CFLAGS) -o $@ $<

$(MCU_BSP_BUILD_OBJS_PATH)/%.o : %.c
	@echo compile: $< | tee -a $(COMPILE_SCRIPT_SAVE_PATH)
	$(MCU_BSP_BUILD_QUIET)$(CC) -c $(CFLAGS) -o $@ $<


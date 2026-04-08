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

# Find the local dir of the make file
MCU_BSP_BUILD_CURDIR        ?= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
MCU_BSP_BUID_DATE           ?= $(shell date +%Y%m%d)

# Paths for Build
#MCU_BSP_TOP_PATH                        ?= $(CURDIR)/../../../..
MCU_BSP_TOP_PATH                        ?= $(shell cd ../../../../;pwd)
#MCU_BSP_DOCUMENTS_PATH                  ?= $(MCU_BSP_TOP_PATH)/documents
MCU_BSP_SCRIPTS_PATH                    ?= $(MCU_BSP_TOP_PATH)/scripts
MCU_BSP_SOURCES_PATH                    ?= $(MCU_BSP_TOP_PATH)/sources
MCU_BSP_SOURCES_BUILD_PATH              ?= $(MCU_BSP_SOURCES_PATH)/build
MCU_BSP_SOURCES_BUILD_TCC70xx_PATH      ?= $(MCU_BSP_SOURCES_BUILD_PATH)/tcc70xx
MCU_BSP_SOURCES_BUILD_TCC70xx_GCC_PATH  ?= $(MCU_BSP_SOURCES_BUILD_TCC70xx_PATH)/gcc
MCU_BSP_TOOLS_PATH                      ?= $(MCU_BSP_TOP_PATH)/../../tools

# Paths for Release
#MCU_BSP_RELEASE_PATH                                    ?= $(MCU_BSP_TOP_PATH)/../mcu.bsp.release
MCU_BSP_RELEASE_PATH                                    ?= $(shell cd ../../../../../;pwd)/tcc70xx.mcu.bsp.release.$(MCU_BSP_BUID_DATE)
MCU_BSP_RELEASE_DOCUMENTS_PATH                          ?= $(MCU_BSP_RELEASE_PATH)/documents
MCU_BSP_RELEASE_SOURCES_PATH                            ?= $(MCU_BSP_RELEASE_PATH)/sources
MCU_BSP_RELEASE_SOURCES_APP_SAMPLE_PATH                 ?= $(MCU_BSP_RELEASE_SOURCES_PATH)/app.sample
MCU_BSP_RELEASE_SOURCES_APP_SAMPLE_TEST_APP_CAN_PATH    ?= $(MCU_BSP_RELEASE_SOURCES_APP_SAMPLE_PATH)/test.app.can
MCU_BSP_RELEASE_SOURCES_APP_SAMPLE_TEST_APP_ETH_PATH    ?= $(MCU_BSP_RELEASE_SOURCES_APP_SAMPLE_PATH)/test.app.eth
MCU_BSP_RELEASE_SOURCES_APP_DRIVERS_PATH                ?= $(MCU_BSP_RELEASE_SOURCES_PATH)/app.drivers
MCU_BSP_RELEASE_SOURCES_BUILD_PATH                      ?= $(MCU_BSP_RELEASE_SOURCES_PATH)/build
MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_PATH              ?= $(MCU_BSP_RELEASE_SOURCES_BUILD_PATH)/tcc70xx
MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH          ?= $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_PATH)/gcc
MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GHC_PATH          ?= $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_PATH)/ghc
MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_MAKE_UTIL_PATH		?= $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_PATH)/make_utility
MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_PRE_BUILT					?= ../../../../../../build/tcc70xx/make_utility/prebuilt
MCU_BSP_RELEASE_SOURCES_DEV_DRIVERS_PATH                ?= $(MCU_BSP_RELEASE_SOURCES_PATH)/dev.drivers
MCU_BSP_RELEASE_SOURCES_DEV_DRIVERS_ETH_PATH						?= $(MCU_BSP_RELEASE_SOURCES_DEV_DRIVERS_PATH)/eth
MCU_BSP_RELEASE_SOURCES_OS_PATH                         ?= $(MCU_BSP_RELEASE_SOURCES_PATH)/os
MCU_BSP_RELEASE_SOURCES_OS_UCOS_PATH                    ?= $(MCU_BSP_RELEASE_SOURCES_OS_PATH)/ucos

# Build Flags

#TCC7022 has 256KB SRAM
#TCC7025,35,45 have 512KB SRAM
MCU_BSP_BUILD_FLAGS_RAM_256                   ?= 1
MCU_BSP_BUILD_FLAGS_RAM_512                   ?= 0

MCU_BSP_BUILD_FLAGS_VERBOSE                   ?= 0
MCU_BSP_BUILD_FLAGS_DEBUG                     ?= 1

MCU_BSP_BUILD_FLAGS_TARGET_OS_FREERTOS        ?= 1
MCU_BSP_BUILD_FLAGS_TARGET_OS_UCOS            ?= 0

MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_NONE           ?= 1
MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_D16            ?= 0
MCU_BSP_BUILD_FLAGS_PFLASH_BOOT               ?= 1

MCU_BSP_BUILD_FLAGS_VCP_MODULE_BOARD          ?= 1

MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE             ?= 0
MCU_BSP_BUILD_FLAGS_APP_FW_UPDATE_ECCP        ?= 1

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

MCU_BSP_CHIPSET_NAME ?= tcc70xx

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

MCU_BSP_ARM_CORE_NAME       ?= ARM_CR5
MCU_BSP_CHIPSET_FAMILY_NAME ?= tcc70xx
MCU_BSP_PROJECT_NAME        ?= boot

MCU_BSP_BUILD_ROOT_PATH     ?= $(MCU_BSP_SOURCES_BUILD_TCC70xx_GCC_PATH)/$(MCU_BSP_CHIPSET_NAME)-$(MCU_BSP_TARGET_OS)-$(MCU_BSP_BUILD_CONFIG)
MCU_BSP_BUILD_OBJS_PATH     ?= $(MCU_BSP_BUILD_ROOT_PATH)/obj
MCU_BSP_BUILD_IMG_OUT_PATH  ?= $(MCU_BSP_SOURCES_BUILD_TCC70xx_GCC_PATH)/output

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
#COMMON_FLAGS += -DDSE_DATAABORT
#COMMON_FLAGS += -DSUBSYS_USERMODETEST
#COMMON_FLAGS += -DGIC_FIQTEST
#COMMON_FLAGS += -DDMA_NC_SIZE=0x4000
#COMMON_FLAGS += -DPMIO_DRIVER

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

ifeq ($(MCU_BSP_BUILD_FLAGS_PFLASH_BOOT), 1)
    COMMON_FLAGS += -DPFLASH_STANDALONE
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_DEBUG), 1)
    COMMON_FLAGS += -DDEBUG_ENABLE=1
else
    COMMON_FLAGS += -DDEBUG_ENABLE=0
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_VCP_MODULE_BOARD), 1)
    COMMON_FLAGS += -DVCP_MODULE_BOARD=1
endif

COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_CLOCK=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GIC=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GPIO=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_MPU=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GDMA=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_EFLASH=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_UART=1
COMMON_FLAGS += -DMCU_BSP_SUPPORT_DRIVER_GPSB=1

COMMON_WARNINGS += -W
COMMON_WARNINGS += -Wall
COMMON_WARNINGS += -Wempty-body
COMMON_WARNINGS += -Werror
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
    LDFLAGS += -T linker_256.ld
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
	./mkimg.sh TOOL_PATH=$(MCU_BSP_TOOLS_PATH) INPUT_PATH=$(MCU_BSP_BUILD_ROOT_PATH) OUTPUT_PATH=$(MCU_BSP_BUILD_IMG_OUT_PATH) TARGET_ADDRESS=0x00000000 IMAGE_VERSION=0.0.0
	@echo ---------------------------------------------------------------
	@echo  copy updater
	cp $(MCU_BSP_BUILD_IMG_OUT_PATH)/updater.rom $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_PRE_BUILT)/.
	@echo ---------------------------------------------------------------
	@echo  Finished ...

.PHONY : release
release :
	@echo "=== Telechips MCU BSP Release ==="
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_PATH)
	$(MCU_BSP_BUILD_QUIET)mkdir -p $(MCU_BSP_RELEASE_PATH)
	$(MCU_BSP_BUILD_QUIET)cp -a    $(MCU_BSP_DOCUMENTS_PATH)    $(MCU_BSP_RELEASE_PATH)
	$(MCU_BSP_BUILD_QUIET)cp -a    $(MCU_BSP_SCRIPTS_PATH)      $(MCU_BSP_RELEASE_PATH)
	$(MCU_BSP_BUILD_QUIET)cp -a    $(MCU_BSP_SOURCES_PATH)      $(MCU_BSP_RELEASE_PATH)
	$(MCU_BSP_BUILD_QUIET)cp -a    $(MCU_BSP_TOOLS_PATH)        $(MCU_BSP_RELEASE_PATH)
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_DOCUMENTS_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)/*.docx
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_APP_SAMPLE_TEST_APP_CAN_PATH)
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_APP_SAMPLE_TEST_APP_ETH_PATH)
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_DEV_DRIVERS_ETH_PATH)
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH)/Makefile
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH)/mkrom.es.sh
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH)/output
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH)/tcc70**
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH)/fwdn_vcp.bat
	$(MCU_BSP_BUILD_QUIET)mv       $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH)/Makefile.release $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GCC_PATH)/Makefile
	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_OS_UCOS_PATH)/*/*

	$(MCU_BSP_BUILD_QUIET)rm -rf   $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GHC_PATH)/mkrom_es.bat
	$(MCU_BSP_BUILD_QUIET)mv       $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GHC_PATH)/mkrom_es_release.bat $(MCU_BSP_RELEASE_SOURCES_BUILD_TCC70xx_GHC_PATH)/mkrom_es.bat
	@echo "=== RELEASE COMPLETE : $(MCU_BSP_RELEASE_PATH)"

.PHONY : clean
clean:
	$(MCU_BSP_BUILD_QUIET)rm -rf $(MCU_BSP_BUILD_ROOT_PATH) $@
	$(MCU_BSP_BUILD_QUIET)rm -rf $(MCU_BSP_BUILD_IMG_OUT_PATH) $@

$(OBJS): | $(MCU_BSP_BUILD_OBJS_PATH)

$(MCU_BSP_BUILD_OBJS_PATH):
	$(MCU_BSP_BUILD_QUIET)mkdir -p $@

# General rules
$(MCU_BSP_BUILD_OBJS_PATH)/%.o : %.S
	@echo compile: $<
	$(MCU_BSP_BUILD_QUIET)$(CC) -c $(CFLAGS) -o $@ $<

$(MCU_BSP_BUILD_OBJS_PATH)/%.o : %.c
	@echo compile: $<
	$(MCU_BSP_BUILD_QUIET)$(CC) -c $(CFLAGS) -o $@ $<


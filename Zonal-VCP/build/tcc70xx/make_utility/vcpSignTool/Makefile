HOST_OS=$(shell uname -o)

MAJOR_VERSION := 0
MINOR_VERSION := 0.1

TARGET :=
GFLAGS :=
CFLAGS :=

#SUPPORT_MBEDTLS := 1

TARGET := vcpSignTool

ifeq ($(HOST_OS), Cygwin)
GFLAGS += -DWIN32
TARGET := $(TARGET).exe
endif

BUILDDIR := build-$(TARGET)

CMDHEADER := $(BUILDDIR)/cmds.h

INCS :=
SRCS :=
LIBS :=
DEFS :=

DEFS += -DTOOL_NAME=\"$(TARGET)\"
DEFS += -DMAJOR_VERSION=\"$(MAJOR_VERSION)\" -DMINOR_VERSION=\"$(MINOR_VERSION)\"

GFLAGS += -include $(CMDHEADER)
CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter
CFLAGS += -Wno-unknown-pragmas -Wno-pointer-to-int-cast -Wno-pointer-sign
CFLAGS += -static #-m32

include make/macro.mk
include lib/rules.mk
include src/rules.mk
include make/compile.mk

all : $(TARGET)

$(TARGET) : $(CMDHEADER) $(OBJS) $(LIBS)
	@gcc $(CFLAGS) $(GFLAGS) -o $(BUILDDIR)/$@ $(OBJS) $(LIBS) #-lpthread
	@strip -s $(BUILDDIR)/$@

# make all object files depend on CMDHEADER
$(OBJS): $(CMDHEADER)

$(CMDHEADER): cmdheader
	@$(call MAKE_CMD_HEADER,$@,CMDS)

clean :
	@rm -rf $(BUILDDIR)

.PHONY: cmdheader


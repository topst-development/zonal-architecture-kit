
####################################################################################
#  makefile
####################################################################################

ROOT_PATH =.

#os := windows
os := linux

ifeq ($(os), windows)
CROSS_COMPILE=
#CROSS_COMPILE=arm-none-linux-gnueabi-

AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
CC = $(CROSS_COMPILE)gcc
CPP = $(CC) -E
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
NM = $(CROSS_COMPILE)nm
STRIP = $(CROSS_COMPILE)strip
OBJDUMP = $(CROSS_COMPILE)objdump
else
COMPILE_OPTION += -g
endif

ifndef $(KDIR)
KDIR := ../../kernel
endif

REL_PATH = $(ROOT_PATH)
SRC_PATH =.

TARGET=$(REL_PATH)/tcc70xx-pflash-mkimage

COMPILE_OPTION	+= -c -O2  -Wall -Werror -fomit-frame-pointer -pipe
COMPILE_OPTION	+= -Wno-unused-result # TODO:
DEFINE		+= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__
INC_PATH	+=  -I$(REL_PATH)/inc -I$(ROOT_PATH)/include

C_OBJ		= 	$(SRC_PATH)/main.o \
				$(SRC_PATH)/mkimage.o \
				$(SRC_PATH)/ecc.o

C_SRCS		=  $(C_OBJ:.o=.c)

CFLAGS		+= $(COMPILE_OPTION) $(INC_PATH) $(DEFINE)
CPPFLAGS	+= $(COMPILE_OPTION) $(INC_PATH) $(DEFINE)
LDFLAGS		= -s

POST_EXE	=

J_LD		=  $(CC)
LINK_LIB        += -lpthread

all: $(TARGET)
	$(POST_EXE)
	@echo ""
	@echo "   ########## [$^] ##########"
	@echo ""

clean:
	$(RM) $(C_OBJ) $(CPP_OBJ) $(TARGET)

$(TARGET) : $(C_OBJ) $(CPP_OBJ)
	$(J_LD) $(LDFLAGS) -o $@ $^ $(LINK_LIB)

.c.o: $(C_SRCS)
	$(CC) $(CFLAGS) -o $@ $^


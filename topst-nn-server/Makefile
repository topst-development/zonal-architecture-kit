TARGET = topst-nn-server
BUILD_DIR := build
OBJDIR := $(BUILD_DIR)/obj
DEPDIR := $(BUILD_DIR)/dep

CROSS_COMPILE = aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)gcc

OPENCV_ROOT ?= ./third_party/opencv

INCS := -I./inc -I./npu -I$(OPENCV_ROOT)/include/opencv4

CFLAGS := -Wall -std=c11 -mcpu=cortex-a53 -pthread -O2 -g -pg -MMD -MP
CXXFLAGS := -Wall -std=c++11 -mcpu=cortex-a53 -pthread -O2 -g -pg -MMD -MP

LIBS := -L$(OPENCV_ROOT)/lib -lpthread -ldl -lm -lstdc++
LIBS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
LIBS += -lz -ltbb -ljpeg -lwebp -lpng16 -ltiff -llzma

SRCS_C := \
	npu/npu_api.c \
	npu/npu_api_ex.c \
	npu/npu_api_mem.c \
	src/app_config.c \
	src/app_control.c \
	src/app_inference.c \
	src/app_input.c \
	src/app_json.c \
	src/app_memory.c \
	src/app_monitor.c \
	src/app_pipeline.c \
	src/app_render.c \
	src/app_runtime.c \
	src/app_tracker.c \
	src/app_types.c \
	src/main.c \
	src/platform_api.c

SRCS_CPP := src/opencv_api.cpp

OBJS := $(addprefix $(OBJDIR)/,$(SRCS_C:.c=.o) $(SRCS_CPP:.cpp=.o))
DEPS := $(addprefix $(DEPDIR)/,$(SRCS_C:.c=.d) $(SRCS_CPP:.cpp=.d))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/$*.d)
	$(CC) $(CFLAGS) $(INCS) -MF $(DEPDIR)/$*.d -c $< -o $@

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/$*.d)
	$(CXX) -x c++ $(CXXFLAGS) $(INCS) -MF $(DEPDIR)/$*.d -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)

LOCAL_DIR := $(GET_LOCAL_DIR)

INCS += \
	$(LOCAL_DIR)/impl/include \

SRCS += \
	$(LOCAL_DIR)/impl/tcc_util.c \

SRCS += \
	$(LOCAL_DIR)/main.c \

include $(LOCAL_DIR)/cmd/rules.mk


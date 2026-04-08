LOCAL_DIR := $(GET_LOCAL_DIR)

INCS += \
	$(LOCAL_DIR)/include \

ifeq ($(SUPPORT_MBEDTLS),)

ifeq ($(HOST_OS), Cygwin)
LIBS += \
	$(LOCAL_DIR)/tcsblib.lib
else
LIBS += \
	$(LOCAL_DIR)/tcsblib.a
endif

else

include $(LOCAL_DIR)/src/rules.mk

endif
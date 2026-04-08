LOCAL_DIR := $(GET_LOCAL_DIR)

INCS += \
	$(LOCAL_DIR)/include \

ifeq ($(HOST_OS), Cygwin)
else
LIBS += \
	$(LOCAL_DIR)/libmbedx509.a \
	$(LOCAL_DIR)/libmbedcrypto.a \
	$(LOCAL_DIR)/libmbedtls.a
endif

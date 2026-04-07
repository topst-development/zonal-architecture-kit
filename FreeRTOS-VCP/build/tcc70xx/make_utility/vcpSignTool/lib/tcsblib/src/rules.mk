LOCAL_DIR := $(GET_LOCAL_DIR)

DEFS += \
	-DSUPPORT_MBEDTLS \

SRCS += \
	$(LOCAL_DIR)/tcc_mbed.c \
	$(LOCAL_DIR)/tcc_sw_cipher.c \
	$(LOCAL_DIR)/tcccipher.c \
	$(LOCAL_DIR)/tcc_auth.c \
	$(LOCAL_DIR)/tcc_cryp.c \
	$(LOCAL_DIR)/tcc_key.c \
	$(LOCAL_DIR)/tcc_secure.c \

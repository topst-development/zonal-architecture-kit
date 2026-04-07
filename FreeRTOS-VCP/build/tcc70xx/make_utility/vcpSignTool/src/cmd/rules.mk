LOCAL_DIR := $(GET_LOCAL_DIR)

CMD_SRCS := $(shell find $(LOCAL_DIR) -name "cmd_*.c" -type f)

ifneq ($(TESTIMAGE),1)
CMD_SRCS := $(filter-out $(LOCAL_DIR)/cmd_decmfmcert.c,$(CMD_SRCS))
endif

CMDS := $(patsubst cmd_%,%,$(patsubst %.c,%,$(notdir $(CMD_SRCS))))
SRCS += $(CMD_SRCS)


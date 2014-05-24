BOARD_DIR = boards/x86

BOARD_SRCS = \
	$(BOARD_DIR)/main.c
	
CFLAGS += 

DEPS += $(call src_to_dep,$(BOARD_SRCS))
OBJS += $(call src_to_obj,$(BOARD_SRCS))

$(call build, $(BOARD_SRCS), $(BOARD_CFLAGS))

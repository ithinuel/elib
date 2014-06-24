UNITY_DIR = third_party/unity

UNITY_SRCS = \
	$(UNITY_DIR)/src/unity.c \
	$(UNITY_DIR)/extras/fixture/src/unity_fixture.c
	

CFLAGS += \
	-I $(UNITY_DIR)/src \
	-I $(UNITY_DIR)/extras/fixture/src

DEPS	+= $(call src_to_dep,$(UNITY_SRCS))
OBJS	+= $(call src_to_obj,$(UNITY_SRCS))

$(call build, $(UNITY_SRCS), )

#	Copyright 2014 Chauveau Wilfried
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		 http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.

BOARD_DIR = boards/unity

BOARD_SRCS = \
	$(BOARD_DIR)/cexcept.c \
	$(BOARD_DIR)/main.c
	
CFLAGS += -I $(BOARD_DIR)/configs -fprofile-arcs -ftest-coverage

include third_party/unity.mk

DEPS += $(call src_to_dep,$(BOARD_SRCS))
OBJS += $(call src_to_obj,$(BOARD_SRCS))

$(call build, $(BOARD_SRCS), $(BOARD_CFLAGS))

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

OS_DIR = os/Unix

OS_SRCS = \
	$(OS_DIR)/task.c \
	$(OS_DIR)/mutex.c \
	$(OS_DIR)/test_mutex.c \
	$(OS_DIR)/system.c
	
OS_CFLAGS +=
LDFLAGS += -lpthread

DEPS += $(call src_to_dep,$(OS_SRCS))
OBJS += $(call src_to_obj,$(OS_SRCS))

$(call build, $(OS_SRCS), $(OS_CFLAGS))

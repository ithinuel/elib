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

CORE_DIR = core

CORE_SRCS = \
	$(CORE_DIR)/common/common.c \
	$(CORE_DIR)/memmgr/memmgr.c \
	$(CORE_DIR)/memmgr/test_memmgr.c \
	$(CORE_DIR)/common/cexcept.c \
	$(CORE_DIR)/common/test_cexcept.c
	
CORE_CFLAGS +=

DEPS += $(call src_to_dep,$(CORE_SRCS))
OBJS += $(call src_to_obj,$(CORE_SRCS))

$(call build, $(CORE_SRCS), $(CORE_CFLAGS))

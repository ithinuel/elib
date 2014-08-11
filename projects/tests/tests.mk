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

PRJ_NAME = unit\ tests

OUT_DIR	= build_tests

MSG_BEGIN	= "-------- tests --------"

BOARD	= unity
OS	= Unix

PRJ_SRCS = projects/tests/mcp/mcp.c

CFLAGS += -I projects/tests/

DEPS += $(call src_to_dep,$(PRJ_SRCS))
OBJS += $(call src_to_obj,$(PRJ_SRCS))

$(call build, $(PRJ_SRCS), $(PRJ_CFLAGS))
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

###################################
## project variables

OUT_DIR		= build_target
DEPS_DIR	= $(OUT_DIR)/deps
OBJS_DIR	= $(OUT_DIR)/objs

MSG_BEGIN	= "-------- target -------"
MSG_END		= "-----------------------"

DEBUG=gdb
DEBUG_LVL=3
OPT = 0
CSTANDARD = gnu99

CFLAGS += \
	-Wall \
	-g$(DEBUG)$(DEBUG_LVL) \
	-O$(OPT) \
	-std=$(CSTANDARD) \
	-I API

###################################
## common makefile functions
src_to_base	= $(basename $(subst /,_,$1))
src_filter_c	= $(filter %.c, $1)
src_filter_asm	= $(filter %.s, $1) $(filter %.S, $1)
src_to_dep	= $(addprefix $(DEPS_DIR)/, $(addsuffix .d, $(call src_to_base,$(call src_filter_c,$1))))
src_to_obj	= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(call src_to_base,$1)))
build		= $(foreach src, $(call src_filter_c,$1), $(eval $(call DEPS_TEMPLATE,$(src), $2))) \
		  $(foreach src, $(call src_filter_c,$1), $(eval $(call COMPILE_TEMPLATE,$(src), $2))) \
		  $(foreach src, $(call src_filter_asm,$1), $(eval $(call ASSEMBLE_TEMPLATE,$(src), $2)))

###################################
## meta rules
define DEPS_TEMPLATE
$(call src_to_dep,$(1)): $(1) Makefile
	@$$(CC) -MT $$(OBJS_DIR)/$(basename $(subst /,_,$(1))).o -MM $$(CFLAGS) $(2) $$< -o $$@
endef

define COMPILE_TEMPLATE
$(call src_to_obj,$(1)): $(1)
	@echo  "Compiling : $$(notdir $$@)"
	@$$(CC) -c $$(CFLAGS) $(2) $$< -o $$@
endef

define ASSEMBLE_TEMPLATE
$(call src_to_obj,$(1)): $(1)
	@echo  "Compiling : $$(notdir $$@)"
	@$$(CC) -c $$(CFLAGS) $(2) $$< -o $$@
endef

###################################
## include project make file
ifneq ($(wildcard $(PROJECT)),)
include $(PROJECT)
else
$(error PROJECT should be defined !)
endif

##################################
## sanity check
ifeq ($(PRJ_NAME),)
$(error PRJ_NAME must be defined)
endif

ifeq ($(wildcard boards/$(BOARD)/board.mk),)
$(error BOARD must be defined)
endif

ifeq ($(wildcard os/$(OS)/os.mk),)
$(error OS must be defined)
endif

include boards/$(BOARD)/board.mk
include os/$(OS)/os.mk
include core/core.mk

###################################
## prepare build
ifneq ($(MAKECMDGOALS),clean)
$(shell mkdir $(OUT_DIR) 2> /dev/null)
$(shell mkdir $(DEPS_DIR) 2> /dev/null)
$(shell mkdir $(OBJS_DIR) 2> /dev/null)

-include $(DEPS)
endif

###################################
## build rules
all: elf
elf: begin $(OUT_DIR)/$(PRJ_NAME).elf end 

begin:
	@echo $(MSG_BEGIN)

$(OUT_DIR)/$(PRJ_NAME).elf: $(OBJS) $(LDSCRIPT)
	@echo "Linking ..."
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o$(OUT_DIR)/$(PRJ_NAME).elf
end:
	@echo $(MSG_END)
	
tests: elf
	-@./$(OUT_DIR)/$(PRJ_NAME).elf
	
coverage: tests
	@mkdir -p $(OUT_DIR)/gcov
	@gcovr --gcov-exclude=third_party* --gcov-exclude=boards#unity* -k --html --html-details -o $(OUT_DIR)/gcov/gcovr.html
	@gcovr --gcov-exclude=third_party* --gcov-exclude=boards#unity* -db > $(OUT_DIR)/gcov/gcovr-branches.txt
	
	
clean:
	@echo "Making $(OUT_DIR) clean..."
	-@rm -fr $(OUT_DIR) 2> /dev/null

.PHONY: all elf begin end tests clean

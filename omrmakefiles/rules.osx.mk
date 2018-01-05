###############################################################################
# Copyright (c) 2016, 2016 IBM Corp. and others
# 
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at https://www.eclipse.org/legal/epl-2.0/
# or the Apache License, Version 2.0 which accompanies this distribution and
# is available at https://www.apache.org/licenses/LICENSE-2.0.
#      
# This Source Code may also be made available under the following
# Secondary Licenses when the conditions for such availability set
# forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
# General Public License, version 2 with the GNU Classpath
# Exception [1] and GNU General Public License, version 2 with the
# OpenJDK Assembly Exception [2].
#    
# [1] https://www.gnu.org/software/classpath/license.html
# [2] http://openjdk.java.net/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
###############################################################################

###
### Global Flags
###

GLOBAL_CPPFLAGS += -DOSX -DJ9HAMMER -D_REENTRANT -D_FILE_OFFSET_BITS=64

# Compile without exceptions

ifeq (1,$(OMR_RTTI))
    GLOBAL_CXXFLAGS+=-fno-exceptions -fno-threadsafe-statics
else
    GLOBAL_CXXFLAGS+=-fno-exceptions -fno-rtti -fno-threadsafe-statics
endif

## Position Independent compile flag
GLOBAL_CFLAGS+=-fPIC
GLOBAL_CXXFLAGS+=-fPIC
GLOBAL_LDFLAGS+=-fpic

GLOBAL_CFLAGS+= -m64
GLOBAL_CXXFLAGS+=-m64
GLOBAL_ASFLAGS+= -noexecstack -64

## Debugging Infomation
# Indicate that GNU debug symbols are being used
# TODO: this should be decided from the SPEC?
USE_GNU_DEBUG:=1

ifeq (1,$(OMR_DEBUG))
  ifeq (1,$(USE_GNU_DEBUG))
    GLOBAL_ASFLAGS+=-gddb
    GLOBAL_CXXFLAGS+=-ggdb
    GLOBAL_CFLAGS+=-ggdb
    GLOBAL_LDFLAGS+=-ggdb
  else
    GLOBAL_ASFLAGS+=-g
    GLOBAL_CXXFLAGS+=-g
    GLOBAL_CFLAGS+=-g
    GLOBAL_LDFLAGS+=-g
  endif
endif

###
### Executable
###

ifneq (,$(findstring executable,$(ARTIFACT_TYPE)))
  # Default Libraries
  GLOBAL_SHARED_LIBS+=m pthread c dl util
  GLOBAL_LDFLAGS+=-Wl,-rpath,\$$ORIGIN
endif

###
### Shared Libraries
###

ifneq (,$(findstring shared,$(ARTIFACT_TYPE)))
GLOBAL_LDFLAGS+=-shared
GLOBAL_SHARED_LIBS+=c m dl

## Export File
$(MODULE_NAME)_LINKER_EXPORT_SCRIPT := $(MODULE_NAME).exp
#GLOBAL_LDFLAGS+=-exported_symbols_list=$($(MODULE_NAME)_LINKER_EXPORT_SCRIPT)

endif # ARTIFACT_TYPE contains "shared"


###
### Warning As Errors
###

ifeq ($(OMR_WARNINGS_AS_ERRORS),1)
  GLOBAL_CFLAGS+=-Wimplicit -Wreturn-type -Werror
  GLOBAL_CXXFLAGS+=-Wreturn-type -Werror
endif


###
### Enhanced Warnings
###

ifeq ($(OMR_ENHANCED_WARNINGS),1)
  GLOBAL_CFLAGS+=-Wall
  GLOBAL_CXXFLAGS+=-Wall -Wno-non-virtual-dtor
endif

###
### Optimization Flags
###

ifeq ($(OMR_OPTIMIZE),1)
  OPTIMIZATION_FLAGS+=-O3 -fno-strict-aliasing
else
  OPTIMIZATION_FLAGS+=-O0
endif
GLOBAL_CFLAGS+=$(OPTIMIZATION_FLAGS)
GLOBAL_CXXFLAGS+=$(OPTIMIZATION_FLAGS)

# Override the default recipe if we are using USE_GNU_DEBUG, so that we strip out the
# symbols and store them seperately.
ifneq (,$(findstring shared,$(ARTIFACT_TYPE)))
ifeq (1,$(OMR_DEBUG))
ifeq (1,$(USE_GNU_DEBUG))

define LINK_C_SHARED_COMMAND
$(CCLINKSHARED) -o $@ $(OBJECTS) $(LDFLAGS) $(MODULE_LDFLAGS) $(GLOBAL_LDFLAGS) -install_name lib$(MODULE_NAME).dylib
cp $@ $@.dbg
endef

define LINK_CXX_SHARED_COMMAND
$(CXXLINKSHARED) -o $@ $(OBJECTS) $(LDFLAGS) $(MODULE_LDFLAGS) $(GLOBAL_LDFLAGS) -install_name lib$(MODULE_NAME).dylib
cp $@ $@.dbg
endef

## Files to clean
CLEAN_FILES=$(OBJECTS) *.d
CLEAN_FILES+=$($(MODULE_NAME)_shared).dbg
define CLEAN_COMMAND
-$(RM) $(CLEAN_FILES)
endef

endif # USE_GNU_DEBUG
endif # OMR_DEBUG
endif # ARTIFACT_TYPE contains "shared"

###
### Dependencies
###

# include *.d files generated by the compiler
DEPS := $(OBJECTS:$(OBJEXT)=.d)
show_deps:
	@echo "Dependencies are: $(DEPS)"

ifneq ($(DEPS),)
-include $(DEPS)
endif


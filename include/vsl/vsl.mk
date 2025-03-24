#*****************************************************************************
# Constants
#*****************************************************************************
PERL ?= perl
PYTHON3 ?= python3
VERILATOR ?= /usr/local/bin/verilator
VERILATOR_ROOT ?= /usr/local/share/verilator
SYSTEMC_INCLUDE ?= /usr/local/systemc-2.3.3/include
SYSTEMC_LIBDIR ?= /usr/local/systemc-2.3.3/lib-linux64
VSL_DIR ?= $(HOME)/projects/verisocks

# Logging levels for vs_log module
LOG_LEVEL_DEBUG = 10
LOG_LEVEL_INFO = 20
LOG_LEVEL_WARNING = 30
LOG_LEVEL_ERROR = 40
LOG_LEVEL_CRITICAL = 50
VS_LOG_LEVEL ?= $(LOG_LEVEL_DEBUG)

#*****************************************************************************
# Variables
#*****************************************************************************
# Build folders
VL_OBJ_DIR ?= vl_obj_dir
VSL_BUILD_DIR ?= vsl_build

#*****************************************************************************
# Verilation
#*****************************************************************************
VL_FLAGS = --cc --timing
VL_FLAGS += -Mdir $(VL_OBJ_DIR)
VL_FLAGS += --top $(VL_TOP)
VL_FLAGS += $(VL_USER_FLAGS)

#*****************************************************************************
# Verisocks integration
#*****************************************************************************
VS_SRCS = \
	cJSON.c \
	vs_msg.c \
	vs_server.c

VSL_SRCS = \
	vsl_utils.cpp \
	vsl_types.cpp

VSL_SRCS += $(TB_CPP_SRCS)

VSL_HEADERS = \
    $(VSL_DIR)/include/vsl.h \
    $(VSL_DIR)/include/vsl/vsl_integ.hpp \
    $(VSL_DIR)/include/vsl/vsl_integ_cmd.hpp \
    $(VSL_DIR)/include/vsl/vsl_integ_cmd_get.hpp \
    $(VSL_DIR)/include/vsl/vsl_integ_cmd_set.hpp \
    $(VSL_DIR)/include/vsl/vsl_integ_cmd_run.hpp \
    $(VSL_DIR)/include/vsl/vsl_utils.hpp \
    $(VSL_DIR)/include/vsl/vsl_types.hpp

VSL_INCDIRS = \
	$(VSL_DIR)/include \
	$(VSL_DIR)/cjson

VSL_OBJS = $(addprefix $(VSL_BUILD_DIR)/,$(subst .c,.o,$(VS_SRCS)))
VSL_OBJS += $(addprefix $(VSL_BUILD_DIR)/,$(subst .cpp,.o,$(VSL_SRCS)))
CPPFLAGS += $(addprefix -I,$(VSL_INCDIRS) $(VL_OBJ_DIR))
CPPFLAGS += -Wall
CPPFLAGS += -DVS_LOG_LEVEL=$(VS_LOG_LEVEL)
CPPFLAGS += -O3
CPPFLAGS += $(CPP_USER_FLAGS)

VPATH += $(VSL_DIR)/cjson $(VSL_DIR)/src

#*****************************************************************************
# Rules
#*****************************************************************************
default: verilate $(VM_PREFIX)

verilate: $(VL_OBJ_DIR)/$(VM_PREFIX)_classes.mk $(VL_OBJ_DIR)/$(VM_PREFIX).mk

$(VL_OBJ_DIR)/%.mk: $(VL_SRCS)
	@mkdir -p $(VL_OBJ_DIR)
	$(VERILATOR) $(VL_FLAGS) $^
VPATH += $(VL_OBJ_DIR)

$(VSL_BUILD_DIR)/%.o: %.cpp $(VSL_HEADERS)
	@mkdir -p $(VSL_BUILD_DIR)
	$(CXX) -o $@ -c $(CPPFLAGS) $<

$(VSL_BUILD_DIR)/%.o: %.c $(VSL_HEADERS)
	@mkdir -p $(VSL_BUILD_DIR)
	$(CXX) -o $@ -c $(CPPFLAGS) $<

VM_USER_CFLAGS = \
	-DVL_TIME_CONTEXT \

### Default rules...
# C++ code coverage  0/1 (from --prof-c)
VM_PROFC = 0
# SystemC output mode?  0/1 (from --sc)
VM_SC = 0
# Legacy or SystemC output mode?  0/1 (from --sc)
VM_SP_OR_SC = $(VM_SC)

include $(VL_OBJ_DIR)/$(VM_PREFIX)_classes.mk
include $(VERILATOR_ROOT)/include/verilated.mk

%.fst: %.vcd
	vcd2fst $< $@
	$(RM) $<

### Link rules
link_args = $(VSL_OBJS) $(VK_GLOBAL_OBJS) $(VM_PREFIX)__ALL.a $(VM_HIER_LIBS)
link_deps = $(link_args) $(VSL_HEADERS)
$(VM_PREFIX): $(link_deps)
	$(LINK) $(LDFLAGS) $(link_args) $(LDLIBS) $(LIBS) $(SC_LIBS) -o $@

.PHONY: clean verilate
clean:
	$(RM) -r $(VSL_BUILD_DIR)
	$(RM) -r $(VL_OBJ_DIR)
	$(RM) $(VK_OBJS) $(VK_GLOBAL_OBJS)
	$(RM) $(VM_PREFIX)__ALL.*
	$(RM) $(VM_PREFIX)
	$(RM) *.d
	$(RM) *.fst *.vcd
	$(RM) *.log
	$(RM) -r __pycache__

<%page
args = "
	prefix,
	top,
	verilog_src_files,
	cpp_src_files,
	verisocks_root,
	verilator_path = '/usr/local/bin/verilator',
	verilator_root = '/usr/local/share/verilator',
	use_tracing = False,
	use_fst = True
"
/>\
#*****************************************************************************
# Configuration
#*****************************************************************************
VERILATOR ?= ${verilator_path}
VERILATOR_ROOT ?= ${verilator_root}
VSL_DIR ?= ${verisocks_root}

# Verilator user flags
# Setup traceing - use $dump() in testbench
% if use_tracing:
CPP_USER_FLAGS = -DDUMP_FILE

% if use_fst:
# Using FST traceing (slower due to compression)
VL_USER_FLAGS = --trace-fst
VL_USER_FLAGS += -DDUMP_FILE=\"test.fst\"
USER_LDLIBS = -lz
% else:
# Using VCD traceing
VL_USER_FLAGS = --trace
VL_USER_FLAGS += -DDUMP_FILE=\"test.vcd\"
% endif

# Design prefix
VM_PREFIX = ${prefix}

# Top module
VL_TOP = ${top}

# List all Verilog/SystemVerilog source files to be verilated
VL_SRCS = \
% for src in verilog_src_files[:-1]:
	${src} \
% endfor
	${verilog_src_files[-1]}

# Testbench C++ source files
TB_CPP_SRCS = \
% for src in cpp_src_files[:-1]:
	${src} \
% endfor
	${cpp_src_files[-1]}

# Build folders
VL_OBJ_DIR = vl_obj_dir
VSL_BUILD_DIR = vsl_build

VS_LOG_LEVEL = $(LOG_LEVEL_DEBUG)

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

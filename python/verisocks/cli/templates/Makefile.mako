<%page
args = "
	target_file,
	config_file,
	tb_file,
	vlt_file,
	prefix,
	top,
	build_dir,
	verilog_src_files,
	verilator_arg_files,
	verisocks_root,
	cpp_src_files,
	verilator_path = '/usr/local/bin/verilator',
	verilator_root = '/usr/local/share/verilator',
	use_tracing = False,
	use_fst = True,
	use_timing = True,
	log_level = 'info'
"
/>\
<%
from os.path import isabs, relpath

def format_path(path):
	if isabs(path):
		return path
	return relpath(path, build_dir)

LOG_LEVELS = {
	"debug":    "$(LOG_LEVEL_DEBUG)",
	"info":     "$(LOG_LEVEL_INFO)",
	"warning":  "$(LOG_LEVEL_WARNING)",
	"error":    "$(LOG_LEVEL_ERROR)",
	"critical": "$(LOG_LEVEL_CRITICAL)"
}
%>\
# Note: This file has been generated from the template ${template_filename}
#*****************************************************************************
# Configuration
#*****************************************************************************
VERILATOR ?= ${format_path(verilator_path)}
VERILATOR_ROOT ?= ${format_path(verilator_root)}
VSL_DIR ?= ${format_path(verisocks_root)}

% if use_timing:
# Use timing option with Verilator
VL_USER_FLAGS += --timing
CPP_USER_FLAGS += -DVSL_TIMING

% endif
% if use_tracing:
# Setup traceing - use $dump() in testbench
% if use_fst:
# Using FST traceing (slower due to compression)
CPP_USER_FLAGS += -DDUMP_FILE -DDUMP_FST
VL_USER_FLAGS += --trace-fst
USER_LDLIBS = -lz
% else:
# Using VCD traceing
CPP_USER_FLAGS += -DDUMP_FILE -DDUMP_VCD
VL_USER_FLAGS += --trace-vcd
% endif

% endif
# Design prefix
VM_PREFIX = ${prefix}

# Top module
VL_TOP = ${top}

% if len(verilog_inc_dirs) > 0:
VL_INCDIRS = \\

% for argf in verilog_inc_dirs[:-1]:
	${format_path(argf)} \\

% endfor
	${format_path(verilog_inc_dirs[-1])}

% endif
% if len(verilator_arg_files) > 0:
VL_ARGS_FILES = \\

% for argf in verilator_arg_files[:-1]:
	${format_path(argf)} \\

% endfor
	${format_path(verilator_arg_files[-1])}

% endif
# List all Verilog/SystemVerilog source files to be verilated
VL_SRCS = \\

% if vlt_file:
	${vlt_file} \\
% endif

% for src in verilog_src_files[:-1]:
	${format_path(src)} \\

% endfor
	${format_path(verilog_src_files[-1])}

# Testbench C++ source files
TB_CPP_SRCS = \\

	${tb_file} \
% if cpp_src_files:
\\

% for src in cpp_src_files[:-1]:
	${format_path(src)} \\

% endfor
	${format_path(cpp_src_files[-1])}

% endif


# Build folders
VL_OBJ_DIR = vl_obj_dir
VSL_BUILD_DIR = vsl_build

VS_LOG_LEVEL = ${LOG_LEVELS[log_level]}

#*****************************************************************************
# Top rule
#*****************************************************************************
all: default

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

.PHONY: all

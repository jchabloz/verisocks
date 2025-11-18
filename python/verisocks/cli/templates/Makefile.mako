<%page
args = "
	target_file,
	config_file,
	tb_file,
	vlt_file,
	prefix,
	top,
	verilog_src_files,
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
VERILATOR ?= ${verilator_path}
VERILATOR_ROOT ?= ${verilator_root}
VSL_DIR ?= ${verisocks_root}

% if use_timing:
# Use timing option with Verilator
VL_USER_FLAGS += --timing
CPP_USER_FLAGS += -DVSL_TIMING
% endif

% if use_tracing:
# Setup traceing - use $dump() in testbench
CPP_USER_FLAGS += -DDUMP_FILE
% if use_fst:

# Using FST traceing (slower due to compression)
VL_USER_FLAGS += --trace-fst
VL_USER_FLAGS += -DDUMP_FILE=\"test.fst\"
USER_LDLIBS = -lz
% else:

# Using VCD traceing
VL_USER_FLAGS = --trace
VL_USER_FLAGS += -DDUMP_FILE=\"test.vcd\"
% endif

% endif
# Design prefix
VM_PREFIX = ${prefix}

# Top module
VL_TOP = ${top}

# List all Verilog/SystemVerilog source files to be verilated
VL_SRCS = \\

% for src in verilog_src_files[:-1]:
	${src} \\

% endfor
	${verilog_src_files[-1]}

# Testbench C++ source files
TB_CPP_SRCS = \\

% for src in cpp_src_files[:-1]:
	${src} \\

% endfor
	${cpp_src_files[-1]}

# Build folders
VL_OBJ_DIR = vl_obj_dir
VSL_BUILD_DIR = vsl_build

VS_LOG_LEVEL = ${LOG_LEVELS[log_level]}

#*****************************************************************************
# Top rule
#*****************************************************************************
% if vlt_file:
all: ${target_file} ${tb_file} ${vlt_file} default
% else:
all: ${target_file} ${tb_file} default
% endif

#*****************************************************************************
# Wizard-generated files
#*****************************************************************************
${target_file}: ${config_file}
	@echo "Re-generating Makefile"
	vsl-wizard --makefile-only --makefile $@ $<

${tb_file}: ${config_file}
	@echo "Re-generating top-level testbench file"
	vsl-wizard --tb-only --testbench-file $@ $<

% if vlt_file:
${vlt_file}: ${config_file}
	@echo "Re-generating variables file"
	vsl-wizard --vlt-only --variables-file $@ $<
% endif

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

.PHONY: all

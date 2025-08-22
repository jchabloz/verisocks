# Note: This file has been generated from the template templates/Makefile.mako
#*****************************************************************************
# Configuration
#*****************************************************************************
VERILATOR ?= /usr/local/bin/verilator
VERILATOR_ROOT ?= /usr/local/share/verilator
VSL_DIR ?= ../../..

# Design prefix
VM_PREFIX = Vmain

# Top module
VL_TOP = main

# List all Verilog/SystemVerilog source files to be verilated
VL_SRCS = \
	variables.vlt \
	../test_0.v

# Testbench C++ source files
TB_CPP_SRCS = \
	test_main.cpp

# Build folders
VL_OBJ_DIR = vl_obj_dir
VSL_BUILD_DIR = vsl_build

VS_LOG_LEVEL = $(LOG_LEVEL_DEBUG)

#*****************************************************************************
# Top rule
#*****************************************************************************
all: Makefile test_main.cpp variables.vlt default

#*****************************************************************************
# Wizard-generated files
#*****************************************************************************
Makefile: config.yaml
	@echo "Re-generating Makefile"
	vsl-wizard --makefile-only --makefile $@ $<

test_main.cpp: config.yaml
	@echo "Re-generating top-level testbench file"
	vsl-wizard --tb-only --testbench-file $@ $<

variables.vlt: config.yaml
	@echo "Re-generating variables file"
	vsl-wizard --vlt-only --variables-file $@ $<

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

.PHONY: all

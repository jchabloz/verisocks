# Note: This file has been generated from the template templates/Makefile.mako
#*****************************************************************************
# Configuration
#*****************************************************************************
VERILATOR ?= /usr/local/bin/verilator
VERILATOR_ROOT ?= /usr/local/share/verilator
VSL_DIR ?= ../../../..

# Setup traceing - use $dump() in testbench
# Using FST traceing (slower due to compression)
CPP_USER_FLAGS += -DDUMP_FILE -DDUMP_FST
VL_USER_FLAGS += --trace-fst
USER_LDLIBS = -lz

# Design prefix
VM_PREFIX = Vcounters

# Top module
VL_TOP = counters

# List all Verilog/SystemVerilog source files to be verilated
VL_SRCS = \
	variables.vlt \
	../counters.v \
	../rst_sync.v \
	../clk_checker.v

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
all: default

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

.PHONY: all

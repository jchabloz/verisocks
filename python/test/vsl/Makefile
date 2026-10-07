# Note: This file has been generated from the template templates/Makefile.mako
#*****************************************************************************
# Configuration
#*****************************************************************************
VERILATOR ?= /usr/local/bin/verilator
VERILATOR_ROOT ?= /usr/local/share/verilator
VSL_DIR ?= ../../..

# Use timing option with Verilator
VL_USER_FLAGS += --timing
CPP_USER_FLAGS += -DVSL_TIMING
# Reduced maximum message length (1 MiB), so that test_message_too_long does
# not need to send a message longer than the default 16 MiB
CPP_USER_FLAGS += -DVS_MSG_MAX_LEN=1048576

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
all: default

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

.PHONY: all

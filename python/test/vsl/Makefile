#*****************************************************************************
# Configuration
#*****************************************************************************
VERILATOR = /usr/local/bin/verilator
VERILATOR_ROOT = /usr/local/share/verilator
VSL_DIR = $(HOME)/projects/verisocks

# Verilator user flags
# Setup traceing - use $dump() in testbench
# CPP_USER_FLAGS = -DDUMP_FILE

# Using FST traceing (slower due to compression)
# VL_USER_FLAGS = --trace-fst
# VL_USER_FLAGS += -DDUMP_FILE=\"test.fst\"
# USER_LDLIBS = -lz

# Using VCD traceing
#VL_USER_FLAGS = --trace
#VL_USER_FLAGS += -DDUMP_FILE=\"test.vcd\"

# Design prefix
VM_PREFIX = Vmain

# Top module
VL_TOP = main

# List all Verilog/SystemVerilog source files to be verilated
VL_SRCS = \
	test_vsl.vlt \
	../test_0.v

# Testbench C++ source files
TB_CPP_SRCS = test_vsl.cpp

# Build folders
VL_OBJ_DIR = vl_obj_dir
VSL_BUILD_DIR = vsl_build

VS_LOG_LEVEL = $(LOG_LEVEL_DEBUG)

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

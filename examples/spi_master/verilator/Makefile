#*****************************************************************************
# Configuration
#*****************************************************************************
VERILATOR = /usr/local/bin/verilator
VERILATOR_ROOT = /usr/local/share/verilator
VSL_DIR = $(HOME)/projects/verisocks

# Verilator user flags
# Setup traceing - use $dump() in testbench
# CPP_USER_FLAGS = -DDUMP_FILE

# Using FST (slower due to compression)
#VL_USER_FLAGS = --trace-fst
#VL_USER_FLAGS += -DDUMP_FILE=\"test.fst\"
#USER_LDLIBS = -lz

# Using VCD
#VL_USER_FLAGS = --trace
#VL_USER_FLAGS += -DDUMP_FILE=\"test.vcd\"

# Design prefix
VM_PREFIX = Vspi_master_tb

# Top module
VL_TOP = spi_master_tb

# List all Verilog/SystemVerilog source files to be verilated
VL_SRCS = \
	spi_master_tb.vlt \
	../spi_slave.v \
	../spi_master.v \
	../spi_master_tb.v

# Testbench C++ source files
TB_CPP_SRCS = test_main.cpp

# Build folders
VL_OBJ_DIR = vl_obj_dir
VSL_BUILD_DIR = vsl_build

VS_LOG_LEVEL = $(LOG_LEVEL_DEBUG)

#*****************************************************************************
# Include generic Makefile
#*****************************************************************************
include $(VSL_DIR)/include/vsl/vsl.mk

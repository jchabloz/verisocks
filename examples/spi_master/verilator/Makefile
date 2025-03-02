#*****************************************************************************
# Constants
#*****************************************************************************
VERILATOR = /usr/local/bin/verilator
VERILATOR_ROOT = /usr/local/share/verilator
VSL_DIR = $(HOME)/projects/verisocks

#*****************************************************************************
# Variables
#*****************************************************************************
# Design prefix
VM_PREFIX = Vspi_master_tb

# Top module
VL_TOP = spi_master_tb

# Verilog/SystemVerilog source files to be verilated
VL_SRCS = \
	../spi_slave.v \
	../spi_master.v \
	../spi_master_tb.v

# Testbench C++ source files
TB_CPP_SRCS = test_main.cpp

# Build folders
VL_OBJ_DIR = vl_obj_dir
VSL_BUILD_DIR = vsl_build

include $(VSL_DIR)/include/vsl/vsl.mk


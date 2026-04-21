<%page
args = "
	build_dir,
	top_mk_file,
	mk_file,
	config_file,
	tb_file,
	vlt_file
"
/>\
#******************************************************************************
# Configuration
#******************************************************************************
BUILD_DIR = ${build_dir}

#******************************************************************************
# Rules
#******************************************************************************
.PHONY: all clean clean-all

all: default

clean:
	$(MAKE) -C $(BUILD_DIR) clean
	$(RM) *.vcd *.fst

clean-all:
	$(RM) -r $(BUILD_DIR)
	$(RM) *.vcd *.fst

%%:
	$(MAKE) -C $(BUILD_DIR) $@


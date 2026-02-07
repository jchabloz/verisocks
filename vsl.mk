all:
	$(MAKE) -C examples/hello_world/verilator
	$(MAKE) -C examples/spi_master/verilator
	$(MAKE) -C python/test/vsl

clean:
	$(MAKE) -C examples/hello_world/verilator clean
	$(MAKE) -C examples/spi_master/verilator clean
	$(MAKE) -C python/test/vsl clean

.PHONY: all clean




#include "verilated.h"
#include "vsl.h"
#include "${prefix}.h"
#include "${prefix}__Syms.h"

#include <cstdlib>
#include <memory>

//======================

int main(int argc, char** argv, char**) {

	//Get arguments for port number and timeout
	int port_number {5100};
	int timeout {5};
	if (argc > 1) {
		port_number = std::atoi(argv[1]);
	}
	if (argc > 2) {
		timeout = std::atoi(argv[2]);
	}

    // Setup context, defaults, and parse command line
    Verilated::debug(0);
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->commandArgs(argc, argv);

    // Construct the Verilated model, from Vtop.h generated from Verilating
    const std::unique_ptr<${prefix}> topp {new ${prefix}{contextp.get()}};

    // Setup traceing
    #ifdef DUMP_FILE
    Verilated::traceEverOn(true);
    #endif

    // Dump public variables
    contextp->internalsDump();

    // Create top VSL instance
    vsl::VslInteg<${prefix}> vslx{topp.get(), port_number, timeout};

    // Register public variables
	% for var in variables:
    vslx.register_scalar("main.clk", &topp->main->clk, VLVT_UINT8, 1u);
    vslx.register_scalar("main.count", &topp->main->count, VLVT_UINT8, 8u);
	vslx.register_array("main.count_memory", topp->main->count_memory.m_storage,
		VLVT_UINT8, 8u, 16u);
	vslx.register_param("main.fclk", &topp->main->fclk, VLVT_REAL, 0u);
	vslx.register_param("main.int_param", &topp->main->int_param, VLVT_UINT32, 32u);
	vslx.register_event("main.counter_end", &topp->main->counter_end);
	% endfor

    // Run simulation
    vslx.run();

    return 0;
}

// Verilated -*- C++ -*-
// DESCRIPTION: main() calling loop, created with Verilator --main

#include "verilated.h"
#include "vsl.h"
#include "Vspi_master_tb.h"
#include "Vspi_master_tb__Syms.h"

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
    const std::unique_ptr<Vspi_master_tb> topp {new Vspi_master_tb{contextp.get()}};

    // Setup traceing
    #ifdef DUMP_FILE
    Verilated::traceEverOn(true);
    #endif

    // Dump public variables
    contextp->internalsDump();

    // Create top VSL instance
    vsl::VslInteg<Vspi_master_tb> vslx{topp.get(), port_number, timeout};

    // Register public variables
    vslx.register_scalar("spi_master_tb.miso",
        &topp->spi_master_tb->miso, VLVT_UINT8, 1u);
    vslx.register_scalar("spi_master_tb.toto",
        &topp->spi_master_tb->toto, VLVT_REAL, 0u);
    vslx.register_scalar("spi_master_tb.tutu",
        &topp->spi_master_tb->tutu, VLVT_UINT32, 32u);
    vslx.register_scalar("spi_master_tb.i_spi_master.transaction_counter",
        &topp->spi_master_tb->i_spi_master->transaction_counter, VLVT_UINT32, 32u);
    vslx.register_array("spi_master_tb.tata",
        topp->spi_master_tb->tata.m_storage, VLVT_UINT8, 7u, 12u);
    vslx.register_array("spi_master_tb.i_spi_master.rx_buffer",
        topp->spi_master_tb->i_spi_master->rx_buffer.m_storage,
        VLVT_UINT8, 8u, 8u);
    vslx.register_array("spi_master_tb.i_spi_master.tx_buffer",
        topp->spi_master_tb->i_spi_master->tx_buffer.m_storage,
        VLVT_UINT8, 8u, 7u);
    vslx.register_event("spi_master_tb.i_spi_master.start_transaction",
        &topp->spi_master_tb->i_spi_master->start_transaction);
    vslx.register_event("spi_master_tb.i_spi_master.end_transaction",
        &topp->spi_master_tb->i_spi_master->end_transaction);

    // Run simulation
    vslx.run();

    return 0;
}

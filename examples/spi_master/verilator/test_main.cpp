// Verilated -*- C++ -*-
// DESCRIPTION: main() calling loop, created with Verilator --main

#include "verilated.h"
#include "vsl.h"
#include "Vspi_master_tb.h"
#include "Vspi_master_tb__Syms.h"

#include <memory>

//======================

int main(int argc, char** argv, char**) {
    // Setup context, defaults, and parse command line
    Verilated::debug(0);
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->commandArgs(argc, argv);

    // Construct the Verilated model, from Vtop.h generated from Verilating
    const std::unique_ptr<Vspi_master_tb> topp {new Vspi_master_tb{contextp.get()}};

    contextp->internalsDump();

    /* Create top VSL instance */
    vsl::VslInteg<Vspi_master_tb> vslx{topp.get(), 5100, 5};

    /* Register public variables */
    vslx.register_scalar("spi_master_tb.miso", &topp->spi_master_tb->miso, VLVT_UINT8, 1u);
    vslx.register_scalar("spi_master_tb.toto", &topp->spi_master_tb->toto, VLVT_REAL, 0u);
    vslx.register_scalar("spi_master_tb.tutu", &topp->spi_master_tb->tutu, VLVT_UINT32, 32u);
    vslx.register_array("spi_master_tb.tata", topp->spi_master_tb->tata.m_storage, VLVT_UINT8, 7u, 12u);
    vslx.register_array("spi_master_tb.i_spi_master.rx_buffer", topp->spi_master_tb->i_spi_master->rx_buffer.m_storage, VLVT_UINT8, 8u, 8u);
    vslx.register_array("spi_master_tb.i_spi_master.tx_buffer", topp->spi_master_tb->i_spi_master->tx_buffer.m_storage, VLVT_UINT8, 8u, 7u);
    vslx.register_event("spi_master_tb.i_spi_master.start_transaction", &topp->spi_master_tb->i_spi_master->start_transaction);
    vslx.register_event("spi_master_tb.i_spi_master.end_transaction", &topp->spi_master_tb->i_spi_master->end_transaction);

    /* Run simulation */
    vslx.run();

    /*
    // Simulate until $finish
    while (!contextp->gotFinish()) {
        // Evaluate model
        topp->eval();
        // Advance time
        if (!topp->eventsPending()) break;
        contextp->time(topp->nextTimeSlot());
    }

    if (!contextp->gotFinish()) {
        VL_DEBUG_IF(VL_PRINTF("+ Exiting without $finish; no events left\n"););
    }

    // Execute 'final' processes
    topp->final();

    // Print statistical summary report
    contextp->statsPrintSummary();

    */
    return 0;
}

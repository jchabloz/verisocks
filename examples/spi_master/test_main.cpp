// Verilated -*- C++ -*-
// DESCRIPTION: main() calling loop, created with Verilator --main

#include "verilated.h"
#include "Vspi_master_tb.h"

#define TRACE_ON

//======================

int main(int argc, char** argv, char**) {
    // Setup context, defaults, and parse command line
    Verilated::debug(0);
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    #ifdef TRACE_ON
    contextp->traceEverOn(true);
    #endif
    contextp->commandArgs(argc, argv);

    // Construct the Verilated model, from Vtop.h generated from Verilating
    const std::unique_ptr<Vspi_master_tb> topp{new Vspi_master_tb{contextp.get()}};

    #ifdef TRACE_ON
    VerilatedVcdC* tfp = new VerilatedVcdC;
    topp->trace(tfp, 99);  // Trace 99 levels of hierarchy (or see below)
    tfp->open("simx.vcd");
    #endif

    // Simulate until $finish
    while (!contextp->gotFinish()) {

        // Evaluate model
        topp->eval();
        #ifdef TRACE_ON
        tfp->dump(contextp->time());
        #endif

        if (contextp->time() == 0.0) {
            contextp->time(100000);
            printf("Trigger event\n");
            topp->spi_master_tb->i_spi_master->trigger_transaction({0x12,0x34,0x56,0x78,0x9a,0xbc,0xde});
            topp->eval();
            #ifdef TRACE_ON
            tfp->dump(contextp->time());
            #endif
        }

        // Advance time
        if (!topp->eventsPending()) {
            contextp->timeInc(100000);
            //contextp->time(contextp->time() + 100000);
            topp->eval();
            #ifdef TRACE_ON
            tfp->dump(contextp->time());
            #endif
            break;
        }
        printf("Advance time to %lu\n", topp->nextTimeSlot());
        contextp->time(topp->nextTimeSlot());
    }

    if (!contextp->gotFinish()) {
        VL_DEBUG_IF(VL_PRINTF("+ Exiting without $finish; no events left\n"););
    }

    // Execute 'final' processes
    topp->final();
    #ifdef TRACE_ON
    tfp->close();
    #endif

    return 0;
}

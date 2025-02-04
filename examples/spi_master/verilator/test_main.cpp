// Verilated -*- C++ -*-
// DESCRIPTION: main() calling loop, created with Verilator --main

#include "verilated.h"
#include "vsl.h"
#include "Vspi_master_tb.h"

//======================

int main(int argc, char** argv, char**) {
    // Setup context, defaults, and parse command line
    Verilated::debug(0);
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->commandArgs(argc, argv);

    // Construct the Verilated model, from Vtop.h generated from Verilating
    const std::unique_ptr<Vspi_master_tb> topp{new Vspi_master_tb{contextp.get()}};


    /* SANDBOX */
    /* Looking up variables - should we use this to access variables/signals?
    Should the usage of scopeFind and varFind methods should stay internal to
    Verilator?
    */
    std::string str_path{"TOP.spi_master_tb.i_spi_master.tx_buffer"};
    //std::string str_path{"TOP.spi_master_tb.i_spi_master.start_transaction"};
    //std::string str_path{"TOP.spi_master_tb.toto"};
    std::string str_scope;
    std::string str_var;
    if (str_path.find_last_of(".") != str_path.npos) {
        str_scope = str_path.substr(0, str_path.find_last_of("."));
        str_var = str_path.substr(str_path.find_last_of(".") + 1);
    }

    contextp->internalsDump();
    auto p_xscope = contextp->scopeFind(str_scope.c_str());
    if (p_xscope != nullptr) {
        printf("Scope %s found\n", p_xscope->name());
        printf("Searching for variable %s in scope\n", str_var.c_str());
        auto p_xvar = p_xscope->varFind(str_var.c_str());
        if (p_xvar != nullptr) {
            printf("Variable %s found\n", p_xvar->name());
            printf("Variable type: %d\n", (int) p_xvar->vltype());
            printf("Variable total size: %d\n", (int) p_xvar->totalSize());
            printf("Variable dims: %d\n", (int) p_xvar->dims());
            printf("Variable dim 0 range: [%d:%d]\n", (int) p_xvar->left(0), (int) p_xvar->right(0));
            printf("Variable dim 1 range: [%d:%d]\n", (int) p_xvar->left(1), (int) p_xvar->right(1));
        }
    }

    vsl::VslInteg<Vspi_master_tb> vslx{topp.get(), 5100, 5};
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

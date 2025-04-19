/*
Note: this file has been generated from the template templates/test_main.cpp.mako

Copyright (c) 2025 Jérémie Chabloz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "verilated.h"
#include "vsl.h"
#include "Vmain.h"
#include "Vmain__Syms.h"

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
    const std::unique_ptr<Vmain> topp {new Vmain{contextp.get()}};

    // Setup traceing
    #ifdef DUMP_FILE
    Verilated::traceEverOn(true);
    #endif

    // Dump public variables
    contextp->internalsDump();

    // Create top VSL instance
    vsl::VslInteg<Vmain> vslx{topp.get(), port_number, timeout};

    // Register public variables
    // Scalar variables
    vslx.register_scalar("main.clk",
        &topp->main->clk,
        VLVT_UINT8, 1u);
    vslx.register_scalar("main.count",
        &topp->main->count,
        VLVT_UINT8, 8u);
    // Array variables
    vslx.register_array("main.count_memory",
        topp->main->count_memory.m_storage,
        VLVT_UINT8, 8u, 16u);
    // Parameters
    vslx.register_param("main.fclk",
        &topp->main->fclk,
        VLVT_REAL, 0u);
    vslx.register_param("main.int_param",
        &topp->main->int_param,
        VLVT_UINT32, 32u);
    // Named events
    vslx.register_event("main.counter_end",
        &topp->main->counter_end);

    // Run simulation
    int retval = vslx.run();

    return retval;
}

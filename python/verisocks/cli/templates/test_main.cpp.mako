<%page args = "prefix, variables"/>\
<%
VLVT_TYPES = {
    "uint8":  "VLVT_UINT8",
    "uint16": "VLVT_UINT16",
    "uint32": "VLVT_UINT32",
    "uint64": "VLVT_UINT64",
    "real":   "VLVT_REAL"
}
%>\
/*
Note: this file has been generated from the template ${template_filename}

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
    % if 'scalars' in variables:
    // Scalar variables
	% for var in variables['scalars']:
    vslx.register_scalar("${var['path']}",
        &topp->${var['path'].replace(".", "->")},
        ${VLVT_TYPES[var['type']]}, ${var['width']}u);
    % endfor
    % endif
    % if 'arrays' in variables:
    // Array variables
    % for var in variables['arrays']:
    vslx.register_array("${var['path']}",
        topp->${var['path'].replace(".", "->")}.m_storage,
        ${VLVT_TYPES[var['type']]}, ${var['width']}u, ${var['depth']}u);
    % endfor
    % endif
    % if 'params' in variables:
    // Parameters
    % for var in variables['params']:
    vslx.register_param("${var['path']}",
        &topp->${var['path'].replace(".", "->")},
        ${VLVT_TYPES[var['type']]}, ${var['width']}u);
    % endfor
    % endif
    % if 'events' in variables:
    // Named events
    % for var in variables['events']:
    vslx.register_event("${var['path']}",
        &topp->${var['path'].replace(".", "->")});
    % endfor
    % endif

    // Run simulation
    int retval = vslx.run();

    return retval;
}

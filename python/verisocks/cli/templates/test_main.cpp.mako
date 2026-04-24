<%page args = "prefix, variables, log_level, exec_version, exec_doc,
    bug_address"
/>\
<%
import datetime
import verisocks
from math import ceil
today = datetime.datetime.today()

VLVT_TYPES = {
    "uint8":  "VLVT_UINT8",
    "uint16": "VLVT_UINT16",
    "uint32": "VLVT_UINT32",
    "uint64": "VLVT_UINT64",
    "real":   "VLVT_REAL"
}

LOG_LEVELS = {
	"debug":    10,
	"info":     20,
	"warning":  30,
	"error":    40,
	"critical": 50
}

def get_var_type(var):
    type = var['type']
    if (type == "int"):
        width = var['width']
        nbytes = ceil(width/8.0)
        if (nbytes > 8):
            raise VerisocksError("Variable width > 64 bits - Not supported yet")
        key = "uint{:d}".format(nbytes*8)
        return VLVT_TYPES[key]
    elif (type in VLVT_TYPES):
        return VLVT_TYPES[type]
    else:
        raise VerisocksError(f"Unknown variable type {type}")

def get_var_name(var):
    """Get variable name/alias"""
    if 'name' in var:
        return var['name']
    return var['path']

def get_clk_dc(clk):
    """Get clock variable duty cycle value"""
    if 'duty_cycle' in clk:
        return clk['duty_cycle']
    return 0.5

def bool_py2c(x):
    if (x):
        return "true"
    else:
        return "false"
%>\
/*
Note: this file has been generated from the template ${template_filename}

Copyright (c) 2025-${today.year} Jérémie Chabloz

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
#include <cstdio>
#include <memory>

#include <argp.h>

// Argp configuration
% if exec_version:
const char* argp_program_version = "${exec_version}";
% endif
% if bug_address:
const char* argp_program_bug_address = "<${bug_address}>";
% endif
% if exec_doc:
static const char* argp_doc = "${exec_doc}";
% endif
static const struct argp_option argp_options[] = {
    {"port",    'p', "PORT",    0, "Use socket port number PORT (default: 5100)"},
    {"timeout", 't', "TIMEOUT", 0, "Use socket TIMEOUT (seconds) (default: 5s)"},
    {0}
};

// Arguments structure
typedef struct arguments {
    int port_number;
    int timeout;
} arguments_t;

// Arguments parsing function
static error_t argp_parse_opt (int key, char* arg, struct argp_state* state) {
    arguments_t* arguments = (arguments_t*) (state->input);
    switch (key) {
        case 'p':
            arguments->port_number = std::atoi(arg);
            if (0 > arguments->port_number) {
                argp_error(state, "Invalid argument for PORT\n");
            }
            break;
        case 't':
            arguments->timeout = std::atoi(arg);
            if (0 >= arguments->timeout) {
                argp_error(state, "Invalid argument for TIMEOUT\n");
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

% if exec_doc:
static const struct argp argp = {argp_options, argp_parse_opt, 0, argp_doc};
% else:
static const struct argp argp = {argp_options, argp_parse_opt, 0, 0};
% endif

//======================

int main(int argc, char** argv, char**) {

    // Initialize arguments with default values
    arguments_t arguments {5100, 5};

    // Parse arguments
    if (0 != argp_parse(&argp, argc, argv, 0, 0, &arguments)) {
        return -1;
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

	% if (LOG_LEVELS[log_level] < 20):
    // Dump public variables
    contextp->internalsDump();

    % endif
    // Create top VSL instance
    vsl::VslInteg<${prefix}> vslx{
        topp.get(), arguments.port_number, arguments.timeout};

    % if variables:
    // Register public variables
    % if 'clocks' in variables:
    // Clocks
    % for clk in variables['clocks']:
    vslx.register_clock("${get_var_name(clk)}",
        &topp->${clk['path'].replace(".", "->")},
        ${clk['period']}, "${clk['unit']}", ${get_clk_dc(clk)}\
% if 'enable' in clk:
, ${bool_py2c(clk['enable'])}
    );
% else:
, false
    );
    % endif
    % endfor
    % endif
    % if 'scalars' in variables:
    // Scalar variables
	% for var in variables['scalars']:
    vslx.register_scalar("${get_var_name(var)}",
        &topp->${var['path'].replace(".", "->")},
    % if var['type'] == "real":
        ${get_var_type(var)}, 0u);
    % else:
        ${get_var_type(var)}, ${var['width']}u);
    % endif
    % endfor
    % endif
    % if 'arrays' in variables:
    // Array variables
    % for var in variables['arrays']:
    vslx.register_array("${get_var_name(var)}",
        topp->${var['path'].replace(".", "->")}.m_storage,
        ${get_var_type(var)}, ${var['width']}u, ${var['depth']}u);
    % endfor
    % endif
    % if 'params' in variables:
    // Parameters
    % for var in variables['params']:
    vslx.register_param("${get_var_name(var)}",
        &topp->${var['path'].replace(".", "->")},
    % if var['type'] == "real":
        ${get_var_type(var)}, 0u);
    % else:
        ${get_var_type(var)}, ${var['width']}u);
    % endif
    % endfor
    % endif
    % if 'events' in variables:
    // Named events
    % for var in variables['events']:
    vslx.register_event("${get_var_name(var)}",
        &topp->${var['path'].replace(".", "->")});
    % endfor
    % endif

    % endif
    // Run simulation
    int retval = vslx.run();

    return retval;
}

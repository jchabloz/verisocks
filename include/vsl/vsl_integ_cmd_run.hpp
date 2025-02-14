/*
Copyright (c) 2024-2025 Jérémie Chabloz

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

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

#ifndef VSL_INTEG_CMD_RUN_HPP
#define VSL_INTEG_CMD_RUN_HPP

#include "cJSON.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "vsl/vsl_integ.hpp"
#include "verilated.h"

#include <string>

namespace vsl{

/******************************************************************************
Run command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run) {

    /* Error handler lambda function */
    auto handle_error = [&]() {
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run - Discarding");
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the callback field from the JSON message content */
    cJSON *p_item_cb = cJSON_GetObjectItem(vx.p_cmd, "cb");
    if (nullptr == p_item_cb) {
        vs_log_mod_error("vsl", "Command field \"cb\" invalid/not found");
        handle_error();
        return;
    }

    /* Get the run command argument cb as a string */
    char* cstr_cb = cJSON_GetStringValue(p_item_cb);
    if ((nullptr == cstr_cb) || std::string(cstr_cb).empty()) {
        vs_log_mod_error("vsl", "Command field \"cb\" NULL or empty");
        handle_error();
        return;
    }
    vs_log_mod_info("vsl", "Command \"run(cb=%s)\" received.", cstr_cb);

    /* Look up and execute sub-command handler */
    std::string cb_key = "run_";
    cb_key.append(cstr_cb);
    auto search = vx.sub_cmd_handlers_map.find(cb_key);
    if (search != vx.sub_cmd_handlers_map.end()) {
        vx.sub_cmd_handlers_map[cb_key](vx);
        return;
    }

    /* Error case - sub-command handler function not found */
    vs_log_mod_error("vsl", "Handler for sub-command %s not found",
        cb_key.c_str());
    vs_msg_return(vx.fd_client_socket, "error",
        "Could not find handler for sub-command. Discarding.");
    vx._state = VSL_STATE_WAITING;
    return;
}

template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run_for_time) {
    //TODO
    return;
}

template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run_until_time) {
    //TODO
    return;
}

template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run_until_change) {
    //TODO
    return;
}

} //namespace vsl

#endif //VSL_INTEG_CMD_RUN_HPP
//EOF

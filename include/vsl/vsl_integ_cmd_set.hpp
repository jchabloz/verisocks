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

#ifndef VSL_INTEG_CMD_SET_HPP
#define VSL_INTEG_CMD_SET_HPP

#include "cJSON.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "vsl/vsl_integ.hpp"
#include "vsl/vsl_utils.hpp"
#include "verilated.h"
//#include "verilated_syms.h"

//#include <cstdio>
#include <string>
#include <cmath>


namespace vsl{

/******************************************************************************
Set command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(set) {

    cJSON *p_msg = nullptr;
    char *str_msg = nullptr;

    /* Error handler lambda function */
    auto handle_error = [&]() {
        if (nullptr != p_msg) cJSON_Delete(p_msg);
        if (nullptr != str_msg) cJSON_free(str_msg);
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command set - Discarding");
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the object path from the JSON message content */
    cJSON *p_item_path = cJSON_GetObjectItem(vx.p_cmd, "path");
    if (nullptr == p_item_path) {
        vs_log_mod_error("vsl", "Command field \"path\" invalid/not found");
        handle_error();
        return;
    }

    /* Get the path argument as a string */
    char *cstr_path = cJSON_GetStringValue(p_item_path);
    std::string str_path(cstr_path);
    if ((nullptr == cstr_path) || std::string(str_path).empty()) {
        vs_log_mod_error("vsl", "Command field \"path\" NULL or empty");
        handle_error();
        return;
    }
    vs_log_mod_info("vsl", "Command \"set(path=%s)\" received.", cstr_path);

    /* Attempt to get a pointer to the variable */
    auto p_var = vx.get_var(str_path);
    if (nullptr == p_var) {
        vs_log_mod_error(
            "vsl", "Variable %s not found in context", str_path.c_str()
        );
        handle_error();
        return;
    }

    cJSON *p_item_val;
    p_item_val = cJSON_GetObjectItem(vx.p_cmd, "value");
    /* Scalar variables */
    if (p_var->dims() < 2) {
        double value {0.0f}; //Default value if not specified in message
        /* Get the value from the JSON message content */
        if (nullptr != p_item_val) {
            value = cJSON_GetNumberValue(p_item_val);
            if (std::isnan(value)) {
                vs_log_mod_error("vsl", "Command field \"value\" invalid (NaN)");
                handle_error();
                return;
            }
        }
        if (0 > set_variable_value(p_var, value)) {
            handle_error();
            return;
        }
    }
    /* Array variables */
    else if (p_var->dims() == 2) {
        if (nullptr == p_item_val) {
            vs_log_mod_error("vsl", "Command field \"value\" invalid/not found");
            handle_error();
            return;
        }
        if (0 > set_array_variable_value(p_var, p_item_val)) {
            vs_log_mod_error("vsl", "Error setting array variable value");
            handle_error();
            return;
        }
    }
    else {
        vs_log_mod_error(
            "vsl", "Arrays with dimensions > 2 are not supported (yet) :-("
        );
        handle_error();
        return;
    }

    vs_msg_return(vx.fd_client_socket, "ack",
        "Processed command \"set\"");

    /* Normal exit */
    if (nullptr != p_msg) cJSON_Delete(p_msg);
    if (nullptr != str_msg) cJSON_free(str_msg);
    vx._state = VSL_STATE_WAITING;
    return;
}

} //namespace vsl

#endif //VSL_INTEG_CMD_SET_HPP
//EOF

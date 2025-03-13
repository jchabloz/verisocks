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

    /* Check if the provided path contains the [ ] range selection operator*/
    bool path_has_range = has_range(str_path);
    VslArrayRange path_range;
    VslVar* p_var;
    if (path_has_range) {
        path_range = get_range(str_path);
        vs_log_mod_debug("vsl", "Range found (left: %d, right %d, incr %d)",
            (int) path_range.left, (int) path_range.right,
            (int) path_range.incr);
        p_var = vx.get_registered_variable(path_range.array_name);
    } else {
        p_var = vx.get_registered_variable(str_path);
    }

    /* Attempt to get a pointer to the variable */
    if (nullptr == p_var) {
        vs_log_mod_error(
            "vsl", "Variable %s not found registered variable map",
            str_path.c_str()
        );
        handle_error();
        return;
    }

    /* Consistency checks on range */
    if (path_has_range) {
        if (p_var->get_type() != VSL_TYPE_ARRAY) {
            vs_log_mod_error(
                "vsl", "Range operator [] only supported for array type");
            handle_error();
            return;
        }
        if ((path_range.left >= p_var->get_depth()) ||
            (path_range.right >= p_var->get_depth())) {
            vs_log_mod_error("vsl", "Range overflow");
            handle_error();
            return;
        }
    }    

    cJSON *p_item_val;
    double value {0.0f};
    p_item_val = cJSON_GetObjectItem(vx.p_cmd, "value");
    /* Scalar variables */
    int ack = 0;
    switch (p_var->get_type()) {
        case VSL_TYPE_SCALAR:
        case VSL_TYPE_EVENT:
            if (nullptr != p_item_val) {
                value = cJSON_GetNumberValue(p_item_val);
                if (std::isnan(value)) {
                    vs_log_mod_error(
                        "vsl", "Command field \"value\" invalid (NaN)");
                    handle_error();
                    return;
                }
            }
            if (0 > p_var->set_value(value)) {
                handle_error();
                return;
            }
            break;
        case VSL_TYPE_ARRAY:
            if (nullptr == p_item_val) {
                vs_log_mod_error("vsl",
                    "Command field \"value\" invalid/not found");
                handle_error();
                return;
            }
            if (path_has_range) {
                if (path_range.left == path_range.right) {
                    /* Range corresponds to a single index */
                    value = cJSON_GetNumberValue(p_item_val);
                    ack = p_var->set_array_value(value, path_range.left);
                } else {
                    /* Range corresponds to multiple indexes */
                    cJSON* iterator;             
                    size_t mem_index = path_range.right;    
                    cJSON_ArrayForEach(iterator, p_item_val) {
                        value = cJSON_GetNumberValue(iterator);
                        if (0 > p_var->set_array_value(value, mem_index)) {
                            ack = -1;
                            break;
                        }
                        mem_index += path_range.incr;
                    }
                }
            } else {
                ack = p_var->set_array_variable_value(p_item_val);
            }
            if (0 > ack) {
                vs_log_mod_error("vsl",
                    "Error setting array variable value");
                handle_error();
                return;
            }
            break;
        default:
            vs_log_mod_error(
                "vsl", "Variable type not support"
            );
            handle_error();
            return;
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

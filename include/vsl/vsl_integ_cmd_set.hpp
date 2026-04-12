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

#undef __MOD__
#define __MOD__ "vsl"

namespace vsl{

/******************************************************************************
Set command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(set) {

    /* Error handler lambda function */
    VSL_ERROR_HANDLER(vx, "Error processing command set - Discarding");

    /* Read optional selection field */
    VSL_MSG_READ_STR_OPT(vx.p_cmd, sel);

    if (has_sel) {
        /* Look up and execute sub-command handler */
        std::string sel_key = "set_";
        sel_key.append(std::string(cstr_sel));
        auto search = vx.sub_cmd_handlers_map.find(sel_key);
        if (search != vx.sub_cmd_handlers_map.end()) {
            vx.sub_cmd_handlers_map[sel_key](vx);
            return;
        }

        /* Error case - sub-command handler function not found */
        vs_log_mod_error(__MOD__, "Handler for sub-command %s not found",
            sel_key.c_str());
        vs_msg_return(vx.fd_client_socket, "error",
            "Could not find handler for sub-command. Discarding.",
            &vx.uuid);
        vx._state = VSL_STATE_WAITING;
        return;   
    }

    /* Default sub-command handler function if sel is not defined */
    vx.sub_cmd_handlers_map["set_value"](vx);
    return;
}

/******************************************************************************
Set value sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(set_value) {

    /* Error handler lambda function */
    VSL_ERROR_HANDLER(vx,
        "Error processing command set(sel=value) - Discarding");

    /* Get the object path from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, path);
    vs_log_mod_info(__MOD__, "Command \"set(path=%s)\" received.", cstr_path);

    /* Check if the provided path contains the [ ] range selection operator*/
    bool path_has_range = has_range(str_path);
    VslArrayRange path_range;
    VslVar* p_var;
    if (path_has_range) {
        path_range = get_range(str_path);
        vs_log_mod_debug(__MOD__, "Range found (left: %d, right %d, incr %d)",
            (int) path_range.left, (int) path_range.right,
            (int) path_range.incr);
        p_var = vx.get_registered_variable(path_range.array_name);
    } else {
        p_var = vx.get_registered_variable(str_path);
    }

    /* Attempt to get a pointer to the variable */
    if (nullptr == p_var) {
        vs_log_mod_error(
            __MOD__, "Variable %s not found registered variable map",
            str_path.c_str()
        );
        handle_error();
        return;
    }

    /* Consistency checks on range */
    if (path_has_range) {
        if (p_var->get_type() != VSL_TYPE_ARRAY) {
            vs_log_mod_error(
                __MOD__, "Range operator [] only supported for array type");
            handle_error();
            return;
        }
        if ((path_range.left >= p_var->get_depth()) ||
            (path_range.right >= p_var->get_depth())) {
            vs_log_mod_error(__MOD__, "Range overflow");
            handle_error();
            return;
        }
    }

    double value {0.0};
    cJSON *p_item_val = cJSON_GetObjectItem(vx.p_cmd, "value");
    /* Scalar variables */
    int ack = 0;
    switch (p_var->get_type()) {
        case VSL_TYPE_SCALAR:
        case VSL_TYPE_EVENT:
            if (nullptr != p_item_val) {
                value = cJSON_GetNumberValue(p_item_val);
                if (std::isnan(value)) {
                    vs_log_mod_error(
                        __MOD__, "Command field \"value\" invalid (NaN)");
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
                vs_log_mod_error(__MOD__,
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
                vs_log_mod_error(__MOD__,
                    "Error setting array variable value");
                handle_error();
                return;
            }
            break;
        default:
            vs_log_mod_error(
                __MOD__, "Variable type not supported"
            );
            handle_error();
            return;
        return;
    }

    vs_msg_return(vx.fd_client_socket, "ack",
        "Processed command \"set\"", &vx.uuid);

    /* Normal exit */
    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Set clk_en sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(set_clk_en) {

    /* Lambda function - error handler */
    VSL_ERROR_HANDLER(vx,
        "Error processing command set(sel=clk_en) - Discarding");

    /* Get the object path from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, path);
    vs_log_mod_info(
        __MOD__, "Command \"set(sel=clk_en, path=%s)\" received.", cstr_path);

    /* Make sure clock exists */
    if (!vx.clock_map.has_clock(str_path)) {
        vs_log_mod_error(__MOD__, "Clock %s not found", cstr_path);
        handle_error();
        return;
    }

    // Enable or disable the clock depending on value
    VSL_MSG_READ_NUM(vx.p_cmd, value);

    if (value > 0) {
        vx.clock_map.get_clock(str_path).enable(vx.p_context);
        vs_log_mod_debug(__MOD__, "Clock with path \"%s\" enabled", cstr_path);
    } else {
        vx.clock_map.get_clock(str_path).disable();
        vs_log_mod_debug(__MOD__, "Clock with path \"%s\" disabled", cstr_path);
    }

    vs_msg_return(vx.fd_client_socket, "ack",
        "Processed command \"set(sel=clk_en)\"", &vx.uuid);

    /* Normal exit */
    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Set clk_cfg sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(set_clk_cfg) {

    /* Lambda function - error handler */
    VSL_ERROR_HANDLER(vx,
            "Error processing command set(sel=clk_cfg) - Discarding");

    /* Get the object path from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, path);
    vs_log_mod_info(
        __MOD__, "Command \"set(sel=clk_cfg, path=%s)\" received.", cstr_path);

    /* Make sure clock exists */
    if (!vx.clock_map.has_clock(str_path)) {
        vs_log_mod_error(__MOD__, "Clock %s not found", cstr_path);
        handle_error();
        return;
    }

    VSL_MSG_READ_NUM(vx.p_cmd, period);
    VSL_MSG_READ_STR(vx.p_cmd, unit);
    VSL_MSG_READ_NUM(vx.p_cmd, dc);

    vx.clock_map.get_clock(str_path).set_period(
        period, cstr_unit, dc, vx.p_context);

    vs_msg_return(vx.fd_client_socket, "ack",
        "Processed command \"set(sel=clk_cfg)\"", &vx.uuid);

    /* Normal exit */
    vx._state = VSL_STATE_WAITING;
    return;
}

} //namespace vsl

#endif //VSL_INTEG_CMD_SET_HPP
//EOF

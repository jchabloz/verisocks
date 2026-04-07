/*****************************************************************************
 @file vsl_integ_cmd_get.hpp
 @brief Command handlers implementations for "get" operations in the
 vsl::VslInteg class template.

 This header defines the implementation of the "get" command and its
 sub-commands for the vsl::VslInteg class template, which is part of the
 verisocks simulation integration. The handlers process JSON-based commands
 received from a client, extract relevant arguments, and return requested
 simulation information or variable values.

 Main handlers:
 - VSL_CMD_HANDLER(get): Dispatches "get" commands to the appropriate
   sub-command handler based on the "sel" field in the JSON message.
 - VSL_CMD_HANDLER(get_sim_info): Returns simulator and model information
   such as product name, version, model name, hierarchy, and time
   unit/precision.
 - VSL_CMD_HANDLER(get_sim_time): Returns the current simulation time in
   seconds.
 - VSL_CMD_HANDLER(get_value): Returns the value of a requested variable or
   array, supporting optional range selection for arrays.

 Error handling is performed via lambda functions that send error messages to
 the client and reset the simulation state as needed.

 @author Jérémie Chabloz
 @copyright Copyright (c) 2024-2025 Jérémie Chabloz Distributed under the MIT
 License. See file for details.
*******************************************************************************/
/*
MIT License

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

#ifndef VSL_INTEG_CMD_GET_HPP
#define VSL_INTEG_CMD_GET_HPP

#include "cJSON.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "vsl/vsl_integ.hpp"
#include "vsl/vsl_types.hpp"
#include "vsl/vsl_utils.hpp"
#include "verilated.h"
#include "verilated_syms.h"

#include <cstdio>
#include <string>
#include <cmath>

#undef __MOD__
#define __MOD__ "vsl"

namespace vsl{

/******************************************************************************
Get command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(get) {

    /* Error handler lambda function */
    VSL_ERROR_HANDLER(vx, "Error processing command get - Discarding");

    /* Get the value from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, sel);
    vs_log_mod_info(__MOD__, "Command \"get(sel=%s)\" received.", cstr_sel);

    /* Look up and execute sub-command handler */
    std::string sel_key = "get_";
    sel_key.append(cstr_sel);
    auto search = vx.sub_cmd_handlers_map.find(sel_key);
    if (search != vx.sub_cmd_handlers_map.end()) {
        vx.sub_cmd_handlers_map[sel_key](vx);
        return;
    }

    /* Error case - sub-command handler function not found */
    vs_log_mod_error(__MOD__, "Handler for sub-command %s not found",
        sel_key.c_str());
    vs_msg_return(vx.fd_client_socket, "error",
        "Could not find handler for sub-command. Discarding.", &vx.uuid);
    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Get sim_info sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(get_sim_info) {
    cJSON *p_msg = nullptr;
    char *str_msg = nullptr;
    vs_msg_info_t msg_info = VS_MSG_INFO_INIT_JSON;
    vs_msg_copy_uuid(&msg_info, &vx.uuid);

    /* Lambda function - error handler */
    auto handle_error = [&](){
        if (nullptr != p_msg) cJSON_Delete(p_msg);
        if (nullptr != str_msg) cJSON_free(str_msg);
        vx._state = VSL_STATE_WAITING;
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command get(sel=sim_info) - Discarding",
			&vx.uuid);
    };

    /* Create return message object */
    VSL_MSG(p_msg);
    VSL_MSG_ADD_STR(p_msg, "type", "result");
    VSL_MSG_ADD_STR(p_msg, "product", Verilated::productName());
    VSL_MSG_ADD_STR(p_msg, "version", Verilated::productVersion());
    VSL_MSG_ADD_STR(p_msg, "model_name", vx.p_model->modelName());
    VSL_MSG_ADD_STR(p_msg, "model_hier_name", vx.p_model->hierName());
    VSL_MSG_ADD_STR(p_msg, "time_unit", vx.p_context->timeunitString());
    VSL_MSG_ADD_STR(p_msg, "time_precision",
        vx.p_context->timeprecisionString());

    VSL_MSG_CREATE(str_msg, p_msg, msg_info);
    VSL_MSG_WRITE(str_msg, vx);

    /* Normal exit */
    if (nullptr != p_msg) cJSON_Delete(p_msg);
    if (nullptr != str_msg) cJSON_free(str_msg);
    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Get sim_time sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(get_sim_time) {
    cJSON *p_msg;
    char *str_msg = nullptr;
    vs_msg_info_t msg_info = VS_MSG_INFO_INIT_JSON;
    vs_msg_copy_uuid(&msg_info, &vx.uuid);

    /* Lambda function - error handler */
    auto handle_error = [&](){
        if (nullptr != p_msg) cJSON_Delete(p_msg);
        if (nullptr != str_msg) cJSON_free(str_msg);
        vx._state = VSL_STATE_WAITING;
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command get(sel=sim_time) - Discarding",
			&vx.uuid);
    };

    /* Create return message object */
    VSL_MSG(p_msg);
    VSL_MSG_ADD_STR(p_msg, "type", "result");

    /* Get simulator time in seconds from Verilator context */
    vs_log_mod_debug(__MOD__, "Getting simulator time...");
    auto time = vx.p_context->time();
    auto timeprecision = vx.p_context->timeprecision();
    double sim_time_sec = time * std::pow(10.0, timeprecision);
    vs_log_mod_debug(__MOD__, "Sim time: %.6e s", sim_time_sec);
    VSL_MSG_ADD_NUM(p_msg, "time", sim_time_sec);

    VSL_MSG_CREATE(str_msg, p_msg, msg_info);
    VSL_MSG_WRITE(str_msg, vx);

    /* Normal exit */
    if (nullptr != p_msg) cJSON_Delete(p_msg);
    if (nullptr != str_msg) cJSON_free(str_msg);
    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Get value sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(get_value) {

    cJSON *p_msg;
    cJSON *p_item_path;
    char *str_msg = nullptr;
    vs_msg_info_t msg_info = VS_MSG_INFO_INIT_JSON;
    vs_msg_copy_uuid(&msg_info, &vx.uuid);

    /* Lambda function - error handler */
    auto handle_error = [&](){
        if (nullptr != p_msg) cJSON_Delete(p_msg);
        if (nullptr != str_msg) cJSON_free(str_msg);
        vx._state = VSL_STATE_WAITING;
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command get(sel=value) - Discarding", &vx.uuid);
    };

    /* Create return message object */
    VSL_MSG(p_msg);
    VSL_MSG_ADD_STR(p_msg, "type", "result");

    /* Get the object path from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, path);

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
            __MOD__, "Variable %s not found in context", str_path.c_str());
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

    /* Scalar variables */
    int ack = 0;
    switch (p_var->get_type()) {
        case VSL_TYPE_SCALAR:
        case VSL_TYPE_PARAM:
        case VSL_TYPE_EVENT:
            if (0 > p_var->add_value_to_msg(p_msg, "value")) {
                vs_log_mod_error(
                    __MOD__, "Error getting value for variable %s",
                    str_path.c_str());
                handle_error();
                return;
            }
            break;
        case VSL_TYPE_ARRAY:
            vs_log_mod_debug(__MOD__,
                "Variable %s detected to be an array", str_path.c_str());
            vs_log_mod_debug(__MOD__,
                "Array width: %d", (int) p_var->get_width());
            vs_log_mod_debug(__MOD__,
                "Array depth: %d", (int) p_var->get_depth());
            ack = path_has_range ?
                p_var->add_array_to_msg(p_msg, "value", path_range) :
                p_var->add_array_to_msg(p_msg, "value");
            if (0 > ack) {
                vs_log_mod_error(__MOD__,
                    "Error getting array values for variable %s",
                    str_path.c_str());
                handle_error();
                return;
            }
            break;
        default:
            vs_log_mod_error(
                __MOD__, "Type not supported (yet) for getting value");
            handle_error();
            return;
    }

    VSL_MSG_CREATE(str_msg, p_msg, msg_info);
    VSL_MSG_WRITE(str_msg, vx);

    /* Normal exit */
    if (nullptr != p_msg) cJSON_Delete(p_msg);
    if (nullptr != str_msg) cJSON_free(str_msg);
    vx._state = VSL_STATE_WAITING;
    return;
}

} //namespace vsl

#endif //VSL_INTEG_CMD_GET_HPP
//EOF

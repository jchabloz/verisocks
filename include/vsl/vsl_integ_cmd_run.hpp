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
#include "vsl/vsl_utils.hpp"
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

    /* Error handler lambda function */
    auto handle_error = [&]() {
        vs_log_mod_warning(
            "vsl", "Error processing command run(for_time) - Discarding");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run(for time) - Discarding");
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the time field from the JSON message content */
    cJSON* p_item_time;
    p_item_time = cJSON_GetObjectItem(vx.p_cmd, "time");
    if (nullptr == p_item_time) {
        vs_log_mod_error("vsl", "Command field \"time\" invalid/not found");
        handle_error();
        return;
    }
    double time_value;
    time_value = cJSON_GetNumberValue(p_item_time);
    if (time_value <= 0.0) {
        vs_log_mod_error("vsl", "Command field \"time\" <= 0.0");
        handle_error();
        return;
    }

    cJSON* p_item_unit;
    p_item_unit = cJSON_GetObjectItem(vx.p_cmd, "time_unit");
    if (nullptr == p_item_unit) {
        vs_log_mod_error(
            "vsl", "Command field \"time_unit\" invalid/not found");
        handle_error();
        return;
    }
    char* str_time_unit;
    str_time_unit = cJSON_GetStringValue(p_item_unit);
    if ((nullptr == str_time_unit) || std::string(str_time_unit).empty()) {
        vs_log_mod_error("vsl", "Command field \"time_unit\" NULL or empty");
        handle_error();
        return;
    }
    vs_log_mod_info(
        "vsl", "Command \"run(cb=for_time, time=%f %s)\" received.",
        time_value, str_time_unit);

    if (time_value <= 0.0f) {
        vs_log_mod_error("vsl", "Invalid time value %f", time_value);
        handle_error();
        return;
    }

    /* Register callback */
    uint64_t cb_time;
    cb_time = double_to_time(time_value, str_time_unit, vx.p_context);
    cb_time += vx.p_context->time();
    if (0 > vx.register_time_callback(cb_time)) {
        handle_error();
        return;
    }

    /* Return control to simulation loop */
    vx._state = VSL_STATE_SIM_RUNNING;
    return;
}

template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run_until_time) {

    /* Error handler lambda function */
    auto handle_error = [&]() {
        vs_log_mod_warning(
            "vsl", "Error processing command run(until_time) - Discarding");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run(until_time) - Discarding");
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the time field from the JSON message content */
    cJSON* p_item_time;
    p_item_time = cJSON_GetObjectItem(vx.p_cmd, "time");
    if (nullptr == p_item_time) {
        vs_log_mod_error("vsl", "Command field \"time\" invalid/not found");
        handle_error();
        return;
    }
    double time_value;
    time_value = cJSON_GetNumberValue(p_item_time);
    if (time_value <= 0.0) {
        vs_log_mod_error("vsl", "Command field \"time\" <= 0.0");
        handle_error();
        return;
    }

    /* Get the time unit field from the JSON message content */
    cJSON* p_item_unit;
    p_item_unit = cJSON_GetObjectItem(vx.p_cmd, "time_unit");
    if (nullptr == p_item_unit) {
        vs_log_mod_error(
            "vsl", "Command field \"time_unit\" invalid/not found");
        handle_error();
        return;
    }
    char* str_time_unit;
    str_time_unit = cJSON_GetStringValue(p_item_unit);
    if ((nullptr == str_time_unit) || std::string(str_time_unit).empty()) {
        vs_log_mod_error("vsl", "Command field \"time_unit\" NULL or empty");
        handle_error();
        return;
    }
    vs_log_mod_info(
        "vsl", "Command \"run(cb=until_time, time=%f %s)\" received.",
        time_value, str_time_unit);

    uint64_t cb_time;
    cb_time = double_to_time(time_value, str_time_unit, vx.p_context);
    if (0 > vx.register_time_callback(cb_time)) {
        handle_error();
        return;
    }

    /* Return control to simulation loop */
    vx._state = VSL_STATE_SIM_RUNNING;
    return;
}

template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run_until_change) {

    /* Error handler lambda function */
    auto handle_error = [&]() {
        vs_log_mod_warning(
            "vsl", "Error processing command run(until_change) - Discarding");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run(until_change) - Discarding");
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the object path from the JSON message content */
    cJSON* p_item_path;
    p_item_path = cJSON_GetObjectItem(vx.p_cmd, "path");
    if (nullptr == p_item_path) {
        vs_log_mod_error("vsl", "Command field \"path\" invalid/not found");
        handle_error();
        return;
    }
    char* str_path;
    str_path = cJSON_GetStringValue(p_item_path);
    if ((nullptr == str_path) || std::string(str_path).empty()) {
        vs_log_mod_error("vsl", "Command field \"path\" NULL or empty");
        handle_error();
        return;
    }

    /* Check if the path corresponds to a valid, registered variable */
    VslVar* p_var = vx.get_registered_variable(std::string(str_path));
    if (nullptr == p_var) {
        vs_log_mod_error("vsl", "Could not access to variable %s", str_path);
        handle_error();
        return;
    }

    /* Get the value from the JSON message content */
    cJSON* p_item_val;
    double value;
    if (p_var->get_type() == VSL_TYPE_SCALAR) {
        p_item_val = cJSON_GetObjectItem(vx.p_cmd, "value");
        if (nullptr == p_item_val) {
            vs_log_mod_error("vsl",
                "Command field \"value\" invalid/not found");
            handle_error();
            return;
        }
        value = cJSON_GetNumberValue(p_item_val);
        if (std::isnan(value)) {
            vs_log_mod_error("vsl", "Command field \"value\" invalid (NaN)");
            handle_error();
            return;
        }
        vs_log_mod_info("vsl",
            "Command \"run(cb=until_change, path=%s, value=%f)\" received.",
            str_path, value);
    } else if (p_var->get_type() == VSL_TYPE_EVENT) {
        value = 1.0f;
        vs_log_mod_info("vsl",
            "Command \"run(cb=until_change, path=%s)\" received.", str_path);
    } else {
        vs_log_mod_error("vsl", "Variable type not supported for callback");
        handle_error();
        return;
    }

    /* Register callback */
    if (0 > vx.register_value_callback(str_path, value)) {
        handle_error();
        return;
    }

    /* Return control to simulation loop */
    vx._state = VSL_STATE_SIM_RUNNING;
    return;
}

} //namespace vsl

#endif //VSL_INTEG_CMD_RUN_HPP
//EOF

/*****************************************************************************
 @file vsl_integ_cmd_run.hpp
 @brief Command handlers implementations for "run" operations in the
 vsl::VslInteg class template.

 This header defines template implementations for handling various "run"
 commands in the vsl::VslInteg class template.

Main handlers:
   - VSL_CMD_HANDLER(run): Dispatches sub-commands based on the "cb" field in
     the JSON message.
   - VSL_CMD_HANDLER(run_for_time): Runs the simulation for a specified time
     duration.
   - VSL_CMD_HANDLER(run_to_next): Runs the simulation until the next event
     time slot.
   - VSL_CMD_HANDLER(run_until_time): Runs the simulation until a specified
     absolute simulation time.
   - VSL_CMD_HANDLER(run_until_change): Runs the simulation until a variable or
     event changes to a specified value.

 Each handler performs input validation, error handling, and registers
 appropriate callbacks to control simulation flow. The handlers interact with
 the simulation context, variable registry, and utilize cJSON for parsing
 command arguments.

 @author Jérémie Chabloz
 @copyright Copyright (c) 2024-2025 Jérémie Chabloz Distributed under the MIT
 License. See file for details.
*******************************************************************************/
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

#undef __MOD__
#define __MOD__ "vsl"

namespace vsl{

/******************************************************************************
Run command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run) {

    /* Error handler lambda function */
    VSL_ERROR_HANDLER(vx, "Error processing command run - Discarding");

    /* Get the callback field from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, cb);

    /* Look up and execute sub-command handler */
    std::string cb_key = "run_";
    cb_key.append(cstr_cb);
    auto search = vx.sub_cmd_handlers_map.find(cb_key);
    if (search != vx.sub_cmd_handlers_map.end()) {
        vx.sub_cmd_handlers_map[cb_key](vx);
        return;
    }

    /* Error case - sub-command handler function not found */
    vs_log_mod_error(__MOD__, "Handler for sub-command %s not found",
        cb_key.c_str());
    vs_msg_return(vx.fd_client_socket, "error",
        "Could not find handler for sub-command. Discarding.", &vx.uuid);
    vx._state = VSL_STATE_WAITING;
    return;
}

template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(run_for_time) {

    /* Error handler lambda function */
    auto handle_error = [&]() {
        vs_log_mod_warning(
            __MOD__, "Error processing command run(for_time) - Discarding");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run(for time) - Discarding", &vx.uuid);
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the time field from the JSON message content */
    double time_value;
    VSL_MSG_READ_NUM_NO_DECL(vx.p_cmd, time, time_value);
    if (time_value <= 0.0) {
        vs_log_mod_error(__MOD__, "Command field \"time\" <= 0.0");
        handle_error();
        return;
    }

    VSL_MSG_READ_STR(vx.p_cmd, time_unit);
    if (!check_time_unit(str_time_unit)) {
        vs_log_mod_error(
            __MOD__, "Wrong time unit identifier: %s", cstr_time_unit);
        handle_error();
        return;
    }
    vs_log_mod_info(
        __MOD__, "Command \"run(cb=for_time, time=%f %s)\" received.",
        time_value, cstr_time_unit);

    if (time_value <= 0.0f) {
        vs_log_mod_error(__MOD__, "Invalid time value %f", time_value);
        handle_error();
        return;
    }

    /* Register callback */
    uint64_t cb_time;
    cb_time = double_to_time(time_value, cstr_time_unit, vx.p_context);
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
void VslInteg<T>::VSL_CMD_HANDLER(run_to_next) {

    /* Error handler lambda function */
    auto handle_error = [&]() {
        vs_log_mod_warning(
            __MOD__, "Error processing command run(to_next) - Discarding");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run(to_next) - Discarding", &vx.uuid);
        vx._state = VSL_STATE_WAITING;
    };

    vs_log_mod_info(__MOD__, "Command \"run(cb=to_next)\" received.");

    /* Evaluate model */
    vx.eval();

    /* Check if next time slot exists */
    if (!vx.has_events_pending()) {
        vs_log_mod_warning(
            "vsl", "No pending event(s) - run(to_next) not possible");
        handle_error();
        return;
    }

    uint64_t cb_time = vx.next_event_time();
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
            __MOD__, "Error processing command run(until_time) - Discarding");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run(until_time) - Discarding", &vx.uuid);
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the time field from the JSON message content */
    double time_value;
    VSL_MSG_READ_NUM_NO_DECL(vx.p_cmd, time, time_value);
    if (time_value <= 0.0) {
        vs_log_mod_error(__MOD__, "Command field \"time\" <= 0.0");
        handle_error();
        return;
    }

    /* Get the time unit field from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, time_unit);
    if (!check_time_unit(str_time_unit)) {
        vs_log_mod_error(
            __MOD__, "Wrong time unit identifier: %s", cstr_time_unit);
        handle_error();
        return;
    }
    vs_log_mod_info(
        __MOD__, "Command \"run(cb=until_time, time=%f %s)\" received.",
        time_value, cstr_time_unit);

    uint64_t cb_time;
    cb_time = double_to_time(time_value, cstr_time_unit, vx.p_context);
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
            __MOD__, "Error processing command run(until_change) - Discarding");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command run(until_change) - Discarding",
            &vx.uuid);
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the object path from the JSON message content */
    VSL_MSG_READ_STR(vx.p_cmd, path);

    /* Check if the path corresponds to a valid, registered variable */
    VslVar* p_var = vx.get_registered_variable(str_path);
    if (nullptr == p_var) {
        vs_log_mod_error(__MOD__, "Could not access to variable %s", cstr_path);
        handle_error();
        return;
    }

    /* Get the value from the JSON message content */
    cJSON* p_item_val;
    double value;
    if (p_var->get_type() == VSL_TYPE_SCALAR) {
        p_item_val = cJSON_GetObjectItem(vx.p_cmd, "value");
        if (nullptr == p_item_val) {
            vs_log_mod_error(__MOD__,
                "Command field \"value\" invalid/not found");
            handle_error();
            return;
        }
        value = cJSON_GetNumberValue(p_item_val);
        if (std::isnan(value)) {
            vs_log_mod_error(__MOD__, "Command field \"value\" invalid (NaN)");
            handle_error();
            return;
        }
        vs_log_mod_info(__MOD__,
            "Command \"run(cb=until_change, path=%s, value=%f)\" received.",
            cstr_path, value);
    } else if (p_var->get_type() == VSL_TYPE_EVENT) {
        value = 1.0f;
        vs_log_mod_info(__MOD__,
            "Command \"run(cb=until_change, path=%s)\" received.", cstr_path);
    } else {
        vs_log_mod_error(__MOD__, "Variable type not supported for callback");
        handle_error();
        return;
    }

    /* Register callback */
    if (0 > vx.register_value_callback(cstr_path, value)) {
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

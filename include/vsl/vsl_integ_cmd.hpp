
/***************************************************************************//**
 @file vsl_integ_cmd.hpp
 @brief Command handler implementations for Verisocks integration.

 This header defines the command handler member functions for the
 `vsl::VslInteg` template class, which process various simulation control
 commands received via JSON messages. Supported commands include "info",
 "exit", "stop", "finish", and a generic handler for unsupported commands.

 Each handler function processes the incoming command, performs the required
 simulation or system actions, sends appropriate acknowledgements or error
 messages back to the client, and updates the simulation state accordingly.

 Command Handlers:
 - info:   Logs and acknowledges informational messages from the client.
 - exit:   Gracefully terminates the simulation and exits Verisocks.
 - stop:   Pauses or stops the simulation, awaiting further commands.
 - finish: Signals simulation termination and finalizes the model.
 - not_supported: Handles unsupported commands with a warning response.

 Dependencies:
 - cJSON for JSON parsing
 - Verilator simulation context and model
 - Verisocks server, logging, and messaging utilities

 @author Jérémie Chabloz
 @copyright Copyright (c) 2024-2025 Jérémie Chabloz Distributed under the MIT
 License. See file for details.
 ******************************************************************************/
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

#ifndef VSL_INTEG_CMD_HPP
#define VSL_INTEG_CMD_HPP

#include "cJSON.h"
#include "vs_server.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "verilated.h"

#include <cstdio>
#include <string>
#include <cmath>


namespace vsl{

/******************************************************************************
Info command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(info) {
    char *str_val;

    vs_log_mod_info("vsl", "Command \"info\" received");

    auto handle_error = [&vx]()
    {
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command info - Discarding", &vx.uuid);
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the value from the JSON message content */
    cJSON *p_item_val = cJSON_GetObjectItem(vx.p_cmd, "value");
    if (nullptr == p_item_val) {
        vs_log_mod_error("vsl", "Command field \"value\" invalid/not found");
        handle_error();
        return;
    }

    /* Get the info command argument */
    str_val = cJSON_GetStringValue(p_item_val);
    if ((nullptr == str_val) || std::string(str_val).empty()) {
        vs_log_mod_error("vsl", "Command field \"value\" NULL or empty");
        handle_error();
        return;
    }

    /* Print received info value */
    vs_log_info("%s", str_val);

    /* Return an acknowledgement */
    vs_msg_return(vx.fd_client_socket, "ack", "command info received", &vx.uuid);

    /* Set state to "waiting next command" */
    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Exit command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(exit) {
    vs_log_mod_info(
        "vsl", "Command \"exit\" received. Quitting Verisocks ...");
    vs_msg_return(vx.fd_client_socket, "ack",
        "Processing exit command - Quitting Verisocks.", &vx.uuid);

    /* Simulate until $finish */
    while (!vx.p_context->gotFinish()) {
        vx.p_model->eval();
        if (!vx.p_model->eventsPending()) break;
        vx.p_context->time(vx.p_model->nextTimeSlot());
    }

    if (!vx.p_context->gotFinish()) {
        vs_log_mod_debug("vsl", "Exiting without $finish; no events left");
    }
    vx.p_model->final();
    vx._state = VSL_STATE_EXIT;
    return;
}

/******************************************************************************
Stop command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(stop) {
    vs_log_mod_info(
        "vsl", "Command \"stop\" received");
    vs_msg_return(vx.fd_client_socket, "ack",
        "Processing stop command - Simulation stopped/paused", &vx.uuid);

    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Finish command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(finish) {
    vs_log_mod_info(
        "vsl", "Command \"finish\" received. Terminating simulation...");
    vs_msg_return(vx.fd_client_socket, "ack",
        "Processing finish command - Terminating simulation.", &vx.uuid);

    vx.p_context->gotFinish(true);
    vx.p_model->final();
    vx._state = VSL_STATE_EXIT;
    return;
}

/******************************************************************************
Not supported
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(not_supported) {
    vs_msg_return(vx.fd_client_socket, "warning",
        "This command is not (yet) supported. Discarding...", &vx.uuid);
    vx._state = VSL_STATE_WAITING;
    return;
}


} //namespace vsl

#endif //VSL_INTEG_CMD_HPP
//EOF

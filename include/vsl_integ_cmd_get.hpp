/*
MIT License

Copyright (c) 2024 Jérémie Chabloz

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
#include "vs_server.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "verilated.h"

#include <cstdio>
#include <string>
#include <cmath>


namespace vsl{

/******************************************************************************
Get command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(get) {
    char *str_sel;

    auto handle_error = [&vx]() {
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command get - Discarding");
        vx._state = VSL_STATE_WAITING;
    };

    /* Get the value from the JSON message content */
    cJSON *p_item_sel = cJSON_GetObjectItem(vx.p_cmd, "sel");
    if (nullptr == p_item_sel) {
        vs_log_mod_error("vsl", "Command field \"sel\" invalid/not found");
        handle_error();
        return;
    }

    /* Get the info command argument as a string */
    str_sel = cJSON_GetStringValue(p_item_sel);
    if ((nullptr == str_sel) || std::string(str_sel).empty()) {
        vs_log_mod_error("vsl", "Command field \"sel\" NULL or empty");
        handle_error();
        return;
    }
    vs_log_mod_info("vsl", "Command \"get(sel=%s)\" received.", str_sel);

    /* Look up and execute sub-command handler */
    std::string sel_key = "get_";
    sel_key.append(str_sel);
    auto search = vx.sub_cmd_handlers_map.find(sel_key);
    if (search != vx.sub_cmd_handlers_map.end()) {
        vx.sub_cmd_handlers_map[sel_key](vx);
        return;
    }

    /* Error case - sub-command handler function not found */
    vs_log_mod_error("vsl", "Handler for sub-command %s not found",
        sel_key.c_str());
    vs_msg_return(vx.fd_client_socket, "error",
        "Could not find handler for command. Discarding.");
    vx._state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Get sim_info sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(get_sim_info) {
    cJSON *p_msg;
    char *str_msg = nullptr;

    /* Lambda function - error handler */
    auto handle_error = [&](){
        if (nullptr != p_msg) cJSON_Delete(p_msg);
        if (nullptr != str_msg) cJSON_free(str_msg);
        vx._state = VSL_STATE_WAITING;
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command get(sel=sim_info) - Discarding");
    };

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (nullptr == p_msg) {
        vs_log_mod_error("vsl", "Could not create cJSON object");
        handle_error();
        return;
    }
    if (nullptr == cJSON_AddStringToObject(p_msg, "type", "result")) {
        vs_log_mod_error("vsl", "Could not add string to object");
        handle_error();
        return;
    }
    if (nullptr == cJSON_AddStringToObject(p_msg, "product",
                                           Verilated::productName())) {
        vs_log_mod_error("vsl", "Could not add string to object");
        handle_error();
        return;
    }
    if (nullptr == cJSON_AddStringToObject(p_msg, "version",
                                           Verilated::productVersion())) {
        vs_log_mod_error("vsl", "Could not add string to object");
        handle_error();
        return;
    }

    //TODO: Add error management
    cJSON_AddStringToObject(p_msg, "model_name", vx.p_model->modelName());
    cJSON_AddStringToObject(p_msg, "model_hier_name", vx.p_model->hierName());
    cJSON_AddNumberToObject(p_msg, "time_unit", vx.p_context->timeunit());
    cJSON_AddNumberToObject(
        p_msg, "time_precision", vx.p_context->timeprecision());

    str_msg = vs_msg_create_message(
        p_msg, vs_msg_info_t{VS_MSG_TXT_JSON, 0});

    if (nullptr == str_msg) {
        vs_log_mod_error("vsl", "NULL pointer");
        handle_error();
        return;
    }
    if (0 > vs_msg_write(vx.fd_client_socket, str_msg)) {
        vs_log_mod_error("vsl", "Error writing return message");
        handle_error();
        return;
    }

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

    /* Lambda function - error handler */
    auto handle_error = [&](){
        if (nullptr != p_msg) cJSON_Delete(p_msg);
        if (nullptr != str_msg) cJSON_free(str_msg);
        vx._state = VSL_STATE_WAITING;
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command get(sel=sim_time) - Discarding");
    };

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (nullptr == p_msg) {
        vs_log_mod_error("vsl", "Could not create cJSON object");
        handle_error();
        return;
    }
    if (nullptr == cJSON_AddStringToObject(p_msg, "type", "result")) {
        vs_log_mod_error("vsl", "Could not add string to object");
        handle_error();
        return;
    }

    vs_log_mod_debug("vsl", "Getting simulator time...");
    auto time = vx.p_context->time();
    auto timeprecision = vx.p_context->timeprecision();
    double sim_time_sec = time * std::pow(10.0, timeprecision);
    vs_log_mod_debug("vsl", "Sim time: %.6f us", sim_time_sec*1.0e6);

    if (nullptr == cJSON_AddNumberToObject(p_msg, "time",
        sim_time_sec)) {
        vs_log_mod_error("vsl", "Could not add number to object");
        handle_error();
        return;
    }

    str_msg = vs_msg_create_message(p_msg, vs_msg_info_t{VS_MSG_TXT_JSON, 0});

    if (nullptr == str_msg) {
        vs_log_mod_error("vsl", "nullptr pointer");
        handle_error();
        return;
    }
    if (0 > vs_msg_write(vx.fd_client_socket, str_msg)) {
        vs_log_mod_error("vsl", "Error writing return message");
        handle_error();
        return;
    }

    /* Normal exit */
    if (nullptr != p_msg) cJSON_Delete(p_msg);
    if (nullptr != str_msg) cJSON_free(str_msg);
    vx._state = VSL_STATE_WAITING;
    return;
}

} //namespace vsl

#endif //VSL_INTEG_CMD_GET_HPP
//EOF

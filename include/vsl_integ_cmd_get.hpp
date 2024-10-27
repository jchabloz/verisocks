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
    if (nullptr == cJSON_AddStringToObject(p_msg, "model_name",
                                           vx.p_model->modelName())) {
        vs_log_mod_error("vsl", "Could not add string to object");
        handle_error();
        return;
    }
    if (nullptr == cJSON_AddStringToObject(p_msg, "model_hier_name",
                                vx.p_model->hierName())) {
        vs_log_mod_error("vsl", "Could not add string to object");
        handle_error();
        return;
    }
    if (nullptr == cJSON_AddNumberToObject(p_msg, "time_unit",
                                           vx.p_context->timeunit())) {
        vs_log_mod_error("vsl", "Could not add number to object");
        handle_error();
        return;
    }
    if (nullptr == cJSON_AddNumberToObject(p_msg, "time_precision",
                                           vx.p_context->timeprecision())) {
        vs_log_mod_error("vsl", "Could not add number to object");
        handle_error();
        return;
    }
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

    /* Get simulator time in seconds from Verilator context */
    vs_log_mod_debug("vsl", "Getting simulator time...");
    auto time = vx.p_context->time();
    auto timeprecision = vx.p_context->timeprecision();
    double sim_time_sec = time * std::pow(10.0, timeprecision);
    vs_log_mod_debug("vsl", "Sim time: %.6e s", sim_time_sec);

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

/******************************************************************************
Get value sub-command handler
******************************************************************************/
template<typename T>
void VslInteg<T>::VSL_CMD_HANDLER(get_value) {

    cJSON *p_msg;
    cJSON *p_item_path;
    char *str_msg = nullptr;
    char *cstr_path = nullptr;

    /* Lambda function - error handler */
    auto handle_error = [&](){
        if (nullptr != p_msg) cJSON_Delete(p_msg);
        if (nullptr != str_msg) cJSON_free(str_msg);
        if (nullptr != cstr_path) cJSON_free(cstr_path);
        vx._state = VSL_STATE_WAITING;
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command get(sel=value) - Discarding");
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

    /* Get the object path from the JSON message content */
    p_item_path = cJSON_GetObjectItem(vx.p_cmd, "path");
    if (nullptr == p_item_path) {
        vs_log_mod_error("vsl", "Command field \"path\" invalid/not found");
        handle_error();
        return;
    }
    cstr_path = cJSON_GetStringValue(p_item_path);
    if (nullptr == cstr_path) {
        vs_log_mod_error("vsl", "Command field \"path\" NULL");
        handle_error();
        return;
    }
    std::string str_path(cstr_path);
    if (str_path.empty()) {
        vs_log_mod_error("vsl", "Command field \"path\" empty");
        handle_error();
        return;
    }

    /* Attempt to get a pointer to the variable */
    std::string str_scope;
    std::string str_var;
    if (str_path.find_last_of(".") != str_path.npos) {
        str_scope = str_path.substr(0, str_path.find_last_of("."));
        str_var = str_path.substr(str_path.find_last_of(".") + 1);
    } else {
        str_scope = "";
        str_var = str_path;
    }

    str_scope = vx.p_model->hierName() + str_scope;
    auto p_xscope = vx.p_context->scopeFind(str_scope.c_str());
    if (nullptr == p_xscope) {
        vs_log_mod_error(
            "vsl", "Scope %s not found in context", str_scope.c_str());
        handle_error();
        return;
    }

    auto p_xvar = p_xscope->varFind(str_var.c_str());
    if (nullptr == p_xvar) {
        vs_log_mod_error(
            "vsl", "Variable %s not found in scope %s",
            str_var.c_str(), str_scope.c_str());
        handle_error();
        return;
    }


}

} //namespace vsl

#endif //VSL_INTEG_CMD_GET_HPP
//EOF

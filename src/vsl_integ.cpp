/*
MIT License

Copyright (c) 2024 Jérémie Chabloz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

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

#include "vsl_integ.h"

#include "vsl_integ_cmd.h"
#include "vs_server.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "cJSON.h"

#include <string>

//#include "verilated.h"
//#include "verilated_vpi.h"

namespace vsl{

/* Constructor */
VslInteg::VslInteg(const int port, const int timeout) {
    num_port = port;
    num_timeout_sec = timeout;
    VSL_CMD_HANDLER_MAP(info);
    //VSL_CMD_HANDLER_MAP(finish);
    //VSL_CMD_HANDLER_MAP(stop);
    //VSL_CMD_HANDLER_MAP(exit);
    //VSL_CMD_HANDLER_MAP(run);
    //VSL_CMD_HANDLER_MAP(get);
    //VSL_CMD_HANDLER_MAP(set);
    return;
}


/* Destructor */
VslInteg::~VslInteg() {
    if (0 < fd_server_socket) vs_server_close_socket(fd_server_socket);
    if (nullptr != p_cmd) cJSON_Delete(p_cmd);
    return;
}


void VslInteg::run() {
    printf("******************************************\n");
    printf("*  __   __       _             _         *\n");
    printf("*  \\ \\ / /__ _ _(_)___ ___  __| |__ ___  *\n");
    printf("*   \\ V / -_) '_| (_-</ _ \\/ _| / /(_-<  *\n");
    printf("*    \\_/\\___|_| |_/__/\\___/\\__|_\\_\\/__/  *\n");
    printf("*                                        *\n");
    printf("*         Verilator integration          *\n");
    printf("*   Copyright (c) 2024 Jérémie Chabloz   *\n");
    printf("******************************************\n");

    while(true) {
        switch (_state) {
        case VSL_STATE_INIT:
            main_init();
            break;
        case VSL_STATE_CONNECT:
            main_connect();
            break;
        case VSL_STATE_WAITING:
            main_wait();
            break;
        case VSL_STATE_PROCESSING:
            main_process();
            if (_state == VSL_STATE_PROCESSING) {
                _state = VSL_STATE_WAITING;
            }
            break;
        case VSL_STATE_SIM_RUNNING:
            main_sim();
            break;
        case VSL_STATE_EXIT:
            if (0 <= fd_server_socket) {
                vs_server_close_socket(fd_server_socket);
                fd_server_socket = -1;
                _is_connected = false;
            }
            return;
        case VSL_STATE_ERROR:
        default:
            vs_log_mod_error("vsl",
                "Exiting Verisocks main loop (error state)");
            if (0 <= fd_server_socket) {
                vs_server_close_socket(fd_server_socket);
                fd_server_socket = -1;
                _is_connected = false;
            }
            return;
        } //switch (_state)
    }
    return;
}


void VslInteg::main_init() {

    /* Check state consistency */
    if (_state != VSL_STATE_INIT) {
        vs_log_mod_error("vsl", "Wrong state in init function %d", _state);
        _state = VSL_STATE_ERROR;
        return;
    }

    /* Create server socket */
    fd_server_socket = vs_server_make_socket(num_port);
    if (0 > fd_server_socket) {
        vs_log_mod_error("vsl", "Issue making socket at port %d", num_port);
        _state = VSL_STATE_ERROR;
        return;
    }

    /* Get server socket address */
    vs_sock_addr_t socket_address = vs_server_get_address(fd_server_socket);

    /* Logs server address and port number */
    vs_log_mod_info("vsl", "Server address: %d.%d.%d.%d",
        (socket_address.address & 0xff000000) >> 24u,
        (socket_address.address & 0x00ff0000) >> 16u,
        (socket_address.address & 0x0000ff00) >> 8u,
        (socket_address.address & 0x000000ff)
    );
    vs_log_mod_info("vsl", "Port: %d", socket_address.port);

    /* Update state */
    _state = VSL_STATE_CONNECT;
    return;
}


void VslInteg::main_connect() {
    char hostname_buffer[128];
    struct timeval timeout;
    timeout.tv_sec = num_timeout_sec;
    timeout.tv_usec = 0;

    vs_log_mod_debug(
        "vsl",
        "Waiting for a client to connect (%ds timeout) ...",
        (int) timeout.tv_sec);
    fd_client_socket = vs_server_accept(
        fd_server_socket, hostname_buffer, sizeof(hostname_buffer), &timeout);
    if (0 > fd_client_socket) {
        vs_log_mod_error("vsl", "Failed to connect");
        _state = VSL_STATE_ERROR;
        return;
    }
    vs_log_mod_info("vsl", "Connected to %s", hostname_buffer);
    _state = VSL_STATE_WAITING;
    return;
}


void VslInteg::main_wait() {
    char read_buffer[4096];
    int msg_len;
    msg_len = vs_msg_read(fd_client_socket,
                          read_buffer,
                          sizeof(read_buffer));
    if (0 > msg_len) {
        vs_server_close_socket(fd_client_socket);
        fd_client_socket = -1;
        vs_log_mod_debug(
            "vsl",
            "Lost connection. Waiting for a client to (re-)connect ..."
        );
        _state = VSL_STATE_CONNECT;
        return;
    }
    if (msg_len >= (int) sizeof(read_buffer)) {
        read_buffer[sizeof(read_buffer) - 1] = '\0';
        vs_log_mod_warning(
            "vsl",
            "Received message longer than RX buffer, discarding it"
        );
        vs_msg_return(fd_client_socket, "error",
            "Message too long - Discarding");
        return;
    }
    else {
        read_buffer[msg_len] = '\0';
    }
    vs_log_mod_debug("vsl", "Message: %s", &read_buffer[2]);
    if (nullptr != p_cmd) {
        cJSON_Delete(p_cmd);
    }
    p_cmd = vs_msg_read_json(read_buffer);
    if (nullptr != p_cmd) {
        _state = VSL_STATE_PROCESSING;
        return;
    }
    vs_log_mod_warning(
        "vsl",
        "Received message content cannot be interpreted as a valid JSON \
content. Discarding it."
    );
    vs_msg_return(fd_client_socket, "error",
        "Invalid message content - Discarding");
    return;
}


void VslInteg::main_process() {
    std::string str_cmd;
    cJSON *p_item_cmd;

    /* Get the command field from the JSON message content */
    p_item_cmd = cJSON_GetObjectItem(p_cmd, "command");
    if (nullptr == p_item_cmd) {
        vs_log_mod_error("vsl", "Command field invalid/not found");
        vs_msg_return(fd_client_socket, "error",
            "Error processing command. Discarding.");
        _state = VSL_STATE_WAITING;
        return;
    }

    /* Get the command as a string */
    str_cmd = std::string(cJSON_GetStringValue(p_item_cmd));
    if (str_cmd.empty() == true) {
        vs_log_mod_error("vsl", "Command field empty/null");
        vs_msg_return(fd_client_socket, "error",
            "Error processing command. Discarding.");
        _state = VSL_STATE_WAITING;
        return;
    }
    vs_log_mod_debug("vsl", "Processing command %s", str_cmd.c_str());

    /* Look up command handler */
    auto search = cmd_handlers_map.find(str_cmd);
    if (search != cmd_handlers_map.end()) {
        cmd_handlers_map[str_cmd](*this);
        _state = VSL_STATE_WAITING;
        return;
    }

    vs_log_mod_error("vsl", "Handler for command %s not found",
        str_cmd.c_str());
    vs_msg_return(fd_client_socket, "error",
        "Could not find handler for command. Discarding.");
    _state = VSL_STATE_WAITING;
    return;
}


void VslInteg::main_sim() {
    vs_log_mod_info("vsl", "Simulation ongoing"); 
    //TODO - implement Verilator simulation code
    return;
}

} //namespace vsl

//EOF

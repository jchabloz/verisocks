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

#ifndef VSL_INTEG_HPP
#define VSL_INTEG_HPP

#include "cJSON.h"
#include "vs_server.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "verilated.h"
#include "verilated_syms.h"
#include "vsl/vsl_types.hpp"

#include <cstdio>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>


/**
 * @brief Helper macro to declare a command handler function
 * @param cmd Command short name
 */
#define VSL_CMD_HANDLER(cmd) VSL_ ## cmd ## _cmd_handler(VslInteg& vx)
#define VSL_CMD_HANDLER_NAME(cmd) VSL_ ## cmd ## _cmd_handler


namespace vsl{

enum VslState {
    VSL_STATE_INIT,        ///Initial state, Verisocks server socket not initialized
    VSL_STATE_CONNECT,     ///Socket created, bound to address, waiting for connection
    VSL_STATE_WAITING,     ///Connected and waiting to receive next command
    VSL_STATE_PROCESSING,  ///Processing a command
    VSL_STATE_SIM_RUNNING, ///Simulation running
    VSL_STATE_EXIT,        ///Exiting Verisocks
    VSL_STATE_ERROR        ///Error state (e.g. timed out while waiting for a connection)
};

template<typename T>
class VslInteg {

public:

    /**
     * @brief Constructs a VslInteg object.
     * 
     * @tparam T The type of the model.
     * @param p_model Pointer to the model object.
     * @param port The port number to be used. Default is 5100.
     * @param timeout The timeout duration in seconds. Default is 120.
     */
    VslInteg(T* p_model, const int port=5100, const int timeout=120);
    ~VslInteg();

    void run();

    void register_variable(const char* namep, std::any datap,
        VerilatedVarType vltype, VslType type, size_t dims, size_t width,
        size_t depth) {
            var_map.add_var(namep, datap, vltype, type, dims, width, depth);
    }
    void register_variable(const char* namep, std::any datap,
        VerilatedVarType vltype, VslType type, size_t width, size_t depth) {
            var_map.add_var(namep, datap, vltype, type, 2u, width, depth);
    }
    void register_variable(const char* namep, std::any datap,
        VerilatedVarType vltype, VslType type, size_t width) {
            var_map.add_var(namep, datap, vltype, type, 0, width, 0u);
    }
    void register_scalar(const char* namep, std::any datap,
        VerilatedVarType vltype, size_t width) {
            register_variable(namep, datap, vltype, VSL_TYPE_SCALAR, width);
    }
    void register_array(const char* namep, std::any datap,
        VerilatedVarType vltype, size_t width, size_t depth) {
            register_variable(
                namep, datap, vltype, VSL_TYPE_ARRAY, width, depth);
    }
    void register_event(const char* namep, VlEvent* eventp) {
        register_variable(namep, eventp, VLVT_UINT8, VSL_TYPE_EVENT, 1u);
    }

private:
    VslState _state {VSL_STATE_INIT}; //Verisocks state
    cJSON* p_cmd {nullptr}; //Pointer to current/latest command

    /* Command handler functions map */
    std::unordered_map<std::string, std::function<void(VslInteg&)>>
    cmd_handlers_map {};

    /* Sub-command handler functions map */
    std::unordered_map<std::string, std::function<void(VslInteg&)>>
    sub_cmd_handlers_map {};

    VslVarMap var_map {};         //Map of Verilator variables

    T* p_model;                   //Pointer to verilated model instance
    VerilatedContext* p_context;  //Pointer to Verilator context
    int num_port {5100};          //Port number
    int num_timeout_sec {120};    //Timeout, in seconds
    int fd_server_socket {-1};    //File descriptor, server socket
    int fd_client_socket {-1};    //File descriptor, connected client socket
    bool _is_connected {false};   //Socket connection status

    /* State machine functions */
    void main_init();
    void main_connect();
    void main_wait();
    void main_process();
    void main_sim();

    /* Utility functions */
    VslVar* get_registered_variable(std::string str_path) {
        return var_map.get_var(str_path);
    }

    /* Get a pointer for a given public variable */
    VerilatedVar* get_var(std::string str_path);

    /* Declaration of command handlers functions */
    /*
    In order to be able to insert functions in a command handlers function map,
    they need to be declared as static functions! Alternatively, they could
    also be declared as friend functions and then be defined outside of the
    VslInteg class, however using a template class makes it a lot more
    difficult. Note that, since static functions cannot access non-static
    members, we anyway have to pass in parameter a reference of the class
    instance to which it has to apply, thus the benefit of encapsulating the
    handler functions as class members is limited to say the least.
    */
    static void VSL_CMD_HANDLER(info);
    static void VSL_CMD_HANDLER(get);
    static void VSL_CMD_HANDLER(get_sim_info);
    static void VSL_CMD_HANDLER(get_sim_time);
    static void VSL_CMD_HANDLER(get_value);
    static void VSL_CMD_HANDLER(finish);
    static void VSL_CMD_HANDLER(stop);
    static void VSL_CMD_HANDLER(exit);
    static void VSL_CMD_HANDLER(run);
    static void VSL_CMD_HANDLER(run_for_time);
    static void VSL_CMD_HANDLER(run_until_time);
    static void VSL_CMD_HANDLER(run_until_change);
    // static void VSL_CMD_HANDLER(run_to_next);
    static void VSL_CMD_HANDLER(set);
    static void VSL_CMD_HANDLER(not_supported);
};

/******************************************************************************
Constructor
******************************************************************************/
template<typename T>
VslInteg<T>::VslInteg(T* p_model, const int port, const int timeout) {
    vs_log_mod_debug("vsl", "Constructor called (%s)", __FILE__);

    /* Check that the type parameter corresponds to a verilated model */
    static_assert(std::is_base_of<VerilatedModel,T>::value,
        "Error, expecting a derived class of VerilatedModel");

    this->p_model = p_model;
    p_context = p_model->contextp();
    num_port = port;
    num_timeout_sec = timeout;

    // Add commands handler functions to the relevant maps
    cmd_handlers_map["info"]   = VSL_CMD_HANDLER_NAME(info);
    cmd_handlers_map["get"]    = VSL_CMD_HANDLER_NAME(get);
    cmd_handlers_map["set"]    = VSL_CMD_HANDLER_NAME(set);
    cmd_handlers_map["run"]    = VSL_CMD_HANDLER_NAME(run);
    cmd_handlers_map["finish"] = VSL_CMD_HANDLER_NAME(finish);
    cmd_handlers_map["stop"]   = VSL_CMD_HANDLER_NAME(stop);
    cmd_handlers_map["exit"]   = VSL_CMD_HANDLER_NAME(exit);

    // Add sub-commands handler functions to the relevant maps
    sub_cmd_handlers_map["get_sim_info"]     = VSL_CMD_HANDLER_NAME(get_sim_info);
    sub_cmd_handlers_map["get_sim_time"]     = VSL_CMD_HANDLER_NAME(get_sim_time);
    sub_cmd_handlers_map["get_type"]         = VSL_CMD_HANDLER_NAME(not_supported);
    sub_cmd_handlers_map["get_value"]        = VSL_CMD_HANDLER_NAME(get_value);
    sub_cmd_handlers_map["run_for_time"]     = VSL_CMD_HANDLER_NAME(run_for_time);
    sub_cmd_handlers_map["run_until_time"]   = VSL_CMD_HANDLER_NAME(run_until_time);
    sub_cmd_handlers_map["run_until_change"] = VSL_CMD_HANDLER_NAME(run_until_change);
    return;
}

/******************************************************************************
Destructor
******************************************************************************/
template<typename T>
VslInteg<T>::~VslInteg() {
    vs_log_mod_debug("vsl", "Destructor called (%s)", __FILE__);
    if (0 < fd_server_socket) vs_server_close_socket(fd_server_socket);
    if (nullptr != p_cmd) cJSON_Delete(p_cmd);
    return;
}

/******************************************************************************
Finite state-machine
******************************************************************************/
template<typename T>
void VslInteg<T>::run() {
    std::printf("*******************************************\n");
    std::printf("*  __   __       _             _          *\n");
    std::printf("*  \\ \\ / /__ _ _(_)___ ___  __| |__ ___   *\n");
    std::printf("*   \\ V / -_) '_| (_-</ _ \\/ _| / /(_-<   *\n");
    std::printf("*    \\_/\\___|_| |_/__/\\___/\\__|_\\_\\/__/   *\n");
    std::printf("*                                         *\n");
    std::printf("*          Verilator integration          *\n");
    std::printf("* Copyright (c) 2024-2025 Jérémie Chabloz *\n");
    std::printf("*******************************************\n");

    while(true) {
        switch (_state) {
        case VSL_STATE_INIT:
            p_model->eval(); //Initial model evaluation
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

/******************************************************************************
Main finite state-machine - Initialization
******************************************************************************/
template<typename T>
void VslInteg<T>::main_init() {

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

/******************************************************************************
Main finite state-machine - Waiting for connection
******************************************************************************/
template<typename T>
void VslInteg<T>::main_connect() {
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

/******************************************************************************
Main finite state-machine - Waiting for command
******************************************************************************/
template<typename T>
void VslInteg<T>::main_wait() {
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

/******************************************************************************
Main finite state-machine - Process command
******************************************************************************/
template<typename T>
void VslInteg<T>::main_process() {
    char *c_str_cmd;
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
    c_str_cmd = cJSON_GetStringValue(p_item_cmd);
    if (nullptr == c_str_cmd) {
        vs_log_mod_error("vsl", "Command field invalid");
        vs_msg_return(fd_client_socket, "error",
            "Error processing command. Discarding.");
        _state = VSL_STATE_WAITING;
        return;
    }

    str_cmd = std::string(c_str_cmd);
    if (str_cmd.empty() == true) {
        vs_log_mod_error("vsl", "Command field empty/null");
        vs_msg_return(fd_client_socket, "error",
            "Error processing command. Discarding.");
        _state = VSL_STATE_WAITING;
        return;
    }
    vs_log_mod_debug("vsl", "Processing command %s", str_cmd.c_str());

    /* Look up and execute command handler */
    auto search = cmd_handlers_map.find(str_cmd);
    if (search != cmd_handlers_map.end()) {
        cmd_handlers_map[str_cmd](*this);
        return;
    }

    /* Handle case for which the command handler is not found */
    vs_log_mod_error("vsl", "Handler for command %s not found",
        str_cmd.c_str());
    vs_msg_return(fd_client_socket, "error",
        "Could not find handler for command. Discarding.");
    _state = VSL_STATE_WAITING;
    return;
}

/******************************************************************************
Main finite state-machine - Simulation ongoing
******************************************************************************/
template<typename T>
void VslInteg<T>::main_sim() {
    vs_log_mod_info("vsl", "Simulation ongoing");
    //TODO - implement Verilator simulation code
    return;
}

/******************************************************************************
Utility functions
******************************************************************************/
/**
 * @brief Get Verilated variable from path
 * 
 * @param str_path Variable path
 * @return (VerilatedVar*) Pointer to variable
 */
template<typename T>
VerilatedVar* VslInteg<T>::get_var(std::string str_path) {

    /* Known limitations/issues
    - Events cannot be distinguished!
    - VerilatedVar uses void pointers with concerns about type safety
    */

    /* Extract scope and variable paths */
    std::string str_scope(p_model->hierName());
    std::string str_var;
    if (str_path.find_last_of(".") != str_path.npos) {
        str_scope += std::string(".");
        str_scope += str_path.substr(0, str_path.find_last_of("."));
        str_var = str_path.substr(str_path.find_last_of(".") + 1);
    } else {
        str_var = str_path;
    }

    const VerilatedScope* p_xscope = p_context->scopeFind(str_scope.c_str());
    if (nullptr == p_xscope) {
        vs_log_mod_error("vsl", "Could not find scope %s", str_scope.c_str());
        return nullptr;
    }
    VerilatedVar* p_xvar = p_xscope->varFind(str_var.c_str());
    if (nullptr == p_xvar) {
        vs_log_mod_error("vsl", "Could not find variable %s", str_var.c_str());
        return nullptr;
    }
    return p_xvar;
}

} //namespace vsl

#endif //VSL_INTEG_HPP
//EOF

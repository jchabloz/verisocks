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

/**
 * @enum VslState
 * @brief Internal state values for the VSL finite-state machine
 */
enum VslState {
    VSL_STATE_INIT,        ///Initial state, Verisocks server socket not initialized
    VSL_STATE_CONNECT,     ///Socket created, bound to address, waiting for connection
    VSL_STATE_WAITING,     ///Connected and waiting to receive next command
    VSL_STATE_PROCESSING,  ///Processing a command
    VSL_STATE_SIM_RUNNING, ///Simulation running
    VSL_STATE_SIM_FINISH,  ///Finish simulation
    VSL_STATE_EXIT,        ///Exiting Verisocks
    VSL_STATE_ERROR        ///Error state (e.g. timed out while waiting for a connection)
};

/**
 * @class VslInteg
 * @brief Top-level class to use for using Verisocks with Verilator
 *
 * @tparam T Model class
 * @param p_model Pointer to the Verilated model instance.
 * @param port The port number to be used. Default is 5100.
 * @param timeout The timeout duration in seconds. Default is 120.
 */
template<typename T>
class VslInteg {

public:

    VslInteg(T* p_model, const int port=5100, const int timeout=120);
    ~VslInteg();

    /**
     * @brief Returns a pointer to the registered model instance
     */
    inline const T* model() {return p_model;}

    /**
     * @brief Returns a pointer to the registered Verilator context
     */
    inline const VerilatedContext* context() {return p_context;}

    /**
     * @brief Run Verisocks FSM
     *
     * This is the main function to be used from the top-level C++ testbench
     * code. It will launch the Verisocks finite-state machine and will expect
     * a client to connect to the exposed socket. Once connected, it will be
     * expecting to receive Verisocks commands to control the Verilated
     * testbench simulation.
     */
    int run();

    /**
     * @brief Register a scalar variable
     *
     * This function shall be used from the top-level C++ testbench code in
     * order to register a scalar variable to be accessible with Verisocks
     * commands.
     *
     * @param namep Name of the variable as registered by Verisocks and how it
     * will be used within Verisocks commands
     * @param datap Pointer to the variable in the verilated C++ code
     * @param vltype Variable verilated type
     * @param width Variable width (should be consistent with vltype)
     */
    inline void register_scalar(const char* namep, std::any datap,
        VerilatedVarType vltype, size_t width) {
            register_variable(namep, datap, vltype, VSL_TYPE_SCALAR, width);
    }

    /**
     * @brief Register a parameter variable
     *
     * This function shall be used from the top-level C++ testbench code in
     * order to register a parameter variable to be accessible with Verisocks
     * commands.
     *
     * @param namep Name of the parameter variable as registered by Verisocks
     * and how it will be used within Verisocks commands
     * @param datap Pointer to the parameter variable in the verilated C++ code
     * @param vltype Variable verilated type
     * @param width Variable width (should be consistent with vltype)
     */
    inline void register_param(const char* namep, std::any datap,
        VerilatedVarType vltype, size_t width) {
            register_variable(namep, datap, vltype, VSL_TYPE_PARAM, width);
    }

    /**
     * @brief Register an array variable
     *
     * This function shall be used from the top-level C++ testbench code in
     * order to register an array variable to be accessible with Verisocks
     * commands.
     *
     * @param namep Name of the array variable as registered by Verisocks and
     * how it will be used within Verisocks commands
     * @param datap Pointer to the array variable in the verilated C++ code
     * @param vltype Variable verilated type
     * @param width Variable width (should be consistent with vltype)
     * @param depth Array depth
     */
    inline void register_array(const char* namep, std::any datap,
        VerilatedVarType vltype, size_t width, size_t depth) {
            register_variable(
                namep, datap, vltype, VSL_TYPE_ARRAY, width, depth);
    }

    /**
     * @brief Register an event variable
     *
     * This function shall be used from the top-level C++ testbench code in
     * order to register an event variable to be accessible with Verisocks
     * commands.
     *
     * @param namep Name of the event variable as registered by Verisocks and
     * how it will be used within Verisocks commands
     * @param eventp Pointer to the event variable in the verilated C++ code
     */
    inline void register_event(const char* namep, VlEvent* eventp) {
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

    VslVarMap var_map {};          //Map of Verilator variables

    T* p_model;                    //Pointer to verilated model instance
    VerilatedContext* p_context;   //Pointer to Verilator context
    int num_port {5100};           //Port number
    int num_timeout_sec {120};     //Timeout, in seconds
    int fd_server_socket {-1};     //File descriptor, server socket
    int fd_client_socket {-1};     //File descriptor, connected client socket
    bool _is_connected {false};    //Socket connection status

    /* Callbacks management */
    bool b_has_time_callback {false};
    bool b_has_value_callback {false};
    uint64_t cb_time {0u};
    std::string cb_value_path;
    double cb_value {0.0f};

    /* State machine functions */
    void main_init();
    void main_connect();
    void main_wait();
    void main_process();
    void main_sim();
    void main_sim_finish();

    /* Function to register variables into the Verisocks variable map */
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

    /* Callback functions*/
    int register_value_callback(const char* path, const double value);
    int register_time_callback(const uint64_t time);
    void clear_callbacks() {
        b_has_time_callback = false;
        b_has_value_callback = false;
    }

    inline const bool has_callback() {
        return (b_has_value_callback || b_has_time_callback);
    }
    inline const bool has_value_callback() {return b_has_value_callback;}
    inline const bool has_time_callback() {return b_has_time_callback;}
    const bool check_value_callback();

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
    static void VSL_CMD_HANDLER(run_to_next);
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
    sub_cmd_handlers_map["run_to_next"]      = VSL_CMD_HANDLER_NAME(run_to_next);
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
int VslInteg<T>::run() {
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
        case VSL_STATE_SIM_FINISH:
            main_sim_finish();
            break;
        case VSL_STATE_EXIT:
            if (0 <= fd_server_socket) {
                vs_server_close_socket(fd_server_socket);
                fd_server_socket = -1;
                _is_connected = false;
            }
            return 0;
        case VSL_STATE_ERROR:
        default:
            vs_log_mod_error("vsl",
                "Exiting Verisocks main loop (error state)");
            if (0 <= fd_server_socket) {
                vs_server_close_socket(fd_server_socket);
                fd_server_socket = -1;
                _is_connected = false;
            }
            return 1;
        } //switch (_state)
    }
    return 2;
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

    vs_log_mod_info(
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
    vs_msg_info_t msg_info = {VS_MSG_UNDEFINED, 0u, 0u, VS_NULL_UUID};

    msg_len = vs_msg_read(
        fd_client_socket, read_buffer, sizeof(read_buffer), &msg_info);
    if (0 > msg_len) {
        vs_server_close_socket(fd_client_socket);
        fd_client_socket = -1;
        vs_log_mod_info(
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
    p_cmd = vs_msg_read_json(read_buffer, &msg_info);
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
Methods that can be used:
vx.p_model->eval();
vx.p_model->eventsPending();
vx.p_model->nextTimeSlot();
vx.p_context->time()
******************************************************************************/
template<typename T>
void VslInteg<T>::main_sim() {
    vs_log_mod_info("vsl", "Simulation ongoing");

    while (!p_context->gotFinish()) {
        /* Evaluate model */
        p_model->eval();

        /* Check if value-based callback has been reached */
        if (check_value_callback()) {
            clear_callbacks();
            vs_msg_return(fd_client_socket, "ack",
                "Reached callback - Getting back to Verisocks main loop");
            _state = VSL_STATE_WAITING;
            return;
        }

        /* If no more pending events remaining ... finish simulation */
        if (!p_model->eventsPending()) {
            vs_log_mod_warning(
                "vsl", "Exiting without $finish; no events left");
            break;
        }

        /* If there is a time-based callback */
        if (has_time_callback() && (p_model->nextTimeSlot() >= cb_time)) {
            p_context->time(cb_time);
            //p_model->eval(); //TBC
            clear_callbacks();
            vs_msg_return(fd_client_socket, "ack",
                "Reached callback - Getting back to Verisocks main loop");
            _state = VSL_STATE_WAITING;
            return;
        }

        /* Advance time */
        p_context->time(p_model->nextTimeSlot());
    }
    /* If there is a callback hanging, it means that the Verisocks client is
    expecting a return message... in this case, an error is returned */
    if (has_callback()) {
        vs_msg_return(fd_client_socket, "error",
            "Exiting Verisocks due to end of simulation");
    }
    _state = VSL_STATE_SIM_FINISH;
    return;
}

/******************************************************************************
Main finite state-machine - Simulation finishing
******************************************************************************/
template<typename T>
void VslInteg<T>::main_sim_finish() {
    p_model->final();
    p_context->statsPrintSummary();
    _state = VSL_STATE_EXIT;
    return;
}

/******************************************************************************
Callbacks management
******************************************************************************/
template<typename T>
int VslInteg<T>::register_value_callback(const char* path, const double value)
{
    if (has_callback()) {
        vs_log_mod_error("vsl", "Could not register new value callback as \
another callback is already registered - Discarding");
        return -1;
    }
    cb_value_path = std::string(path);
    if (nullptr == get_registered_variable(cb_value_path)) {
        vs_log_mod_error("vsl", "Could not register new value callback - Path \
not found in registered variables - Discarding");
        return -1;
    }
    cb_value = value;
    b_has_value_callback = true;
    return 0;
}

template<typename T>
int VslInteg<T>::register_time_callback(uint64_t time)
{
    if (has_callback()) {
        vs_log_mod_error("vsl", "Could not register new time callback as \
another callback is already registered - Discarding");
        return -1;
    }

    if (time <= p_context->time()) {
        vs_log_mod_error("vsl", "Could not register new time callback - \
Time value is not in the future - Discarding");
        return -1;
    }

    cb_time = time;
    b_has_time_callback = true;
    return 0;
}

template<typename T>
const bool VslInteg<T>::check_value_callback() {
    if (has_value_callback()) {
        auto p_var = get_registered_variable(cb_value_path);
        switch (p_var->get_type()) {
            case VSL_TYPE_SCALAR:
                if (p_var->get_value() == cb_value) return true;
                return false;
            case VSL_TYPE_EVENT:
                if (p_var->get_value() == 1.0f) return true;
                return false;
            default:
                vs_log_mod_warning("vsl", "Value callback not supported with \
this type of variable - Discarding");
                return false;
        }
    }
    return false;
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

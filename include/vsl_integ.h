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

#ifndef VSL_INTEG_H
#define VSL_INTEG_H

#include "cJSON.h"
#include "verilated.h"

#include <memory>

namespace vsl{

typedef enum {
    VSL_STATE_INIT,        ///Initial state, Verisocks server socket not initialized
    VSL_STATE_CONNECT,     ///Socket created, bound to address, waiting for connection
    VSL_STATE_WAITING,     ///Connected and waiting to receive next command
    VSL_STATE_PROCESSING,  ///Processing a command
    VSL_STATE_SIM_RUNNING, ///Simulation running
    VSL_STATE_EXIT,        ///Exiting Verisocks
    VSL_STATE_ERROR        ///Error state (e.g. timed out while waiting for a connection)
} vsl_state_t;


class VslInteg {

public:
    //VslInteg(VerilatedModel* const model, const int port=5100, const int timeout=120);
    VslInteg(const int port=5100, const int timeout=120);
    ~VslInteg();

    void run();

private:

    //VerilatedModel* p_model; //Pointer to VerilatedModel derived class

    vsl_state_t _state {VSL_STATE_INIT}; //Verisocks state
    cJSON* p_cmd {nullptr}; //Pointer to current/latest command

    int num_port {5100}; // Port number
    int num_timeout_sec {120}; //Timeout, in seconds
    int fd_server_socket {-1}; //File descriptor, server socket
    int fd_client_socket {-1}; //File descriptor, connected client socket
    bool _is_connected {false};

    void main_init();
    void main_connect();
    void main_wait();
    void main_process();
    void main_sim();

};



/**
 * @brief Type for a command handler function pointer
 */
typedef void (*vsl_cmd_handler_t)(void);

/**
 * @brief Struct type for commands
 * Associates a command name with a command handler function pointer
 */
typedef struct vsl_cmd {
    vsl_cmd_handler_t cmd_handler;  // Pointer to handler function
    const char *cmd_name;           // Command name
    const char *cmd_key;            // Command key if not cmd_name, NULL otherwise
} vsl_cmd_t;



/**
 * @brief Helper macro to declare a command handler function
 * @param cmd Command short name
 */
#define VSL_CMD_HANDLER(cmd) static void VSL_ ## cmd ## _cmd_handler()

/**
 * @brief Helper marco to define a command structure with a command name and
 * associated command handler function pointer.
 * @param cmd Command short name
 */
#define VSL_CMD(cmd) {VSL_ ## cmd ## _cmd_handler, #cmd, NULL}

/**
 * @brief Helper macro to define a command structure with a command name and
 * associated command handler function pointer.
 * @param cmd Command short name
 * @param key Command selection key
 */
#define VSL_CMDKEY(cmd, key) {VSL_ ## cmd ## _cmd_handler, #cmd, #key}


} //namespace vsl

#endif //VS_VLINTEG_H
//EOF

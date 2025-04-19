/**************************************************************************//**
@file vs_vpi.h
@author jchabloz
@brief Verisocks VPI
@date 2022-08-07
******************************************************************************/
/*
MIT License

Copyright (c) 2022-2024 Jérémie Chabloz

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

#ifndef VS_VPI_H
#define VS_VPI_H

#include "vpi_config.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enum state
 */
typedef enum {
    VS_VPI_STATE_START,         ///Initial state, server socket not initialized
    VS_VPI_STATE_CONNECT,       ///Socket created, bound to address, waiting for connection
    VS_VPI_STATE_WAITING,       ///Connected and waiting to receive a command
    VS_VPI_STATE_PROCESSING,    ///Processing a command
    VS_VPI_STATE_SIM_RUNNING,   ///Simulation running
    VS_VPI_STATE_EXIT,          ///Exiting Verisocks
    VS_VPI_STATE_ERROR          ///Error state (e.g. timed out while waiting for a connection)
} vs_vpi_state_t;

/**
 * @brief Structure type to hold VPI user data
 */
typedef struct vs_vpi_data {
    vs_vpi_state_t state;   ///Current state
    vpiHandle h_systf;      ///VPI handle for system task instance
    int timeout_sec;        ///Socket timeout setting in seconds
    int fd_server_socket;   ///File descriptor for open server socket
    int fd_client_socket;   ///File descriptor for currently open connection
    cJSON *p_cmd;           ///Pointer to current/latest command
    vpiHandle h_cb;         ///Callback handle (used for value change callback)
    s_vpi_value value;      ///Value (used for value change callback)
} vs_vpi_data_t;

/**
 * @brief Process a command receives as a JSON message content
 *
 * @param p_data Pointer to a VPI instance-specfic data
 * @return Integer return value of executed command handler
 */
int vs_vpi_process_command(vs_vpi_data_t *p_data);

/**
 * @brief Return message to client.
 *
 * @param fd I/O descriptor (client)
 * @param str_type Type
 * @param str_value Value
 * @return Returns 0 if successful, -1 if an error occurred
 */
int vs_vpi_return(int fd, const char *str_type, const char *str_value);

extern PLI_INT32 verisocks_cb(p_cb_data cb_data);
extern PLI_INT32 verisocks_cb_value_change(p_cb_data cb_data);

/**
 * @brief Type for a command handler function pointer
 */
typedef int (*cmd_handler_t)(vs_vpi_data_t*);

/**
 * @brief Struct type for commands
 * Associates a command name with a command handler function pointer
 */
typedef struct vs_vpi_cmd {
    cmd_handler_t cmd_handler;  // Pointer to handler function
    const char *cmd_name;       // Command name
    const char *cmd_key;        // Command key if not cmd_name, NULL otherwise
} vs_vpi_cmd_t;

/**
 * @brief Helper macro to declare a command handler function
 * @param cmd Command short name
 */
#define VS_VPI_CMD_HANDLER(cmd) static int VS_VPI_ ## cmd ## _cmd_handler(\
vs_vpi_data_t *p_data)

/**
 * @brief Helper marco to define a command structure with a command name and
 * associated command handler function pointer.
 * @param cmd Command short name
 */
#define VS_VPI_CMD(cmd) {VS_VPI_ ## cmd ## _cmd_handler, #cmd, NULL}

/**
 * @brief Helper macro to define a command structure with a command name and
 * associated command handler function pointer.
 * @param cmd Command short name
 * @param key Command selection key
 */
#define VS_VPI_CMDKEY(cmd, key) {VS_VPI_ ## cmd ## _cmd_handler, #cmd, #key}

/**
 * @brief Find a command or sub-command handler in a command handler register
 * table.
 *
 * @param p_cmd_table Pointer to command handler register table
 * @param str_cmd Command selection key
 * @return Pointer to command handler function if found, NULL otherwise.
 */
cmd_handler_t vs_vpi_get_cmd_handler(
    const vs_vpi_cmd_t *p_cmd_table, const char *str_cmd);

extern const vs_vpi_cmd_t vs_vpi_cmd_get_table[];
extern const vs_vpi_cmd_t vs_vpi_cmd_run_table[];

#ifdef __cplusplus
}
#endif

#endif //VS_VPI_H
//EOF

/**
 * @file vs_vpi.h
 * @author jchabloz (jeremie.chabloz@a3.epfl.ch)
 * @brief Verisocks VPI
 * @version 0.1
 * @date 2022-08-07
 * 
 */

#ifndef VS_VPI_H
#define VS_VPI_H

#include <iverilog/vpi_user.h>
#include "vs_msg.h"
#include "vs_server.h"


/**
 * @brief Enum state
 */
typedef enum {
    VS_VPI_STATE_START,       // Initial state, server socket not initialized
    VS_VPI_STATE_CONNECT,     // Socket created, bound to address, waiting for connection
    VS_VPI_STATE_WAITING,     // Connected and waiting to receive a command
    VS_VPI_STATE_PROCESSING,  // Processing a command
    VS_VPI_STATE_SIM_RUNNING, // Simulation running
    VS_VPI_STATE_STOPPED,     // Simulation stopped
    VS_VPI_STATE_ERROR        // Error state (e.g. timed out while waiting for a connection)
} vs_vpi_state_t;


/**
 * @brief Structure type to hold VPI user data
 */
typedef struct vs_vpi_data {
    vs_vpi_state_t state;       // Current state
    int fd_server;              // File descriptor for open server socket
    int fd_connected;           // File descriptor for currently open connection
    vpiHandle h_obj;            // VPI handle
} vs_vpi_data_t;


#endif //VS_VPI_H
//EOF

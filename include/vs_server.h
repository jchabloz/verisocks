/**
 * @file vs_server.h
 * @author jchabloz
 * @brief Verisocks TCP server
 * @version 0.1
 * @date 2022-08-07
 * 
 */

#ifndef VS_SERVER_H
#define VS_SERVER_H

#define VS_MAX_CONNECT_REQUEST 3

//#define VS_SERVER_VERBOSE

/**
 * @brief Creates and binds a socket to the given localhost port
 * 
 * @param num_port Number of the port to be bound to
 * @return Returns the socket descriptor if successful, -1 in case of error.
 */
int vs_server_make_socket(uint16_t num_port);

/**
 * @brief Accepts a connection
 * 
 * @param fd_socket Server socket descriptor
 * @param hostname Will be overwritten with a pointer to the host name or a
 * NULL pointer if it was not possible to properly recover it. The string with
 * the hostname is dynamically allocated; to be freed accordingly when possible.
 * @return Returns the descriptor for the new connection, -1 in case of error.
 */
int vs_server_accept(int fd_socket, char *hostname);

#endif //VS_SERVER_H

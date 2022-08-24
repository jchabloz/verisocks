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

#include <stdint.h>
#include <stddef.h>

#define VS_MAX_CONNECT_REQUEST 3

//#define VS_SERVER_DEBUG

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
 * @param hostname Pointer to a buffer to which the hostname shall be written.
 * @param hn_len Maximum buffer size. If the hostname exceeds the available
 * buffer size, it is truncated accordingly (with null-termination).
 * @return Returns the descriptor for the new connection, -1 in case of error.
 */
int vs_server_accept(int fd_socket, char *hostname, size_t hn_len);

#endif //VS_SERVER_H

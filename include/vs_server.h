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

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VS_MAX_CONNECT_REQUEST 3

/**
 * @brief Checks if a socket descriptor is non-blocking.
 * 
 * @param fd_socket Socket descriptor to be checked.
 * @return Returns 1 if non-blocking, 0 otherwise. If -1, this indicates that
 * there was an error while getting the socket file status flags.
 */
int vs_server_is_nonblock(int fd_socket);

/**
 * @brief Sets the non-blocking behavior of a socket descriptor.
 * 
 * @param fd_socket Socket descriptor.
 * @param nonblock If > 0, the socket will be changed to non-blocking, if 0, to blocking.
 * @return Returns 0 if successful, -1 in case of error.
 */
int vs_server_set_nonblock(int fd_socket, int nonblock);

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
 * If NULL, hostname collection is not performed.
 * @param len Maximum buffer size. If the hostname exceeds the available buffer
 * size, it is truncated accordingly (with added null-termination). If 0,
 * hostname collection is not performed.
 * @param timeout Timeout
 * @return Returns the descriptor for the new connection, -1 in case of error.
 */
int vs_server_accept(int fd_socket, char *hostname, const size_t len, struct timeval *timeout);

#ifdef __cplusplus
}
#endif

#endif //VS_SERVER_H

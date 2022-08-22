/**
 * @file vs_server.c
 * @author jchabloz
 * @brief Verisocks TCP server
 * @version 0.1
 * @date 2022-08-17
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cjson/cJSON.h>

#include "vs_server.h"

int vs_server_make_socket(unsigned short num_port)
{

    int fd_socket;
    struct sockaddr_in s_addr;

    /* Create socket descriptor */
    fd_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (fd_socket < 0) {
        #ifdef VS_SERVER_VERBOSE
        perror("Could not create socket descriptor");
        #endif
        return -1;
    }

    /* Socket address */
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(num_port);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Bind socket */
	if (bind(fd_socket, (struct sockaddr *) &s_addr , sizeof(s_addr)) < 0) {
        #ifdef VS_SERVER_VERBOSE
        perror("Could not bind socket");
        #endif
        return -1;
	}

    /* Listen */
    if (listen(fd_socket, VS_MAX_CONNECT_REQUEST) < 0) {
        #ifdef VS_SERVER_VERBOSE
        perror("Error listening to socket");
        #endif
        return -1;
    }

    return fd_socket;
}

/**
 * @brief Sets the non-blocking behavior of a socket descriptor.
 * 
 * @param fd_socket Socket descriptor.
 * @param nonblock If > 0, the socket will be changed to non-blocking, if 0, to blocking.
 * @return Returns -1
 */
static int set_nonblock(int fd_socket, int nonblock)
{
    int flags = fcntl(fd_socket, F_GETFL);
    if (0 > flags) {
        return -1;
    }
    if (0 > nonblock) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if (0 > fnctl(fd_socket, F_SETFL, flags)) {
        #ifdef VS_SERVER_VERBOSE
        perror("Issue setting descriptor's file status flags");
        #endif
    }
}

/**
 * @brief Checks if a socket descriptor is non-blocking.
 * 
 * @param fd_socket Socket descriptor to be checked.
 * @return Returns 1 if non-blocking, 0 otherwise. If -1, this indicates that
 * there was an error while getting the socket file status flags.
 */
static int is_nonblock(int fd_socket)
{
    int flags = fnctl(fd_socket, F_GETFL);
    if (0 > flags) {
        #ifdef VS_SERVER_VERBOSE
        perror("Issue getting descriptor's file status flags");
        #endif
        return -1;
    }
    flags &= O_NONBLOCK;
    if (flags > 0) {
        return 1;
    }
    return 0;
}

int vs_server_accept(int fd_socket, char *hostname)
{
    struct sockaddr_in s_addr;
    socklen_t addr_len;
    int fd_conn_socket;
    struct hostent *host_info;

    fd_conn_socket = accept(fd_socket, (struct sockaddr*) &s_addr, &addr_len);
    if (0 > fd_conn_socket) {
        #ifdef VS_SERVER_VERBOSE
        perror("Error accepting connection");
        #endif
        return -1;
    }

    host_info = gethostbyaddr(s_addr.sin_addr.s_addr, addr_len, AF_INET);
    hostname = NULL;
    if (NULL == host_info || NULL == host_info->h_name) {
        #ifdef VS_SERVER_VERBOSE
        fprintf(stderr, "Could not get host info\n");
        #endif
    } else {
        size_t hostname_len = strlen(host_info->h_name);
        char *str_hostname = malloc(hostname_len + 1);
        if (NULL != str_hostname) {
            memcpy(str_hostname, host_info->h_name);
            hostname = str_hostname;
        }
    }

    return fd_conn_socket;
}

//EOF
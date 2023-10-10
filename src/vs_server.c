/**
 * @file vs_server.c
 * @author jchabloz
 * @brief Verisocks TCP server
 * @version 0.1
 * @date 2022-08-17
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>

#include "vs_logging.h"
#include "vs_server.h"

int vs_server_set_nonblock(int fd_socket, int nonblock)
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
    if (0 > fcntl(fd_socket, F_SETFL, flags)) {
        vs_log_mod_perror("vs_server",
            "Issue setting descriptor's file status flags");
    }
    return 0;
}

int vs_server_is_nonblock(int fd_socket)
{
    int flags = fcntl(fd_socket, F_GETFL);
    if (0 > flags) {
        vs_log_mod_perror("vs_server",
            "Issue getting descriptor's file status flags");
        return -1;
    }
    flags &= O_NONBLOCK;
    if (0 < flags) {
        return 1;
    }
    return 0;
}

int vs_server_make_socket(uint16_t num_port)
{
    int fd_socket;
    struct sockaddr_in s_addr;

    /* Create socket descriptor */
    fd_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (fd_socket < 0) {
        vs_log_mod_perror("vs_server",
            "Could not create socket descriptor");
        return -1;
    }

    /* Socket address */
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(num_port);
    // s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* Bind socket */
	if (bind(fd_socket, (struct sockaddr *) &s_addr , sizeof(s_addr)) < 0) {
        vs_log_mod_perror("vs_server",
            "Could not bind socket to given address");
        close(fd_socket);
        return -1;
	}

    /* Listen */
    if (listen(fd_socket, VS_MAX_CONNECT_REQUEST) < 0) {
        vs_log_mod_perror("vs_server", "Error listening to socket");
        close(fd_socket);
        return -1;
    }

    return fd_socket;
}

int vs_server_accept(int fd_socket, char *hostname, const size_t len,
                     struct timeval *p_timeout)
{
    struct sockaddr_in s_addr;
    socklen_t addr_len = sizeof(s_addr);
    int fd_conn_socket = -1;
    struct hostent *host_info;
    uint32_t addr;

    /* Use select mechanism uniquely to easily implement a timeout - The
    vs_server_accept function will keep being blocking until it times out.*/
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd_socket, &set);

    /* If a client attempts a connection, accept it */
    int selval = select(FD_SETSIZE, &set, NULL, NULL, p_timeout);
    if (0 > selval) {
        vs_log_mod_perror("vs_server", "");
        goto error;
    } else if (0 == selval) {
        vs_log_mod_error("vs_server",
            "Timed out while waiting for a connection");
        goto error;
    } else {
        fd_conn_socket = accept(fd_socket, (struct sockaddr*) &s_addr,
                                &addr_len);
        if (0 > fd_conn_socket) {
            vs_log_mod_perror("vs_server", "Error accepting connection");
            goto error;
        }
    }

    addr = s_addr.sin_addr.s_addr;
    host_info = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if ((NULL != hostname) && (0 < len)) {
        if (NULL == host_info || NULL == host_info->h_name) {
            vs_log_mod_warning("vs_server", "Could not get host info");
        }
        else {
            size_t hostname_len = strlen(host_info->h_name);
            size_t read_len =
                ((hostname_len + 1) > len) ? len - 1 : hostname_len;
            memcpy(hostname, host_info->h_name, read_len);
            hostname[read_len] = '\0';
        }
    }
    return fd_conn_socket;

    error:
    close(fd_socket);
    return -1;
}

//EOF

/**
 * @file cunit_test.c
 * @author jchabloz
 * @brief Test suite for the verisocks module using CUnit
 * @version 0.1
 * @date 2022-08-25
 * 
 */

#include <stdlib.h>
#include <netdb.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include "vs_server.h"

/******************************************************************************
* Test suite - vs_server module
******************************************************************************/
static int fd_socket = -1;
static int fd_client_socket = -1;

int init_suite_vs_server(void)
{
    return 0;
}

int clean_suite_vs_server(void)
{
    close(fd_client_socket);
    close(fd_socket);
    return 0;
}

void test_vs_server_make_socket(void)
{
    uint16_t num_port = 5100;
    fd_socket = vs_server_make_socket(num_port);
    CU_ASSERT(-1 < fd_socket);

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    CU_ASSERT(-1 < getsockname(fd_socket, (struct sockaddr *) &sin, &len));
    CU_ASSERT_EQUAL(ntohs(sin.sin_port), num_port);

    uint32_t s_addr = ntohl(sin.sin_addr.s_addr);
    printf("\nServer socket address: ");
    printf("%d.%d.%d.%d\n",
        (s_addr & 0xff000000) >> 24,
        (s_addr & 0x00ff0000) >> 16,
        (s_addr & 0x0000ff00) >> 8,
        (s_addr & 0x000000ff)
    );
    printf("Server socket port: %d\n", ntohs(sin.sin_port));
}

void test_vs_server_accept(void)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;

    printf("\nWaiting for a client to connect ... ");
    char hn_buffer[64];
    int fd_client_socket = vs_server_accept(fd_socket, hn_buffer, 64, &timeout);
    if (0 <= fd_client_socket) {
        puts("Connected to ");
        puts(hn_buffer);
        puts("\n");
    } else {
        puts("Timed out\n");
    }
}

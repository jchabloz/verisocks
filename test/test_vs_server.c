
#include <sys/time.h>
#include <stdlib.h>
#include <netdb.h>
#include "unity.h"
#include "vs_server.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_vs_server_make_socket(void)
{
    uint16_t num_port = 5100;
    int fd_socket = vs_server_make_socket(num_port);
    TEST_ASSERT_GREATER_THAN(-1, fd_socket);

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    char str_hostname[64];
    if (0 > gethostname(str_hostname, 64)) {
        perror("");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Server host name: ");
        puts(str_hostname);
    }

    struct hostent *hs;
    hs = gethostbyname(str_hostname);
    if (NULL == hs) exit(EXIT_FAILURE);
    printf("Server address and port: %d.%d.%d.%d:%d\n",
           hs->h_addr[0],
           hs->h_addr[1],
           hs->h_addr[2],
           hs->h_addr[3],
           num_port
    );
    printf("Waiting for a client to connect ...\n");

    char hn_buffer[64];
    int fd_client_socket = vs_server_accept(fd_socket, hn_buffer, 64, &timeout);
    if (0 <= fd_client_socket) {
        puts("Client connected:");
        puts(hn_buffer);
    }
    close(fd_client_socket);
    close(fd_socket);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_vs_server_make_socket);
    return UNITY_END();
}

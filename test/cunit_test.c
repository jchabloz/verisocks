/**
 * @file cunit_test.c
 * @author jchabloz
 * @brief Test suite for the verisocks module using CUnit
 * @version 0.1
 * @date 2022-08-25
 * 
 */

#include "vs_msg.h"
#include "vs_server.h"
#include <stdlib.h>
#include <netdb.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

/******************************************************************************
* Test suite - vs_msg module
******************************************************************************/
cJSON *p_msg_json;
static char read_buffer[1024];
size_t read_buffer_len = 1024;

/* Test messages contents */
const size_t msg_json_len = 47;
const size_t msg_json_hdr_len = 66;
const char* str_msg_json_string = "{\"author_name\":\"Chabloz\",\"author_firstname\":\"Jérémie\"}";
const char* str_msg_text = "This is a simple test message";
const unsigned char msg_bin[6] = {45, 32, 0, 2, 1, 248};
const size_t msg_bin_len = 6;

int init_suite_vs_msg(void)
{
    p_msg_json = cJSON_CreateObject();
    if ((NULL == p_msg_json) ||
        (NULL == cJSON_AddStringToObject(p_msg_json, "name", "Jérémie Chabloz")) ||
        (NULL == cJSON_AddStringToObject(p_msg_json, "mood", "😀👏")))
    {
        return -1;
    }
    return 0;
}

int clean_suite_vs_msg(void)
{
    cJSON_Delete(p_msg_json);
    return 0;
}

void test_vs_msg_create_header(cJSON* p_header, vs_msg_info_t msg_info,
    enum vs_msg_content_type type, size_t len) {

    CU_ASSERT_PTR_NOT_NULL(p_header);
    CU_ASSERT_EQUAL(type, msg_info.type);
    CU_ASSERT_EQUAL(len, msg_info.len);

    /* Check content-type item */
    cJSON *p_item_type = cJSON_GetObjectItemCaseSensitive(p_header, "content-type");
    CU_ASSERT_STRING_EQUAL(VS_MSG_TYPES[type], cJSON_GetStringValue(p_item_type));

    /* Check content-length item */
    cJSON *p_item_length = cJSON_GetObjectItemCaseSensitive(p_header, "content-length");
    CU_ASSERT_EQUAL(len, cJSON_GetNumberValue(p_item_length));
}

void test_vs_msg_create_header_json(void)
{
    cJSON *p_header;
    vs_msg_info_t msg_info;
    msg_info.type = VS_MSG_TXT_JSON;
    p_header = vs_msg_create_header(p_msg_json, &msg_info);

    test_vs_msg_create_header(p_header, msg_info, VS_MSG_TXT_JSON, msg_json_len);

    cJSON_Delete(p_header);
}

void test_vs_msg_create_header_text(void)
{
    size_t msg_len = strlen(str_msg_text) + 1;
    cJSON *p_header;
    vs_msg_info_t msg_info;
    msg_info.type = VS_MSG_TXT;
    p_header = vs_msg_create_header(str_msg_text, &msg_info);

    test_vs_msg_create_header(p_header, msg_info, VS_MSG_TXT, msg_len);

    cJSON_Delete(p_header);
}

void test_vs_msg_create_header_bin(void)
{
    cJSON *p_header;
    vs_msg_info_t msg_info;
    msg_info.type = VS_MSG_BIN;
    msg_info.len = msg_bin_len;
    p_header = vs_msg_create_header(msg_bin, &msg_info);

    test_vs_msg_create_header(p_header, msg_info, VS_MSG_BIN, msg_bin_len);

    cJSON_Delete(p_header);
}

void test_vs_msg_create_message(const void *p_msg, vs_msg_info_t msg_info)
{
    char *str_msg;
    str_msg = vs_msg_create_message(p_msg, msg_info);
    CU_ASSERT_PTR_NOT_NULL(str_msg);

    vs_msg_info_t msg_info_read;

    if (VS_MSG_TXT_JSON == msg_info.type) {
        cJSON *p_msg_read = vs_msg_read_json(str_msg);
        CU_ASSERT_PTR_NOT_NULL(p_msg_read);
        CU_ASSERT(cJSON_Compare(p_msg, p_msg_read, cJSON_True));
        cJSON_Delete(p_msg_read);
    } else {
        char *str_msg_read = vs_msg_read_content(str_msg, &msg_info_read);
        CU_ASSERT_PTR_NOT_NULL(str_msg_read);
        if (VS_MSG_TXT == msg_info.type) CU_ASSERT_STRING_EQUAL((char *) p_msg, str_msg_read);
        if (VS_MSG_BIN == msg_info.type) CU_ASSERT_NSTRING_EQUAL(p_msg, str_msg_read, msg_info.len);
        free(str_msg_read);
    }
    free(str_msg);
}

void test_vs_msg_create_message_json(void)
{
    test_vs_msg_create_message(p_msg_json, (vs_msg_info_t) {VS_MSG_TXT_JSON, 0});
}

void test_vs_msg_create_message_text(void)
{
    test_vs_msg_create_message(str_msg_text, (vs_msg_info_t) {VS_MSG_TXT, 0});
}

void test_vs_msg_create_message_bin(void)
{
    test_vs_msg_create_message(msg_bin, (vs_msg_info_t) {VS_MSG_BIN, msg_bin_len});
}

void test_vs_msg_create_json_message_from_string(void)
{
    char *str_msg;
    str_msg = vs_msg_create_json_message_from_string(str_msg_json_string);
    CU_ASSERT_PTR_NOT_NULL(str_msg);

    vs_msg_info_t msg_info_read;

    char *str_msg_read = vs_msg_read_content(str_msg, &msg_info_read);
    CU_ASSERT_PTR_NOT_NULL(str_msg_read);
    CU_ASSERT_STRING_EQUAL(str_msg_json_string, str_msg_read);
    free(str_msg_read);
    free(str_msg);
}

void test_vs_msg_read_write_loopback(void)
{
    /* Open a file descriptor that will use to mimick the exchanges with a
    socket when using vs_msg_write() and vs_msg_read() functions. This is not
    exactly the same and will not be able to simulate incomplete read/writes,
    but it is a start... */
    int fd_test = open("./test.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    CU_ASSERT(fd_test != -1);

    char *str_msg;
    vs_msg_info_t msg_info;
    msg_info.type = VS_MSG_TXT_JSON;
    msg_info.len = 0;
    str_msg = vs_msg_create_message(p_msg_json, msg_info);
    CU_ASSERT_PTR_NOT_NULL(str_msg);

    /* Write message to file descriptor */
    int retval = vs_msg_write(fd_test, str_msg);
    CU_ASSERT_EQUAL(0, retval);

    /* Read back message from file descriptor */
    retval = (int) lseek(fd_test, 0, SEEK_SET); //Reset descriptor position to start of file
    CU_ASSERT_EQUAL(0, retval);
    retval = vs_msg_read(fd_test, read_buffer, read_buffer_len);
    CU_ASSERT_EQUAL(0, retval);

    cJSON *p_msg_read = vs_msg_read_json(read_buffer);
    CU_ASSERT_PTR_NOT_NULL(p_msg_read);
    CU_ASSERT(cJSON_Compare(p_msg_json, p_msg_read, cJSON_True));
    cJSON_Delete(p_msg_read);

    free(str_msg);
    close(fd_test);
}

/******************************************************************************
* Test suite - vs_server module
******************************************************************************/
int fd_socket = -1;
int fd_client_socket = -1;

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
}

void test_vs_server_accept(void)
{
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    printf("Waiting for a client to connect ...\n");
    char hn_buffer[64];
    int fd_client_socket = vs_server_accept(fd_socket, hn_buffer, 64, &timeout);
    if (0 <= fd_client_socket) {
        puts("Client connected:");
        puts(hn_buffer);
    }
}

/******************************************************************************
* Main
******************************************************************************/
int main(void)
{
    CU_pSuite pSuite = NULL;

    /* Initialize CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add vs_msg module test suite to registry */
    pSuite = CU_add_suite("Test suite vs_msg",
        init_suite_vs_msg, clean_suite_vs_msg);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add tests to suite*/
    if (
        (NULL == CU_add_test(pSuite,
            "Tests creating a JSON header for a JSON message content",
            test_vs_msg_create_header_json)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a JSON header for a plain text message content",
            test_vs_msg_create_header_text)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a JSON header for a binary message content",
            test_vs_msg_create_header_bin)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a JSON content",
            test_vs_msg_create_message_json)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a JSON content from a string",
            test_vs_msg_create_json_message_from_string)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a plain text content",
            test_vs_msg_create_message_text)) ||
        (NULL == CU_add_test(pSuite,
            "Tests creating a message with a binary content",
            test_vs_msg_create_message_bin))
    ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add vs_server module test suite to registry */
    pSuite = CU_add_suite("Test suite vs_server",
        init_suite_vs_server, clean_suite_vs_server);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add tests to suite*/
    if (
        (NULL == CU_add_test(pSuite,
            "Tests vs_server_make_socket",
            test_vs_server_make_socket)) ||
        (NULL == CU_add_test(pSuite,
            "Tests vs_server_accept",
            test_vs_server_accept))
    ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

/******************************************************************************
 * Run test suites
******************************************************************************/

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    /* Run all tests using the automated interface */
    // CU_automated_run_tests();
    // CU_list_tests_to_file();

    /* Clean-up */
    CU_cleanup_registry();
    return CU_get_error();
}

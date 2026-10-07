/**
 * @file test_vs_msg.c
 * @author jchabloz
 * @brief Test suite for the verisocks vs_msg module using CUnit
 * @version 0.1
 * @date 2022-08-25
 * 
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

#include "vs_msg.h"


/******************************************************************************
* Test suite - vs_msg module
******************************************************************************/
cJSON *p_msg_json;
static char read_buffer[1024];
size_t read_buffer_len = 1024;

/* Test messages contents */
const size_t msg_json_len = 46;
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
    enum vs_msg_content_type type, size_t len)
{

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

    /* Error cases */
    p_header = vs_msg_create_header(NULL, &msg_info);
    CU_ASSERT_PTR_NULL(p_header);
    p_header = vs_msg_create_header(p_msg_json, NULL);
    CU_ASSERT_PTR_NULL(p_header);
    p_header = vs_msg_create_header(NULL, NULL);
    CU_ASSERT_PTR_NULL(p_header);
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

void test_vs_msg_create_header_wrong_type(void)
{
    cJSON *p_header;
    vs_msg_info_t msg_info;
    msg_info.type = VS_MSG_ENUM_LEN + 1;
    msg_info.len = 0;
    p_header = vs_msg_create_header(str_msg_json_string, &msg_info);
    CU_ASSERT_PTR_NULL(p_header);
}

void test_vs_msg_create_message(const void *p_msg, vs_msg_info_t *p_msg_info)
{
    char *str_msg;
    str_msg = vs_msg_create_message(p_msg, p_msg_info);
    CU_ASSERT_PTR_NOT_NULL(str_msg);

    //vs_msg_info_t msg_info_read = {VS_MSG_UNDEFINED, 0u, {0u, VS_UUID_NULL}};
	//msg_info_read.type = p_msg_info->type;

    if (VS_MSG_TXT_JSON == p_msg_info->type) {
        //cJSON *p_msg_read = vs_msg_read_json(str_msg, &msg_info_read);
        cJSON *p_msg_read = vs_msg_read_json(str_msg, p_msg_info);
        CU_ASSERT_PTR_NOT_NULL(p_msg_read);
        CU_ASSERT(cJSON_Compare(p_msg, p_msg_read, cJSON_True));
        cJSON_Delete(p_msg_read);
    } else {
        //char *str_msg_read = vs_msg_read_content(str_msg, &msg_info_read);
        char *str_msg_read = vs_msg_read_content(str_msg, p_msg_info);
        CU_ASSERT_PTR_NOT_NULL(str_msg_read);
        if (VS_MSG_TXT == p_msg_info->type) CU_ASSERT_STRING_EQUAL((char *) p_msg, str_msg_read);
        if (VS_MSG_BIN == p_msg_info->type) CU_ASSERT_NSTRING_EQUAL(
			p_msg, str_msg_read, p_msg_info->len);
        free(str_msg_read);
    }
    free(str_msg);
}

void test_vs_msg_create_message_json(void)
{
	vs_msg_info_t msg_info = {VS_MSG_TXT_JSON, 0u, {0u, VS_UUID_NULL}};
    test_vs_msg_create_message(p_msg_json, &msg_info);
}

void test_vs_msg_create_message_text(void)
{
	vs_msg_info_t msg_info = {VS_MSG_TXT, 0u, {0u, VS_UUID_NULL}};
    test_vs_msg_create_message(p_msg_json, &msg_info);
}

void test_vs_msg_create_message_bin(void)
{
	vs_msg_info_t msg_info = {VS_MSG_BIN, msg_bin_len, {0u, VS_UUID_NULL}};
    test_vs_msg_create_message(p_msg_json, &msg_info);
}

void test_vs_msg_create_json_message_from_string(void)
{
    char *str_msg;
	vs_msg_info_t msg_info = {VS_MSG_TXT_JSON, 0u, {0u, VS_UUID_NULL}};
    str_msg = vs_msg_create_json_message_from_string(
		str_msg_json_string, &msg_info);
    CU_ASSERT_PTR_NOT_NULL(str_msg);

    vs_msg_info_t msg_info_read;
    CU_ASSERT_EQUAL(0, vs_msg_read_info(str_msg, &msg_info_read));

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
    str_msg = vs_msg_create_message(p_msg_json, &msg_info);
    CU_ASSERT_PTR_NOT_NULL(str_msg);

    /* Write message to file descriptor */
    int retval = vs_msg_write(fd_test, str_msg);
    CU_ASSERT_EQUAL(0, retval);

    /* Read back message from file descriptor */
    retval = (int) lseek(fd_test, 0, SEEK_SET); //Reset descriptor position to start of file
    CU_ASSERT_EQUAL(0, retval);
    retval = vs_msg_read(fd_test, read_buffer, read_buffer_len, &msg_info);
    CU_ASSERT(0 < retval);

    cJSON *p_msg_read = vs_msg_read_json(read_buffer, &msg_info);
    CU_ASSERT_PTR_NOT_NULL(p_msg_read);
    CU_ASSERT(cJSON_Compare(p_msg_json, p_msg_read, cJSON_True));
    cJSON_Delete(p_msg_read);

    free(str_msg);
    close(fd_test);
}

void test_vs_msg_read_too_long(void)
{
    /* A message longer than the read buffer is truncated, and its remaining
    content discarded so that the next message can still be read */
    int fd_test = open("./test_long.txt",
        O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    CU_ASSERT(fd_test != -1);

    char long_value[3 * 1024];
    memset(long_value, 'x', sizeof(long_value) - 1);
    long_value[sizeof(long_value) - 1] = '\0';
    cJSON *p_msg_long = cJSON_CreateObject();
    CU_ASSERT_PTR_NOT_NULL(
        cJSON_AddStringToObject(p_msg_long, "value", long_value));

    vs_msg_info_t msg_info = {VS_MSG_TXT_JSON, 0u, {0u, VS_UUID_NULL}};
    char *str_msg_long = vs_msg_create_message(p_msg_long, &msg_info);
    CU_ASSERT_PTR_NOT_NULL(str_msg_long);
    msg_info.len = 0u;
    char *str_msg = vs_msg_create_message(p_msg_json, &msg_info);
    CU_ASSERT_PTR_NOT_NULL(str_msg);
    CU_ASSERT_EQUAL(0, vs_msg_write(fd_test, str_msg_long));
    CU_ASSERT_EQUAL(0, vs_msg_write(fd_test, str_msg));
    CU_ASSERT_EQUAL(0, (int) lseek(fd_test, 0, SEEK_SET));

    /* First message: too long for the buffer */
    int retval = vs_msg_read(fd_test, read_buffer, read_buffer_len,
        &msg_info);
    CU_ASSERT(retval > (int) read_buffer_len);

    /* Second message: read correctly (only parsed if read correctly, as
    msg_info would otherwise be inconsistent with the buffer) */
    retval = vs_msg_read(fd_test, read_buffer, read_buffer_len, &msg_info);
    CU_ASSERT(0 < retval && retval < (int) read_buffer_len);
    if (0 < retval && retval < (int) read_buffer_len) {
        cJSON *p_msg_read = vs_msg_read_json(read_buffer, &msg_info);
        CU_ASSERT_PTR_NOT_NULL(p_msg_read);
        CU_ASSERT(cJSON_Compare(p_msg_json, p_msg_read, cJSON_True));
        cJSON_Delete(p_msg_read);
    }

    cJSON_Delete(p_msg_long);
    free(str_msg);
    free(str_msg_long);
    close(fd_test);
}

/* Write a message as a JSON object with a string value of the given length,
in chunks of chunk_len bytes (all at once if 0). Returns 0 if successful. */
static int write_long_msg(int fd, size_t value_len, size_t chunk_len)
{
    char *value = (char*) malloc(value_len + 1);
    if (NULL == value) return -1;
    memset(value, 'x', value_len);
    value[value_len] = '\0';
    cJSON *p_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(p_msg, "value", value);
    free(value);
    vs_msg_info_t msg_info = {VS_MSG_TXT_JSON, 0u, {0u, VS_UUID_NULL}};
    char *str_msg = vs_msg_create_message(p_msg, &msg_info);
    cJSON_Delete(p_msg);
    if (NULL == str_msg) return -1;
    size_t len = 2 + vs_msg_read_header_length(str_msg) + msg_info.len;
    int retval = 0;
    if (0 == chunk_len) chunk_len = len;
    for (size_t k = 0; k < len && 0 == retval; k += chunk_len) {
        size_t n = (len - k < chunk_len) ? len - k : chunk_len;
        if ((ssize_t) n != write(fd, str_msg + k, n)) retval = -1;
    }
    free(str_msg);
    return retval;
}

void test_vs_msg_read_alloc(void)
{
    /* Messages longer than max_len are discarded (returns 0), the others are
    returned in an allocated buffer, whatever their length */
    int fd_test = open("./test_alloc.txt",
        O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    CU_ASSERT(fd_test != -1);
    CU_ASSERT_EQUAL(0, write_long_msg(fd_test, 100u * 1024u, 0u));
    CU_ASSERT_EQUAL(0, write_long_msg(fd_test, 10u * 1024u, 0u));
    CU_ASSERT_EQUAL(0, write_long_msg(fd_test, 10u, 0u));
    CU_ASSERT_EQUAL(0, (int) lseek(fd_test, 0, SEEK_SET));

    char *buffer = NULL;
    vs_msg_info_t msg_info;
    const size_t max_len = 64u * 1024u;

    /* 100 kB message: too long, discarded */
    CU_ASSERT_EQUAL(0, vs_msg_read_alloc(fd_test, &buffer, max_len,
        &msg_info));
    CU_ASSERT_PTR_NULL(buffer);

    /* 10 kB and 10 B messages: read */
    size_t expected_len[] = {10u * 1024u, 10u};
    for (int k = 0; k < 2; k++) {
        int retval = vs_msg_read_alloc(fd_test, &buffer, max_len, &msg_info);
        CU_ASSERT(0 < retval);
        CU_ASSERT_PTR_NOT_NULL_FATAL(buffer);
        cJSON *p_msg = vs_msg_read_json(buffer, &msg_info);
        CU_ASSERT_PTR_NOT_NULL(p_msg);
        const char *value = cJSON_GetStringValue(
            cJSON_GetObjectItem(p_msg, "value"));
        CU_ASSERT(NULL != value && strlen(value) == expected_len[k]);
        cJSON_Delete(p_msg);
        free(buffer);
    }

    /* Nothing left to read */
    CU_ASSERT_EQUAL(-1, vs_msg_read_alloc(fd_test, &buffer, max_len,
        &msg_info));
    close(fd_test);
}

void test_vs_msg_read_alloc_partial_reads(void)
{
    /* A long message arriving in many small chunks (i.e. many partial reads
    on the receiver side) is read completely */
    int fds[2];
    CU_ASSERT_FATAL(0 == socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    pid_t pid = fork();
    CU_ASSERT_FATAL(pid >= 0);
    if (0 == pid) {
        close(fds[0]);
        int retval = write_long_msg(fds[1], 32u * 1024u, 100u);
        close(fds[1]);
        _exit(retval == 0 ? 0 : 1);
    }
    close(fds[1]);
    char *buffer = NULL;
    vs_msg_info_t msg_info;
    int retval = vs_msg_read_alloc(fds[0], &buffer, VS_MSG_MAX_LEN,
        &msg_info);
    CU_ASSERT(0 < retval);
    CU_ASSERT_PTR_NOT_NULL(buffer);
    if (NULL != buffer) {
        cJSON *p_msg = vs_msg_read_json(buffer, &msg_info);
        const char *value = cJSON_GetStringValue(
            cJSON_GetObjectItem(p_msg, "value"));
        CU_ASSERT(NULL != value && strlen(value) == 32u * 1024u);
        cJSON_Delete(p_msg);
        free(buffer);
    }
    close(fds[0]);
    int status;
    waitpid(pid, &status, 0);
    CU_ASSERT(WIFEXITED(status) && 0 == WEXITSTATUS(status));
}

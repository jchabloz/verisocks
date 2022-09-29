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
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include "vs_msg.h"
#include "fff.h"

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
    CU_ASSERT(0 < retval);

    cJSON *p_msg_read = vs_msg_read_json(read_buffer);
    CU_ASSERT_PTR_NOT_NULL(p_msg_read);
    CU_ASSERT(cJSON_Compare(p_msg_json, p_msg_read, cJSON_True));
    cJSON_Delete(p_msg_read);

    free(str_msg);
    close(fd_test);
}

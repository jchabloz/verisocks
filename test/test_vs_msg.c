
#include "unity.h"
#include "vs_msg.h"
#include <stdlib.h>
#include <string.h>

/* Test messages contents */
cJSON *p_msg_json;
const size_t msg_json_len = 47;
const size_t msg_json_hdr_len = 66;
const char* str_msg_json_string = "{\"author_name\":\"Chabloz\",\"author_firstname\":\"Jérémie\"}";
const char* str_msg_text = "This is a simple test message";
const unsigned char msg_bin[6] = {45, 32, 0, 2, 1, 248};
const size_t msg_bin_len = 6;

void setUp(void)
{
    p_msg_json = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(cJSON_AddStringToObject(p_msg_json, "name", "Jérémie Chabloz"));
    TEST_ASSERT_NOT_NULL(cJSON_AddStringToObject(p_msg_json, "mood", "😀👏"));
}

void tearDown(void)
{
}

void test_vs_msg_create_header(cJSON* p_header, vs_msg_info_t msg_info,
    enum vs_msg_content_type type, size_t len) {

    TEST_ASSERT_NOT_NULL(p_header);
    TEST_ASSERT_EQUAL(type, msg_info.type);
    TEST_ASSERT_EQUAL(len, msg_info.len);

    /* Check content-type item */
    cJSON *p_item_type = cJSON_GetObjectItemCaseSensitive(p_header, "content-type");
    TEST_ASSERT_EQUAL_STRING(VS_MSG_TYPES[type], cJSON_GetStringValue(p_item_type));

    /* Check content-length item */
    cJSON *p_item_length = cJSON_GetObjectItemCaseSensitive(p_header, "content-length");
    TEST_ASSERT_EQUAL(len, cJSON_GetNumberValue(p_item_length));
}

/* Tests creating a JSON header for a JSON message */
void test_vs_msg_create_header_json(void)
{
    cJSON *p_header;
    vs_msg_info_t msg_info;
    msg_info.type = VS_MSG_TXT_JSON;
    p_header = vs_msg_create_header(p_msg_json, &msg_info);

    test_vs_msg_create_header(p_header, msg_info, VS_MSG_TXT_JSON, msg_json_len);

    cJSON_Delete(p_header);
}

/* Tests creating a JSON header for a plain text message */
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

/* Tests creating a JSON header for a binary message */
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
    TEST_ASSERT_NOT_NULL(str_msg);

    vs_msg_info_t msg_info_read;

    if (VS_MSG_TXT_JSON == msg_info.type) {
        cJSON *p_msg_read = vs_msg_read_json(str_msg);
        TEST_ASSERT_NOT_NULL(p_msg_read);
        TEST_ASSERT(cJSON_Compare(p_msg, p_msg_read, cJSON_True));
        cJSON_Delete(p_msg_read);
    } else {
        char *str_msg_read = vs_msg_read_content(str_msg, &msg_info_read);
        TEST_ASSERT_NOT_NULL(str_msg_read);
        if (VS_MSG_TXT == msg_info.type) TEST_ASSERT_EQUAL_STRING((char *) p_msg, str_msg_read);
        if (VS_MSG_BIN == msg_info.type) TEST_ASSERT_EQUAL_UINT8_ARRAY(p_msg, str_msg_read, msg_info.len);
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
    TEST_ASSERT_NOT_NULL(str_msg);

    vs_msg_info_t msg_info_read;

    char *str_msg_read = vs_msg_read_content(str_msg, &msg_info_read);
    TEST_ASSERT_NOT_NULL(str_msg_read);
    TEST_ASSERT_EQUAL_STRING(str_msg_json_string, str_msg_read);
    free(str_msg_read);
    free(str_msg);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_vs_msg_create_header_json);
    RUN_TEST(test_vs_msg_create_header_text);
    RUN_TEST(test_vs_msg_create_header_bin);
    RUN_TEST(test_vs_msg_create_message_json);
    RUN_TEST(test_vs_msg_create_message_text);
    RUN_TEST(test_vs_msg_create_message_bin);
    RUN_TEST(test_vs_msg_create_json_message_from_string);
    return UNITY_END();
}

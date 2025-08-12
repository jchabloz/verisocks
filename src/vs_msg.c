/**************************************************************************//**
@file vs_msg.c
@author jchabloz
@brief Verisocks messages definition and utilities
@date 2022-08-22
@note Only UTF-8 text encoding is supported for now.
******************************************************************************/
/*
MIT License

Copyright (c) 2022-2025 Jérémie Chabloz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include "vs_logging.h"
#include "vs_msg.h"

/**************************************************************************//**
This constant array of strings defines the values to be used for the
content-type field of the header depending on the type numerical index as
defined in the header file vs_msg.h. Update to add types.
******************************************************************************/
const char* VS_MSG_TYPES[VS_MSG_ENUM_LEN] =
{
    "text/plain",
    "application/json",
    "application/octet-stream",
    "undefined"
};

static void sprintf_uuid(char *str_uuid, const vs_uuid_t *p_uuid)
{
    snprintf(str_uuid, VS_UUID_STR_LEN, VS_UUID_STR_FMT,
        p_uuid->value[0],
        p_uuid->value[1],
        p_uuid->value[2],
        p_uuid->value[3],
        p_uuid->value[4],
        p_uuid->value[5],
        p_uuid->value[6],
        p_uuid->value[7],
        p_uuid->value[8],
        p_uuid->value[9],
        p_uuid->value[10],
        p_uuid->value[11],
        p_uuid->value[12],
        p_uuid->value[13],
        p_uuid->value[14],
        p_uuid->value[15]
    );
}

static void sscanf_uuid(const char *str_uuid, vs_uuid_t *p_uuid)
{
    sscanf(str_uuid, VS_UUID_STR_FMT,
        &p_uuid->value[0],
        &p_uuid->value[1],
        &p_uuid->value[2],
        &p_uuid->value[3],
        &p_uuid->value[4],
        &p_uuid->value[5],
        &p_uuid->value[6],
        &p_uuid->value[7],
        &p_uuid->value[8],
        &p_uuid->value[9],
        &p_uuid->value[10],
        &p_uuid->value[11],
        &p_uuid->value[12],
        &p_uuid->value[13],
        &p_uuid->value[14],
        &p_uuid->value[15]
    );
}

/**************************************************************************//**
Returns the header length
******************************************************************************/
static uint16_t get_header_length(const char *str_header)
{
    /* Get the header as a string and get its length */
    size_t header_length = strlen(str_header);

    uint16_t retval;
    if (header_length > UINT16_MAX) {
        vs_log_mod_error("vs_msg",
            "Header too long for its length to be encoded within 2 bytes!");
        retval = 0;
    } else {
        retval = (uint16_t) header_length;
    }
    return retval;
}

void vs_msg_copy_uuid(vs_msg_info_t *p_msg_info, const vs_uuid_t *p_uuid)
{
    if (p_uuid->valid > 0u) {
        p_msg_info->uuid.valid = p_uuid->valid;
        memcpy(p_msg_info->uuid.value, p_uuid->value, VS_UUID_LEN);
    }
}

/**************************************************************************//**
* Create message header
******************************************************************************/
cJSON* vs_msg_create_header(const void *p_msg, vs_msg_info_t *p_msg_info)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_create_header");

    cJSON *p_header;
    char *str_msg = NULL;
    char str_uuid[VS_UUID_STR_LEN] = "";

    /* Sanity checks on parameters */
    if (NULL == p_msg || NULL == p_msg_info) {
        vs_log_mod_error("vs_msg", "NULL pointer");
        return NULL;
    }

    /* Create header JSON object, including calculated message length*/
    p_header = cJSON_CreateObject();
    if (NULL == p_header) {
        vs_log_mod_error("vs_msg", "Failed to create header JSON object");
        return NULL;
    }

    /* Create header depending on message type */
    switch (p_msg_info->type) {
    case VS_MSG_TXT :
        str_msg = (char *) p_msg;
        p_msg_info->len = strlen(str_msg) + 1;
        /* Add content type item */
        if (NULL == cJSON_AddStringToObject(p_header, "content-type",
                                            VS_MSG_TYPES[VS_MSG_TXT])) {
            vs_log_mod_error("vs_msg", "Failed to add string to cJSON object");
            goto error;
        }
        /* Add content encoding item */
        if (NULL == cJSON_AddStringToObject(p_header, "content-encoding",
                                            "UTF-8")) {
            vs_log_mod_error("vs_msg", "Failed to add string to cJSON object");
            goto error;
        }
        break;
    case VS_MSG_TXT_JSON :
        /* Get the message as a string and get its length, WITHOUT ending
        null character */
        str_msg = cJSON_PrintUnformatted((cJSON*) p_msg);
        p_msg_info->len = strlen(str_msg);
        /* Add content type item */
        if (NULL == cJSON_AddStringToObject(p_header, "content-type",
                                            VS_MSG_TYPES[VS_MSG_TXT_JSON])) {
            vs_log_mod_error("vs_msg", "Failed to add string to cJSON object");
            cJSON_free(str_msg);
            goto error;
        }
        /* Add content encoding item */
        if (NULL == cJSON_AddStringToObject(p_header, "content-encoding",
                                            "UTF-8")) {
            vs_log_mod_error("vs_msg",
                "Failed to add string to cJSON object");
            cJSON_free(str_msg);
            goto error;
        }
        cJSON_free(str_msg);
        break;
    case VS_MSG_BIN :
        /* Add content type item */
        if (NULL == cJSON_AddStringToObject(p_header, "content-type",
                                            VS_MSG_TYPES[VS_MSG_BIN])) {
            vs_log_mod_error("vs_msg", "Failed to add string to cJSON object");
            goto error;
        }
        break;
    default:
        vs_log_mod_error("vs_msg", "Message type %d not supported",
            p_msg_info->type);
        goto error;
    }

    /* Add message length item */
    if (p_msg_info->len < 1) {
        vs_log_mod_error("vs_msg", "Message length invalid (< 1)");
        goto error;
    }
    if (NULL == cJSON_AddNumberToObject(p_header, "content-length",
                                        p_msg_info->len)) {
        vs_log_mod_error("vs_msg", "Failed to add number to cJSON object");
        goto error;
    };

    /* If present, add transaction UUID to header encoded as a string */
    if (p_msg_info->uuid.valid > 0) {
        sprintf_uuid(str_uuid, &(p_msg_info->uuid));
        if (NULL == cJSON_AddStringToObject(p_header, "uuid", str_uuid)) {
            vs_log_mod_error("vs_msg", "Failed to add string to cJSON object");
            goto error;
        }
    }

    return p_header;

    error:
    cJSON_Delete(p_header);
    return NULL;
}

/**************************************************************************//**
* Returns a formatted message as a character string, including pre-header,
* header and message content.
******************************************************************************/
char* vs_msg_create_message(const void *p_msg, vs_msg_info_t *p_msg_info)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_create_message");

    /* Create header cJSON object handle */
    cJSON *p_header = vs_msg_create_header(p_msg, p_msg_info);
    if (NULL == p_header) {
        vs_log_mod_error("vs_msg", "Failed to create header JSON object");
        return NULL;
    }
    char *str_header = cJSON_PrintUnformatted(p_header);
    if (NULL == str_header) {
        vs_log_mod_error("vs_msg", "Failed to create header string");
        cJSON_Delete(p_header);
        return NULL;
    }
    cJSON_Delete(p_header);

    /* Calculate pre-header based on the header length */
    uint16_t header_length = get_header_length(str_header);
    if (0 == header_length) {
        vs_log_mod_error("vs_msg", "Pre-header value is 0");
        cJSON_free(str_header);
        return NULL;
    }
    uint8_t pre_header_lsb = (uint8_t) (header_length & 0x00ff);
    uint8_t pre_header_msb = (uint8_t) ((header_length & 0xff00) >> 8);

    vs_log_mod_debug("vs_msg", "Encoded pre-header value: [0x%02x,0x%02x], %d",
        pre_header_msb,
        pre_header_lsb,
        (uint16_t) pre_header_lsb + ((uint16_t) pre_header_msb << 8)
    );

    /* Message content, formatted depending on message type */
    char *str_msg = NULL;
    switch (p_msg_info->type) {
    case VS_MSG_TXT :
        break;
    case VS_MSG_TXT_JSON :
        str_msg = cJSON_PrintUnformatted((cJSON*) p_msg);
        vs_log_mod_debug("vs_msg", "Preparing message: %s", str_msg);
        break;
    case VS_MSG_BIN :
        break;
    default:
        vs_log_mod_error("vs_msg", "Message type not supported");
        cJSON_free(str_header);
        return NULL;
    }

    /* Put the full message together */
    /* The full size for the message is the sum of the header and message
    lengths + 2 (2 bytes for the pre-header */
    size_t alloc_size = header_length + p_msg_info->len + 2;
    char *result = (char*) malloc(alloc_size);

    vs_log_mod_debug("vs_msg",
        "Allocated %d bytes in virtual memory for the formatted message",
        (int) alloc_size);

    if (NULL != result) {

        /*Pre-header*/
        result[0] = pre_header_msb;
        result[1] = pre_header_lsb;

        /*Header*/
        memcpy(result + 2, str_header, header_length);

        /*Payload*/
        if (NULL != str_msg)
            memcpy(result + 2 + header_length, str_msg, p_msg_info->len);
        else
            memcpy(result + 2 + header_length, p_msg, p_msg_info->len);
    }
    cJSON_free(str_header);
    if (NULL != str_msg) free(str_msg);
    return result;
}

/**************************************************************************//**
* Create JSON message from string
******************************************************************************/
char* vs_msg_create_json_message_from_string(const char *str_message,
    vs_msg_info_t *p_msg_info)
{
    /* Attempts to scan the string as a JSON object */
    cJSON *p_obj_msg = cJSON_Parse(str_message);
    if (NULL == p_obj_msg || cJSON_IsInvalid(p_obj_msg) ) {
        vs_log_mod_error("vs_msg",
            "Failed to parse message string as a JSON object");
        return NULL;
    }

    /* If valid, pass the created object to the existing function */
    char *str_msg = vs_msg_create_message(p_obj_msg, p_msg_info);

    /* Clean up */
    cJSON_Delete(p_obj_msg);

    return str_msg;
}

/**************************************************************************//**
* Read header length from pre-header
******************************************************************************/
size_t vs_msg_read_header_length(const char *message)
{
    /* Extract the header length from the pre-header */
    const size_t header_length =
        ((uint8_t)(message[0]) << 8) + (uint8_t) message[1];
    return header_length;
}

/**************************************************************************//**
* Read message information from header
******************************************************************************/
int vs_msg_read_info(const char *message, vs_msg_info_t *p_msg_info)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_read_info");

    /* Extract the header length from the pre-header */
    const size_t header_length = vs_msg_read_header_length(message);
    if (1 > header_length) {
        vs_log_mod_error("vs_msg", "Header length is invalid (< 1)");
        return -1;
    }
    vs_log_mod_debug("vs_msg",
        "Found header length = %d", (int) header_length);

    /* Get the JSON header as a proper, null-terminated string from the message
    */
    char *str_header = (char*) malloc(header_length + 1);
    if (NULL == str_header) {
        vs_log_mod_perror("vs_msg",
            "malloc could not allocate memory!");
        free(str_header);
        return -1;
    }
    memcpy(str_header, message + 2, header_length);
    str_header[header_length] = '\0';

    /* Parse the header to find the message length */
    cJSON *p_obj_header = cJSON_Parse(str_header);
    free(str_header);
    if (NULL == p_obj_header) {
        vs_log_mod_error("vs_msg", "Failed to parse header");
        return -1;
    }
    cJSON *p_item_msg_length =
        cJSON_GetObjectItemCaseSensitive(p_obj_header, "content-length");
    if (!cJSON_IsNumber(p_item_msg_length)) {
        vs_log_mod_error("vs_msg", "Failed to parse message length in header");
        cJSON_Delete(p_obj_header);
        return -1;
    }
    p_msg_info->len = (size_t) p_item_msg_length->valueint;
    vs_log_mod_debug("vs_msg",
        "Found message length = %d", (int) p_msg_info->len);

    /* Parse the header to find an optional UUID value */
    char *str_uuid;
    cJSON *p_item_msg_uuid =
        cJSON_GetObjectItemCaseSensitive(p_obj_header, "uuid");
    if (NULL == p_item_msg_uuid) {
        p_msg_info->uuid.valid = 0u;
    } else {
        p_msg_info->uuid.valid = 1u;
        str_uuid = cJSON_GetStringValue(p_item_msg_uuid);
        vs_log_mod_debug("vs_msg", "Found UUID in header: %s", str_uuid);
        sscanf_uuid(str_uuid, &(p_msg_info->uuid));
    }
    
    /* Find the message type */
    cJSON *p_item_msg_type =
        cJSON_GetObjectItemCaseSensitive(p_obj_header, "content-type");
    char *str_type = cJSON_GetStringValue(p_item_msg_type);

    if (VS_CMP_TYPE(str_type, VS_MSG_TXT)) {
        p_msg_info->type = VS_MSG_TXT;
    } else if (VS_CMP_TYPE(str_type, VS_MSG_TXT_JSON)) {
        p_msg_info->type = VS_MSG_TXT_JSON;
    } else if (VS_CMP_TYPE(str_type, VS_MSG_BIN)) {
        p_msg_info->type = VS_MSG_BIN;
    } else {
        vs_log_mod_error("vs_msg", "Unsupported content type: %s",
                     str_type);
        cJSON_Delete(p_obj_header);
        return -1;
    }
    cJSON_Delete(p_obj_header);
    return 0;
}

/**************************************************************************//**
 * Parses a received message and returns a character array with the message
 * content. The message info struct is also updated.
 *****************************************************************************/
char* vs_msg_read_content(const char* message, const vs_msg_info_t *p_msg_info)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_read_content");

    /* Get the header length from the pre-header */
    const size_t header_length = vs_msg_read_header_length(message);

    /* Get back message content. Could be either text or binary */
    char *str_msg;
    if (VS_MSG_TXT == p_msg_info->type || VS_MSG_TXT_JSON == p_msg_info->type) {
        str_msg = (char*) malloc(p_msg_info->len + 1);
    } else {
        str_msg = (char*) malloc(p_msg_info->len);
    }
    if (NULL == str_msg) {
        vs_log_mod_perror("vs_msg", "Failed to allocated virtual memory");
        return NULL;
    }
    memcpy(str_msg, message + 2 + header_length, p_msg_info->len);

    /* Add null-termination */
    if (VS_MSG_TXT == p_msg_info->type || VS_MSG_TXT_JSON == p_msg_info->type)
    {
        str_msg[p_msg_info->len] = '\0';
    }
    return str_msg;
}

/**************************************************************************//**
 * Parses a received message and returns a pointer to a cJSON object with the
 * message content.
 *****************************************************************************/
cJSON* vs_msg_read_json(const char* message, const vs_msg_info_t *p_msg_info)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_read_json");

    /* Get message content */
    char *str_msg;
    str_msg = vs_msg_read_content(message, p_msg_info);
    if (NULL == str_msg) {
        vs_log_mod_error("vs_msg", "Failed to parse message - No content");
        return NULL;
    }
    if (p_msg_info->type != VS_MSG_TXT_JSON) {
        vs_log_mod_error("vs_msg",
            "Header not consistent with JSON content type");
        free(str_msg);
        return NULL;
    }
    vs_log_mod_debug("vs_msg", "Message content: %s", str_msg);

    /* Parse the message string and return the JSON object if valid */
    cJSON *p_obj_msg;
    p_obj_msg = cJSON_Parse(str_msg);
    free(str_msg);

    if (NULL == p_obj_msg || cJSON_IsInvalid(p_obj_msg) ) {
        vs_log_mod_error("vs_msg", "Failed to parse message");
        return NULL;
    }

    return p_obj_msg;
}

/**************************************************************************//**
 * Writes message to I/O (file) descriptor
 *****************************************************************************/
int vs_msg_write(int fd, const char *str_msg)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_write");

    vs_msg_info_t msg_info = VS_MSG_INFO_INIT_UNDEF;

    if (0 > vs_msg_read_info(str_msg, &msg_info)) {
        vs_log_mod_error("vs_msg", "Could not get message info");
        return -1;
    }
    size_t header_len = vs_msg_read_header_length(str_msg);
    size_t full_len = header_len + msg_info.len + 2;
    size_t write_count = 0; //written bytes counter
    unsigned int trials = VS_MSG_MAX_WRITE_TRIALS;
    ssize_t retval;

    while (write_count < full_len && trials > 0) {
        retval = write(fd, str_msg + write_count, full_len - write_count);
        if (0 > retval) {
            vs_log_mod_perror("vs_msg", "Message cannot be written");
            return -1;
        }
        write_count += retval;
        trials--;
    }
    return full_len - write_count;
}

/**************************************************************************//**
 * Writes message to I/O (file) descriptor
 *****************************************************************************/
int vs_msg_return(int fd, const char *str_type, const char *str_value)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_return");

    cJSON *p_msg;
    char *str_msg = NULL;
    vs_msg_info_t msg_info = VS_MSG_INFO_INIT_JSON;

    p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_msg", "Could not create cJSON object");
        return -1;
    }

    if (NULL == cJSON_AddStringToObject(p_msg, "type", str_type)) {
        vs_log_mod_error("vs_msg", "Could not add string to object");
        goto error;
    }

    if (NULL == cJSON_AddStringToObject(p_msg, "value", str_value)) {
        vs_log_mod_error("vs_msg", "Could not add string to object");
        goto error;
    }

    str_msg = vs_msg_create_message(p_msg, &msg_info);
    if (NULL == str_msg) {
        vs_log_mod_error("vs_vpi", "NULL pointer");
        goto error;
    }

    int retval;
    retval = vs_msg_write(fd, str_msg);
    if (0 > retval) {
        vs_log_mod_error("vs_msg", "Error writing return message");
        goto error;
    }

    if (NULL != p_msg) cJSON_Delete(p_msg);
    if (NULL != str_msg) cJSON_free(str_msg);
    return 0;

    error:
    if (NULL != p_msg) cJSON_Delete(p_msg);
    if (NULL != str_msg) cJSON_free(str_msg);
    return -1;
}

/******************************************************************************
 * Read messages
 *****************************************************************************/
/**
 * @brief Helper function - Reads n bytes from descriptor to buffer
 *
 * @param fd I/O descriptor to be read from
 * @param len Number of bytes to be read
 * @param buffer Pointer to read buffer
 * @return Returns 0 if successful, -1 if an error occurred. Values > 0
 * indicate the number of bytes that were not successfully read.
 */
static int readn(int fd, size_t len, char *buffer)
{
    size_t read_count = 0; //read bytes counter
    unsigned int trials = VS_MSG_MAX_READ_TRIALS;
    ssize_t retval;

    while (read_count < len && trials > 0) {
        retval = read(fd, buffer + read_count, len - read_count);
        if (0 > retval) {
            vs_log_mod_perror("vs_msg", "Cannot read message");
            return -1;
        }
        read_count += retval;
        trials --;
    }
    return len - read_count;
}

int vs_msg_read(int fd, char *buffer, size_t len, vs_msg_info_t *p_msg_info)
{
    vs_log_mod_debug("vs_msg", "Function vs_msg_read");

    if (3 > len) {
        vs_log_mod_error("vs_msg", "Buffer depth not sufficient (%d)",
                     (int) len);
        return -1;
    }
    /* Get pre-header */
    if (0 != readn(fd, 2u, buffer)) {
        vs_log_mod_debug("vs_msg", "Could not read pre-header value. \
Socket probably disconnected");
        return -1;
    }
    /* Get header length from pre-header */
    size_t header_length = vs_msg_read_header_length(buffer);
    if (1 > header_length) {
        vs_log_mod_error("vs_msg", "Issue with header length (value %d)",
                     (int) header_length);
        return -1;
    }
    vs_log_mod_debug("vs_msg", "Received message header length: %d",
        (int) header_length);

    /* Read header */
    size_t read_len = header_length;
    if ((header_length + 2) > len) {
        read_len = len - 2;
    }
    if ((header_length + 2) > len) {
        vs_log_mod_error("vs_msg", "Buffer depth not sufficient (%d)",
                     (int) len);
        return -1;
    }
    if (0 != readn(fd, read_len, buffer + 2)) {
        vs_log_mod_error("vs_msg", "Issue while reading header");
        return -1;
    }

    /* Parse header */
    if (0 > vs_msg_read_info(buffer, p_msg_info)) {
        vs_log_mod_error("vs_msg", "Issue while parsing message info");
        return -1;
    }
    vs_log_mod_debug("vs_msg", "Received message type: %s",
        VS_MSG_TYPES[p_msg_info->type]);
    vs_log_mod_debug("vs_msg", "Received message length: %d",
        (int) p_msg_info->len);

    /* Read message content - If longer than buffer depth, truncate it*/
    read_len = p_msg_info->len;
    size_t total_len = p_msg_info->len + header_length + 2;
    if (total_len > len) {
        read_len = len - header_length - 2;
        vs_log_mod_warning("vs_msg", "Truncated message content by %d bytes",
            (int) (total_len - len));
    }
    if (0 != readn(fd, read_len, buffer + 2 + header_length)) {
        vs_log_mod_error("vs_msg", "Issue while reading message content");
        return -1;
    }

    /* Return received message total length */
    return total_len;
}

//EOF

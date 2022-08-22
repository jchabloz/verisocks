/**
 * @file vs_msg.c
 * @author jchabloz
 * @brief Verisocks messages definition and utilities
 * @version 0.1
 * @date 2022-08-22
 * @note Only UTF-8 text encoding is supported for now.
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "vs_msg.h"


/* This constant array of strings defines the values to be used for the
content-type field of the header depending on the type numerical index as
defined in the header file vs_msg.h. Update to add types.*/
const char* __VS_MSG_TYPES[VS_MSG_ENUM_LEN] = {
    "text/plain",
    "application/json",
    "application/octet-stream",
};


const uint16_t vs_msg_get_pre_header_value(const cJSON *p_header)
{
    /* Get the header as a string and get its length */
    char *str_header = cJSON_PrintUnformatted(p_header);
    size_t header_length = strlen(str_header);
    free(str_header);
    uint16_t retval;
    if (header_length > UINT16_MAX) {
        fprintf(stderr, "Header too long for its length to be encoded within 2 bytes!\n");
        retval = 0;
    } else {
        retval = (uint16_t) header_length;
    }
    return retval;
}


cJSON* vs_msg_create_header(const void *p_msg, vs_msg_info_t msg_info)
{
    cJSON *p_header;
    char *str_msg = NULL;

    /* Create header JSON object, including calculated message length*/
    p_header = cJSON_CreateObject();
    if (NULL == p_header) {
        fprintf(stderr, "Failed to create header JSON object.\n");
        return NULL;
    }

    switch (msg_info.type) {
    case VS_MSG_TXT :
        str_msg = (char *) p_msg;
        msg_info.len = strlen(str_msg) + 1;
        /* Add content type item */
        if (NULL == cJSON_AddStringToObject(p_header, "content-type", __VS_MSG_TYPES[VS_MSG_TXT])) {
            fprintf(stderr, "ERROR: Failed to add string to cJSON object.\n");
            return NULL;
        }
        /* Add content encoding item */
        if (NULL == cJSON_AddStringToObject(p_header, "charset", "UTF-8")) {
            fprintf(stderr, "ERROR: Failed to add string to cJSON object.\n");
            return NULL;
        }
        break;
    case VS_MSG_TXT_JSON :
        /* Get the message as a string and get its length, including ending
        null character */
        str_msg = cJSON_PrintUnformatted((cJSON*) p_msg);
        msg_info.len = strlen(str_msg) + 1;
        /* Add content type item */
        if (NULL == cJSON_AddStringToObject(p_header, "content-type", __VS_MSG_TYPES[VS_MSG_TXT_JSON])) {
            fprintf(stderr, "ERROR: Failed to add string to cJSON object.\n");
            return NULL;
        }
        /* Add content encoding item */
        if (NULL == cJSON_AddStringToObject(p_header, "charset", "UTF-8")) {
            fprintf(stderr, "ERROR: Failed to add string to cJSON object.\n");
            return NULL;
        }
        break;
    case VS_MSG_BIN :
        /* Get the message as a string and get its length, including ending
        null character */
        /* Add content type item */
        if (NULL == cJSON_AddStringToObject(p_header,
                "content-type", __VS_MSG_TYPES[VS_MSG_BIN])) {
            fprintf(stderr, "ERROR: Failed to add string to cJSON object.\n");
            return NULL;
        }
        break;
    default:
        fprintf(stderr, "ERROR: Message type not supported\n");
        return NULL;
    }

    /* Add message length item */
    if (NULL == cJSON_AddNumberToObject(p_header, "content-length", msg_info.len)) {
        fprintf(stderr, "ERROR: Failed to add number to cJSON object.\n");
        return NULL;
    };
    return p_header;
}


char* vs_msg_format_message(const void *p_msg, vs_msg_info_t msg_info)
{
    /* Create header cJSON object handle */
    cJSON *p_header = vs_msg_create_header(p_msg, msg_info);
    if (NULL == p_header) {
        fprintf(stderr, "Failed to create header JSON object.\n");
        return NULL;
    }

    /* Calculate pre-header based on the header length */
    uint16_t header_length = vs_msg_get_pre_header_value(p_header);
    if (0 == header_length) {
        fprintf(stderr, "Pre-header value is 0.\n");
        return NULL;
    }
    char pre_header_lsb = header_length & 0xff;
    char pre_header_msb = (header_length & 0xff00) >> 8;
    char *str_header = cJSON_PrintUnformatted(p_header);
    cJSON_Delete(p_header);

    char *str_msg = NULL;
    switch (msg_info.type)
    {
    case VS_MSG_TXT :
        str_msg = (char *) p_msg;
        msg_info.len = strlen(str_msg) + 1;
        break;
    case VS_MSG_TXT_JSON :
        str_msg = cJSON_PrintUnformatted((cJSON*) p_msg);
        msg_info.len = strlen(str_msg) + 1;
        break;
    case VS_MSG_BIN :
        break;
    default:
        fprintf(stderr, "ERROR: Message type not supported\n");
        return NULL;
    }

    /* Put the full message together */
    /* The full size for the message is the sum of the header and message
    lengths + 2 (2 bytes for the pre-header */
    size_t alloc_size = header_length + msg_info.len + 2;
    char *result = malloc(alloc_size);
    #ifdef VS_MSG_DEBUG
    printf("Allocated %d bytes in virtual memory for the formatted message\n", (int) alloc_size);
    #endif
    if (NULL != result) {

        /*Pre-header*/
        result[0] = pre_header_msb;
        result[1] = pre_header_lsb;

        /*Header*/
        memcpy(result + 2, str_header, header_length);

        /*Payload*/
        if (NULL != str_msg)
            memcpy(result + 2 + header_length, str_msg, msg_info.len);
        else
            memcpy(result + 2 + header_length, p_msg, msg_info.len);
    }
    free(str_header);
    if (NULL != str_msg) free(str_msg);
    return result;
}


char* vs_msg_format_message_from_string(const char *str_message)
{
    /* Attempts to scan the string as a JSON object */
    cJSON *p_obj_msg = cJSON_Parse(str_message);
    if (NULL == p_obj_msg || cJSON_IsInvalid(p_obj_msg) ) {
        fprintf(stderr, "Failed to parse message string as a JSON object\n");
        return NULL;
    }
    
    /* If valid, pass the created object to the existing function */
    char *str_msg = vs_msg_format_message(p_obj_msg, (vs_msg_info_t) {VS_MSG_TXT_JSON, 0});

    /* Clean up */
    cJSON_Delete(p_obj_msg);

    return str_msg;
}


const size_t vs_msg_get_header_length(const char *message)
{
    /* Extract the header length from the pre-header */
    const size_t header_length = (message[0] << 8) + message[1];
    return header_length;
}


int vs_msg_get_info(const char *message, vs_msg_info_t *msg_info)
{
    /* Extract the header length from the pre-header */
    const size_t header_length = (message[0] << 8) + message[1];
    if (0 == header_length) {
        fprintf(stderr, "Header length is 0\n");
        return -1;
    }
    #ifdef VS_MSG_DEBUG
    printf("Found header length = %d\n", (int) header_length);
    #endif

    /* Get the JSON header as a proper, null-terminated string from the message */
    char *str_header = malloc(header_length + 1);
    if (NULL == str_header) {
        perror("malloc could not allocate memory!");
        free(str_header);
        return -1;
    }
    memcpy(str_header, message + 2, header_length);
    str_header[header_length] = '\0';

    /* Parse the header to find the message length */
    cJSON *p_obj_header = cJSON_Parse(str_header);
    free(str_header);
    if (NULL == p_obj_header) {
        fprintf(stderr, "Failed to parse header\n");
        return -1;
    }

    cJSON *p_item_msg_length =
        cJSON_GetObjectItemCaseSensitive(p_obj_header, "content-length");
    if (!cJSON_IsNumber(p_item_msg_length)) {
        fprintf(stderr, "Failed to parse message length in header\n");
        cJSON_Delete(p_obj_header);
        return -1;
    }
    msg_info->len = (size_t) p_item_msg_length->valueint;
    #ifdef VS_MSG_DEBUG
    printf("Found message length = %d\n", (int) msg_info->len);
    #endif

    /* Find the message type */
    cJSON *p_item_msg_type =
        cJSON_GetObjectItemCaseSensitive(p_obj_header, "content-type");
    char *str_type = cJSON_GetStringValue(p_item_msg_type);

    if (VS_CMP_TYPE(str_type, VS_MSG_TXT_JSON)) {
        msg_info->type = VS_MSG_TXT_JSON;
    } else if (VS_CMP_TYPE(str_type, VS_MSG_BIN)) {
        msg_info->type = VS_MSG_BIN;
    } else {
        fprintf(stderr, "ERROR: Unsupported content type: %s\n", str_type);
        cJSON_Delete(p_obj_header);
        return -1;
    }
    cJSON_Delete(p_obj_header);
    return 0;
}

/**************************************************************************//**
 * @brief Parses a received message and returns a character array with the
 * message content. The message info struct is also updated.
 *****************************************************************************/
char* vs_msg_get_content(const char* message, vs_msg_info_t *msg_info)
{
    /* Get the header length from the pre-header */
    const size_t header_length = vs_msg_get_header_length(message);

    /* Get the message length and type from the header */
    // vs_msg_info_t msg_info;
    if (0 > vs_msg_get_info(message, msg_info)) {
        return NULL;
    }

    /* Get back message content. Could be either text or binary */
    char *str_msg = malloc(msg_info->len);
    if (NULL == str_msg) {
        perror("Failed to allocated virtual memory");
        return NULL;
    }
    memcpy(str_msg, message + 2 + header_length, msg_info->len);

    /* Normally not required... but let's make that if we expect a string, it
    is forced to be null-terminated.*/
    if (VS_MSG_TXT_JSON == msg_info->type) {
        str_msg[msg_info->len - 1] = '\0';
    }
    return str_msg;
}

/**************************************************************************//**
 * @brief Parses a received message and returns a pointer to a cJSON object
 * with the message content.
 *****************************************************************************/
cJSON* vs_msg_get_json(const char* message)
{
    /* Get message content */
    char *str_msg;
    vs_msg_info_t msg_info;
    str_msg = vs_msg_get_content(message, &msg_info);
    if (NULL == str_msg) {
        fprintf(stderr, "ERROR: Failed to parse message - No content\n");
        return NULL;
    }
    if (msg_info.type != VS_MSG_TXT_JSON) {
        fprintf(stderr, "ERROR: Header not consistent with JSON type\n");
        free(str_msg);
        return NULL;
    } 

    /* Parse the message string and return the JSON object if valid */
    cJSON *p_obj_msg;
    p_obj_msg = cJSON_Parse(str_msg);
    free(str_msg);

    if (NULL == p_obj_msg || cJSON_IsInvalid(p_obj_msg) ) {
        fprintf(stderr, "ERROR: Failed to parse message\n");
        return NULL;
    }

    return p_obj_msg;
}

/**************************************************************************//**
 * @brief Writes message to I/O (file) descriptor
 *****************************************************************************/
int vs_msg_write(int fd, const char *str_msg)
{
    vs_msg_info_t msg_info;
    if (0 > vs_msg_get_info(str_msg, &msg_info)) {
        fprintf(stderr, "ERROR: Could not get message info\n");
        return -1;
    }
    size_t header_len = vs_msg_get_header_length(str_msg);
    size_t full_len = header_len + msg_info.len + 2;
    size_t cnt = 0; //written bytes counter
    unsigned int trials = VS_MSG_MAX_WRITE_TRIALS;
    ssize_t retval;

    while (cnt < full_len && trials > 0) {
        retval = write(fd, str_msg + cnt, full_len - cnt);
        if (0 > retval) {
            perror("ERROR: Message cannot be written");
            return -1;
        }
        cnt += retval;
        trials--;
    }
    return full_len - cnt;
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
    size_t cnt = 0; //read bytes counter
    unsigned int trials = VS_MSG_MAX_READ_TRIALS;
    ssize_t retval;

    while (cnt < len && trials > 0) {
        retval = read(fd, buffer + cnt, len - cnt);
        if (0 > retval) {
            perror("ERROR: Cannot read message");
            return -1;
        }
        cnt += retval;
        trials --;
    }
    return len - cnt;
}

int vs_msg_read(int fd, char *buffer)
{
    /* Get pre-header */
    if (0 != readn(fd, 2u, buffer)) {
        return -1;
    }
    /* Get header length from pre-header */
    size_t header_length = vs_msg_get_header_length(buffer);
    if (1 > header_length) {
        fprintf(stderr, "Issue with header length (value %d)\n", (int) header_length);
        return -1;
    }
    /* Read header */
    if (0 != readn(fd, header_length, buffer + 2)) {
        fprintf(stderr, "Issue while reading header\n");
        return -1;
    }
    /* Parse header */
    vs_msg_info_t msg_info;
    if (0 > vs_msg_get_info(buffer, &msg_info)) {
        return -1;
    }
    /* Read message content */
    if (0 != readn(fd, msg_info.len, buffer + 2 + header_length)) {
        fprintf(stderr, "Issue while reading message content\n");
        return -1;
    }
    return 0;
}

//EOF

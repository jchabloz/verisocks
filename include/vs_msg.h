/**
 * @file vs_msg.h
 * @author jchabloz
 * @brief Verisocks TCP message definition and utilities
 * @version 0.1
 * @date 2022-08-07
 * @note Only JSON message content with UTF-8 encoding is supported for now.
 * 
 * Verisocks TCP socket message format:
 * --------------------------------------------------------------
 * | Pre-header |   Header (JSON)   |   Message payload (JSON)  |
 * --------------------------------------------------------------
 * 
 */

#ifndef VS_MSG_H
#define VS_MSG_H

#include <stdint.h>
#include <cjson/cJSON.h>

/**
 * @brief Message info structure
 * 
 */
typedef struct vs_msg_info {
    unsigned short int type; /** Message content type */
    size_t len; /** Message length */
} vs_msg_info_t;


#define VS_MSG_MAX_READ_TRIALS 10u /** Defines how many read trials should be attempted*/
#define VS_MSG_MAX_WRITE_TRIALS 10u /** Defines how many write trials should be attempted*/

#define VS_MSG_TXT_JSON 0
#define VS_MSG_BIN 1

extern const char* __VS_MSG_TYPES[2];
#define VS_CMP_TYPE(str, num_type) (strcmp(str, __VS_MSG_TYPES[num_type]) == 0)

/**
 * @brief Returns the pre-header value as the length of the JSON header as an
 * (unformatted) string.
 * 
 * @param header Pointer to a cJSON struct with the message header.
 * @return uint16_t Header length. If longer than can be contained in a 2-bytes
 * value, returns 0.
 */
const uint16_t vs_msg_get_pre_header_value(const cJSON *header);

/**
 * @brief Returns the header for a message with JSON content.
 * 
 * @param msg Pointer the message content. Depending on the type, a pointer to
 * a cJSON struct is expected.
 * @param msg_type Message type (struct vs_msg_info)
 * @return Pointer to a cJSON struct with the message header.
 */
cJSON* vs_msg_create_header(const void *p_msg, vs_msg_info_t msg_type);

#define vs_msg_create_header_json(m) vs_msg_create_header(m, (vs_msg_info_t) {VS_MSG_TXT_JSON, 0})
#define vs_msg_create_header_bin(m,n) vs_msg_create_header(m, (vs_msg_info_t) {VS_MSG_BIN, n})

/**
 * @brief Returns a fully formatted message as based on a cJSON object content,
 * including header and pre-header.
 * 
 * @param msg Pointer the message content. Depending on the type, a pointer to
 * a cJSON struct is expected.
 * @param msg_type Message type (struct vs_msg_info)
 * @return char* Formatted message string.
 * @warning The returned formatted message includes a 2-byte pre-header
 * corresponding to the length of the header directly represented as a binary
 * value. There is a great chance that one of the bytes is 0, thus
 * corresponding to a null character and preventing to directly handle the
 * returned value as a string.
 * @warning The function uses malloc() to reserve a memory block for the
 * returned string. To be freed accordingly if needed.
 */
char* vs_msg_format_message(const void *p_msg, vs_msg_info_t msg_type);

/**
 * @brief Returns a fully formatted message with JSON content, including header
 * and pre-header.
 * 
 * @param msg JSON content as a string. The function checks if the string can
 * be parsed as a valid JSON object.
 * @return char* Formatted message string.
 * @warning The function uses malloc() to reserve a memory block for the
 * returned string. To be freed accordingly if needed.
 */
char* vs_msg_format_message_from_string(const char *msg);

/**
 * @brief Scans a partial or full message to get the header length.
 * 
 * @param message Formatted message, including at least the pre-header.
 * @return const size_t Header length as contained in the pre-header.
 */
const size_t vs_msg_get_header_length(const char* message);

/**
 * @brief Scans the message and extract the type and length information.
 * 
 * @param message Formatted message, including at least pre-header and header.
 * @param msg_info Pointer to a vs_msg_info_t structure.
 * @return Returns 0 if successful, -1 in case of error.
 */
int vs_msg_get_info(const char *message, vs_msg_info_t *msg_info);

/**
 * @brief Scans the message and extract its content as a string/byte array.
 * 
 * @param message Formatted message, including pre-header, header and payload.
 * @param msg_info Pointer to a vs_msg_info_t structure.
 * @return String/byte array. Returns a NULL pointer in case of error.
 * @note Returned array allocated dynamically (malloc).
 */
char* vs_msg_get_content(const char* message, vs_msg_info_t *msg_info);

/**
 * @brief Scans a message and extract its JSON payload.
 * 
 * @param message Formatted message, including pre-header, header and payload.
 * @return cJSON* Pointer to a cJSON struct with the message payload. Returns
 * NULL pointer in case of an error.
 */
cJSON* vs_msg_get_json(const char *message);

/**
 * @brief Write a formatted message to the given descriptor.
 * 
 * @param fd I/O descriptor
 * @param str_msg Formatted message
 * @return Returns 0 if successful (all characters have been written). If > 0,
 * indicates the remaining number of characters. If -1, indicates an error.
 */
int vs_msg_write(int fd, const char *str_msg);

/**
 * @brief Reads formatted message from the given descriptor.
 * 
 * @param fd I/O descriptor
 * @param buffer Pointer to read buffer
 * @return Returns 0 if succesful or -1 if an error occurred.
 */
int vs_msg_read(int fd, char *buffer);

#endif //VS_MSG_H
//EOF

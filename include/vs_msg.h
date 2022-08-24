/**
 * @file vs_msg.h
 * @author jchabloz
 * @brief Verisocks messages definition and utilities
 * @version 0.1
 * @date 2022-08-07
 * @note Only UTF-8 text encoding is supported for now.
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
#include <stddef.h>
#include "cJSON.h"

#define VS_MSG_MAX_READ_TRIALS  10u //Defines how many read trials should be attempted
#define VS_MSG_MAX_WRITE_TRIALS 10u //Defines how many write trials should be attempted

/**
 * @brief Message content type enumeration
 */
enum vs_msg_content_type {
    VS_MSG_TXT = 0,
    VS_MSG_TXT_JSON,
    VS_MSG_BIN,
    VS_MSG_ENUM_LEN //Don't use as a content type! Used to track number of entries.
};

extern const char* VS_MSG_TYPES[VS_MSG_ENUM_LEN];
#define VS_CMP_TYPE(str, num_type) (strcmp(str, VS_MSG_TYPES[num_type]) == 0)

/**
 * @brief Message info structure
 */
typedef struct vs_msg_info {
    enum vs_msg_content_type type; //Content type
    size_t len; //Message content length
} vs_msg_info_t;

/**
 * @brief Returns the header for a message.
 * 
 * @param p_msg Pointer to the message content.
 * @param p_msg_info Pointer to message info structure. If the message content
 * type is binary, the len field is mandatory, for a string-based content type,
 * it can be left uninitialized. For text-based content, the struct is updated
 * with the correct length value.
 * @return Pointer to a cJSON struct representing the message header. Returns a
 * NULL pointer in case of error.
 */
cJSON* vs_msg_create_header(const void *p_msg, vs_msg_info_t *p_msg_info);

/**
 * @brief Returns a fully formatted message as based on a cJSON object content,
 * including header and pre-header.
 * 
 * @param p_msg Pointer the message content. Depending on the type, a pointer to
 * a cJSON struct is expected.
 * @param msg_info Message information (type and length)
 * @return char* Formatted message string.
 * @warning The returned formatted message includes a 2-byte pre-header
 * corresponding to the length of the header directly represented as a binary
 * value. There is a great chance that one of the bytes is 0, thus
 * corresponding to a null character and preventing to directly handle the
 * returned value as a string.
 * @warning The function uses malloc() to reserve a memory block for the
 * returned string. To be freed accordingly if needed.
 */
char* vs_msg_create_message(const void *p_msg, vs_msg_info_t msg_info);

/**
 * @brief Returns a fully formatted message with JSON content, including header
 * and pre-header.
 * 
 * @param str_message JSON content as a string. The function checks if the
 * string can be parsed as a valid JSON object.
 * @return char* Formatted message string. Returns NULL if error.
 * @warning The function uses malloc() to reserve a memory block for the
 * returned string. To be freed accordingly if needed.
 */
char* vs_msg_create_json_message_from_string(const char *str_message);

/**
 * @brief Scans a partial or full message to get the header length.
 * 
 * @param message Formatted message, including at least the pre-header.
 * @return const size_t Header length as contained in the pre-header.
 * @warning The caller is responsible to guarantee that at least the 2-bytes
 * pre-header is available and not garbage.
 */
size_t vs_msg_read_header_length(const char* message);

/**
 * @brief Scans a partial or full message to extract the type and length
 * information from the message header.
 * 
 * @param message Formatted message, including at least pre-header and header.
 * @param msg_info Pointer to a vs_msg_info_t structure.
 * @return Returns 0 if successful, -1 in case of error.
 */
int vs_msg_read_info(const char *message, vs_msg_info_t *p_msg_info);

/**
 * @brief Scans the message and extract its content as a string/byte array.
 * 
 * @param message Formatted message, including pre-header, header and payload.
 * @param msg_info Pointer to a vs_msg_info_t structure.
 * @return String/byte array. Returns a NULL pointer in case of error.
 * @note Returned array allocated dynamically (malloc).
 */
char* vs_msg_read_content(const char* message, vs_msg_info_t *p_msg_info);

/**
 * @brief Scans a message and extract its JSON payload.
 * 
 * @param message Formatted message, including pre-header, header and payload.
 * @return cJSON* Pointer to a cJSON struct with the message payload. Returns
 * NULL pointer in case of an error.
 */
cJSON* vs_msg_read_json(const char *message);

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
 * @param len Size of buffer. If the message to read is longer than the buffer,
 * the buffer is filled to its max (without null termination guaranteed!) and
 * the function returns -1.
 * @return Returns 0 if succesful or -1 if an error occurred.
 */
int vs_msg_read(int fd, char *buffer, size_t len);

#endif //VS_MSG_H
//EOF

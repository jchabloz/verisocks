#ifndef VSL_MACROS_HPP
#define VSL_MACROS_HPP

/**
 * @brief Helper macro - Error handler lambda function
 * 
 * @param vx VslInteg object
 * @param message Message to be used in case of error
 */
#define VSL_ERROR_HANDLER(vx, message) \
    auto handle_error = [&vx]() { \
        vs_msg_return(vx.fd_client_socket, "error", \
            message, &vx.uuid); \
        vx._state = VSL_STATE_WAITING; \
    }

/**
 * @brief Helper macro to read a numerical field from a JSON command
 *
 * This macro assumes the existence of an "handle_error" function (typ a
 * lambda) within the current scope.
 *
 * @param obj Pointer to cJSON object
 * @param name Name of the field
 * @param var Variable to store the value (double)
 */
#define VSL_MSG_READ_NUM_NO_DECL(obj, name, var) \
    do { \
        cJSON *p_item_ ## name; \
        p_item_ ## name = cJSON_GetObjectItem(obj, #name); \
        if (nullptr == p_item_ ## name) { \
            vs_log_mod_error(__MOD__, \
                "Numerical field " #name " invalid/not found"); \
            handle_error(); \
            return; \
        } \
        var = cJSON_GetNumberValue(p_item_ ## name); \
        if (std::isnan(var)) { \
            vs_log_mod_error(__MOD__, \
                "Numerical field " #name " invalid (NaN)"); \
            handle_error(); \
            return; \
        } \
    } while (0)

/**
 * @brief Helper macro to read a numerical field from a JSON command
 *
 * This macro declares a double name_value variable within the current scope.
 * If the name_value variable shall have a wide scope, use rather the macro
 * VS_MSG_READ_NUM_NO_DECL instead.
 *
 * This macro assumes the existence of an "handle_error" function (typ a
 * lambda) within the current scope.
 *
 * @param obj Pointer to cJSON object
 * @param name Name of the field
 */
#define VSL_MSG_READ_NUM(obj, name) \
    double name; \
    VSL_MSG_READ_NUM_NO_DECL(obj, name, name)

/**
 * @brief Helper macro to read a text field from a JSON command
 *
 * This macro assumes the existence of an "handle_error" function (typ a
 * lambda) within the current scope.
 *
 * @param obj Pointer to cJSON object
 * @param name Name of the field
 * @param var Variable to store the read text (char*)
 */
#define VSL_MSG_READ_STR_NO_DECL(obj, name, var) \
    do { \
        cJSON *p_item_ ## name; \
        p_item_ ## name = cJSON_GetObjectItem(obj, #name); \
        if (nullptr == p_item_ ## name) { \
            vs_log_mod_error(__MOD__, \
                "String field " #name " invalid/not found"); \
            handle_error(); \
            return; \
        } \
        var = cJSON_GetStringValue(p_item_ ## name); \
        if ((nullptr == var) || std::string(var).empty()) { \
            vs_log_mod_error(__MOD__, \
                "String field " #name " NULL or empty"); \
            handle_error(); \
            return; \
        } \
    } while (0)

/**
 * @brief Helper macro to read an optional text field from a JSON command
 *
 * This macro assumes the existence of an "handle_error" function (typ a
 * lambda) within the current scope.
 *
 * @param obj Pointer to cJSON object
 * @param name Name of the field
 * @param var Variable to store the read text (char*)
 * @param flag Variable to indicate if the field exists (bool)
 */
#define VSL_MSG_READ_STR_NO_DECL_OPT(obj, name, var, flag) \
    do { \
        cJSON *p_item_ ## name; \
        p_item_ ## name = cJSON_GetObjectItem(obj, #name); \
        flag = false; \
        if (nullptr != p_item_ ## name) { \
            var = cJSON_GetStringValue(p_item_ ## name); \
            if ((nullptr == var) || std::string(var).empty()) { \
                vs_log_mod_error(__MOD__, \
                "String field " #name " NULL or empty"); \
                handle_error(); \
                return; \
            } \
            flag = true; \
        } \
    } while (0)

/**
 * @brief Helper macro to read a text field from a JSON command
 *
 * This macro declares a char *str_name variable within the current scope.
 * If the name_value variable shall have a wide scope, use rather the macro
 * VS_MSG_READ_STR_NO_DECL instead.
 *
 * This macro assumes the existence of an "handle_error" function (typ a
 * lambda) within the current scope.
 *
 * @param obj Pointer to cJSON object
 * @param name Name of the field
 */
#define VSL_MSG_READ_STR(obj, name) \
    char *cstr_ ## name; \
    VSL_MSG_READ_STR_NO_DECL(obj, name, cstr_ ## name); \
    std::string str_ ## name(cstr_ ## name);

/**
 * @brief Helper macro to read an optional text field from a JSON command
 *
 * This macro declares a char *str_name variable within the current scope.
 * If the name_value variable shall have a wide scope, use rather the macro
 * VS_MSG_READ_STR_NO_DECL instead.
 *
 * This macro assumes the existence of an "handle_error" function (typ a
 * lambda) within the current scope.
 *
 * @param obj Pointer to cJSON object
 * @param name Name of the field
 */
#define VSL_MSG_READ_STR_OPT(obj, name) \
    char *cstr_ ## name; \
    bool has_ ## name; \
    VSL_MSG_READ_STR_NO_DECL_OPT(obj, name, cstr_ ## name, has_ ## name)

/**
 * @brief Helper macro to initialize a cJSON object
 * 
 * @param obj Pointer to cJSON object
 */
#define VSL_MSG(obj) \
    do { \
        obj = cJSON_CreateObject(); \
        if (nullptr == obj) { \
            vs_log_mod_error(__MOD__, "Could not create cJSON object"); \
            handle_error(); \
            return; \
        } \
    } while (0)

/**
 * @brief Helper macro to add a text field to a cJSON object
 * 
 * @param obj Pointer to cJSON object
 * @param key Key to use for the text field
 * @param val Value to use for the text field
 */
#define VSL_MSG_ADD_STR(obj, key, val) \
    do { \
        if (nullptr == cJSON_AddStringToObject(obj, key, val)) { \
            vs_log_mod_error(__MOD__, "Could not add string to object"); \
            handle_error(); \
            return; \
        } \
    } while (0)

/**
 * @brief Helper macro to add a numerical field to a cJSON object
 * 
 * @param obj Pointer to cJSON object
 * @param key Key to use for the numerical field
 * @param val Value to use for the numerical field
 */
#define VSL_MSG_ADD_NUM(obj, key, val) \
    do { \
        if (nullptr == cJSON_AddNumberToObject(obj, key, val)) { \
            vs_log_mod_error(__MOD__, "Could not add number to object"); \
            handle_error(); \
            return; \
        } \
    } while (0)

/**
 * @brief Helper macro to transform a cJSON object to a text
 * 
 * @param str_obj Pointer to char object
 * @param obj Pointer to cJSON object
 * @param msg_info msg_info structure
 */
#define VSL_MSG_CREATE(str_obj, obj, msg_info) \
    do { \
        str_obj = vs_msg_create_message(obj, &msg_info); \
        if (nullptr == str_obj) { \
            vs_log_mod_error(__MOD__, "NULL pointer"); \
            handle_error(); \
            return; \
        } \
    } while (0)

/**
 * @brief Helper macro to write a return message
 * 
 * @param str_obj Pointer to char object to be written
 * @param vx Pointer to VslInteg object
 */
#define VSL_MSG_WRITE(str_obj, vx) \
    do { \
        if (0 > vs_msg_write(vx.fd_client_socket, str_obj)) { \
            vs_log_mod_error(__MOD__, "Error writing return message"); \
            handle_error(); \
            return; \
        } \
    } while (0)


#endif // VSL_MACROS_HPP
// EOF
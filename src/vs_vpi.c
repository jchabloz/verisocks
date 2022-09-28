/**************************************************************************//**
 * @file vs_vpi.c
 * @author jchabloz (jeremie.chabloz@a3.epfl.ch)
 * @brief Verisocks VPI functions 
 * @version 0.1
 * @date 2022-08-27
 * 
 * @copyright Copyright (c) Jérémie Chabloz, 2022
 * 
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <iverilog/vpi_user.h>

#include "vs_logging.h"
#include "vs_msg.h"
#include "vs_server.h"
#include "vs_vpi.h"


/**
 * @brief Type for a command handler function pointer
 */
typedef int (*cmd_handler_t)(vs_vpi_data_t*, const cJSON*);

/**
 * @brief Struct type for commands
 * Associates a command name with a command handler function pointer
 */
typedef struct vs_vpi_cmd {
    cmd_handler_t cmd_handler;  // Pointer to handler function
    const char *cmd_name;       // Command name
} vs_vpi_cmd_t;

/**
 * @brief Helper macro to declare a command handler function
 * @param cmd Command short name
 */
#define VS_VPI_CMD_HANDLER(cmd) static int VS_VPI_ ## cmd ## _cmd_handler(\
vs_vpi_data_t *p_data, const cJSON *p_msg)

/**
 * @brief Helper marco to define a command structure with a command name and
 * associated command handler function pointer.
 * @param cmd Command short name
 */
#define VS_VPI_CMD(cmd) {VS_VPI_ ## cmd ## _cmd_handler, #cmd}

/* Declare prototypes for command handler functions so that they can be used
 * in the following command table. Commands are implemented at the end of this
 * file.
 */
VS_VPI_CMD_HANDLER(info);
VS_VPI_CMD_HANDLER(finish);
VS_VPI_CMD_HANDLER(stop);
VS_VPI_CMD_HANDLER(set_value);
VS_VPI_CMD_HANDLER(get_value);
VS_VPI_CMD_HANDLER(run);

/**
 * @brief Table registering the command handlers
 * @warning The table has to be terminated by a struct vs_vpi_cmd with a NULL
 * cmd_name field.
 */
static const vs_vpi_cmd_t vs_vpi_cmd_table[] =
{
    VS_VPI_CMD(info),
    VS_VPI_CMD(finish),
    VS_VPI_CMD(stop),
    VS_VPI_CMD(set_value),
    VS_VPI_CMD(get_value),
    VS_VPI_CMD(run),
    {NULL, NULL}
};

/**
 * @brief Return a command handler pointer for a given command name (case
 * insensitive)
 * @param cmd Command name
 * @return Command handler function pointer. Returns NULL if the command has no
 * registered handler.
 */
static cmd_handler_t vs_vpi_get_cmd_handler(const char *cmd)
{
    const vs_vpi_cmd_t *ptr_cmd = vs_vpi_cmd_table;

    while(ptr_cmd->cmd_name != NULL) {
        if (0 == strcasecmp(ptr_cmd->cmd_name, cmd)) {
            return ptr_cmd->cmd_handler;
        }
        ptr_cmd++;
    }
    return NULL;
}

int vs_vpi_process_command(vs_vpi_data_t *p_data, const cJSON *p_cmd)
{
    /* Sanity check on parameters */
    if (NULL == p_data) {
        vs_vpi_log_error("NULL pointer to data");
        return -1;
    }
    if (NULL == p_cmd) {
        vs_vpi_log_error("NULL pointer to cJSON cmd");
        goto error;
    }

    /* Get the command field from the JSON message content */
    cJSON *p_item_cmd = cJSON_GetObjectItem(p_cmd, "command");
    if (NULL == p_item_cmd) {
        vs_vpi_log_error("Command field invalid/not found");
        goto error;
    }

    /* Get the command as a string */
    char *str_cmd = cJSON_GetStringValue(p_item_cmd);
    if (NULL == str_cmd) {
        vs_vpi_log_error("Command field NULL pointer");
        goto error;
    }
    if (strcmp(str_cmd, "") == 0) {
        vs_vpi_log_warning("Command empty");
        goto warning;
    }
    vs_vpi_log_debug("Processing command %s", str_cmd);

    /* Look up command handler */
    cmd_handler_t cmd_handler = vs_vpi_get_cmd_handler(str_cmd);
    if (NULL == cmd_handler) {
        vs_vpi_log_error("Command handler not found for command %s",
            str_cmd);
        goto warning;
    }

    /* Note: No need to clean-up either str_cmd or p_item_cmd as they are both
       freed when using cJSON_Delete(p_cmd) from the upper level. Freeing them
       here would result in an error.
    */

    /* Execute command handler and forward returned value */
    return (*cmd_handler)(p_data, p_cmd);

    /* Error handling - Discard and wait for new command */
    warning:
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command, discarding it");
    p_data->state = VS_VPI_STATE_WAITING;
    return -1;

    /* Error handling - Aborting simulation */
    error:
    p_data->state = VS_VPI_STATE_ERROR;
    return -1;
}

int vs_vpi_return(int fd, const char *str_type, const char *str_value)
{
    cJSON *p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_vpi", "Could not create cJSON object");
        return -1;
    }

    if (NULL == cJSON_AddStringToObject(p_msg, "type", str_type)) {
        vs_log_mod_error("vs_vpi", "Could not add string to object");
        goto error;
    }

    if (NULL == cJSON_AddStringToObject(p_msg, "value", str_value)) {
        vs_log_mod_error("vs_vpi", "Could not add string to object");
        goto error;
    }

    char *str_msg = vs_msg_create_message(p_msg,
        (vs_msg_info_t) {VS_MSG_TXT_JSON, 0});
    if (NULL == str_msg) {
        vs_log_mod_error("vs_vpi", "NULL pointer");
        goto error;
    }
    cJSON_Delete(p_msg);

    int retval;
    retval = vs_msg_write(fd, str_msg);
    if (0 > retval) {
        vs_log_mod_error("vs_vpi", "Error writing return message");
        free(str_msg);
        goto error;
    }

    return 0;

    error:
    cJSON_Delete(p_msg);
    return -1;
}

/******************************************************************************
 * Command handler functions - Implementation
 * Note: using the helper macro VS_VPI_CMD_HANDLER() makes the following
 * parameters always available:
 * - vs_vpi_data_t *p_data
 * - cJSON *p_msg
 *****************************************************************************/
VS_VPI_CMD_HANDLER(info)
{
    vs_vpi_log_info("Command \"info\" received.");

    /* Get the value from the JSON message content */
    cJSON *p_item_val = cJSON_GetObjectItem(p_msg, "value");
    if (NULL == p_item_val) {
        vs_vpi_log_error("Command field invalid/not found");
        goto error;
    }

    /* Get the command as a string */
    char *str_val = cJSON_GetStringValue(p_item_val);
    if ((NULL == str_val) || (strcmp(str_val, "") == 0)) {
        vs_vpi_log_error("Command field NULL or empty");
        goto error;
    }

    /* Print info value */
    vpi_printf("INFO [Verisocks]: received info %s\n", str_val);

    /* Return an acknowledgement return message */
    vs_vpi_return(p_data->fd_client_socket, "ack", "command received");
    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    error:
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command");
    p_data->state = VS_VPI_STATE_WAITING;
    return -1;
}

VS_VPI_CMD_HANDLER(finish)
{
    vs_vpi_log_info("Command \"finish\" received. Terminating simulation...");
    vpi_control(vpiFinish);
    p_data->state = VS_VPI_STATE_FINISHED;
    return 0;
}

VS_VPI_CMD_HANDLER(stop)
{
    vs_vpi_log_info("Command \"stop\" received. Stopping simulation...");
    vpi_control(vpiStop);
    return 0;
}

VS_VPI_CMD_HANDLER(set_value)
{
    /* Check state consistency */
    if (VS_VPI_STATE_PROCESSING != p_data->state) {
        vs_vpi_log_error("[set_value] Inconsistent state");
        goto error;
    }

    /* Get value path */
    cJSON *p_item_path = cJSON_GetObjectItem(p_msg, "path");
    char *str_path = cJSON_GetStringValue(p_item_path);
    if (NULL == str_path || (strcmp(str_path, "") == 0)) {
        vs_vpi_log_error("[set_value] Path field empty/not found");
        goto error;
    }

    /* Get value type */
    cJSON *p_item_type = cJSON_GetObjectItem(p_msg, "type");
    char *str_type = cJSON_GetStringValue(p_item_type);
    if (NULL == str_type || (strcmp(str_path, "") == 0)) {
        vs_vpi_log_error("[set_value] Type field empty/not found");
        goto error;
    }

    /* Get value, depends on type */
    //cJSON *p_item_value = cJSON_GetObjectItem(p_msg, "value");
    do {
        if (strcmp(str_type, "logic") == 0) {
            /* TODO */
            break;
        }
        if (strcmp(str_type, "logic_array") == 0) {
            /* TODO */
            break;
        }
        if (strcmp(str_type, "real") == 0) {
            /* TODO */
            break;
        }
        vs_vpi_log_error("[set_value] Unknown type");
        goto error;
    } while(0);

    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_ERROR;
    return -1;
}

VS_VPI_CMD_HANDLER(get_value)
{
    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    /* Error handling */
    // error:
    p_data->state = VS_VPI_STATE_ERROR;
    return -1;
}

VS_VPI_CMD_HANDLER(run)
{
    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    /* Error handling */
    // error:
    p_data->state = VS_VPI_STATE_ERROR;
    return -1;
}

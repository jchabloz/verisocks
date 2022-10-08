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
#include <math.h>
#include <iverilog/vpi_user.h>

#include "vs_logging.h"
#include "vs_msg.h"
#include "vs_server.h"
#include "vs_utils.h"
#include "vs_vpi.h"


/* Declare prototypes for command handler functions so that they can be used
 * in the following command tables. Commands are implemented at the end of this
 * file.
 */
VS_VPI_CMD_HANDLER(info);
VS_VPI_CMD_HANDLER(finish);
VS_VPI_CMD_HANDLER(stop);
VS_VPI_CMD_HANDLER(exit);
VS_VPI_CMD_HANDLER(run);
VS_VPI_CMD_HANDLER(get);

/**
 * @brief Table registering the command handlers
 * @warning The table has to be terminated by a struct vs_vpi_cmd with NULL
 * fields.
 */
static const vs_vpi_cmd_t vs_vpi_cmd_table[] =
{
    VS_VPI_CMD(info),
    VS_VPI_CMD(finish),
    VS_VPI_CMD(stop),
    VS_VPI_CMD(exit),
    VS_VPI_CMD(run),
    VS_VPI_CMD(get),
    {NULL, NULL, NULL}
};

/**
 * @brief Return a command handler pointer for a given command name (case
 * insensitive) and a given command handlers register table.
 * @param p_cmd_table Pointer to a command handlers register table
 * @param str_cmd Command name (case-insensitive)
 * @return Command handler function pointer. Returns NULL if the command has no
 * registered handler.
 */
cmd_handler_t vs_vpi_get_cmd_handler(
    const vs_vpi_cmd_t *p_cmd_table, const char *str_cmd)
{
    vs_log_mod_debug("vs_vpi", "Looking for command with key %s", str_cmd);
    while(p_cmd_table->cmd_name != NULL) {
        if (
            ((NULL == p_cmd_table->cmd_key) &&
             (0 == strcasecmp(p_cmd_table->cmd_name, str_cmd))) ||
            ((NULL != p_cmd_table->cmd_key) &&
             (0 == strcasecmp(p_cmd_table->cmd_key, str_cmd)))
        ) {
            return p_cmd_table->cmd_handler;
        }
        p_cmd_table++;
    }
    return NULL;
}

int vs_vpi_process_command(vs_vpi_data_t *p_data)
{
    /* Sanity check on parameters */
    if (NULL == p_data) {
        vs_vpi_log_error("NULL pointer to data");
        return -1;
    }
    if (NULL == p_data->p_cmd) {
        vs_vpi_log_error("NULL pointer to cJSON cmd");
        goto error;
    }

    /* Get the command field from the JSON message content */
    cJSON *p_item_cmd = cJSON_GetObjectItem(p_data->p_cmd, "command");
    if (NULL == p_item_cmd) {
        vs_vpi_log_error("Command field invalid/not found");
        goto warning;
    }

    /* Get the command as a string */
    char *str_cmd = cJSON_GetStringValue(p_item_cmd);
    if (NULL == str_cmd) {
        vs_vpi_log_error("Command field NULL pointer");
        goto warning;
    }
    if (strcmp(str_cmd, "") == 0) {
        vs_vpi_log_warning("Command empty");
        goto warning;
    }
    vs_vpi_log_debug("Processing command %s", str_cmd);

    /* Look up command handler */
    cmd_handler_t cmd_handler =
        vs_vpi_get_cmd_handler(vs_vpi_cmd_table, str_cmd);
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
    return (*cmd_handler)(p_data);

    /* Error handling - Discard and wait for new command */
    warning:
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command. Discarding.");
    p_data->state = VS_VPI_STATE_WAITING;
    return -1;

    /* Error handling - Aborting simulation */
    error:
    p_data->state = VS_VPI_STATE_ERROR;
    return -1;
}

int vs_vpi_return(int fd, const char *str_type, const char *str_value)
{
    cJSON *p_msg;
    char *str_msg;

    p_msg = cJSON_CreateObject();
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

    str_msg = vs_msg_create_message(p_msg,
        (vs_msg_info_t) {VS_MSG_TXT_JSON, 0});
    if (NULL == str_msg) {
        vs_log_mod_error("vs_vpi", "NULL pointer");
        goto error;
    }

    int retval;
    retval = vs_msg_write(fd, str_msg);
    if (0 > retval) {
        vs_log_mod_error("vs_vpi", "Error writing return message");
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
 * Command handler functions definitions
 *****************************************************************************/
VS_VPI_CMD_HANDLER(info)
{
    vs_vpi_log_info("Command \"info\" received.");

    /* Get the value from the JSON message content */
    cJSON *p_item_val = cJSON_GetObjectItem(p_data->p_cmd, "value");
    if (NULL == p_item_val) {
        vs_vpi_log_error("Command field \"value\" invalid/not found");
        goto error;
    }

    /* Get the info command argument as a string */
    char *str_val = cJSON_GetStringValue(p_item_val);
    if ((NULL == str_val) || (strcmp(str_val, "") == 0)) {
        vs_vpi_log_error("Command field \"value\" NULL or empty");
        goto error;
    }

    /* Print received info value */
    vs_vpi_log_info("%s", str_val);

    /* Return an acknowledgement */
    vs_vpi_return(p_data->fd_client_socket, "ack", "command info received");

    /* Set state to "waiting next command" */
    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    /* Error handling */
    error:
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command info - Discarding");
    p_data->state = VS_VPI_STATE_WAITING;
    return -1;
}

VS_VPI_CMD_HANDLER(finish)
{
    vs_vpi_log_info("Command \"finish\" received. Terminating simulation...");
    vs_vpi_return(p_data->fd_client_socket, "ack",
        "Processing finish command - Terminating simulation.");
    vpi_control(vpiFinish);
    p_data->state = VS_VPI_STATE_FINISHED;
    return 0;
}

VS_VPI_CMD_HANDLER(stop)
{
    vs_vpi_log_info("Command \"stop\" received. Stopping simulation and \
relaxing control to simulator...");
    vs_vpi_return(p_data->fd_client_socket, "ack",
        "Processing stop command - Stopping simulation.");
    vpi_control(vpiStop);
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;
}

VS_VPI_CMD_HANDLER(exit)
{
    vs_vpi_log_info("Command \"exit\" received. Quitting Verisocks ...");
    vs_vpi_return(p_data->fd_client_socket, "ack",
        "Processing exit command - Quitting Verisocks.");
    p_data->state = VS_VPI_STATE_EXIT;
    return 0;
}

VS_VPI_CMD_HANDLER(run)
{
    /* Get the callback type (cb_type) field from the JSON message content */
    cJSON *p_item_cb = cJSON_GetObjectItem(p_data->p_cmd, "cb_type");
    if (NULL == p_item_cb) {
        vs_vpi_log_error("Command field \"cb_type\" invalid/not found");
        goto error;
    }
    /* Get the cb_type command argument as a string */
    char *str_cb = cJSON_GetStringValue(p_item_cb);
    if ((NULL == str_cb) || (strcmp(str_cb, "") == 0)) {
        vs_vpi_log_error("Command field \"sel\" NULL or empty");
        goto error;
    }

    /* Look up sub-command handler */
    cmd_handler_t cmd_handler =
        vs_vpi_get_cmd_handler(vs_vpi_cmd_run_table, str_cb);
    if (NULL == cmd_handler) {
        vs_vpi_log_error("Command handler not found for cb_type=%s", str_cb);
        goto error;
    }

    /* Execute sub-command handler and forward returned value */
    return (*cmd_handler)(p_data);

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command run - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(get)
{
    /* Get the value from the JSON message content */
    cJSON *p_item_sel = cJSON_GetObjectItem(p_data->p_cmd, "sel");
    if (NULL == p_item_sel) {
        vs_vpi_log_error("Command field \"sel\" invalid/not found");
        goto error;
    }

    /* Get the info command argument as a string */
    char *str_sel = cJSON_GetStringValue(p_item_sel);
    if ((NULL == str_sel) || (strcmp(str_sel, "") == 0)) {
        vs_vpi_log_error("Command field \"sel\" NULL or empty");
        goto error;
    }
    vs_vpi_log_info("Command \"get(sel=%s)\" received.", str_sel);

    /* Look up sub-command handler */
    cmd_handler_t cmd_handler =
        vs_vpi_get_cmd_handler(vs_vpi_cmd_get_table, str_sel);
    if (NULL == cmd_handler) {
        vs_vpi_log_error("Command handler not found for sel=%s", str_sel);
        goto error;
    }

    /* Execute sub-command handler and forward returned value */
    return (*cmd_handler)(p_data);

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command get - Discarding");
    return -1;
}

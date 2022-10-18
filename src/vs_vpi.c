/**************************************************************************//**
 * @file vs_vpi.c
 * @author jchabloz
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
VS_VPI_CMD_HANDLER(set);

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
    VS_VPI_CMD(set),
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
Info command handler
******************************************************************************/
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

/******************************************************************************
Finish command handler
******************************************************************************/
VS_VPI_CMD_HANDLER(finish)
{
    vs_vpi_log_info("Command \"finish\" received. Terminating simulation...");
    vs_vpi_return(p_data->fd_client_socket, "ack",
        "Processing finish command - Terminating simulation.");
    vpi_control(vpiFinish);
    p_data->state = VS_VPI_STATE_EXIT;
    return 0;
}

/******************************************************************************
Stop command handler
******************************************************************************/
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

/******************************************************************************
Exit command handler
******************************************************************************/
VS_VPI_CMD_HANDLER(exit)
{
    vs_vpi_log_info("Command \"exit\" received. Quitting Verisocks ...");
    vs_vpi_return(p_data->fd_client_socket, "ack",
        "Processing exit command - Quitting Verisocks.");
    p_data->state = VS_VPI_STATE_EXIT;
    return 0;
}

/******************************************************************************
Run command handler
******************************************************************************/
VS_VPI_CMD_HANDLER(run)
{
    /* Get the callback type (cb) field from the JSON message content */
    cJSON *p_item_cb = cJSON_GetObjectItem(p_data->p_cmd, "cb");
    if (NULL == p_item_cb) {
        vs_vpi_log_error("Command field \"cb\" invalid/not found");
        goto error;
    }
    /* Get the cb command argument as a string */
    char *str_cb = cJSON_GetStringValue(p_item_cb);
    if ((NULL == str_cb) || (strcmp(str_cb, "") == 0)) {
        vs_vpi_log_error("Command field \"cb\" NULL or empty");
        goto error;
    }

    /* Look up sub-command handler */
    cmd_handler_t cmd_handler =
        vs_vpi_get_cmd_handler(vs_vpi_cmd_run_table, str_cb);
    if (NULL == cmd_handler) {
        vs_vpi_log_error("Command handler not found for cb=%s", str_cb);
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

/******************************************************************************
Get command handler
******************************************************************************/
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

/******************************************************************************
Set command handler
******************************************************************************/
VS_VPI_CMD_HANDLER(set)
{
    /* Get the object path from the JSON message content */
    cJSON *p_item_path = cJSON_GetObjectItem(p_data->p_cmd, "path");
    if (NULL == p_item_path) {
        vs_vpi_log_error("Command field \"path\" invalid/not found");
        goto error;
    }

    /* Get the info command argument as a string */
    char *str_path = cJSON_GetStringValue(p_item_path);
    if ((NULL == str_path) || (strcmp(str_path, "") == 0)) {
        vs_vpi_log_error("Command field \"path\" NULL or empty");
        goto error;
    }

    /* Attempt to get the object handle */
    vpiHandle h_obj;
    h_obj = vpi_handle_by_name(str_path, NULL);
    if (NULL == h_obj) {
        vs_vpi_log_error("Attempt to get handle to %s unsuccessful", str_path);
        goto error;
    }

    /* If the object is a named event, there is no need to get a value
    command argument */
    if (vpiNamedEvent == vpi_get(vpiType, h_obj)) {
        vs_vpi_log_info("Command \"set(path=%s)\" received. Target path \
corresponds to a named event.", str_path);
        vpi_put_value(h_obj, NULL, NULL, vpiNoDelay);
        vs_vpi_return(p_data->fd_client_socket, "ack",
            "Processed command \"set\"");
        return 0;
    }

    cJSON *p_item_val;
    double value;

    /* If the object is a memory array, we expect the value command argument to
    be a list of values with the same length */
    if (vpiMemory == vpi_get(vpiType, h_obj)) {
        p_item_val = cJSON_GetObjectItem(p_data->p_cmd, "value");
        if (NULL == p_item_val) {
            vs_vpi_log_error("Command field \"value\" invalid/not found");
            goto error;
        }
        if (!cJSON_IsArray(p_item_val)) {
            vs_vpi_log_error("Command field \"value\" should be an array");
            goto error;
        }
        PLI_INT32 mem_size = vpi_get(vpiSize, h_obj);
        if (mem_size != cJSON_GetArraySize(p_item_val)) {
            vs_vpi_log_error(
                "Command field \"value\" should be an array of length %d",
                mem_size);
            goto error;
        }
        vs_vpi_log_info("Command \"set(path=%s, value=[...])\" received. \
Target path corresponds to a memory array.", str_path);
        vpiHandle mem_iter;
        vpiHandle h_mem_word;
        mem_iter = vpi_iterate(vpiMemoryWord, h_obj);
        if (NULL == mem_iter) {
            vs_log_mod_error("vs_vpi", "Could not initialize memory iterator");
            goto error;
        }

        cJSON *iterator;
        cJSON_ArrayForEach(iterator, p_item_val) {
            value = cJSON_GetNumberValue(iterator);
            h_mem_word = vpi_scan(mem_iter);
            if (0 > vs_utils_set_value(h_mem_word, value)) {
                vpi_free_object(mem_iter);
                goto error;
            }
        }

        vpi_free_object(mem_iter);
        vs_vpi_return(p_data->fd_client_socket, "ack",
            "Processed command \"set\"");
        return 0;
    }

    /* All other object types */

    /* Get the value from the JSON message content */
    p_item_val = cJSON_GetObjectItem(p_data->p_cmd, "value");
    if (NULL == p_item_val) {
        vs_vpi_log_error("Command field \"value\" invalid/not found");
        goto error;
    }
    value = cJSON_GetNumberValue(p_item_val);
    if (isnan(value)) {
        vs_vpi_log_error("Command field \"value\" invalid (NaN)");
        goto error;
    }
    vs_vpi_log_info("Command \"set(path=%s, value=%f)\" received.",
        str_path, value);

    if (0 > vs_utils_set_value(h_obj, value)) goto error;

    vs_vpi_return(p_data->fd_client_socket, "ack",
        "Processed command \"set\"");
    return 0;

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command set - Discarding");
    return -1;
}

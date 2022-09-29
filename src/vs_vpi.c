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
#include "vs_vpi.h"

/**
 * @brief Type for a command handler function pointer
 */
typedef int (*cmd_handler_t)(vs_vpi_data_t*);

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
vs_vpi_data_t *p_data)

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
VS_VPI_CMD_HANDLER(exit);
VS_VPI_CMD_HANDLER(run);
VS_VPI_CMD_HANDLER(get);

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
    VS_VPI_CMD(exit),
    VS_VPI_CMD(run),
    VS_VPI_CMD(get),
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
 * Command handler functions - Implementation
 * Note: using the helper macro VS_VPI_CMD_HANDLER() makes the following
 * parameter(s) always available:
 * - vs_vpi_data_t *p_data
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
    vpi_printf("INFO [Verisocks]: Received info: %s\n", str_val);

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
    vs_vpi_log_info("Command \"exit\" received. Quitting Verisocks and \
relaxing control to simulator to resume simulation...");
    vs_vpi_return(p_data->fd_client_socket, "ack",
        "Processing exit command - Quitting Verisocks.");
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;
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

VS_VPI_CMD_HANDLER(get)
{
    char *str_msg;
    cJSON *p_msg;

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

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_vpi", "Could not create cJSON object");
        goto error;
    }

	PLI_INT32 retval;
    /* Use "sel" field to choose action */
    if (strcasecmp("sim_info", str_sel) == 0) {
        vs_vpi_log_debug("Get simulator info...");
        s_vpi_vlog_info vlog_info;
        retval = vpi_get_vlog_info(&vlog_info);
	    if (0 > retval) {
            vs_log_mod_error("vs_vpi", "Could not get vlog_info");
            goto error;
        }
        if (NULL == cJSON_AddStringToObject(p_msg, "product",
                                            vlog_info.product)) {
            vs_log_mod_error("vs_vpi", "Could not add string to object");
            goto error;
        }
        if (NULL == cJSON_AddStringToObject(p_msg, "version",
                                            vlog_info.version)) {
            vs_log_mod_error("vs_vpi", "Could not add string to object");
            goto error;
        }

        str_msg = vs_msg_create_message(p_msg,
            (vs_msg_info_t) {VS_MSG_TXT_JSON, 0});
        if (NULL == str_msg) {
            vs_log_mod_error("vs_vpi", "NULL pointer");
            goto error;
        }

        retval = (PLI_INT32) vs_msg_write(p_data->fd_client_socket, str_msg);
        if (0 > retval) {
            vs_log_mod_error("vs_vpi", "Error writing return message");
            goto error;
        }
    }
    else if (strcasecmp("sim_time", str_sel) == 0) {
        vs_vpi_log_debug("Getting simulator time...");

	    s_vpi_time s_time;
	    s_time.type = vpiSimTime;
	    vpi_get_time(NULL, &s_time);
	    PLI_INT32 time_precision = vpi_get(vpiTimePrecision, NULL);
	    PLI_UINT64 sim_time =
            (PLI_UINT64) s_time.low + ((PLI_UINT64) s_time.high << 32u);
	    double sim_time_sec = sim_time * pow(10.0, time_precision);
    	vs_vpi_log_debug("Sim time: %.3f us\n", sim_time_sec);

        if (NULL == cJSON_AddNumberToObject(p_msg, "sim_time_sec",
            sim_time_sec)) {
            vs_log_mod_error("vs_vpi", "Could not add number to object");
            goto error;
        }
        str_msg = vs_msg_create_message(p_msg,
            (vs_msg_info_t) {VS_MSG_TXT_JSON, 0});
        if (NULL == str_msg) {
            vs_log_mod_error("vs_vpi", "NULL pointer");
            goto error;
        }
        retval = (PLI_INT32) vs_msg_write(p_data->fd_client_socket, str_msg);
        if (0 > retval) {
            vs_log_mod_error("vs_vpi", "Error writing return message");
            goto error;
        }
    } else {
        vs_vpi_log_error("Command field \"sel\" value unknown (%s)", str_sel);
        goto error;
    }

    /* Normal exit */
    if (NULL != p_msg) cJSON_Delete(p_msg);
    if (NULL != str_msg) cJSON_free(str_msg);
    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    /* Error handling */
    error:
    if (NULL != p_msg) cJSON_Delete(p_msg);
    if (NULL != str_msg) cJSON_free(str_msg);
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command get - Discarding");
    return -1;
}

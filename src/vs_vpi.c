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
 * in the following command tables. Commands are implemented at the end of this
 * file.
 */
VS_VPI_CMD_HANDLER(info);
VS_VPI_CMD_HANDLER(finish);
VS_VPI_CMD_HANDLER(stop);
VS_VPI_CMD_HANDLER(exit);
VS_VPI_CMD_HANDLER(run);
VS_VPI_CMD_HANDLER(run_for_time);   //sub-command of "run"
VS_VPI_CMD_HANDLER(run_until_time); //sub-command of "run"
VS_VPI_CMD_HANDLER(run_until_change);  //sub-command of "run"
VS_VPI_CMD_HANDLER(get);
VS_VPI_CMD_HANDLER(get_sim_info);   //sub-command of "get"
VS_VPI_CMD_HANDLER(get_sim_time);   //sub-command of "get"
VS_VPI_CMD_HANDLER(get_value);      //sub-command of "get"

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
    {NULL, NULL}
};

/**
 * @brief Table registering the sub-command handlers for the get command
 * @warning The table has to be terminated by a struct vs_vpi_cmd with NULL
 * fields.
 */
static const vs_vpi_cmd_t vs_vpi_cmd_get_table[] =
{
    VS_VPI_CMD(get_sim_info),
    VS_VPI_CMD(get_sim_time),
    VS_VPI_CMD(get_value),
    {NULL, NULL}
};

/**
 * @brief Table registering the sub-command handlers for the run command
 * @warning The table has to be terminated by a struct vs_vpi_cmd with NULL
 * fields.
 */
static const vs_vpi_cmd_t vs_vpi_cmd_run_table[] =
{
    VS_VPI_CMD(run_for_time),
    VS_VPI_CMD(run_until_time),
    VS_VPI_CMD(run_until_change),
    {NULL, NULL}
};

/**
 * @brief Return a command handler pointer for a given command name (case
 * insensitive) and a given command handlers register table.
 * @param p_cmd_table Pointer to a command handlers register table
 * @param str_cmd Command name (case-insensitive)
 * @return Command handler function pointer. Returns NULL if the command has no
 * registered handler.
 */
static cmd_handler_t vs_vpi_get_cmd_handler(
    const vs_vpi_cmd_t *p_cmd_table, const char *str_cmd)
{
    while(p_cmd_table->cmd_name != NULL) {
        if (0 == strcasecmp(p_cmd_table->cmd_name, str_cmd)) {
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

    /* Dispatch to sub-command handler */
    cmd_handler_t cmd_handler;
    if (strcasecmp("for_time", str_cb) == 0) {
        cmd_handler = 
            vs_vpi_get_cmd_handler(vs_vpi_cmd_run_table, "run_for_time");
    } else if (strcasecmp("until_time", str_cb) == 0) {
        cmd_handler = 
            vs_vpi_get_cmd_handler(vs_vpi_cmd_run_table, "run_until_time");
    } else if (strcasecmp("until_change", str_cb) == 0) {
        cmd_handler = 
            vs_vpi_get_cmd_handler(vs_vpi_cmd_run_table, "run_until_change");
    } else {
        cmd_handler = NULL;
    }
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

VS_VPI_CMD_HANDLER(run_for_time)
{
    /* Get the time field from the JSON message content */
    cJSON *p_item_time = cJSON_GetObjectItem(p_data->p_cmd, "time");
    if (NULL == p_item_time) {
        vs_vpi_log_error("Command field \"time\" invalid/not found");
        goto error;
    }
    double time_value = cJSON_GetNumberValue(p_item_time);
    if (time_value <= 0.0) {
        vs_vpi_log_error("Command field \"time\" <= 0.0");
        goto error;
    }
    cJSON *p_item_unit = cJSON_GetObjectItem(p_data->p_cmd, "time_unit");
    if (NULL == p_item_unit) {
        vs_vpi_log_error("Command field \"time_unit\" invalid/not found");
        goto error;
    }
    char *str_time_unit = cJSON_GetStringValue(p_item_unit);
    if ((NULL == str_time_unit) || (strcmp(str_time_unit, "") == 0)) {
        vs_vpi_log_error("Command field \"time_unit\" NULL or empty");
        goto error;
    }
    vs_vpi_log_info("Command \"run(cb_type=for_time, time=%f %s)\" received.",
        time_value, str_time_unit);

    s_vpi_time cb_time = vs_utils_double_to_time(time_value, str_time_unit);

    /* Register callback */
    s_cb_data cb_data;
    cb_data.reason = cbAfterDelay;
    cb_data.time = &cb_time;
    cb_data.obj = NULL;
    cb_data.value = NULL;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_data->h_systf;
    cb_data.cb_rtn = verisocks_cb;
    vpiHandle h_cb = vpi_register_cb(&cb_data);
    vpi_free_object(h_cb);

    /* Return control to simulator */
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command run - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(run_until_time)
{
    /* Get the time field from the JSON message content */
    cJSON *p_item_time = cJSON_GetObjectItem(p_data->p_cmd, "time");
    if (NULL == p_item_time) {
        vs_vpi_log_error("Command field \"time\" invalid/not found");
        goto error;
    }
    double time_value = cJSON_GetNumberValue(p_item_time);
    cJSON *p_item_unit = cJSON_GetObjectItem(p_data->p_cmd, "time_unit");
    if (NULL == p_item_unit) {
        vs_vpi_log_error("Command field \"time_unit\" invalid/not found");
        goto error;
    }
    char *str_time_unit = cJSON_GetStringValue(p_item_unit);
    if ((NULL == str_time_unit) || (strcmp(str_time_unit, "") == 0)) {
        vs_vpi_log_error("Command field \"time_unit\" NULL or empty");
        goto error;
    }

    vs_vpi_log_info(
        "Command \"run(cb_type=until_time, time=%f %s)\" received.",
        time_value, str_time_unit);

    /* Compare wanted time value with current simulation time */
    s_vpi_time s_time;
	s_time.type = vpiSimTime;
	vpi_get_time(NULL, &s_time);
    double time_sim = vs_utils_time_to_double(s_time, str_time_unit);
    if (time_value <= time_sim) {
        vs_vpi_log_error("Command field \"time\" <= current simulation time");
        goto error;
    }

    s_vpi_time cb_time = vs_utils_double_to_time(time_value, str_time_unit);

    /* Register callback */
    s_cb_data cb_data;
    cb_data.reason = cbAtStartOfSimTime;
    cb_data.time = &cb_time;
    cb_data.obj = NULL;
    cb_data.value = NULL;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_data->h_systf;
    cb_data.cb_rtn = verisocks_cb;
    vpiHandle h_cb = vpi_register_cb(&cb_data);
    vpi_free_object(h_cb);

    /* Return control to simulator */
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command run - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(run_until_change)
{
    /* Get the object path from the JSON message content */
    cJSON *p_item_path = cJSON_GetObjectItem(p_data->p_cmd, "path");
    if (NULL == p_item_path) {
        vs_vpi_log_error("Command field \"path\" invalid/not found");
        goto error;
    }
    char *str_path = cJSON_GetStringValue(p_item_path);
    if ((NULL == str_path) || (strcmp(str_path, "") == 0)) {
        vs_vpi_log_error("Command field \"path\" NULL or empty");
        goto error;
    }

    /* Get the value from the JSON message content */
    cJSON *p_item_val = cJSON_GetObjectItem(p_data->p_cmd, "value");
    if (NULL == p_item_val) {
        vs_vpi_log_error("Command field \"value\" invalid/not found");
        goto error;
    }
    double value = cJSON_GetNumberValue(p_item_val);
    if (isnan(value)) {
        vs_vpi_log_error("Command field \"value\" invalid (NaN)");
        goto error;
    }

    /* Attempt to get the object handle */
    vpiHandle h_obj;
    h_obj = vpi_handle_by_name(str_path, NULL);
    if (NULL == h_obj) {
        vs_vpi_log_error("Attempt to get handle to %s unsuccessful", str_path);
        goto error;
    }

    /* Store value as user data, depending on desired format */
    PLI_INT32 format = vs_utils_get_format(h_obj);
    s_vpi_value target_value;
    target_value.format = format;
    if (0 > format) goto error;
    switch (format) {
    case vpiIntVal:
        target_value.value.integer = (PLI_INT32) value;
        break;
    case vpiRealVal:
        target_value.value.real = value;
        break;
    default:
        goto error;
    }
    p_data->value = target_value;

    /* Log received command */
    vs_vpi_log_info(
        "Command \"run(cb_type=until_change, path=%s, value=%f)\" received.",
            str_path, value);

    /* Register callback */
    s_vpi_value cb_value;
    s_vpi_time cb_time;
    cb_time.type = vpiSimTime;
    cb_value.format = format;

    s_cb_data cb_data;
    cb_data.reason = cbValueChange;
    cb_data.time = &cb_time;
    cb_data.obj = h_obj;
    cb_data.value = &cb_value;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_data->h_systf;
    cb_data.cb_rtn = verisocks_cb_value_change;
    vpiHandle h_cb = vpi_register_cb(&cb_data);
    p_data->h_cb = h_cb;

    /* Return control to simulator */
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;

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

    /* Dispatch to sub-command handler */
    cmd_handler_t cmd_handler;
    if (strcasecmp("sim_info", str_sel) == 0) {
        cmd_handler = 
            vs_vpi_get_cmd_handler(vs_vpi_cmd_get_table, "get_sim_info");
    } else if (strcasecmp("sim_time", str_sel) == 0) {
        cmd_handler = 
            vs_vpi_get_cmd_handler(vs_vpi_cmd_get_table, "get_sim_time");
    } else if (strcasecmp("value", str_sel) == 0) {
        cmd_handler = 
            vs_vpi_get_cmd_handler(vs_vpi_cmd_get_table, "get_value");
    } else {
        cmd_handler = NULL;
    }
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

VS_VPI_CMD_HANDLER(get_sim_info)
{
    cJSON *p_msg;
    char *str_msg;

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_vpi", "Could not create cJSON object");
        goto error;
    }

    vs_vpi_log_debug("Get simulator info...");
    s_vpi_vlog_info vlog_info;
	if (0 > vpi_get_vlog_info(&vlog_info)) {
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
    if (0 > vs_msg_write(p_data->fd_client_socket, str_msg)) {
        vs_log_mod_error("vs_vpi", "Error writing return message");
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
        "Error processing command get(sel=sim_info) - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(get_sim_time)
{
    cJSON *p_msg;
    char *str_msg;

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_vpi", "Could not create cJSON object");
        goto error;
    }

    vs_vpi_log_debug("Getting simulator time...");
	s_vpi_time s_time;
	s_time.type = vpiSimTime;
	vpi_get_time(NULL, &s_time);
    double sim_time_sec = vs_utils_time_to_double(s_time, NULL);
    vs_vpi_log_debug("Sim time: %.6f us", sim_time_sec*1.0e6);
    if (NULL == cJSON_AddNumberToObject(p_msg, "time",
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
    if (0 > vs_msg_write(p_data->fd_client_socket, str_msg)) {
        vs_log_mod_error("vs_vpi", "Error writing return message");
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
        "Error processing command get(sel=sim_time) - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(get_value)
{
    cJSON *p_msg;
    char *str_msg;

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_vpi", "Could not create cJSON object");
        goto error;
    }

    /* Get the object path from the JSON message content */
    cJSON *p_item_path = cJSON_GetObjectItem(p_data->p_cmd, "path");
    if (NULL == p_item_path) {
        vs_vpi_log_error("Command field \"path\" invalid/not found");
        goto error;
    }
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

    /* Get object value */
    s_vpi_value vpi_value;
    if (0 > vs_utils_get_value(h_obj, &vpi_value)) {
        goto error;
    }

    /* Check format */
    cJSON *p_value;
    switch (vpi_value.format) {
    case vpiBinStrVal:
    case vpiOctStrVal:
    case vpiDecStrVal:
    case vpiHexStrVal:
    case vpiStringVal:
        p_value = cJSON_AddStringToObject(
            p_msg, "value", vpi_value.value.str);
        break;
    case vpiScalarVal:
        p_value = cJSON_AddNumberToObject(
            p_msg, "value", vpi_value.value.scalar);
        break;
    case vpiIntVal:
        p_value = cJSON_AddNumberToObject(
            p_msg, "value", (double) vpi_value.value.integer);
        break;
    case vpiRealVal:
        p_value = cJSON_AddNumberToObject(
            p_msg, "value", vpi_value.value.real);
        break;
    default:
        vs_vpi_log_info("Format %d not supported", vpi_value.format);
        goto error;
        break;
    }
    if (NULL == p_value) {
        vs_log_mod_error("vs_vpi", "Could not add value to object");
        goto error;
    }
    str_msg = vs_msg_create_message(p_msg,
        (vs_msg_info_t) {VS_MSG_TXT_JSON, 0});
    if (NULL == str_msg) {
        vs_log_mod_error("vs_vpi", "NULL pointer");
        goto error;
    }
    if (0 > vs_msg_write(p_data->fd_client_socket, str_msg)) {
        vs_log_mod_error("vs_vpi", "Error writing return message");
        goto error;
    }

    /* Normal exit */
    if (NULL != p_msg) cJSON_Delete(p_msg);
    if (NULL != str_msg) cJSON_free(str_msg);
    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    /* Handle errors */
    error:
    if (NULL != p_msg) cJSON_Delete(p_msg);
    if (NULL != str_msg) cJSON_free(str_msg);
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command get(sel=value) - Discarding");
    return -1;
}

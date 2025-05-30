/**************************************************************************//**
@file vs_vpi_run.c
@author jchabloz
@brief Verisocks VPI functions - sub-commands for command run
@date 2022-08-27
******************************************************************************/
/*
MIT License

Copyright (c) 2022-2024 Jérémie Chabloz

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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vpi_config.h"
#include "vs_logging.h"
#include "vs_msg.h"
#include "vs_server.h"
#include "vs_utils.h"
#include "vs_vpi.h"


/* Declare prototypes for command handler functions so that they can be used
 * in the following command tables. Commands are implemented at the end of this
 * file.
 */
VS_VPI_CMD_HANDLER(run_for_time);      //sub-command of "run"
VS_VPI_CMD_HANDLER(run_until_time);    //sub-command of "run"
VS_VPI_CMD_HANDLER(run_until_change);  //sub-command of "run"
VS_VPI_CMD_HANDLER(run_to_next);       //sub-command of "run"

/**
 * @brief Table registering the sub-command handlers for the run command
 * @warning The table has to be terminated by a struct vs_vpi_cmd with NULL
 * fields.
 */
const vs_vpi_cmd_t vs_vpi_cmd_run_table[] =
{
    VS_VPI_CMDKEY(run_for_time, for_time),
    VS_VPI_CMDKEY(run_until_time, until_time),
    VS_VPI_CMDKEY(run_until_change, until_change),
    VS_VPI_CMDKEY(run_to_next, to_next),
    {NULL, NULL, NULL}
};

VS_VPI_CMD_HANDLER(run_for_time)
{
    s_vpi_time cb_time;
    s_cb_data cb_data;
    vpiHandle h_cb;
    cJSON *p_item_time;
    cJSON *p_item_unit;
    char *str_time_unit;
    double time_value;

    /* Get the time field from the JSON message content */
    p_item_time = cJSON_GetObjectItem(p_data->p_cmd, "time");
    if (NULL == p_item_time) {
        vs_vpi_log_error("Command field \"time\" invalid/not found");
        goto error;
    }
    time_value = cJSON_GetNumberValue(p_item_time);
    if (time_value <= 0.0) {
        vs_vpi_log_error("Command field \"time\" <= 0.0");
        goto error;
    }
    p_item_unit = cJSON_GetObjectItem(p_data->p_cmd, "time_unit");
    if (NULL == p_item_unit) {
        vs_vpi_log_error("Command field \"time_unit\" invalid/not found");
        goto error;
    }
    str_time_unit = cJSON_GetStringValue(p_item_unit);
    if ((NULL == str_time_unit) || (strcmp(str_time_unit, "") == 0)) {
        vs_vpi_log_error("Command field \"time_unit\" NULL or empty");
        goto error;
    }
    vs_vpi_log_info("Command \"run(cb=for_time, time=%f %s)\" received.",
        time_value, str_time_unit);

    cb_time = vs_utils_double_to_time(time_value, str_time_unit);

    /* Register callback */
    cb_data.reason = cbAfterDelay;
    cb_data.time = &cb_time;
    cb_data.obj = NULL;
    cb_data.value = NULL;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_data;
    cb_data.cb_rtn = verisocks_cb;
    h_cb = vpi_register_cb(&cb_data);
    if (NULL == h_cb) {
        vs_vpi_log_error("Could not register callback");
        goto error;
    }
    vpi_free_object(h_cb);
    p_data->h_cb = NULL;

    /* Return control to simulator */
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_log_warning(
        "Error processing command run(for_time) - Discarding");
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command run - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(run_until_time)
{
    cJSON *p_item_time;
    s_vpi_time cb_time;
    double time_value;
    cJSON *p_item_unit;
    char *str_time_unit;
    s_vpi_time s_time;
    double time_sim;
    s_cb_data cb_data;
    vpiHandle h_cb;

    /* Get the time field from the JSON message content */
    p_item_time = cJSON_GetObjectItem(p_data->p_cmd, "time");
    if (NULL == p_item_time) {
        vs_vpi_log_error("Command field \"time\" invalid/not found");
        goto error;
    }
    time_value = cJSON_GetNumberValue(p_item_time);
    p_item_unit = cJSON_GetObjectItem(p_data->p_cmd, "time_unit");
    if (NULL == p_item_unit) {
        vs_vpi_log_error("Command field \"time_unit\" invalid/not found");
        goto error;
    }
    str_time_unit = cJSON_GetStringValue(p_item_unit);
    if ((NULL == str_time_unit) || (strcmp(str_time_unit, "") == 0)) {
        vs_vpi_log_error("Command field \"time_unit\" NULL or empty");
        goto error;
    }

    vs_vpi_log_info(
        "Command \"run(cb=until_time, time=%f %s)\" received.",
        time_value, str_time_unit);

    /* Compare wanted time value with current simulation time */
    s_time.type = vpiSimTime;
    vpi_get_time(NULL, &s_time);
    time_sim = vs_utils_time_to_double(s_time, str_time_unit);
    if (time_value <= time_sim) {
        vs_vpi_log_error("Command field \"time\" <= current simulation time");
        goto error;
    }

    cb_time = vs_utils_double_to_time(time_value, str_time_unit);

    /* Register callback */
    cb_data.reason = cbAtStartOfSimTime;
    cb_data.time = &cb_time;
    cb_data.obj = NULL;
    cb_data.value = NULL;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_data;
    cb_data.cb_rtn = verisocks_cb;
    h_cb = vpi_register_cb(&cb_data);
    if (NULL == h_cb) {
        vs_vpi_log_error("Could not register callback");
        goto error;
    }
    vpi_free_object(h_cb);
    p_data->h_cb = NULL;

    /* Return control to simulator */
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_log_warning(
        "Error processing command run(until_time) - Discarding");
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command run - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(run_until_change)
{
    cJSON *p_item_path;
    char *str_path;
    vpiHandle h_obj;
    double value = NAN;
    cJSON *p_item_val;
    PLI_INT32 format;
    s_vpi_value target_value;
    s_vpi_value cb_value;
    s_vpi_time cb_time;
    s_cb_data cb_data;
    vpiHandle h_cb;

    /* Get the object path from the JSON message content */
    p_item_path = cJSON_GetObjectItem(p_data->p_cmd, "path");
    if (NULL == p_item_path) {
        vs_vpi_log_error("Command field \"path\" invalid/not found");
        goto error;
    }
    str_path = cJSON_GetStringValue(p_item_path);
    if ((NULL == str_path) || (strcmp(str_path, "") == 0)) {
        vs_vpi_log_error("Command field \"path\" NULL or empty");
        goto error;
    }

    /* Attempt to get the object handle */
    h_obj = vpi_handle_by_name(str_path, NULL);
    if (NULL == h_obj) {
        vs_vpi_log_error("Attempt to get handle to %s unsuccessful", str_path);
        goto error;
    }

    if (vpi_get(vpiType, h_obj) != vpiNamedEvent) {
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
        vs_vpi_log_info(
            "Command \"run(cb=until_change, path=%s, value=%f)\" received.",
            str_path, value);
    } else {
        vs_vpi_log_info(
            "Command \"run(cb=until_change, path=%s)\" received.",
            str_path);
    }

    /* Store value as user data, depending on desired format */
    format = vs_utils_get_format(h_obj);
    target_value.format = format;
    if (0 > format) goto error;
    switch (format) {
    case vpiIntVal:
        target_value.value.integer = (PLI_INT32) value;
        break;
    case vpiRealVal:
        target_value.value.real = value;
        break;
    case vpiSuppressVal:
        target_value.value.real = 0;
        break;
    default:
        goto error;
    }
    p_data->value = target_value;

    /* Register callback */
    cb_time.type = vpiSimTime;
    cb_value.format = format;

    cb_data.reason = cbValueChange;
    cb_data.time = &cb_time;
    cb_data.obj = h_obj;
    cb_data.value = &cb_value;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_data;
    cb_data.cb_rtn = verisocks_cb_value_change;
    h_cb = vpi_register_cb(&cb_data);
    if (NULL == h_cb) {
        vs_vpi_log_error("Could not register callback");
        goto error;
    }
    p_data->h_cb = h_cb;

    /* Return control to simulator */
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_log_warning(
        "Error processing command run(until_change) - Discarding");
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command run - Discarding");
    return -1;
}

VS_VPI_CMD_HANDLER(run_to_next)
{
    /* Log received command */
    vs_vpi_log_info(
        "Command \"run(cb=to_next)\" received.");

    /* Register callback */
    s_vpi_time cb_time;
    cb_time.type = vpiSimTime;

    s_cb_data cb_data;
    cb_data.reason = cbNextSimTime;
    cb_data.time = &cb_time;
    cb_data.obj = NULL;
    cb_data.value = NULL;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_data;
    cb_data.cb_rtn = verisocks_cb;
    vpiHandle h_cb = vpi_register_cb(&cb_data);
    if (NULL == h_cb) {
        vs_vpi_log_error("Could not register callback");
        goto error;
    }
    vpi_free_object(h_cb);
    p_data->h_cb = NULL;

    /* Return control to simulator */
    p_data->state = VS_VPI_STATE_SIM_RUNNING;
    return 0;

    error:
    p_data->state = VS_VPI_STATE_WAITING;
    vs_vpi_log_warning(
        "Error processing command run(to_next) - Discarding");
    vs_vpi_return(p_data->fd_client_socket, "error",
        "Error processing command run - Discarding");
    return -1;
}

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
#include <vpi_user.h>

#include "vs_logging.h"
#include "vs_msg.h"
#include "vs_server.h"
#include "vs_utils.h"
#include "vs_vpi.h"

/* Declare prototypes for command handler functions so that they can be used
 * in the following command tables. Commands are implemented at the end of this
 * file.
 */
VS_VPI_CMD_HANDLER(get_sim_info);   //sub-command of "get"
VS_VPI_CMD_HANDLER(get_sim_time);   //sub-command of "get"
VS_VPI_CMD_HANDLER(get_value);      //sub-command of "get"
VS_VPI_CMD_HANDLER(get_type);       //sub-command of "get"

/**
 * @brief Table registering the sub-command handlers for the get command
 * @warning The table has to be terminated by a struct vs_vpi_cmd with NULL
 * fields.
 */
const vs_vpi_cmd_t vs_vpi_cmd_get_table[] =
{
    VS_VPI_CMDKEY(get_sim_info, sim_info),
    VS_VPI_CMDKEY(get_sim_time, sim_time),
    VS_VPI_CMDKEY(get_value, value),
    VS_VPI_CMDKEY(get_type, type),
    {NULL, NULL, NULL}
};

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
    if (NULL == cJSON_AddStringToObject(p_msg, "type", "result")) {
        vs_log_mod_error("vs_vpi", "Could not add string to object");
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
    if (NULL == cJSON_AddStringToObject(p_msg, "type", "result")) {
        vs_log_mod_error("vs_vpi", "Could not add string to object");
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
    char *str_msg = NULL;

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_vpi", "Could not create cJSON object");
        goto error;
    }
    if (NULL == cJSON_AddStringToObject(p_msg, "type", "result")) {
        vs_log_mod_error("vs_vpi", "Could not add string to object");
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

    s_vpi_value vpi_value;
    /* Check if memory array */
    if (vpiMemory == vpi_get(vpiType, h_obj)) {
        vs_log_mod_debug("vs_vpi", "Memory array identified!");
        vpiHandle mem_iter;
        mem_iter = vpi_iterate(vpiMemoryWord, h_obj);
        if (NULL == mem_iter) {
            vs_log_mod_error("vs_vpi", "Could not initialize memory iterator");
            goto error;
        } else {
            PLI_INT32 mem_size = vpi_get(vpiSize, h_obj);
            vs_log_mod_debug("vs_vpi", "Memory array depth: %d", mem_size);
            cJSON *p_array = cJSON_AddArrayToObject(p_msg, "value");
            if (NULL == p_array) {
                vs_log_mod_error("vs_vpi", "Could not create cJSON array");
                goto error;
            }
            vpiHandle h_mem_word;
            while (mem_size > 0) {
                h_mem_word = vpi_scan(mem_iter);
                if (NULL == h_mem_word) {
                    goto error;
                }
                if (0 > vs_utils_get_value(h_mem_word, &vpi_value)) {
                    goto error;
                }
                cJSON_AddItemToArray(p_array,
                    cJSON_CreateNumber(vpi_value.value.integer));
                mem_size--;
            }
            vpi_free_object(mem_iter);
        }
    } else {
        /* Get object value */
        if (0 > vs_utils_get_value(h_obj, &vpi_value)) {
            goto error;
        }

        /* Add value to message */
        if (0 > vs_utils_add_value(vpi_value, p_msg, "value")) {
            goto error;
        }
    }

    /* Create message */
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

VS_VPI_CMD_HANDLER(get_type)
{
    cJSON *p_msg;
    char *str_msg;

    /* Create return message object */
    p_msg = cJSON_CreateObject();
    if (NULL == p_msg) {
        vs_log_mod_error("vs_vpi", "Could not create cJSON object");
        goto error;
    }
    if (NULL == cJSON_AddStringToObject(p_msg, "type", "result")) {
        vs_log_mod_error("vs_vpi", "Could not add string to object");
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

    if (NULL == cJSON_AddNumberToObject(
            p_msg, "type", vpi_get(vpiType, h_obj))) {
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

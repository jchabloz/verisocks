/**************************************************************************//**
@file verisocks.c
@author jchabloz
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
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>

#include "verisocks.h"
#include "vs_logging.h"
#include "vs_utils.h"
#include "vs_server.h"
#include "vs_msg.h"
#include "vs_vpi.h"

#define READ_BUFFER_SIZE 4096

/* Prototypes for some static functions */
static PLI_INT32 verisocks_main(vs_vpi_data_t *p_vpi_data);
static PLI_INT32 verisocks_main_connect(vs_vpi_data_t *p_vpi_data);
static PLI_INT32 verisocks_main_waiting(vs_vpi_data_t *p_vpi_data);
static PLI_INT32 verisocks_cb_exit(p_cb_data cb_data);

void verisocks_register_tf()
{
    s_vpi_systf_data tf_data;

    tf_data.type         = vpiSysTask;
    tf_data.sysfunctype  = 0;
    tf_data.tfname       = "$verisocks_init";
    tf_data.calltf       = verisocks_init_calltf;
    tf_data.compiletf    = verisocks_init_compiletf;
    tf_data.sizetf       = NULL;
    tf_data.user_data    = NULL;

    vpi_register_systf(&tf_data);
    return;
}

PLI_INT32 verisocks_init_compiletf(PLI_BYTE8 *user_data)
{
    if (NULL != user_data) {
        vs_vpi_log_warning("Expected NULL pointer (arg not used)");
    }

    /* Get handle to system task instance */
    vpiHandle h_systf;
    h_systf = vpi_handle(vpiSysTfCall, NULL);

    /* Obtain handles to arguments */
    vpiHandle arg_iterator;
    arg_iterator = vpi_iterate(vpiArgument, h_systf);
    if (NULL == arg_iterator) {
        vs_vpi_log_error("$verisocks_init requires at least 1 argument");
        goto error;
    }

    /* Check argument type */
    vpiHandle h_arg;
    PLI_INT32 tfarg_type;
    h_arg = vpi_scan(arg_iterator);
    tfarg_type = vpi_get(vpiType, h_arg);
    if ((tfarg_type != vpiConstant) &&
        (tfarg_type != vpiIntegerVar) &&
        (tfarg_type != vpiParameter))
    {
        vs_vpi_log_error("$verisocks_init 1st argument must be a constant, \
a parameter or an integer variable");
        vpi_free_object(arg_iterator);
        goto error;
    }

    /* Check that the argument can indeed be parsed as an integer */
    s_vpi_value arg_value;
    arg_value.format = vpiIntVal;
    vpi_get_value(h_arg, &arg_value);
    if (vpiIntVal != arg_value.format) {
        vs_vpi_log_error("$verisocks_init 1st argument must be an integer");
        vpi_free_object(arg_iterator);
        goto error;
    }

    /* Check the second, optional argument */
    h_arg = vpi_scan(arg_iterator);
    if (NULL != h_arg) {
        /* Check argument type */
        tfarg_type = vpi_get(vpiType, h_arg);
        if ((tfarg_type != vpiConstant) &&
            (tfarg_type != vpiIntegerVar) &&
            (tfarg_type != vpiParameter))
        {
            vs_vpi_log_error("$verisocks_init 2nd argument must be a \
constant, a parameter or an integer variable");
            vpi_free_object(arg_iterator);
            goto error;
        }
        /* Check that the argument can indeed be parsed as an integer */
        arg_value.format = vpiIntVal;
        vpi_get_value(h_arg, &arg_value);
        if (vpiIntVal != arg_value.format) {
            vs_vpi_log_error(
                "$verisocks_init 2nd argument must be an integer");
            vpi_free_object(arg_iterator);
            goto error;
        }
        /* Check that there is no 3rd argument to the system task */
        h_arg = vpi_scan(arg_iterator);
        if (NULL != h_arg) {
            vs_vpi_log_error("$verisocks_init supports at most 2 arguments");
            vpi_free_object(arg_iterator);
            goto error;
        }
    }

    /* No errors */
    vs_vpi_log_debug("Exiting compiletf callback without errors");
    return 0;

    /* Errors */
    error:
    vpi_control(vpiFinish, 1);
    return -1;
}

PLI_INT32 verisocks_init_calltf(PLI_BYTE8 *user_data)
{
    vpiHandle h_cb_eos;
    uint32_t s_addr;
    socklen_t len;

    if (NULL != user_data) {
        vs_vpi_log_warning("Expected NULL pointer (not used)");
    }

    int fd_socket = -1;

    /* Get handle to system task instance */
    vpiHandle h_systf;
    h_systf = vpi_handle(vpiSysTfCall, NULL);

    /* Obtain handles to 1st argument */
    vpiHandle arg_iterator;
    vpiHandle h_arg;
    arg_iterator = vpi_iterate(vpiArgument, h_systf);
    h_arg = vpi_scan(arg_iterator);

    /* Get 1st argument value */
    s_vpi_value s_value;
    s_value.format = vpiIntVal;
    vpi_get_value(h_arg, &s_value);
    uint16_t num_port = (uint16_t) s_value.value.integer;

    /* Obtain handle to 2nd (optional) argument */
    h_arg = vpi_scan(arg_iterator);
    PLI_INT32 num_timeout_sec;
    if (NULL != h_arg) {
        vpi_get_value(h_arg, &s_value);
        num_timeout_sec = s_value.value.integer;
    } else {
        num_timeout_sec = 120;
    }

    /* Create and allocate instance-specific storage */
    vs_vpi_data_t *p_vpi_data;
    p_vpi_data = (vs_vpi_data_t*) malloc(sizeof(vs_vpi_data_t));
    if (NULL == p_vpi_data) {
        vs_vpi_log_error("Issue allocating virtual memory");
        goto error;
    }

    /* Initialize instance-specific user data */
    s_vpi_value default_value;
    default_value.format = vpiIntVal;
    default_value.value.integer = 0;
    p_vpi_data->state = VS_VPI_STATE_START;
    p_vpi_data->h_systf = h_systf;
    p_vpi_data->timeout_sec = (int) num_timeout_sec;
    p_vpi_data->fd_server_socket = -1;
    p_vpi_data->fd_client_socket = -1;
    p_vpi_data->p_cmd = NULL;
    p_vpi_data->h_cb = 0;
    p_vpi_data->value = default_value;
    vpi_put_userdata(h_systf, (void*) p_vpi_data);

    /* Create and bind server socket */
    fd_socket = vs_server_make_socket(num_port);
    if (0 > fd_socket) {
        vs_vpi_log_error("Issue making socket at port %d", num_port);
        goto error;
    }

    /* Get and display socket address */
    struct sockaddr_in sin;
    len = sizeof(sin);
    if (0 > getsockname(fd_socket, (struct sockaddr *) &sin, &len)) {
        vs_vpi_log_error("Issue getting socket address info");
        goto error;
    }
    s_addr = ntohl(sin.sin_addr.s_addr);

    vpi_printf("******************************************\n");
    vpi_printf("*  __   __       _             _         *\n");
    vpi_printf("*  \\ \\ / /__ _ _(_)___ ___  __| |__ ___  *\n");
    vpi_printf("*   \\ V / -_) '_| (_-</ _ \\/ _| / /(_-<  *\n");
    vpi_printf("*    \\_/\\___|_| |_/__/\\___/\\__|_\\_\\/__/  *\n");
    vpi_printf("*                                        *\n");
    vpi_printf("******************************************\n");

    vs_vpi_log_info("Server address: %d.%d.%d.%d",
        (s_addr & 0xff000000) >> 24u,
        (s_addr & 0x00ff0000) >> 16u,
        (s_addr & 0x0000ff00) >> 8u,
        (s_addr & 0x000000ff)
    );
    vs_vpi_log_info("Port: %d", ntohs(sin.sin_port));

    /* Update stored data */
    p_vpi_data->state = VS_VPI_STATE_CONNECT;
    p_vpi_data->fd_server_socket = fd_socket;

    /* Register end of simulation callback */
    s_cb_data cb_data;
    s_vpi_time cb_time;
    cb_time.type = vpiSimTime;
    cb_data.reason = cbEndOfSimulation;
    cb_data.time = &cb_time;
    cb_data.obj = NULL;
    cb_data.value = NULL;
    cb_data.index = 0;
    cb_data.user_data = (PLI_BYTE8*) p_vpi_data;
    cb_data.cb_rtn = verisocks_cb_exit;
    h_cb_eos = vpi_register_cb(&cb_data);
    vpi_free_object(h_cb_eos);

    /* Call verisocks main loop */
    if (0 > verisocks_main(p_vpi_data)) {
        goto error;
    }
    vs_vpi_log_info("Returning control to simulator");
    return 0;

    /* Error management */
    error:
    if (0 <= fd_socket) {
        close(fd_socket);
        if (NULL != p_vpi_data) {p_vpi_data->fd_server_socket = -1;}
    }
    vs_vpi_log_info("Aborting simulation");
    vpi_control(vpiFinish, 1);
    //if (NULL != p_vpi_data) {free(p_vpi_data);}  //Will be freed in cb_exit
    return -1;
}

/**
 * @brief Callback function - Used for for_time and until_time
 *
 * @param cb_data Pointer to s_cb_data struct
 * @return Returns 0 if successful, -1 in case of error
 */
PLI_INT32 verisocks_cb(p_cb_data cb_data)
{
    /* Retrieve stored user data */
    vs_vpi_data_t *p_vpi_data = NULL;
    p_vpi_data = (vs_vpi_data_t*) cb_data->user_data;
    if (NULL == p_vpi_data) {
        vs_vpi_log_error("Could not get stored data - Aborting callback");
        goto error;
    }

    /* Check state */
    if (p_vpi_data->state != VS_VPI_STATE_SIM_RUNNING) {
        vs_vpi_log_error("Inconsistent state");
        goto error;
    }

    /* Signalling that the callback function has been reached */
    vs_vpi_log_info("Reached callback - Verisocks taking over and waiting \
for command ...");
    vs_vpi_return(p_vpi_data->fd_client_socket, "ack",
        "Reached callback - Getting back to Verisocks main loop");

    /* Call verisocks main loop */
    p_vpi_data->state = VS_VPI_STATE_WAITING;
    if (0 > verisocks_main(p_vpi_data)) {
        goto error;
    }
    vs_vpi_log_info("Returning control to simulator");
    return 0;

    /* Error management */
    error:
    if (NULL != p_vpi_data) {
        p_vpi_data->state = VS_VPI_STATE_ERROR;
        if (0 <= p_vpi_data->fd_server_socket) {
            close(p_vpi_data->fd_server_socket);
            p_vpi_data->fd_server_socket = -1;
        }
        //free(p_vpi_data);  //Will be freed in exit callback handler
    }
    vs_vpi_log_info("Aborting simulation");
    vpi_control(vpiFinish, 1);
    return -1;
}

/**
 * @brief Callback function - Used for for_until_change
 *
 * @param cb_data Pointer to s_cb_data struct
 * @return Returns 0 if successful, -1 in case of error
 */
PLI_INT32 verisocks_cb_value_change(p_cb_data cb_data)
{
    /* Retrieve stored user data */
    vs_vpi_data_t *p_vpi_data = NULL;
    p_vpi_data = (vs_vpi_data_t*) cb_data->user_data;
    if (NULL == p_vpi_data) {
        vs_vpi_log_error("Could not get stored data - Aborting callback");
        goto error;
    }

    /* Check state */
    if (p_vpi_data->state != VS_VPI_STATE_SIM_RUNNING) {
        vs_vpi_log_error("Inconsistent state - Aborting callback");
        goto error;
    }

    /* If the value is not the same, get back to sim until next time */
    if (vpi_get(vpiType, cb_data->obj) != vpiNamedEvent) {
        if (vs_utils_compare_values(
            p_vpi_data->value, *(cb_data->value)) != 0) {
            return 0;
        }
    }

    /* Remove callback */
    vpi_remove_cb(p_vpi_data->h_cb);

    /* Signalling that the callback function has been reached */
    vs_vpi_log_info("Reached callback - Verisocks taking over and waiting \
for command ...");
    vs_vpi_return(p_vpi_data->fd_client_socket, "ack",
        "Reached callback - Getting back to Verisocks main loop");

    /* Call verisocks main loop */
    p_vpi_data->state = VS_VPI_STATE_WAITING;
    if (0 > verisocks_main(p_vpi_data)) {
        goto error;
    }
    vs_vpi_log_info("Returning control to simulator");
    return 0;

    /* Error management */
    error:
    if (NULL != p_vpi_data) {
        p_vpi_data->state = VS_VPI_STATE_ERROR;
        if (0 <= p_vpi_data->fd_server_socket) {
            close(p_vpi_data->fd_server_socket);
            p_vpi_data->fd_server_socket = -1;
        }
        //free(p_vpi_data);  //Will be freed in exit callback handler
    }
    vs_vpi_log_info("Aborting simulation");
    vpi_control(vpiFinish, 1);
    return -1;
}

PLI_INT32 verisocks_cb_exit(p_cb_data cb_data)
{
    vs_vpi_log_debug("Reached exit callback (error or end-of-sim)");

    /* Retrieve stored instance data */
    vs_vpi_data_t *p_vpi_data = NULL;
    p_vpi_data = (vs_vpi_data_t*) cb_data->user_data;
    if (NULL == p_vpi_data) {
        vs_vpi_log_error("Could not get stored data - Aborting callback");
        return -1;
    }

    /* Return something on socket in case client is expecting something */
    if ((VS_VPI_STATE_SIM_RUNNING == p_vpi_data->state) ||
        (VS_VPI_STATE_PROCESSING == p_vpi_data->state)) {
        vs_vpi_return(p_vpi_data->fd_client_socket, "error",
            "Exiting Verisocks due to end of simulation");
    }

    /* Clean-up and exit */
    if (0 <= p_vpi_data->fd_server_socket) {
        close(p_vpi_data->fd_server_socket);
        p_vpi_data->fd_server_socket = -1;
    }
    if (NULL != p_vpi_data->p_cmd) {
        cJSON_Delete(p_vpi_data->p_cmd);
        p_vpi_data->p_cmd = NULL;
        //free(p_vpi_data);  //Will be freed in exit callback handler
    }
    return 0;
}

/**
 * @brief State machine main loop
 *
 * @param p_vpi_data
 * @return PLI_INT32
 */
static PLI_INT32 verisocks_main(vs_vpi_data_t *p_vpi_data)
{
    if (NULL == p_vpi_data) {
        vs_vpi_log_error("NULL pointer to user data");
        p_vpi_data->state = VS_VPI_STATE_ERROR;
    }

    while(1) {
        switch (p_vpi_data->state) {
        case VS_VPI_STATE_CONNECT:
            verisocks_main_connect(p_vpi_data);
            break;
        case VS_VPI_STATE_WAITING:
            if (NULL != p_vpi_data->p_cmd) {
                cJSON_Delete(p_vpi_data->p_cmd);
                p_vpi_data->p_cmd = NULL;
            }
            verisocks_main_waiting(p_vpi_data);
            break;
        case VS_VPI_STATE_PROCESSING:
            vs_vpi_log_debug("Processing received message");
            vs_vpi_process_command(p_vpi_data);
            vs_vpi_log_debug("Finished processing command - Back to main loop");

            /* Fly catch - Normally, the state should be updated in the command
            handler function. We catch a possible error here to avoid an
            infinite loop and use the waiting state as a default*/
            if (p_vpi_data->state == VS_VPI_STATE_PROCESSING) {
                p_vpi_data->state = VS_VPI_STATE_WAITING;
            }
            break;
        case VS_VPI_STATE_SIM_RUNNING:
            /* In this case, we exit the main loop function. Depending on the
            latest processed instruction, it may be called again later from a
            callback handler function or not, normally with the state updated
            to VS_VPI_STATE_WAITING.*/
            return 0;
        case VS_VPI_STATE_EXIT:
            if (0 <= p_vpi_data->fd_server_socket) {
                close(p_vpi_data->fd_server_socket);
                p_vpi_data->fd_server_socket = -1;
            }
            if (NULL != p_vpi_data->p_cmd) {
                cJSON_Delete(p_vpi_data->p_cmd);
                p_vpi_data->p_cmd = NULL;
            }
            return 0;
        case VS_VPI_STATE_START:
        case VS_VPI_STATE_ERROR:
        default:
            vs_vpi_log_error("Exiting main loop (error state)");
            if (0 <= p_vpi_data->fd_server_socket) {
                close(p_vpi_data->fd_server_socket);
                p_vpi_data->fd_server_socket = -1;
            }
            if (NULL != p_vpi_data->p_cmd) {
                cJSON_Delete(p_vpi_data->p_cmd);
                p_vpi_data->p_cmd = NULL;
            }
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Static function to handle VS_VPI_STATE_CONNECT state
 *
 * @param p_vpi_data
 * @return PLI_INT32
 */
static PLI_INT32 verisocks_main_connect(vs_vpi_data_t *p_vpi_data)
{
    struct timeval timeout;
    char hostname_buffer[128];
    timeout.tv_sec = p_vpi_data->timeout_sec;
    timeout.tv_usec = 0;
    vs_vpi_log_debug(
        "Waiting for a client to connect (%ds timeout) ...",
        (int) timeout.tv_sec);
    p_vpi_data->fd_client_socket = vs_server_accept(
        p_vpi_data->fd_server_socket, hostname_buffer,
        sizeof(hostname_buffer), &timeout
    );
    if (0 > p_vpi_data->fd_client_socket) {
        vs_vpi_log_error("Failed to connect");
        p_vpi_data->state = VS_VPI_STATE_ERROR;
        return -1;
    }
    vs_vpi_log_info("Connected to %s", hostname_buffer);
    p_vpi_data->state = VS_VPI_STATE_WAITING;
    return 0;
}

/**
 * @brief Static function to handle VS_VPI_STATE_WAITING state
 *
 * @param p_vpi_data
 * @return PLI_INT32
 */
static PLI_INT32 verisocks_main_waiting(vs_vpi_data_t *p_vpi_data)
{
    char read_buffer[READ_BUFFER_SIZE];
    int msg_len;
    vs_msg_info_t msg_info = {VS_MSG_UNDEFINED, 0u, 0u, VS_NULL_UUID};

    msg_len = vs_msg_read(p_vpi_data->fd_client_socket,
                          read_buffer,
                          sizeof(read_buffer),
                          &msg_info);

    if (0 > msg_len) {
        close(p_vpi_data->fd_client_socket);
        vs_vpi_log_debug(
            "Lost connection. Waiting for a client to (re-)connect ..."
        );
        p_vpi_data->state = VS_VPI_STATE_CONNECT;
        return 0;
    }
    if (msg_len >= (int) sizeof(read_buffer)) {
        read_buffer[sizeof(read_buffer) - 1] = '\0';
        vs_vpi_log_warning(
            "Received message longer than RX buffer, discarding it"
        );
        vs_vpi_return(p_vpi_data->fd_client_socket, "error",
            "Message too long - Discarding");
        return -1;
    }
    else {
        read_buffer[msg_len] = '\0';
    }
    vs_vpi_log_debug("Message: %s", &read_buffer[2]);
    if (NULL != p_vpi_data->p_cmd) cJSON_Delete(p_vpi_data->p_cmd);
    p_vpi_data->p_cmd = vs_msg_read_json(read_buffer, &msg_info);
    if (NULL != p_vpi_data->p_cmd) {
        p_vpi_data->state = VS_VPI_STATE_PROCESSING;
        return 0;
    }
    vs_vpi_log_warning("Received message content cannot be interpreted as a \
valid JSON content. Discarding it.");
    vs_vpi_return(p_vpi_data->fd_client_socket, "error",
        "Invalid message content - Discarding");
    return 0;
}

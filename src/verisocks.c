/**
 * @file verisocks.c
 * @author jchabloz (jeremie.chabloz@a3.epfl.ch)
 * @brief 
 * @version 0.1
 * @date 2022-08-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>

#include "verisocks.h"
#include "vs_logging.h"
#include "vs_server.h"
#include "vs_msg.h"
#include "vs_vpi.h"

/* Protoypes for some static functions */
static PLI_INT32 verisocks_main(vs_vpi_data_t *p_vpi_data);
static PLI_INT32 verisocks_cb(p_cb_data cb_data);
static PLI_INT32 verisocks_main_connect(vs_vpi_data_t *p_vpi_data);
static PLI_INT32 verisocks_main_waiting(vs_vpi_data_t *p_vpi_data);

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

void verisocks_register_cb(s_cb_data cb_data)
{
    /* Complete callback data definition with callback function */
    cb_data.cb_rtn = verisocks_cb;

    /* Register callback */
    vpiHandle h_cb;
    h_cb = vpi_register_cb(&cb_data);
    vpi_free_object(h_cb);
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
        vs_vpi_log_error("$verisocks_init requires 1 argument");
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
        vs_vpi_log_error("$verisocks_init arg must be a constant, \
a parameter or an integer variable");
        vpi_free_object(arg_iterator);
        goto error;
    }

    /* Check that the argument can indeed be parsed as an integer */
    s_vpi_value arg_value;
    arg_value.format = vpiIntVal;
    vpi_get_value(h_arg, &arg_value);
    if (vpiIntVal != arg_value.format) {
        vs_vpi_log_error("$verisocks_init arg must be an integer");
        vpi_free_object(arg_iterator);
        goto error;
    }

    /* Check that there is only 1 system task argument */
    h_arg = vpi_scan(arg_iterator);
    if (NULL != h_arg) {
        vs_vpi_log_error("$verisocks_init supports only 1 argument");
        vpi_free_object(arg_iterator);
        goto error;
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
    if (NULL != user_data) {
        vs_vpi_log_warning("Expected NULL pointer (not used)");
    }

    /* Get handle to system task instance */
    vpiHandle h_systf;
    h_systf = vpi_handle(vpiSysTfCall, NULL);

    /* Obtain handles to argument */
    vpiHandle arg_iterator;
    vpiHandle h_arg;
    arg_iterator = vpi_iterate(vpiArgument, h_systf);
    h_arg = vpi_scan(arg_iterator);
    vpi_free_object(arg_iterator);

    /* Get argument value */
    s_vpi_value num_port_value;
    num_port_value.format = vpiIntVal;
    vpi_get_value(h_arg, &num_port_value);
    uint16_t num_port = (uint16_t) num_port_value.value.integer;

    /* Create and allocate instance-specific storage */
    vs_vpi_data_t *p_vpi_data;
    p_vpi_data = (vs_vpi_data_t*) malloc(sizeof(vs_vpi_data_t));
    if (NULL == p_vpi_data) {
        vs_vpi_log_error("Issue allocating virtual memory");
        goto error;
    }

    /* Initialize instance-specific user data */
    p_vpi_data->state = VS_VPI_STATE_START;
    p_vpi_data->h_systf = h_systf;
    p_vpi_data->fd_server_socket = -1;
    p_vpi_data->fd_client_socket = -1;
    p_vpi_data->p_cmd = NULL;
    vpi_put_userdata(h_systf, (void*) p_vpi_data);

    /* Create and bind server socket */
    int fd_socket;
    fd_socket = vs_server_make_socket(num_port);
    if (0 > fd_socket) {
        vs_vpi_log_error("Issue making socket at port %d", num_port);
        goto error;
    }

    /* Get and display socket address */
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (0 > getsockname(fd_socket, (struct sockaddr *) &sin, &len)) {
        vs_vpi_log_error("Issue getting socket address info");
        goto error;
    }
    uint32_t s_addr = ntohl(sin.sin_addr.s_addr);

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
    if (NULL != p_vpi_data) {free(p_vpi_data);}
    return -1;
}

/**
 * @brief Callback function
 * 
 * @param cb_data Pointer to s_cb_data struct
 * @return Returns 0 if successful, -1 in case of error
 */
static PLI_INT32 verisocks_cb(p_cb_data cb_data)
{
    /* Retrieve initial system task instance handle */
    vpiHandle h_systf;
    h_systf = (vpiHandle) cb_data->user_data;
    if (NULL == h_systf) {
        vs_vpi_log_error("Could not get systf handle - Aborting callback");
        goto error;
    }

    /* Retrieve stored instance data */
    vs_vpi_data_t *p_vpi_data;
    p_vpi_data = (vs_vpi_data_t*) vpi_get_userdata(h_systf);
    if (NULL == h_systf) {
        vs_vpi_log_error("Could not get stored data - Aborting callback");
        goto error;
    }

    /* Check state */
    if (p_vpi_data->state != VS_VPI_STATE_SIM_RUNNING) {
        vs_vpi_log_error("Inconsistent state");
        vs_vpi_return(p_vpi_data->fd_client_socket, "error",
            "Reached callback with inconsistent state - Aborting");
        goto error;
    }

    /* Signalling that the callback function has been reached */
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
    p_vpi_data->state = VS_VPI_STATE_ERROR;
    if (NULL != p_vpi_data) close(p_vpi_data->fd_server_socket);
    vs_vpi_log_info("Aborting simulation");
    vpi_control(vpiFinish, 1);
    if (NULL != p_vpi_data) free(p_vpi_data);
    return -1;
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
            if (NULL != p_vpi_data->p_cmd) {cJSON_Delete(p_vpi_data->p_cmd);}
            return 0;
        case VS_VPI_STATE_FINISHED:
            /* Return control to the simulator */
            if (NULL != p_vpi_data->p_cmd) {cJSON_Delete(p_vpi_data->p_cmd);}
            return 0;
        case VS_VPI_STATE_START:
        case VS_VPI_STATE_ERROR:
        default:
            if (NULL != p_vpi_data->p_cmd) {cJSON_Delete(p_vpi_data->p_cmd);}
            vs_vpi_log_error("Exiting main loop (error state)");
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
    timeout.tv_sec = 120;
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
    char read_buffer[4096];
    int msg_len;
    msg_len = vs_msg_read(p_vpi_data->fd_client_socket,
                          read_buffer,
                          sizeof(read_buffer));

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
    p_vpi_data->p_cmd = vs_msg_read_json(read_buffer);
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

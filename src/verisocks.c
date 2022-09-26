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

static PLI_INT32 verisocks_main(vpiHandle h_systf);


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

    p_vpi_data->state = VS_VPI_STATE_START;
    p_vpi_data->fd_server_socket = -1;
    p_vpi_data->fd_client_socket = -1;
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
    if (0 > verisocks_main(h_systf)) {
        goto error;
    }
    vs_vpi_log_info("Returning control to simulator");
    return 0;

    /* Error management */
    error:
    if (0 <= fd_socket) {
        close(fd_socket);
        p_vpi_data->fd_server_socket = -1;
    }
    vs_vpi_log_info("Aborting simulation");
    vpi_control(vpiFinish, 1);
    free(p_vpi_data);
    return -1;
}

static PLI_INT32 verisocks_main(vpiHandle h_systf)
{
    /* Get user data from the provided system task handle */
    vs_vpi_data_t *p_vpi_data;
    p_vpi_data = (vs_vpi_data_t*) vpi_get_userdata(h_systf);
    if (NULL == p_vpi_data) {
        vpi_printf("ERROR [verisocks]: Could not get user data back\n");
        p_vpi_data->state = VS_VPI_STATE_ERROR;
    }

    struct timeval timeout;
    char hostname_buffer[128];
    char read_buffer[4096];
    int msg_len;
    cJSON *p_msg;
    char *str_msg;

    while(1) {
        switch (p_vpi_data->state) {
        /*********************************************************************/
        case VS_VPI_STATE_CONNECT:
        /*********************************************************************/
            timeout.tv_sec = 120;  //FIXME temporary fixed value
            timeout.tv_usec = 0;
            vs_vpi_log_debug(
                "Waiting for a client to connect (%ds timeout) ...",
                (int) timeout.tv_sec);
            p_vpi_data->fd_client_socket = vs_server_accept(
                p_vpi_data->fd_server_socket,hostname_buffer,
                sizeof(hostname_buffer), &timeout
            );
            if (0 > p_vpi_data->fd_client_socket) {
                vs_vpi_log_error("Failed to connect");
                p_vpi_data->state = VS_VPI_STATE_ERROR;
            }
            else {
                vs_vpi_log_info("Connected to %s", hostname_buffer);
                p_vpi_data->state = VS_VPI_STATE_WAITING;
            }
            break;
        /*********************************************************************/
        case VS_VPI_STATE_WAITING:
        /*********************************************************************/
            /* FIXME: Temporary test code */
            msg_len = vs_msg_read(p_vpi_data->fd_client_socket,
                                  read_buffer,
                                  sizeof(read_buffer));
            if (0 > msg_len) {
                close(p_vpi_data->fd_client_socket);
                vs_vpi_log_debug(
                    "Lost connection. Waiting for a client to (re-)connect ..."
                );
                p_vpi_data->state = VS_VPI_STATE_CONNECT;
                break;
            }
            if (msg_len >= (int) sizeof(read_buffer)) {
                read_buffer[sizeof(read_buffer) - 1] = '\0';
                vs_vpi_log_warning(
                    "Received message longer than RX buffer, discarding it"
                );
                break;
            }
            else {
                read_buffer[msg_len] = '\0';
            }
            vs_vpi_log_debug("Message: %s", &read_buffer[2]);
            p_msg = vs_msg_read_json(read_buffer);

            // if (0 < read(p_vpi_data->fd_client_socket,
            //              read_buffer, sizeof(read_buffer))) {
            //     vpi_printf("%s\n", read_buffer);
            // }
            break;
        /*********************************************************************/
        case VS_VPI_STATE_PROCESSING:
        /*********************************************************************/
            //TODO: Process received instruction
            //break;
            /* FIXME: Temporary test code */
            return 0;
        /*********************************************************************/
        case VS_VPI_STATE_SIM_RUNNING:
        /*********************************************************************/
            /* In this case, we exit the main loop function. Depending on the
            latest processed instruction, it may be called again later from a
            callback handler function or not, normally with the state updated
            to VS_VPI_STATE_WAITING.*/
            return 0;
        /*********************************************************************/
        case VS_VPI_STATE_FINISHED:
        /*********************************************************************/
            /* Return control to the simulator */
            return 0;
        /*********************************************************************/
        case VS_VPI_STATE_START:
        case VS_VPI_STATE_ERROR:
        default:
        /*********************************************************************/
            p_vpi_data->state = VS_VPI_STATE_ERROR;
            if (NULL != p_msg) {cJSON_Delete(p_msg);}
            vs_vpi_log_error("State error - Exiting main loop");
            return -1;
        }
    }
    return 0;
}

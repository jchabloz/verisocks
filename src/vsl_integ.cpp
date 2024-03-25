
#include <cstdlib>
#include <cstdio>
#include <type_traits>
//#include <unistd.h>
//#include <netdb.h>
//#include <sys/time.h>

#include "vsl_integ.h"
#include "verilated.h"
#include "vs_server.h"
#include "vs_logging.h"


namespace vsl{

template <class T> VslInteg<T>::VslInteg(T* const _p_model__, const int port, const int timeout) {
    static_assert(std::is_base_of<VerilatedModel, T>::value,
        "Expected a Verilated model for type");
    p_model = _p_model__;
    num_port = port;
    num_timeout_sec = timeout;
}


template <class T> VslInteg<T>::~VslInteg() {
    if (0 <= fd_server_socket) close(fd_server_socket);
    if (NULL != p_cmd) cJSON_Delete(p_cmd);
}


template <class T> void VslInteg<T>::run() {
    printf("******************************************\n");
    printf("*  __   __       _             _         *\n");
    printf("*  \\ \\ / /__ _ _(_)___ ___  __| |__ ___  *\n");
    printf("*   \\ V / -_) '_| (_-</ _ \\/ _| / /(_-<  *\n");
    printf("*    \\_/\\___|_| |_/__/\\___/\\__|_\\_\\/__/  *\n");
    printf("*                                        *\n");
    printf("*   For Verilator                        *\n");
    printf("*   (c) 2022-2024 Jérémie Chabloz        *\n");
    printf("******************************************\n");

    while(true) {
        switch (_state) {
        case VSL_STATE_INIT:
            main_init();
            break;
        case VSL_STATE_CONNECT:
            main_connect();
            break;
        case VSL_STATE_WAITING:
            main_wait();
        case VSL_STATE_PROCESSING:
            main_process();
        case VSL_STATE_SIM_RUNNING:
        case VSL_STATE_EXIT:

        case VSL_STATE_ERROR:
        default:
            vs_log_error("Exiting Verisocks main loop (error state)");
            if (0 <= fd_server_socket) {
                close(fd_server_socket);
                fd_server_socket = -1;
                is_connected = false;
            }
            return;
        } //switch (_state)
    }
    return;
}



template <class T> void VslInteg<T>::main_init() {
   
    /* Check state consistency */
    if (_state != VSL_STATE_INIT) {
        vs_log_error("Wrong state in init function %d", _state);
        _state = VSL_STATE_ERROR;
        return;
    }

    /* Create server socket */
    num_timeout_sec = timeout;
    fd_server_socket = vs_server_make_socket(num_port);
    if (0 > fd_server_socket) {
        vs_log_error("Issue making socket at port %d", num_port);
        _state = VSL_STATE_ERROR;
        return;
    }

    /* Get server socket address */
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (0 > getsockname(fd_server_socket, (struct sockaddr *) &sin, &len)) {
        vs_log_error("Issue getting socket address info");
        _state = VSL_STATE_ERROR;
        return;
    }
    uint32_t s_addr = ntohl(sin.sin_addr.s_addr);

    /* Logs server address and port number */
    vs_log_info("Server address: %d.%d.%d.%d",
        (s_addr & 0xff000000) >> 24u,
        (s_addr & 0x00ff0000) >> 16u,
        (s_addr & 0x0000ff00) >> 8u,
        (s_addr & 0x000000ff)
    );
    vs_log_info("Port: %d", ntohs(sin.sin_port));

    /* Update state */
    _state = VSL_STATE_CONNECT;
    return;
}




} //namespace vsl

int main() { } //Temporary for compilation
//EOF

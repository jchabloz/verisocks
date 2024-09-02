
#include "vsl_integ_cmd.h"
#include "vsl_integ.h"
#include "vs_logging.h"
#include "vs_msg.h"
// #include "verilated.h"

#include <cstring>

namespace vsl {

VSL_CMD_HANDLER(info) {
    char *str_val;

    vs_log_mod_info("vsl", "Command \"info\" received");

    /* Get the value from the JSON message content */
    cJSON *p_item_val = cJSON_GetObjectItem(vx.p_cmd, "value");
    if (nullptr == p_item_val) {
        vs_log_mod_error("vsl", "Command field \"value\" invalid/not found");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command info - Discarding");
        vx._state = VSL_STATE_WAITING;
        return;
    }

    /* Get the info command argument */
    str_val = cJSON_GetStringValue(p_item_val);
    if ((nullptr == str_val) || (std::strcmp(str_val, "") == 0)) {
        vs_log_mod_error("vsl", "Command field \"value\" NULL or empty");
        vs_msg_return(vx.fd_client_socket, "error",
            "Error processing command info - Discarding");
        vx._state = VSL_STATE_WAITING;
        return;
    }

    /* Print received info value */
    vs_log_info("%s", str_val);

    /* Return an acknowledgement */
    vs_msg_return(vx.fd_client_socket, "ack", "command info received");

    /* Set state to "waiting next command" */
    vx._state = VSL_STATE_WAITING;
    return;
}

}

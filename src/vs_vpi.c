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

#include "vs_vpi.h"


/** Error handling function (generic) */
static void vs_vpi_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vpi_printf(fmt, args);
    va_end(args);
}


/** Type for a command handler function pointer */
typedef int (*cmd_handler_t)(vs_vpi_data_t*, const cJSON*);

/** Struct type for commands, associating a command name with a command handler
 * function pointer */
typedef struct vs_vpi_cmd {
    cmd_handler_t cmd_handler;  // Pointer to handler function
    const char *cmd_name;       // Command name
} vs_vpi_cmd_t;


/**
 * @brief Helper macro to declare a command handler function
 * @param cmd Command short name
 */
#define VS_VPI_CMD_HANDLER(cmd) static int VS_VPI_ ## cmd ## _cmd_handler(\
vs_vpi_data_t *p_data, const cJSON *p_msg)

/**
 * @brief Helper marco to define a command structure with a command name and
 * associated command handler function pointer.
 * @param cmd Command short name
 */
#define VS_VPI_CMD(cmd) {VS_VPI_ ## cmd ## _cmd_handler, #cmd}

/* Declare prototypes for command handler functions so that they can be used
 * in the following command table. Commands are implemented at the end of this
 * file.*/
VS_VPI_CMD_HANDLER(finish);
VS_VPI_CMD_HANDLER(set_value);
VS_VPI_CMD_HANDLER(get_value);
VS_VPI_CMD_HANDLER(run);

/**
 * @brief Table registering the command handlers
 * @warning The table has to be terminated by a struct vs_vpi_cmd with a NULL
 * cmd_name field.
 */
static const vs_vpi_cmd_t vs_vpi_cmd_table[] = {
    VS_VPI_CMD(finish),
    VS_VPI_CMD(set_value),
    VS_VPI_CMD(get_value),
    VS_VPI_CMD(run),
    (vs_vpi_cmd_t) {NULL, NULL}
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
        if (strcasecmp(ptr_cmd->cmd_name, cmd)) {
            return ptr_cmd->cmd_handler;
        }
        ptr_cmd++;
    }
    return NULL;
}

/**
 * @brief Process a command receives as a JSON message content
 * @param p_cmd Pointer to a cJSON struct with the message content. It is
 * expected that the message JSON content contains at least a "command" field.
 * @return int 
 */
int vs_vpi_process_command(vs_vpi_data_t *p_data, const cJSON *p_cmd)
{
    /* Sanity check on parameters */
    if (NULL == p_data) {
        vs_vpi_error("ERROR: [Verisocks] NULL pointer to data");
        return -1;
    }
    if (NULL == p_cmd) {
        vs_vpi_error("ERROR: [Verisocks] NULL pointer to cJSON cmd");
        goto error;
    }

    /* Get the command field from the JSON message content */
    cJSON *p_item_cmd =
        cJSON_GetObjectItem(p_cmd, "command");
    if (NULL == p_item_cmd) {
        vs_vpi_error("ERROR: [Verisocks] Command field invalid/not found\n");
        goto error;
    }
    /* Get the command as a string */
    char *str_cmd = cJSON_GetStringValue(p_item_cmd);
    if ((NULL == str_cmd) || (strcmp(str_cmd, "") == 0)) {
        vs_vpi_error("ERROR: [Verisocks] Command field NULL or empty\n");
        goto error;
    }
    /* Look up command handler */
    cmd_handler_t cmd_handler = vs_vpi_get_cmd_handler(str_cmd);
    if (NULL == cmd_handler) {
        vs_vpi_error("ERROR: [Verisocks] Command handler not found\n");
        cJSON_free(str_cmd); //Not sure if needed?
        goto error;
    }

    /* Clean-up */
    cJSON_free(str_cmd); //Not sure if needed?

    /* Execute command handler and forward returned value */
    p_data->state = VS_VPI_STATE_PROCESSING;
    return (*cmd_handler)(p_data, p_cmd);

    /* Error handling */
    error:
    p_data->state = VS_VPI_STATE_ERROR;
    return -1;
}

/******************************************************************************
 * Command handler functions - Implementation
 * Note: using the helper macro VS_VPI_CMD_HANDLER() makes the following
 * parameters always available:
 * - vs_vpi_data_t *p_data
 * - cJSON *p_msg
 *****************************************************************************/
VS_VPI_CMD_HANDLER(finish)
{
    vpi_printf("INFO: [Verisocks] Command \"finish\" received. Terminating simulation...\n");
    vpi_control(vpiFinish);
    //Other clean-up??
    // vpi_flush(); //??
    p_data->state = VS_VPI_STATE_STOPPED;
    return 0;
}


VS_VPI_CMD_HANDLER(set_value)
{
    /* Check state consistency */
    if (VS_VPI_STATE_PROCESSING != p_data->state) {
        vs_vpi_error("ERROR: [Verisocks][set_value] Inconsistent state\n");
        goto error;
    }

    /* Get value path */
    cJSON *p_item_path = cJSON_GetObjectItem(p_msg, "path");
    char *str_path = cJSON_GetStringValue(p_item_path);
    if (NULL == str_path || (strcmp(str_path, "") == 0)) {
        vs_vpi_error("ERROR: [Verisocks][set_value] Path field empty/not found\n");
        goto error;
    }

    /* Get value type */
    cJSON *p_item_type = cJSON_GetObjectItem(p_msg, "type");
    char *str_type = cJSON_GetStringValue(p_item_type);
    if (NULL == str_type || (strcmp(str_path, "") == 0)) {
        vs_vpi_error("ERROR: [Verisocks][set_value] Type field empty/not found\n");
        goto error;
    }

    /* Get value, depends on type */
    //cJSON *p_item_value = cJSON_GetObjectItem(p_msg, "value");
    do {
        if (strcmp(str_type, "logic") == 0) {
            /* TODO */
            break;
        }
        if (strcmp(str_type, "logic_array") == 0) {
            /* TODO */
            break;
        }
        if (strcmp(str_type, "real") == 0) {
            /* TODO */
            break;
        }
        vs_vpi_error("ERROR: [Verisocks][set_value] Unknown type\n");
        goto error;
    } while(0);

    p_data->state = VS_VPI_STATE_WAITING;
    return 0;

    error:
    p_data->state = VS_VPI_STATE_ERROR;
    return -1;
}

VS_VPI_CMD_HANDLER(get_value)
{
    return 0;
}

VS_VPI_CMD_HANDLER(run)
{
    return 0;
}

/**
 * @file verisocks.h
 * @author jchabloz
 * @brief Verisocks VPI
 * @version 0.1
 * @date 2022-09-13
 * 
 */

#ifndef VERISOCKS_H
#define VERISOCKS_H

#include <vpi_user.h>

/** Callback at (pseudo) compile time for user task verisock_init */
PLI_INT32 verisocks_init_compiletf(PLI_BYTE8* user_data);

/** Callback for user task verisock_init */
PLI_INT32 verisocks_init_calltf(PLI_BYTE8* user_data);

/** Non-specific runtime callback */
PLI_INT32 verisocks_cb(p_cb_data cb_data);

/** Specific runtime callback (value_change) */
PLI_INT32 verisocks_cb_value_change(p_cb_data cb_data);

void verisocks_register_tf();

#endif //VS_VPI_H
//EOF

/**
 * @file verisocks.h
 * @author jchabloz (jeremie.chabloz@a3.epfl.ch)
 * @brief Verisocks VPI
 * @version 0.1
 * @date 2022-09-13
 * 
 */

#ifndef VERISOCKS_H
#define VERISOCKS_H

#include <iverilog/vpi_user.h>

PLI_INT32 verisocks_init_compiletf(PLI_BYTE8* user_data);
PLI_INT32 verisocks_init_calltf(PLI_BYTE8* user_data);
void verisocks_register_tf();


#endif //VS_VPI_H
//EOF

/**************************************************************************//**
@file verisocks.h
@author jchabloz
@brief Verisocks VPI
@date 2022-09-13
******************************************************************************/
/*
MIT License

Copyright (c) 2022-2025 Jérémie Chabloz

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

#ifndef VERISOCKS_H
#define VERISOCKS_H

#include "vpi_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Callback at (pseudo) compile time for user task verisock_init */
PLI_INT32 verisocks_init_compiletf(PLI_BYTE8* user_data);

/** Callback for user task verisock_init */
PLI_INT32 verisocks_init_calltf(PLI_BYTE8* user_data);

/** Non-specific runtime callback */
PLI_INT32 verisocks_cb(p_cb_data cb_data);

/** Specific runtime callback (value_change) */
PLI_INT32 verisocks_cb_value_change(p_cb_data cb_data);

/** VPI register function */
void verisocks_register_tf();

#ifdef __cplusplus
}
#endif

#endif //VS_VPI_H
//EOF

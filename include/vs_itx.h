/**************************************************************************//**
@file vs_itx.h
@author jchabloz
@brief Verisocks VPI interrupts definition and methods
@date 2026-05-05
******************************************************************************/
/*
MIT License

Copyright (c) 2026 Jérémie Chabloz

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

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

#ifndef VS_ITX_H
#define VS_ITX_H

#include "vpi_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of interrupts
 */
#define VS_VPI_MAX_ITX 8u

/**
 * @brief Interrupt (ITX) type
 */
typedef enum {
    VS_VPI_ITX_INACTIVE,        ///Inactive ITX 
    VS_VPI_ITX_TIME,            ///Time-related ITX
    VS_VPI_ITX_VALUE            ///Value-related ITX
} vs_vpi_itx_type_t;

/**
 * @brief Structure type f
 */
typedef struct vs_vpi_itx {
    const char *name;           ///Interrupt name
    vs_vpi_itx_type_t type;     ///Interrupt type    
    vpiHandle h_cb;             ///Callback handle
    
} vs_vpi_itx_t;

#define VS_VPI_ITX_NULL {NULL, VS_VPI_ITX_INACTIVE, NULL}

#ifdef __cplusplus
}
#endif

#endif //VS_ITX_H
//EOF

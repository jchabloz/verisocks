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

#define VS_VPI_ITX_NAME_MAX_LENGTH 128u
#define VS_VPI_MAX_ITX 8u

/**
 * @brief Interrupt (ITX) enumerated type
 * 
 * This value defines the type of the interrupt; it can be either related to a
 * time-related condition (*in time* or *at time* interrupts) or a
 * value-related condition (*on change* interrupts).
 */
typedef enum {
    VS_VPI_ITX_NONE,            ///<Void itx 
    VS_VPI_ITX_IN_TIME,         ///<"in time" ITX
    VS_VPI_ITX_AT_TIME,         ///<"at time" ITX
    VS_VPI_ITX_ON_CHANGE        ///<"on change" ITX
} vs_vpi_itx_type_t;

#define VS_VPI_ITX_ACTIVE    1u ///<Flag: ITX is active
#define VS_VPI_ITX_NONBLOCK  2u ///<Flag: ITX is non-blocking
#define VS_VPI_ITX_RECURRENT 4u ///<Flag: ITX is recurrent
typedef uint8_t vs_vpi_itx_flags_t;

/** @brief Interrupt (ITX) struct type */
typedef struct vs_vpi_itx {
    char name[VS_VPI_ITX_NAME_MAX_LENGTH];  ///<Interrupt name (unique identifier)
    vs_vpi_itx_type_t type;                 ///<Interrupt type 
    vs_vpi_itx_flags_t flags;               ///<Options flags
    vpiHandle h_cb;                         ///<Associated callback handle
    PLI_BYTE8 *user_data;                   ///<
} vs_vpi_itx_t;

/** @brief Macro corresponding to a null/void interrupt
 * 
 * This macro should typically be used to initialize the interrupt table.
 */
#define VS_VPI_ITX_NULL {"", VS_VPI_ITX_NONE, 0u, NULL, NULL}

/**
 * @brief Checks if an interrupt is active
 * @param p_itx Pointer to interrupt
 * @returns 0: the interrupt is inactive, 1: the interrupt is active
 */
inline uint8_t itx_is_active(const vs_vpi_itx_t *p_itx) {
    return (uint8_t) ((p_itx->flags & VS_VPI_ITX_ACTIVE) > 0u);
}

/**
 * @brief Checks if an interrupt is blocking
 * @param p_itx Pointer to interrupt
 * @returns 0: the interrupt is blocking, 1: the interrupt is non-blocking
 */
inline uint8_t itx_is_blocking(const vs_vpi_itx_t *p_itx) {
    return (uint8_t) ((p_itx->flags & VS_VPI_ITX_NONBLOCK) == 0u);
}

/**
 * @brief Checks if an interrupt is recurrent
 * @param p_itx Pointer to interrupt
 * @returns 0: the interrupt is recurrent, 1: the interrupt is non-recurrent 
 */
inline uint8_t itx_is_recurrent(const vs_vpi_itx_t *p_itx) {
    return (uint8_t) ((p_itx->flags & VS_VPI_ITX_RECURRENT) > 0u);
}

/**
 * @brief Deactivate all interrupts
 * @param itx_table Interrupt table
 */
void vs_vpi_deactivate_all_itx(vs_vpi_itx_t *itx_table);

/**
 * @brief Deactivate a specific interrupt
 * @param name Name of the interrupt to be deactivated
 * @param itx_table Interrupt table
 */
void vs_vpi_deactivate_itx(const char *name, vs_vpi_itx_t *itx_table);

/**
 * @brief Find previously registered (active) interrupt by name
 * @param name Name of the interrupt to be deactivated
 * @param itx_table Interrupt table
 * @returns Pointer to interrupt or NULL if not found 
 */
vs_vpi_itx_t* find_itx_by_name(const char *name, vs_vpi_itx_t *itx_table);

/**
 * @brief Find a free slot in the interrupts table
 * @param itx_table Interrupt table
 * @returns Pointer to interrupt or NULL if no slot left
 */
vs_vpi_itx_t* find_free_itx_slot(vs_vpi_itx_t *itx_table);

/**
 * @brief Callback handler function for interrupts
 */
extern PLI_INT32 verisocks_cb_itx(p_cb_data cb_data);

#ifdef __cplusplus
}
#endif

#endif //VS_ITX_H
//EOF

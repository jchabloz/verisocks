/**************************************************************************//**
@file vs_itx.c
@author jchabloz
@brief Verisocks VPI ITX functions
@date 2026-05-17
******************************************************************************/
/*
MIT License

Copyright (c) 2026 Jérémie Chabloz

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

#include <string.h>
#include <stdlib.h>

#include "vs_itx.h"
#include "vpi_config.h"
#include "vs_logging.h"
#include "vs_utils.h"

#undef __MOD__
#define __MOD__ "vs_itx"

vs_vpi_itx_t vs_vpi_table[VS_VPI_MAX_ITX] = {VS_VPI_ITX_NULL};

void vs_vpi_deactivate_all_itx(vs_vpi_itx_t *itx_table)
{
    vs_log_mod_debug(__MOD__, "Deactivating all interrupts ...");
    size_t idx = 0u;
    while(idx < VS_VPI_MAX_ITX) {
        if (itx_is_active(itx_table + idx)) {
            vpi_remove_cb(itx_table[idx].h_cb);
            vs_vpi_table[idx].h_cb = NULL;
            vs_vpi_table[idx].flags &= (255u ^ VS_VPI_ITX_ACTIVE);
        }
        idx += 1u;
    }
}

void vs_vpi_deactivate_itx(const char *name, vs_vpi_itx_t *itx_table)
{
    vs_log_mod_debug(__MOD__, "Deactivating interrupt %s ...", name);
    vs_vpi_itx_t *p_itx;
    p_itx = find_itx_by_name(name, itx_table);
    if (NULL != p_itx && itx_is_active(p_itx)) {
        vpi_remove_cb(p_itx->h_cb);
        p_itx->h_cb = NULL;
        p_itx->flags &= (255u ^ VS_VPI_ITX_ACTIVE);
    }
}

vs_vpi_itx_t* find_itx_by_name(const char *name, vs_vpi_itx_t *itx_table)
{
    size_t idx = 0u;
    while(idx < VS_VPI_MAX_ITX) {
        if (itx_is_active(itx_table + idx) &&
            0 == strcmp(name, itx_table[idx].name))
        {
            return itx_table + idx;
        }
        idx += 1u;
    }
    vs_log_mod_debug(__MOD__, "Interrupt %s not found", name);
    return NULL;
}

vs_vpi_itx_t* find_free_itx_slot(vs_vpi_itx_t *itx_table)
{
    size_t idx = 0u;
    while(idx < VS_VPI_MAX_ITX) {
        if (!itx_is_active(itx_table + idx)) {
            vs_log_mod_debug(__MOD__, "Interrupt slot %d free", (int) idx);
            return itx_table + idx;
        }
        idx += 1u;
    }
    vs_log_mod_debug(__MOD__, "No free interrupt slot found");
    return NULL;
}

/**************************************************************************//**
@file vs_utils.c
@author jchabloz
@brief Utilities for Verisocks VPI
@date 2022-09-30
******************************************************************************/
/*
MIT License

Copyright (c) 2022-2024 Jérémie Chabloz

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

#include <math.h>
#include <string.h>
#include <stdint.h>

#include "vpi_config.h"
#include "cJSON.h"
#include "vs_logging.h"
#include "vs_utils.h"

#ifndef PLI_UINT64
#define PLI_UINT64 uint64_t
#endif


static const vs_time_def_t TIME_DEF_TABLE[] = {
    {0, "s"},
    {-3, "ms"},
    {-6, "us"},
    {-9, "ns"},
    {-12, "ps"},
    {-15, "fs"},
    {0, NULL}
};

static PLI_INT32 get_time_factor(const char *time_unit)
{
    const vs_time_def_t *ptr_tdef = TIME_DEF_TABLE;
    while(ptr_tdef->name != NULL) {
        if (strcmp(ptr_tdef->name, time_unit) == 0) {
            return ptr_tdef->factor;
        }
        ptr_tdef++;
    }
    vs_log_mod_error("vs_utils", "Wrong time unit identifier %s", time_unit);
    return 0;
}

double vs_utils_time_to_double(s_vpi_time time, const char *time_unit)
{
    double time_factor;
    if (NULL == time_unit || strcmp("", time_unit) == 0) {
        time_factor = 0.0;
    } else {
        time_factor = (double) get_time_factor(time_unit);
    }
    double time_precision = (double) vpi_get(vpiTimePrecision, NULL);

    double time_value = NAN;
    if (vpiSimTime == time.type) {
        PLI_UINT64 time_value_int =
            (PLI_UINT64) time.low + ((PLI_UINT64) time.high << 32u);
        time_value =
            time_value_int * pow(10.0, time_precision - time_factor);
    } else if (vpiScaledRealTime == time.type) {
        time_value =
            time.real * pow(10.0, time_precision - time_factor);
    } else {
        vs_log_mod_error("vs_utils",
            "Unknown or non-supported time type value %d", (int) time.type);
    }
    return time_value;
}

s_vpi_time vs_utils_double_to_time(double time_value, const char *time_unit)
{
    double time_factor;
    if (NULL == time_unit || strcmp("", time_unit) == 0) {
        time_factor = 0.0;
    } else {
        time_factor = (double) get_time_factor(time_unit);
    }
    double time_precision = (double) vpi_get(vpiTimePrecision, NULL);
    time_value *= pow(10.0, time_factor - time_precision);

    PLI_UINT64 time_int = (PLI_UINT64) time_value;

    s_vpi_time vpi_time;
    vpi_time.type = vpiSimTime;
    vpi_time.low = (PLI_INT32) (time_int & 0xffffffff);
    vpi_time.high = (PLI_INT32) (time_int >> 32u);
    vpi_time.real = 0.0;

    return vpi_time;
}

typedef struct s_obj_format {
    PLI_INT32 obj_type;
    PLI_INT32 format;
} obj_format_t;

static obj_format_t obj_format_table[] = {
    {vpiNet,            vpiIntVal},
    {vpiReg,            vpiIntVal},
    {vpiIntegerVar,     vpiIntVal},
    {vpiMemoryWord,     vpiIntVal},
    {vpiRealVar,        vpiRealVal},
    {vpiParameter,      vpiRealVal},
    {vpiConstant,       vpiRealVal},
    {vpiNamedEvent,     vpiSuppressVal},
    {vpiUndefined,      vpiUndefined} //Mandatory last table item
};

PLI_INT32 vs_utils_get_format(vpiHandle h_obj)
{
    PLI_INT32 obj_type = vpi_get(vpiType, h_obj);
    obj_format_t *ptr = obj_format_table;
    while (ptr->format != vpiUndefined) {
        if (obj_type == ptr->obj_type) {
            return ptr->format;
        }
        ptr++;
    }
    vs_log_mod_error("vs_utils",
        "Object type %d currently not supported", obj_type);
    return -1;
}

PLI_INT32 vs_utils_get_value(vpiHandle h_obj, s_vpi_value *p_value)
{
    PLI_INT32 format = vs_utils_get_format(h_obj);
    if (0 > format) {
        return -1;
    }
    p_value->format = format;
    vpi_get_value(h_obj, p_value);
    return 0;
}

PLI_INT32 vs_utils_compare_values(s_vpi_value val1, s_vpi_value val2)
{
    if (val1.format != val2.format) return 1;
    switch (val1.format) {
    case vpiIntVal:
        if (val1.value.integer == val2.value.integer) return 0;
        break;
    case vpiRealVal:
        if (val1.value.real == val2.value.real) return 0;
        break;
    default:
        vs_log_mod_error("vs_utils", "vs_utils_compare_values, format %d is \
currently not supported", val1.format);
        return -1;
    }
    return 1;
}

PLI_INT32 vs_utils_set_value(vpiHandle h_obj, double value)
{
    s_vpi_value vpi_value;
    vpi_value.format = vs_utils_get_format(h_obj);
    if (0 > vpi_value.format) {
        return -1;
    }
    switch (vpi_value.format) {
    case vpiIntVal:
        vpi_value.value.integer = (PLI_INT32) value;
        break;
    case vpiRealVal:
        vpi_value.value.real = value;
        break;
    default:
        vs_log_mod_error("vs_utils", "vs_utils_set_value, format %d is \
currently not supported", vpi_value.format);
        return -1;
    }

    vpi_put_value(h_obj, &vpi_value, NULL, vpiNoDelay);
    return 0;
}

PLI_INT32 vs_utils_add_value(s_vpi_value value, cJSON* p_msg, const char* key)
{
    cJSON *p_value;
    switch (value.format) {
    case vpiBinStrVal:
    case vpiOctStrVal:
    case vpiDecStrVal:
    case vpiHexStrVal:
    case vpiStringVal:
        p_value = cJSON_AddStringToObject(
            p_msg, key, value.value.str);
        break;
    case vpiScalarVal:
        p_value = cJSON_AddNumberToObject(
            p_msg, key, value.value.scalar);
        break;
    case vpiIntVal:
        p_value = cJSON_AddNumberToObject(
            p_msg, key, (double) value.value.integer);
        break;
    case vpiRealVal:
        p_value = cJSON_AddNumberToObject(
            p_msg, key, value.value.real);
        break;
    default:
        vs_vpi_log_info("Format %d currently not supported", value.format);
        goto error;
        break;
    }
    if (NULL == p_value) {
        vs_log_mod_error("vs_vpi", "Could not add value to object");
        goto error;
    }
    return 0;

    error:
    return -1;
}

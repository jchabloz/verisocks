/**************************************************************************//**
@file vsl_utils.cpp
@author jchabloz
@brief Utilities for Verisocks Verilator integration
@date 2022-09-30
******************************************************************************/
/*
MIT License

Copyright (c) 2024 Jérémie Chabloz

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

#include "vsl/vsl_utils.hpp"

#include "vs_logging.h"
#include "verilated.h"
#include "verilated_syms.h"

#include <cmath>
#include <map>
#include <string>

namespace vsl{

static std::map<const char*, int16_t> TIME_DEF_MAP {
    {"s", 0},
    {"ms", -3},
    {"us", -6},
    {"ns", -9},
    {"ps", -12},
    {"fs", -15}
};

static int16_t get_time_factor(const char* time_unit)
{
    if (TIME_DEF_MAP.find(time_unit) == TIME_DEF_MAP.end()) {
        vs_log_mod_error(
            "vsl_utils", "Wrong time unit identifier %s", time_unit);
        return 0;
    }
    return TIME_DEF_MAP[time_unit];
}

double time_to_double(uint64_t time, const char* time_unit,
                      VerilatedContext* p_context)
{
    double time_precision = static_cast<double>(p_context->timeprecision());
    double time_factor = static_cast<double>(get_time_factor(time_unit));
    double time_value = time * std::pow(10.0, time_precision - time_factor);
    return time_value;
}

uint64_t double_to_time(double time_value, const char* time_unit,
                      VerilatedContext* p_context)
{
    double time_precision = static_cast<double>(p_context->timeprecision());
    double time_factor = static_cast<double>(get_time_factor(time_unit));
    time_value *= std::pow(10.0, time_factor - time_precision);
    uint64_t time = static_cast<uint64_t>(time_value);
    return time;
}

int get_scalar_value(VerilatedVar* p_var, VslVar_t& x)
{
    if (p_var->dims() > 1) {
        vs_log_mod_error(
            "vsl_utils", "Cannot extract scalar value from array");
        return -1;
    }
    switch (p_var->vltype()) {
        case VLVT_UINT8:
            x = *(static_cast<uint8_t*>(p_var->datap()));
            return 0;
        case VLVT_UINT16:
            x = *(static_cast<uint16_t*>(p_var->datap()));
            return 0;
        case VLVT_UINT32:
            x = *(static_cast<uint32_t*>(p_var->datap()));
            return 0;
        case VLVT_UINT64:
            x = *(static_cast<uint64_t*>(p_var->datap()));
            return 0;
        case VLVT_REAL:
            x = *(static_cast<double*>(p_var->datap()));
            return 0;
        case VLVT_UNKNOWN:
        case VLVT_WDATA:
        case VLVT_STRING:
        case VLVT_PTR:
        default:
            vs_log_mod_error(
                "vsl_utils", "Not supported type for scalar value");
            return -1;
    }
}





/*
typedef struct s_obj_format {
    PLI_INT32 obj_type;
    PLI_INT32 format;
} obj_format_t;

static const obj_format_t obj_format_table[] = {
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
    const obj_format_t *ptr = obj_format_table;
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
*/

} //namespace vsl
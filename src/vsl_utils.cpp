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
#include "cJSON.h"

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

template <typename T>
inline auto get_value(VerilatedVar* p_var)
{
    T retval = *(static_cast<T*>(p_var->datap()));
    return retval;
}

template <typename T>
inline void set_value(VerilatedVar* p_var, T x)
{
    *(p_var->datap()) = x;
}

template <typename T>
auto get_array_value(VerilatedVar* p_var, T* p_array)
{
    //TODO
    /*
    1. Check dimension, if > 2, consider as not supported for now
    2. 
    
    */
}

int vsl_utils_add_value(VerilatedVar* p_var, cJSON* p_msg, const char* key)
{
    double number_value {0.0f};
    std::string str_value {""};

    /* Detect if non-scalar */
    if (p_var->dims() > 1) {
        vs_log_mod_error(
            "vsl_utils", "Cannot extract scalar value (higher dimension)");
        return -1;
    }

    /* Get value from variable pointer */
    switch (p_var->vltype()) {
        case VLVT_UINT8:
            number_value = static_cast<double>(
                get_value<uint8_t>(p_var));
            break;
        case VLVT_UINT16:
            number_value = static_cast<double>(
                get_value<uint16_t>(p_var));
            break;
        case VLVT_UINT32:
            number_value = static_cast<double>(
                get_value<uint32_t>(p_var));
            break;
        case VLVT_UINT64:
            number_value = static_cast<double>(
                get_value<uint64_t>(p_var));
            break;
        case VLVT_REAL:
            number_value = get_value<double>(p_var);
            break;
        case VLVT_STRING:
            str_value = get_value<std::string>(p_var);
            break;
        case VLVT_UNKNOWN:
        case VLVT_WDATA:
        case VLVT_PTR:
        default:
            vs_log_mod_error(
                "vsl_utils", "Type not supported (yet) for scalar value");
            return -1;
    }

    cJSON* p_value = nullptr;
    switch (p_var->vltype()) {
        case VLVT_UINT8:
        case VLVT_UINT16:
        case VLVT_UINT32:
        case VLVT_UINT64:
        case VLVT_REAL:
            p_value = cJSON_AddNumberToObject(p_msg, key, number_value);
            break;
        case VLVT_STRING:
            p_value = cJSON_AddStringToObject(p_msg, key, str_value.c_str());
            break;
        default:
            return -1;
    }
    if (nullptr == p_value) {
        return -1;
    }
    return 0;
}




/*

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

*/

} //namespace vsl

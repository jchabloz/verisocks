/**************************************************************************//**
@file vsl_utils.cpp
@author jchabloz
@brief Utilities for Verisocks Verilator integration
******************************************************************************/
/*
Copyright (c) 2024-2025 Jérémie Chabloz

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

/**
 * @brief Retrieves the value from a VerilatedVar pointer.
 *
 * This function template takes a pointer to a VerilatedVar and returns the
 * value stored in it, cast to the specified type T.
 *
 * @tparam T The type to which the value should be cast.
 * @param p_var Pointer to the VerilatedVar from which the value is to be
 * retrieved.
 * @return The value stored in the VerilatedVar, cast to type T.
 */
template <typename T>
static inline auto get_value(VerilatedVar* p_var)
{
    T retval = *(static_cast<T*>(p_var->datap()));
    return retval;
}

/**
 * @brief Retrieves a value from an array stored in a VerilatedVar object.
 * 
 * This function template takes a VerilatedVar pointer and an index, and
 * returns the value at the specified index in the array. The type of the value
 * is determined by the template parameter T.
 * 
 * @tparam T The type of the value to be retrieved from the array.
 * @param p_var Pointer to the VerilatedVar object containing the array.
 * @param index The index of the value to retrieve from the array.
 * @return The value at the specified index in the array, cast to type T.
 */
template <typename T>
static inline auto get_array_value(VerilatedVar* p_var, size_t index)
{
    T retval = static_cast<T*>(p_var->datap())[index];
    return retval;
}

/**
 * @brief Sets the value of a VerilatedVar object.
 *
 * This function template sets the value of a VerilatedVar object by casting
 * the provided double value to the specified template type T and assigning it
 * to the data pointer of the VerilatedVar object.
 *
 * @tparam T The type to which the value will be cast and assigned.
 * @param p_var Pointer to the VerilatedVar object whose value is to be set.
 * @param value The double value to be cast and assigned to the VerilatedVar
 * object.
 */
template <typename T>
static inline void set_value(VerilatedVar* p_var, double value)
{
    *(static_cast<T*>(p_var->datap())) = static_cast<T>(value);
}

/**
 * @brief Sets the value of an element in an array.
 *
 * This function template assigns a specified value to an element at a given
 * index in an array represented by a VerilatedVar pointer.
 *
 * @tparam T The type of the array element.
 * @param p_var Pointer to the VerilatedVar array.
 * @param value The value to be assigned to the array element.
 * @param index The index of the array element to be set.
 */
template <typename T>
static inline void set_array_value(
    VerilatedVar* p_var, double value, size_t index)
{
    static_cast<T*>(p_var->datap())[index] = static_cast<T>(value);
}

int add_value_to_msg(VerilatedVar* p_var, cJSON* p_msg, const char* key)
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


int add_array_to_msg(VerilatedVar* p_var, cJSON* p_msg, const char* key)
{

    if (p_var->dims() != 2) {
        vs_log_mod_error(
            "vsl_utils", "Cannot extract value as an array - check dimensions!");
        return -1;
    }
    cJSON *p_array = cJSON_AddArrayToObject(p_msg, key);
    if (nullptr == p_array) {
        vs_log_mod_error("vsl_utils", "Could not create cJSON array");
        return -1;
    }
    size_t mem_size = p_var->elements(1);
    size_t mem_index = 0;
    while (mem_index < mem_size) {
        if (0 > add_value_to_array(p_var, p_array, mem_index)) {
            vs_log_mod_error(
                "vsl_utils", "Error getting value for array variable");
                return -1;
            }
            mem_index++;
    }
    return 0;
}

int add_value_to_array(VerilatedVar* p_var, cJSON* p_array, size_t index)
{
    double number_value {0.0f};


    if (p_var->dims() != 2) {
        vs_log_mod_error(
            "vsl_utils", "Cannot extract value as an array - check dimensions!");
        return -1;
    }
    if (index > (p_var->elements(1) - 1)) {
        vs_log_mod_error(
            "vsl_utils", "Index exceeds array depth");
        return -1;
    }

    /* Get value from variable pointer */
    switch (p_var->vltype()) {
        case VLVT_UINT8:
            number_value = static_cast<double>(
                get_array_value<uint8_t>(p_var, index));
            break;
        case VLVT_UINT16:
            number_value = static_cast<double>(
                get_array_value<uint16_t>(p_var, index));
            break;
        case VLVT_UINT32:
            number_value = static_cast<double>(
                get_array_value<uint32_t>(p_var, index));
            break;
        case VLVT_UINT64:
            number_value = static_cast<double>(
                get_array_value<uint64_t>(p_var, index));
            break;
        case VLVT_REAL:
            number_value = get_array_value<double>(p_var, index);
            break;
        case VLVT_STRING:
        case VLVT_UNKNOWN:
        case VLVT_WDATA:
        case VLVT_PTR:
        default:
            vs_log_mod_error(
                "vsl_utils", "Type not supported for array value");
            return -1;
    }

    cJSON_bool retval = cJSON_AddItemToArray(
        p_array, cJSON_CreateNumber(number_value));
    if (1 != retval) {
        vs_log_mod_error(
            "vsl_utils", "Error adding number to array");
        return -1;
    }

    return 0;
}

int set_variable_value(VerilatedVar* p_var, double value)
{
    /* Detect if non-scalar */
    if (p_var->dims() > 1) {
        vs_log_mod_error(
            "vsl_utils", "Cannot set variable value (non-scalar)");
        return -1;
    }

    switch (p_var->vltype()) {
        case VLVT_UINT8:
            set_value<uint8_t>(p_var, value);
            break;
        case VLVT_UINT16:
            set_value<uint16_t>(p_var, value);
            break;
        case VLVT_UINT32:
            set_value<uint32_t>(p_var, value);
            break;
        case VLVT_UINT64:
            set_value<uint32_t>(p_var, value);
            break;
        case VLVT_REAL:
            *((double*)p_var->datap()) = value;
            break;
        case VLVT_STRING:
        case VLVT_UNKNOWN:
        case VLVT_WDATA:
        case VLVT_PTR:
        default:
            vs_log_mod_error(
                "vsl_utils", "Type not supported (yet) for scalar variable");
            return -1;
    }
    return 0;
}

int set_array_variable_value(VerilatedVar* p_var, cJSON* p_obj)
{
    /* Detect if the variable is not an array */
    if (p_var->dims() != 2) {
        vs_log_mod_error(
            "vsl_utils", "Variable is not an array as expected");
        return -1;
    }

    /* Detect if the JSON object is not an array object */
    if (!cJSON_IsArray(p_obj)) {
        vs_log_mod_error(
            "vsl_utils", "Command field \"value\" should be an array");
        return -1;
    }

    /* Verify that the arrays sizes are corresponding */
    size_t mem_size = p_var->elements(1);
    if (mem_size != (size_t) cJSON_GetArraySize(p_obj)) {
        vs_log_mod_error(
            "vsl_utils",
            "Command field \"value\" should be an array of length %d",
            (int) mem_size);
        return -1;
    }

    cJSON *iterator;
    double value = 0.0f;
    size_t index = 0;
    cJSON_ArrayForEach(iterator, p_obj) {
        value = cJSON_GetNumberValue(iterator);
        switch (p_var->vltype()) {
            case VLVT_UINT8:
                set_array_value<uint8_t>(p_var, value, index);
                break;
            case VLVT_UINT16:
                set_array_value<uint16_t>(p_var, value, index);
                break;
            case VLVT_UINT32:
                set_array_value<uint32_t>(p_var, value, index);
                break;
            case VLVT_UINT64:
                set_array_value<uint64_t>(p_var, value, index);
                break;
            case VLVT_REAL:
                static_cast<double*>(p_var->datap())[index] = value;
                break;
            default:
                vs_log_mod_error(
                    "vsl_utils", "Type not supported for array value");
                return -1;
        }
        index++;
    }
    return 0;
}


} //namespace vsl
//EOF

/*
Copyright (c) 2024-2025 Jérémie Chabloz

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

#ifndef VSL_UTILS_HPP
#define VSL_UTILS_HPP

#include "verilated.h"
#include "cJSON.h"
#include <variant>
#include <string>

namespace vsl{

using VslValue = std::variant<
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    double,
    std::string
>;

/**
 * @brief Converts a given simulation integer time value to a double
 * representation based on the specified time unit.
 * 
 * @param time The integer time value to be converted.
 * @param time_unit The unit of the time value (e.g., "ns" for nanoseconds,
 * "us" for microseconds).
 * @param p_context Pointer to the VerilatedContext, which may be used for
 * simulation context.
 * @return double The converted time value as a double.
 */
double time_to_double(
    uint64_t time, const char* time_unit, VerilatedContext* p_context);

/**
 * @brief Utility function that can be used to convert a real time value in a
 * given time unit to an integer value with the context's simulation time
 * precision.
 *
 * @param time_value Time real value in time unit
 * @param time_unit Time unit (e.g. "us")
 * @param p_context Pointer to Verilator's current context
 * @return (uint64_t) Time integer value (simulation precision)
 */
uint64_t double_to_time(
    double time_value, const char* time_unit, VerilatedContext* p_context);

/**
 * @brief Utility function to get the value of a scalar or string variable and
 * include it in a JSON message.
 *
 * @param p_var Pointer to VerilatedVar instance
 * @param p_msg Pointer to cJSON message
 * @param key Key to use in cJSON message for the value
 * @return int Returns 0 on success, or a non-zero error code on failure.
 */
int add_value_to_msg(VerilatedVar* p_var, cJSON* p_msg, const char* key);

/**
 * @brief Utility function to get the value of an array variable and include it
 * in a JSON message.
 *
 * @param p_var Pointer to VerilatedVar instance
 * @param p_msg Pointer to cJSON message
 * @param key Key to use in cJSON message for the value
 * @return int Returns 0 on success, or a non-zero error code on failure.
 */
int add_array_to_msg(VerilatedVar* p_var, cJSON* p_msg, const char* key);

/**
 * @brief Adds a value to a JSON array at a specified index.
 *
 * This function takes a pointer to a VerilatedVar object and adds its value
 * to a cJSON array at the given index.
 *
 * @param p_var Pointer to the VerilatedVar object containing the value to add.
 * @param p_array Pointer to the cJSON array where the value will be added.
 * @param index The index at which the value should be added in the array.
 * @return int Returns 0 on success, or a non-zero error code on failure.
 */
int add_value_to_array(VerilatedVar* p_var, cJSON* p_array, size_t index);

/**
 * @brief Sets the value of a Verilated variable.
 *
 * This function assigns a given double value to a specified Verilated variable.
 *
 * @param p_var Pointer to the Verilated variable to be set.
 * @param value The double value to assign to the variable.
 * @return int Returns 0 on success, or a non-zero error code on failure.
 */
int set_variable_value(VerilatedVar* p_var, double value);

/**
 * @brief Sets the value of an array variable in a Verilated model.
 *
 * This function assigns a value to an array variable represented by a 
 * VerilatedVar pointer using the data provided in a cJSON object.
 *
 * @param p_var Pointer to the VerilatedVar representing the array variable.
 * @param p_obj Pointer to the cJSON object containing the value to be set.
 * @return int Returns 0 on success, or a non-zero error code on failure.
 */
int set_array_variable_value(VerilatedVar* p_var, cJSON* p_obj);

} //namespace vsl

#endif //VSL_SIGNALS_HPP
//EOF

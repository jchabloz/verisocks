/*
MIT License

Copyright (c) 2024 Jérémie Chabloz

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
 * @brief Utility function that can be used to convert an integer simulation
 * time value in a real value based on a given time unit.
 * 
 * @param time Time integer value (simulation precision)
 * @param time_unit Time unit (e.g. "us")
 * @param p_context Pointer to Verilator's current context
 * @return (double) Time value in time unit
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
 * @brief 
 * 
 * @param p_var Pointer to VerilatedVar instance
 * @param p_msg Pointer to cJSON message
 * @param key Key to use in JSON message for the value
 * @return (int) Status - 0 if successful
 */
int vsl_utils_get_value(VerilatedVar* p_var, cJSON* p_msg, const char* key);

} //namespace vsl

#endif //VSL_SIGNALS_HPP
//EOF

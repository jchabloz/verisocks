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
#include <string>

namespace vsl{

/**
 * @brief Checks the validity of a time unit identifier key.check_time_unit
 * 
 * @param time_unit Time unit key (e.g. "us")
 * @return bool If the key is valid, returns true, false otherwise
 */
bool check_time_unit(std::string time_unit);

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
 * @brief Array range
 */
struct VslArrayRange {
    std::string array_name;
    size_t left;
    size_t right;
};

/**
 * @brief Check if a variable path contains the [] operator
 * 
 * @param path Variable path
 * @return (bool) Returns true if the path contains a range definition
 */
bool has_range(const std::string& path);



VslArrayRange get_range(const std::string& path);






} //namespace vsl

#endif //VSL_SIGNALS_HPP
//EOF

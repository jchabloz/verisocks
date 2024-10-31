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

#include <variant>

namespace vsl{

using VslVar_t = std::variant<uint8_t,uint16_t,uint32_t,uint64_t,double>;

// using VslVar_t = union {
//     void* vsl_ptr;
//     uint8_t vsl_uint8;
//     uint16_t vsl_uint16;
//     uint32_t vsl_uint32;
//     uint64_t vsl_uint64;
//     double vsl_real;
// };

double time_to_double(
    uint64_t time, const char* time_unit, VerilatedContext* p_context);

uint64_t double_to_time(
    double time_value, const char* time_unit, VerilatedContext* p_context);

} //namespace vsl

#endif //VSL_SIGNALS_HPP
//EOF

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

static std::map<std::string, int16_t> TIME_DEF_MAP {
    {"s", 0},
    {"ms", -3},
    {"us", -6},
    {"ns", -9},
    {"ps", -12},
    {"fs", -15}
};

bool check_time_unit(std::string time_unit) {
    return (TIME_DEF_MAP.find(time_unit) != TIME_DEF_MAP.end());
}

static int16_t get_time_factor(const char* time_unit)
{
    std::string str_key {time_unit};
    if (check_time_unit(str_key)) {return TIME_DEF_MAP[str_key];}
    return 0;
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
    if (time_value <= 0.0f) {
        vs_log_mod_warning("vsl_utils", "Time value nul or negative");
        return 0u;
    }
    vs_log_mod_debug("vsl_utils", "Time value: %f", time_value);
    vs_log_mod_debug("vsl_utils", "Time unit: %s", time_unit);
    double time_precision = static_cast<double>(p_context->timeprecision());
    double time_factor = static_cast<double>(get_time_factor(time_unit));
    time_value *= std::pow(10.0, time_factor - time_precision);
    return static_cast<uint64_t>(time_value);
}

} //namespace vsl
//EOF

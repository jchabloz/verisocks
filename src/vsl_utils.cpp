/**************************************************************************//**
@file vsl_utils.cpp
@author jchabloz
@brief Utilities for Verisocks Verilator integration
******************************************************************************/
/*
Copyright (c) 2024-2026 Jérémie Chabloz

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
#include <regex>

#undef __MOD__
#define __MOD__ "vsl_utils"

namespace vsl{

static std::map<std::string, int16_t> TIME_DEF_MAP {
    {"s", 0},
    {"ms", -3},
    {"us", -6},
    {"ns", -9},
    {"ps", -12},
    {"fs", -15}
};

static const VslTimeDef VSL_TIME_DEF_TABLE[] = {
    {2,   "100s",  100, "s" },
    {1,   "10s",   10,  "s" },
    {0,   "1s",    1,   "s" },
    {-1,  "100ms", 100, "ms"},
    {-2,  "10ms",  10,  "ms"},
    {-3,  "1ms",   1,   "ms"},
    {-4,  "100us", 100, "us"},
    {-5,  "10us",  10,  "us"},
    {-6,  "1us",   1,   "us"},
    {-7,  "100ns", 100, "ns"},
    {-8,  "10ns",  10,  "ns"},
    {-9,  "1ns",   1,   "ns"},
    {-10, "100ps", 100, "ps"},
    {-11, "10ps",  10,  "ps"},
    {-12, "1ps",   1,   "ps"},
    {-14, "100fs", 100, "fs"},
    {-13, "10fs",  10,  "fs"},
    {-15, "1fs",   1,   "fs"}
};

bool check_time_unit(std::string time_unit)
{
    return (TIME_DEF_MAP.find(time_unit) != TIME_DEF_MAP.end());
}

static int16_t get_time_factor(const char* time_unit)
{
    std::string str_key {time_unit};
    if (check_time_unit(str_key)) {return TIME_DEF_MAP[str_key];}
    return 0;
}

VslTimeDef get_sim_time_def(VerilatedContext* p_context)
{
    int time_precision = p_context->timeprecision();
    if ((time_precision < -15) || (time_precision > 2)) {
        return {0, nullptr, 0, nullptr};
    }
    return VSL_TIME_DEF_TABLE[2 - time_precision];
}

inline uint64_t get_sim_time(VerilatedContext* p_context, VslTimeDef time_def)
{
    uint64_t time = p_context->time();
    return time * time_def.repr_factor;
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
        vs_log_mod_warning(__MOD__, "Time value nul or negative");
        return 0u;
    }
    vs_log_mod_debug(__MOD__, "Time value: %f", time_value);
    vs_log_mod_debug(__MOD__, "Time unit: %s", time_unit);
    double time_precision = static_cast<double>(p_context->timeprecision());
    double time_factor = static_cast<double>(get_time_factor(time_unit));
    time_value *= std::pow(10.0, time_factor - time_precision);
    return static_cast<uint64_t>(time_value);
}

bool has_range(const std::string& path) {
    std::regex range_regex {"\\[([0-9]+)(:([0-9]+))?\\]$"};
    std::smatch m;
    std::regex_search(path, m, range_regex);
    if (m.empty()) return false;
    return true;
}

VslArrayRange get_range(const std::string& path) {
    VslArrayRange range {0, 0, 1, "None"};
    const std::regex range_regex {"^(.+)\\[([0-9]+)(:([0-9]+))?\\]$"};
    std::smatch m;
    std::regex_search(path, m, range_regex);
    if (m.empty()) return range;
    range.array_name = m[1];
    range.left = static_cast<size_t>(std::stoi(m[2]));    
    if (0 < m[4].length()) {
        range.right = static_cast<size_t>(std::stoi(m[4]));    
    } else {
        range.right = static_cast<size_t>(std::stoi(m[2]));    
    }
    if (range.right > range.left) range.incr = -1;
    return range;
}

} //namespace vsl
//EOF

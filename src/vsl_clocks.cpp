/**************************************************************************//**
 @file vsl_clocks.cpp
 @brief Clocks class and methods for Verisocks Verilator integration

 @author Jérémie Chabloz
 @copyright Copyright (c) 2025 Jérémie Chabloz Distributed under the MIT
 License. See file for details.
******************************************************************************/
/*
Copyright (c) 2025 Jérémie Chabloz

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

#include "vsl/vsl_clocks.hpp"
#include "vsl/vsl_utils.hpp"

#include <cmath>

namespace vsl{

    int VslClock::set_period(const uint64_t period, const double duty_cycle)
    {
        if (0 == period) {return -1;}
        if (0.0 >= duty_cycle || 1.0 <= duty_cycle) {return -1;}
        this->period = period;
        this->duty_cycle = duty_cycle;
        period_high = static_cast<uint64_t>(
            duty_cycle*static_cast<double>(period));
        period_low = period - period_high;
        return 0;
    }

    int VslClock::set_period(const double period, const char* unit,
        const double duty_cycle, VerilatedContext* const p_context)
    {
        if (check_time_unit(unit) && 0.0 < period) {
            uint64_t period_int = double_to_time(period, unit, p_context);
            set_period(period_int, duty_cycle);
            return 0;
        }
        return -1;
    }

    void VslClock::enable(const uint64_t time) {
        if (!b_is_enabled) {
            if (0 < period_low && 0 < period_high) {
                cycles_counter = 0u; // Reset events counter
                b_is_enabled = true; // Set enabled flag
                next_event_time = time + period_low;
                prev_event_time = time;
            }
        }
    }

    void VslClock::disable() {
        b_is_enabled = false;
    }

    void VslClock::eval(const uint64_t time) {
        if (b_is_enabled && (time == next_event_time)) {
            prev_event_time = next_event_time;
            if (0.0 == get_value()) {
                // Rising edge
                set_value(1.0);
                next_event_time += period_high;
            } else {
                // Falling edge
                set_value(0.0);
                next_event_time += period_low;
                cycles_counter += 1;
            }
        }
    }

    void VslClock::eval(VerilatedContext* const p_context) {
        uint64_t time = p_context->time();
        eval(time);
    }

    void VslClockMap::add_clock(const char* namep, std::any datap) {
        clock_map[namep] = VslClock {namep, datap, 0, 0.5};
    };

    void VslClockMap::add_clock(const char* namep, std::any datap,
        const uint64_t period, const double duty_cycle)
    {
        clock_map[namep] = VslClock {namep, datap, period, duty_cycle};
    };

    void VslClockMap::add_clock(const char* namep, std::any datap,
        const double period, const char* unit, const double duty_cycle,
        VerilatedContext* const p_context)
    {
        clock_map[namep] = VslClock {
            namep, datap, period, unit, duty_cycle, p_context};
    };

} // namespace vsl
// EOF

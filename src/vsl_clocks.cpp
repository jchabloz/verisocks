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
#include "vs_logging.h"
#include <algorithm>

#undef __MOD__
#define __MOD__ "vsl_clocks"

namespace vsl{

    /***************************************************************************
    VslClock class constructors
    ***************************************************************************/
    VslClock::VslClock(const char* namep, std::any datap, const vsl_time_t period,
        const double duty_cycle) :
        VslVar(namep, datap, VLVT_UINT8, VSL_TYPE_CLOCK, 0, 0, 0)
        {
            if (0 > set_period(period, duty_cycle)) {return;}
            if (0 > set_value(0)) {return;}
        };

    VslClock::VslClock(const char* namep, std::any datap, const double period,
        const char* unit, const double duty_cycle,
        VerilatedContext* const p_context) :
        VslVar(namep, datap, VLVT_UINT8, VSL_TYPE_CLOCK, 0, 0, 0)
        {
            if (0 > set_period(period, unit, duty_cycle, p_context)) {return;}
            if (0 > set_value(0)) {return;}
        };

    VslClock::VslClock(const char* namep, std::any datap, const double period,
        const char* unit, const double duty_cycle,
        VerilatedContext* const p_context, const bool enable) :
        VslVar(namep, datap, VLVT_UINT8, VSL_TYPE_CLOCK, 0, 0, 0)
        {
            if (0 > set_period(period, unit, duty_cycle, p_context)) {return;}
            if (0 > set_value(0)) {return;}
            if (enable) this->enable(p_context);
        };

    /***************************************************************************
    VslClock class methods
    ***************************************************************************/
    int VslClock::set_period(const vsl_time_t period, const double duty_cycle)
    {
        if (1 >= period) {return -1;} // Period has to be > 1 time unit
        if (0.0 >= duty_cycle || 1.0 <= duty_cycle) {return -1;}
        this->period = period;
        this->duty_cycle = duty_cycle;
        period_high = static_cast<vsl_time_t>(
            duty_cycle*static_cast<double>(period));
        period_low = period - period_high;
        return 0;
    }

    int VslClock::set_period(const double period, const char* unit,
        const double duty_cycle, VerilatedContext* const p_context)
    {
        if (check_time_unit(unit) && 0.0 < period) {
            vsl_time_t period_int = double_to_time(period, unit, p_context);
            set_period(period_int, duty_cycle);
            return 0;
        }
        return -1;
    }

    int VslClock::enable(const vsl_time_t time) {
        if (!b_is_enabled) {
            if (0 < period_low && 0 < period_high) {
                cycles_counter = 0u; // Reset events counter
                b_is_enabled = true; // Set enabled flag
                b_wait_dis = false;
                prev_event_time = time;
                next_event_time = time + period_high;
                set_value(1.0);
                vs_log_mod_debug(__MOD__, "Enabled clock %s",
                    get_name().c_str());
                return 0;
            }
            vs_log_mod_error(__MOD__,
                "Inconsistent period values for clock %s",
                get_name().c_str());
            return -1;
        } else {
            vs_log_mod_warning(__MOD__, "Clock %s is already enabled",
                get_name().c_str());
            return 0;
        }
    }

    int VslClock::enable(VerilatedContext* const p_context) {
        vsl_time_t time = p_context->time();
        return enable(time);
    }

    void VslClock::disable() {
        if (b_is_enabled) {
            if (0.0 == get_value()) {
                next_event_time = 0ul;
                b_is_enabled = false;
                vs_log_mod_debug(__MOD__, "Disabled clock %s",
                    get_name().c_str());
            } else {
                b_wait_dis = true;
                vs_log_mod_debug(__MOD__,
                    "Set clock %s to be disabled after next falling edge",
                    get_name().c_str()
                );
            }
        } else {
            vs_log_mod_warning(__MOD__, "Clock %s already disabled",
                get_name().c_str());
        }
    }

    int VslClock::eval(const vsl_time_t time) {
        vs_log_mod_debug(__MOD__,
            "Evaluate clock %s, time: %ld, next event: %ld",
            get_name().c_str(), time, next_event_time);
        if (!b_is_enabled || time < next_event_time) {
            vs_log_mod_debug(__MOD__, "Evaluate clock - Disabled or no event");
            return 0;
        }
        if (time == next_event_time) {
            prev_event_time = next_event_time;
            if (0.0 == get_value()) {
                vs_log_mod_debug(__MOD__, "Evaluate clock - Rising edge");
                // Rising edge
                set_value(1.0);
                next_event_time += period_high;
                return 1;
            }
            // Falling edge
            vs_log_mod_debug(__MOD__, "Evaluate clock - Falling edge");
            if (b_wait_dis) {
                next_event_time = 0ul;
                b_is_enabled = false;
                b_wait_dis = false;
            } else {
                next_event_time += period_low;
            }
            set_value(0.0);
            cycles_counter += 1;
            return 2;
        }
        // Error case: clock is enabled but time is > than the next event!
        return -1;
    }

    /***************************************************************************
    VslClockMap class methods
    ***************************************************************************/
    void VslClockMap::add_clock(const char* namep, std::any datap) {
        clock_list.push_front(VslClock {namep, datap, 2ul, 0.5});
        sort();
    }

    void VslClockMap::add_clock(const char* namep, std::any datap,
        const vsl_time_t period, const double duty_cycle)
    {
        clock_list.push_front(
            VslClock {namep, datap, period, duty_cycle});
        sort();
    }

    void VslClockMap::add_clock(const char* namep, std::any datap,
        const double period, const char* unit, const double duty_cycle,
        VerilatedContext* const p_context)
    {
        clock_list.push_front(
            VslClock {namep, datap, period, unit, duty_cycle, p_context});
        sort();
    }

    const bool VslClockMap::has_next_event(void) const {
        // Check that there is at least one clock enabled
        if (clock_list.empty()) {return false;}
        for (VslClock clock : clock_list) {
            if (clock.is_enabled()) {
                vs_log_mod_debug(__MOD__,
                    "At least clock %s is enabled",
                    clock.get_name().c_str());
                return true;
            }
        }
        return false;
    }

    const vsl_time_t VslClockMap::get_next_event(void) const {
        // !! Assumes that the clocks list is not empty and already sorted
        auto it = clock_list.begin();
        return it->get_next_event();
    }

    int VslClockMap::eval(vsl_time_t time) {
        // !! Assumes that the clocks list is already sorted
        unsigned int total_evals = 0u;
        int eval_status = 0;
        if (clock_list.empty()) {return 0;}
        // display_list();
        do {
            auto clk = clock_list.front();
            vs_log_mod_debug(__MOD__, "First clock: %s", clk.get_name().c_str());
            eval_status = clk.eval(time);
            if (eval_status > 0) {total_evals++;}
            sort();
        } while (eval_status > 0);
        return total_evals;
    }

    const bool VslClockMap::has_clock(const std::string name) {
        if (clock_list.empty()) {return false;}
        auto has_name = [&name](VslClock& clock) {
            return clock.get_name().compare(name) == 0;
        };
        auto it = std::find_if(clock_list.begin(), clock_list.end(), has_name);
        return it != clock_list.end();
    }

    VslClock& VslClockMap::get_clock(const std::string name) {
        auto has_name = [&name](VslClock& clock) {
            return clock.get_name().compare(name) == 0;
        };
        auto it = std::find_if(clock_list.begin(), clock_list.end(), has_name);
        return *it;
    }

    void VslClockMap::display_list(void) {
        if (clock_list.empty()) {return;}
        fprintf(stderr, "Name,Enabled?,Next event\n");
        fprintf(stderr, "************************\n");
        for (VslClock clock : clock_list) {
            fprintf(stderr, "%s,%d,%lu\n",
                clock.get_name().c_str(),
                clock.is_enabled(),
                clock.get_next_event()
            );
        }
        return;
    }

} // namespace vsl
// EOF

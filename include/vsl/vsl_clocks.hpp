/***************************************************************************//**
 @file vsl_clocks.hpp
 @brief Clock class definition

 @author Jérémie Chabloz
 @copyright Copyright (c) 2025 Jérémie Chabloz Distributed under the MIT
 License. See file for details.
*******************************************************************************/
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
#ifndef VSL_CLOCKS_HPP
#define VSL_CLOCKS_HPP

#include "vsl/vsl_types.hpp"
#include <string>
#include <list>

namespace vsl {

/**
 * @class VslClock
 * @brief Class that represents a clock variable
 */
class VslClock : VslVar {

public:

    VslClock() = default;

    /**
    * @brief Constructs a new Vsl Clock object
    *
    * The oscillator period is expected to provided as an integer, simulator
    * time. The conversion from a real time value can be done using vsl_utils
    * double_to_time function.
    *
    * @param namep Variable name
    * @param datap Pointer to Verilator variable
    * @param period Period in integer simulator time
    * @param duty_cycle Duty cycle
    */
    VslClock(const char* namep, std::any datap, const uint64_t period,
        const double duty_cycle) :
        VslVar(namep, datap, VLVT_UINT8, VSL_TYPE_CLOCK, 0, 0, 0)
        {
            set_period(period, duty_cycle);
            set_value(0);
            enable(0ul); // Enable by default
        };

    /**
     * @brief Constructs a new Vsl Clock object
     *
     * @param namep Variable name
     * @param datap Pointer to Verilator variable
     * @param period Period in real value
     * @param unit Period time unit
     * @param duty_cycle Duty cycle
     * @param p_context Simulation context
     */
    VslClock(const char* namep, std::any datap, const double period,
        const char* unit, const double duty_cycle,
        VerilatedContext* const p_context) :
        VslVar(namep, datap, VLVT_UINT8, VSL_TYPE_CLOCK, 0, 0, 0)
        {
            set_period(period, unit, duty_cycle, p_context);
            set_value(0);
            enable(0ul); // Enable by default
        };

    /**
    * @brief Set period and duty cycle
    *
    * @return 0 No errors
    * @return -1 Errors
    */
    int set_period(const uint64_t period, const double duty_cycle);

    /**
     * @brief Set the clock period and duty cycle
     *
     * @param period Period real value
     * @param unit Period time unit
     * @param duty_cycle Duty cycle
     * @param p_context Context (useful to get simulation time resolution)
     *
     * @return 0 No errors
     * @return -1 Errors
     */
    int set_period(const double period, const char* unit,
        const double duty_cycle, VerilatedContext* const p_context);

    /**
     * @brief Enable clock
     *
     * If the clock is already enabled, this function won't have any effect.
     *
     * @param time Time from which the clock has to be enabled
     */
    void enable(const uint64_t time);

    /**
     * @brief Enable clock
     *
     * If the clock is already enabled, this method won't have any effect.
     *
     * @param p_context Pointer to the current simulation context
     */
    void enable(VerilatedContext* const p_context);

    /**
     * @brief Disable clock
     */
    inline void disable();

    /**
     * @brief Evaluate (toggle) clock if relevant
     *
     * If the provided time value corresponds to the time at which the time is
     * expected to toggle, it shall be toggled. Otherwise, nothing shall
     * happen. If relevant, the next event time and the cycles counter is
     * incremented at each falling edge.
     *
     * @param time Simulator time value
     * @return Evaluation status:
     * @return -1: Error case (clock outdated!)
     * @return  0: no evaluation
     * @return  1: rising edge event evaluated
     * @return  2: falling edge event evaluated
     */
    int eval(const uint64_t time);

    /**
     * @brief Evaluate (toggle) clock if relevant in context
     *
     * @param p_context Pointer to current simulation context
     * @return Evaluation status:
     * @return -1: Error case (clock outdated!)
     * @return  0: no evaluation
     * @return  1: rising edge event evaluated
     * @return  2: falling edge event evaluated
     */
    inline int eval(VerilatedContext* const p_context) {
        return eval(p_context->time());
    };

    /**
     * @brief Return clock current status
     *
     * @return true: The clock is enabled
     * @return false: The clock is disabled
     */
    inline bool is_enabled() const {return b_is_enabled;};

    /**
     * @brief Get next event time
     *
     * @return Simulation time at next event for the given clock
     */
    inline uint64_t get_next_event() const {return next_event_time;};

    // Define < operator to allow using sorting algorithms
    bool operator<(const VslClock& other) const {
        if (!b_is_enabled) return false; // Disabled clocks at the back
        return next_event_time < other.next_event_time;
    }

private:

    bool b_is_enabled {false};     // Enabled flag
    uint32_t cycles_counter {0u};  // Number of cycles that occured since last enable
    uint64_t prev_event_time {0u}; // Time of the latest, previous event
    uint64_t next_event_time {0u}; // Time of the next, upcoming event
    double duty_cycle {0.5f};      // Duty cycle of the clock (must be >0 and <1)
    uint64_t period;               // Period (in simulation time unit)
    uint64_t period_low;           // Low portion of the period
    uint64_t period_high;          // High portion of the period

};

class VslClockMap {

public:

    VslClockMap() = default;
    virtual ~VslClockMap() = default;

    /**
     * @brief Add (register) a clock variable
     * 
     * @param namep Name of the clock
     * @param datap Pointer to the corresponding Verilator variable.
     */
    void add_clock(const char* namep, std::any datap);

    /**
     * @brief Add (register) a clock variable
     * 
     * @param namep Name of the clock
     * @param datap Pointer to the corresponding Verilator variable.
     * @param period Period of the clock given (in simulation time)
     * @param duty_cycle Clock duty cycle (has to be > 0 and < 1)
     */
    void add_clock(const char* namep, std::any datap, const uint64_t period,
        const double duty_cycle);

    /**
     * @brief Add (register) a clock variable
     * 
     * @param namep Name of the clock
     * @param datap Pointer to the corresponding Verilator variable.
     * @param period Period of the clock
     * @param unit Time unit used for the clock period parameter
     * @param duty_cycle Clock duty cycle (has to be > 0 and < 1)
     */
    void add_clock(const char* namep, std::any datap, const double period,
        const char* unit, const double duty_cycle,
        VerilatedContext* const p_context);

    uint64_t get_next_event();

    /**
     * @brief Evaluate all clocks at a given simulation time
     *
     * @param time Simulation time
     * @return Total count of evaluated clocks
     */
    int eval(uint64_t time);

    /**
     * @brief Evaluate all clocks within a given simulation context
     *
     * @param p_context Pointer to simulation context
     * @return Total count of evaluated clocks
     */
    inline int eval(VerilatedContext* const p_context) {
        return eval(p_context->time());
    };

private:

    std::list<VslClock> clock_list;
};

} // namespace vsl
#endif //VSL_CLOCKS_HPP
// EOF

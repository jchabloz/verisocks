/***************************************************************************//**
 @file vsl_clocks.hpp
 @brief Clock class definition

 @author Jérémie Chabloz
 @copyright Copyright (c) 2025-2026 Jérémie Chabloz Distributed under the MIT
 License. See file for details.
*******************************************************************************/
/*
Copyright (c) 2025-2026 Jérémie Chabloz

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

using vsl_time_t = uint64_t;

/**
 * @class VslClock
 * @brief Class that represents a clock variable
 *
 * In order to use verilator without the --timing option, it is necessary to
 * have a verilated design that does not contain any time-dependency and whose
 * state evolution only depends on changes on its various inputs, such as e.g.
 * clocks. In order to get a most efficient way to design a top-level C++
 * testbench environments, it would be recommended to have all non-clock inputs
 * evaluated synchronously with any periodic clock such that the resulting
 * compiled model is purely cycle-based. In order to improve the model
 * efficiency it is also crucial to avoid having to drive the clock signals
 * with verisocks commands.
 */
class VslClock : public VslVar {

public:

    VslClock() = default;

    /**
    * @brief Constructs a new Vsl Clock object
    *
    * The oscillator period is expected to be provided as an integer, simulator
    * time. The conversion from a real time value can be done using vsl_utils
    * double_to_time function.
    *
    * @param namep Variable name
    * @param datap Pointer to Verilator variable
    * @param period Period in integer simulator time
    * @param duty_cycle Duty cycle
    */
    VslClock(const char* namep, std::any datap, const vsl_time_t period,
        const double duty_cycle);

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
        VerilatedContext* const p_context);

    /**
     * @brief Constructs a new Vsl Clock object
     *
     * @param namep Variable name
     * @param datap Pointer to Verilator variable
     * @param period Period in real value
     * @param unit Period time unit
     * @param duty_cycle Duty cycle
     * @param p_context Simulation context
     * @param enable Enable clock
     */
    VslClock(const char* namep, std::any datap, const double period,
        const char* unit, const double duty_cycle,
        VerilatedContext* const p_context, const bool enable = false);

    /**
    * @brief Set period and duty cycle
    *
    * @return 0 No errors
    * @return -1 Errors
    */
    int set_period(const vsl_time_t period, const double duty_cycle);

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
     *
     * @return 0 No errors
     * @return -1 Errors
     */
    int enable(const vsl_time_t time);

    /**
     * @brief Enable clock
     *
     * If the clock is already enabled, this method won't have any effect.
     *
     * @param p_context Pointer to the current simulation context
     *
     * @return 0 No errors
     * @return -1 Errors
     */
    int enable(VerilatedContext* const p_context);

    /**
     * @brief Disable clock
     */
    void disable();

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
    int eval(const vsl_time_t time);

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
    inline bool is_enabled(void) const {
        return b_is_enabled;
    };

    /**
     * @brief Get next event time
     *
     * @return Simulation time at next event for the given clock
     */
    inline vsl_time_t get_next_event(void) const {return next_clk_event_time;};

    /**
     * @brief "Smaller than" operator
     *
     * Defining the < operator is one possible way to allow using the
     * std::list::sort() sorting method.
     * 
     * @note Note that it is necessary for the operator to be logically complete
     * as it shall also be usable to infer the inverse proposition (if it is not
     * smaller, then it means that it is larger or equal). If it is not the
     * case, then the sorting algorithm will yield correct results.
     */
    bool operator<(const VslClock& clk) const {
        if (!b_is_enabled && clk.b_is_enabled) {return false;}
        if (b_is_enabled && !clk.b_is_enabled) {return true;}
        return next_clk_event_time < clk.next_clk_event_time;
    }

    inline vsl_time_t get_period_low(void) const {return period_low;};
    inline vsl_time_t get_period_high(void) const {return period_high;};
    inline bool is_waiting_disable(void) const {return b_wait_dis;};

private:

    bool b_is_enabled {false};       // Enabled flag
    bool b_wait_dis {false};         // Flag: waiting to get disabled
    uint32_t cycles_counter {0u};    // Number of cycles that occured since last enable
    vsl_time_t prev_clk_event_time {0u}; // Time of the latest, previous event
    vsl_time_t next_clk_event_time {0u}; // Time of the next, upcoming event
    double duty_cycle {0.5f};        // Duty cycle of the clock (must be >0 and <1)
    vsl_time_t period;               // Period (in simulation time unit)
    vsl_time_t period_low;           // Low portion of the period
    vsl_time_t period_high;          // High portion of the period

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
    void add_clock(const char* namep, std::any datap, const vsl_time_t period,
        const double duty_cycle);

    /**
     * @brief Add (register) a clock variable
     *
     * @param namep Name of the clock
     * @param datap Pointer to the corresponding Verilator variable.
     * @param period Period of the clock
     * @param unit Time unit used for the clock period parameter
     * @param duty_cycle Clock duty cycle (has to be > 0 and < 1)
     * @param p_context Pointer to Verilator simulation context
     */
    void add_clock(const char* namep, std::any datap, const double period,
        const char* unit, const double duty_cycle,
        VerilatedContext* const p_context, const bool enable);

    /**
     * @brief Checks if any of the registered clock is enabled
     * 
     * This function checks that there is at least one clock registered that is
     * currently properly constructed AND enabled. If the clock map is empty,
     * this function also returns false.
     * 
     * @return true At least one of the clocks is enabled
     * @return false None of the clocks are enabled
     */
    const bool has_next_event(void) const;

    /**
     * @brief Get the time at which the next event shall occur for registered
     * clocks
     *
     * @return vsl_time_t Next event (simulation) time
     */
    const vsl_time_t get_next_event(void) const;

    /**
     * @brief Evaluate all clocks at a given simulation time
     *
     * @param time Simulation time
     * @return Total count of evaluated clocks
     */
    int eval(vsl_time_t time);

    /**
     * @brief Evaluate all clocks within a given simulation context
     *
     * @param p_context Pointer to simulation context
     * @return Total count of evaluated clocks
     */
    inline int eval(VerilatedContext* const p_context) {
        return eval(p_context->time());
    };

    /**
     * @brief Indicates if the list of registered clocks is empty
     * 
     * @return true No clock registered
     * @return false Clocks registered
     */
    inline const bool empty(void) const {return clock_list.empty();}

    /**
     * @brief Checks if a clock by a given name exists in the clocks list
     * 
     * @param name Name of the clock to search for
     * @return bool true if the clock by the given name has been found
     */
    const bool has_clock(const std::string name);

    /**
     * @brief Get a registered clock by its name.
     *
     * It is recommended to use the function has_clock first in order to verify
     * that the clock exists.
     * 
     * @param name Name of the clock
     * @return Clock
     */
    VslClock& get_clock(const std::string name);

private:

    /* Clock list */
    std::list<VslClock> clock_list;

    /* Sort clock list */
    inline void sort_clocks(void) {clock_list.sort();}
    
    /* For debug purposes, print the clock list content */
    void display_list(void);
};

} // namespace vsl
#endif //VSL_CLOCKS_HPP
// EOF

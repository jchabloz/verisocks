/***************************************************************************//**
 @file vsl_types.hpp
 @brief Types definitions for Verisocks Verilator integration.

 @author Jérémie Chabloz
 @copyright Copyright (c) 2024-2025 Jérémie Chabloz Distributed under the MIT
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

#ifndef VSL_TYPES_HPP
#define VSL_TYPES_HPP

#include "vsl/vsl_utils.hpp"
#include "verilated.h"
#include "cJSON.h"
#include <string>
#include <any>
#include <unordered_map>

namespace vsl {

/**
 * @enum VslType
 * @brief Variable types
 */
enum VslType {
    VSL_TYPE_SCALAR,        ///<Scalar variable type
    VSL_TYPE_CLOCK,         ///<Clock variable type
    VSL_TYPE_ARRAY,         ///<Array variable type (dimension = 2)
    VSL_TYPE_MDARRAY,       ///<Multi-dimensional array variable type (dimension > 2)
    VSL_TYPE_EVENT,         ///<Event variable type
    VSL_TYPE_PARAM,         ///<Parameter type (assumed to be scalar)
    VSL_TYPE_NOT_SUPPORTED, ///<Not supported variable type
    VSL_TYPE_UNKNOWN        ///<Unknown variable type
};

/**
 * @class VslVar
 * @brief Represents a variable in the Verisocks library.
 *
 * This class encapsulates a variable with its associated metadata and provides
 * methods to get and set its value.
 *
 */
class VslVar {
public:

    /* Default constructor */
    VslVar() = default;

    /* Default destructor */
    virtual ~VslVar() = default;

    /**
    * @brief Constructor for class VslVar
    * 
    * @param namep Name of the variable.
    * @param datap Pointer to the corresponding Verilator variable.
    * @param vltype Type of the variable as defined by VerilatedVarType.
    * @param type Type of the variable as defined by VslType.
    * @param dims Number of dimensions of the variable (default is 0).
    * @param depth Depth of the variable (default is 0).
    */
    VslVar(const char* namep, std::any datap, VerilatedVarType vltype,
        VslType type, size_t dims, size_t width, size_t depth) :
        namep {namep}, datap {datap}, vltype {vltype}, type {type},
        dims {dims}, width {width}, depth {depth} {};

    /**
     * @brief Returns the value of a scalar variable
     *
     * If the variable is a scalar or a parameter, the function returns the
     * numerical value of the variable as a double.
     * If the variable is an event, it returns 1.0f if the event is triggered
     * and 0.0f if it is not.
     * In case of a non-scalar variable, the value 0.0f is returned by default.
     * It is expected that this has been checked before calling this function.
     *
     * @return Variable numeric value as a double
     */
    double get_value();

    /**
     * @brief Returns the value of an array variable at a given index
     *
     * If the variable is an array, the function returns the numerical value of
     * the array that is located at the index.
     * If the variable is not an array, the value 0.0f is returned by default.
     * It is expected that this has been checked before calling this function.
     *
     * @param index Index of the value to be returned
     * @return Variable numeric value as a double
     */
    double get_array_value(size_t index);

    /**
     * @brief Sets the value of a scalar variable
     * @param value Value to be set
     * @return Returns 0 in case of success, -1 otherwise
     */
    int set_value(double value);

    /**
     * @brief Sets the value of an array variable at a given index
     * @param value Value to be set
     * @param index Index of the variable to be set in the array
     * @return Returns 0 in case of success, -1 otherwise
     */
    int set_array_value(double value, size_t index);

    /**
     * @brief Sets a full array from a cJSON array object
     *
     * @param p_obj Pointer to cJSON array object
     * @return Returns 0 in case of success, -1 otherwise
     */
    int set_array_variable_value(cJSON* p_obj);

    /**
     * @brief Add the value of a scalar variable to a cJSON object
     * @param p_msg Pointer to cJSON message object
     * @param key Key to be used in the cJSON object
     * @return Returns 0 in case of success, -1 otherwise
     */
    int add_value_to_msg(cJSON* p_msg, const char* key);

    /**
     * @brief Add an array variable to a cJSON object
     * @param p_msg Pointer to cJSON message object
     * @param key Key to be used in the cJSON object
     * @return Returns 0 in case of success, -1 otherwise
     */
    int add_array_to_msg(cJSON* p_msg, const char* key);

    /**
     * @brief Add a sub-range of an array variable to a cJSON object
     *
     * If the length of the specified sub-range is 1, the corresponding value
     * is added to the cJSON objet as a numerical value and not an array.
     *
     * @param p_msg Pointer to cJSON message object
     * @param key Key to be used in the cJSON object
     * @param range Sub-range definition
     * @return Returns 0 in case of success, -1 otherwise
     */
    int add_array_to_msg(cJSON* p_msg, const char* key,
        const VslArrayRange& range);

    /**
     * @brief Returns the variable name
     * @return Variable name
     */
    const std::string get_name() { return std::string(namep); }

    /**
     * @brief Compare the variable name with the provided argument
     * @param name Name against which to test the variable's name
     * @return True if the variable's name matches
     */
    const bool is_named(const std::string name) {
        return (name.compare(std::string {namep}) == 0);
    }

    /**
     * @brief Returns the number of dimensions for the variable
     * @return Number of dimensions
     */
    const size_t get_dims() { return dims; }

    /**
     * @brief Returns the width of the variable (as represented in the original
     * (System)Verilog HDL code)
     * @return Width value
     */
    const size_t get_width() { return width; }

    /**
     * @brief Returns the depth of an array variable
     * @note At the moment, only 2-dimensional arrays are supported
     * @return Depth value
     */
    const size_t get_depth() { return depth; }

    /**
     * @brief Returns the corresponding Verilator variable type
     * @return Verilated variable type
     */
    const VerilatedVarType get_vltype() { return vltype; }

    /**
     * @brief Returns the variable type
     * @return Variable type
     */
    const VslType get_type() { return type; }

private:
    const char* namep;
    std::any datap;
    VerilatedVarType vltype {VLVT_UNKNOWN};
    VslType type {VSL_TYPE_UNKNOWN};
    size_t dims {0};
    size_t width {0};
    size_t depth {0};
};

class VslVarMap {

public:
    VslVarMap() = default;
    virtual ~VslVarMap() = default;

    /**
     * @brief Adds a variable to the variable map.
     *
     * This function inserts a variable into the variable map with the
     * specified name.
     *
     * @param namep The name of the variable to be added.
     * @param var The variable to be added to the map.
     */
    void add_var(const char* namep, VslVar& var) {
        var_map[namep] = var;
    }

    /**
     * @brief Adds a variable to the variable map with the specified
     * properties.
     *
     * @param namep The name of the variable.
     * @param datap The data associated with the variable, stored as std::any.
     * @param vltype The type of the variable as defined by VerilatedVarType.
     * @param type The type of the variable as defined by VslType.
     * @param dims The number of dimensions of the variable.
     * @param width The width of the variable.
     * @param depth The depth of the variable.
     */
    void add_var(const char* namep, std::any datap, VerilatedVarType vltype,
        VslType type, size_t dims, size_t width, size_t depth) {
        var_map[namep] = VslVar {
            namep, datap, vltype, type, dims, width, depth};
    }

    /**
     * @brief Checks if a variable exists in the variable map.
     *
     * This function checks whether a given string path exists as a key in the
     * var_map.
     *
     * @param str_path The string path to check in the var_map.
     * @return true if the string path exists in the var_map, false otherwise.
     */
    const bool has_var(const std::string& str_path) const {
        if (var_map.count(str_path) > 0) return true;
        return false;
    }

    /**
     * @brief Checks if the variables map is empty
     *
     * @return bool True if the variable map is empty
     */
    inline const bool empty() const {return var_map.empty();}

    /**
     * @brief Retrieves a variable from the variable map.
     *
     * This function searches for a variable in the `var_map` using the
     * provided string path. If the variable is found, a pointer to the
     * variable is returned. If the variable is not found, an error is logged
     * and a `nullptr` is returned.
     *
     * @param str_path The string path used to search for the variable in the
     * map.
     * @return VslVar* Pointer to the variable if found, otherwise `nullptr`.
     */
    VslVar* get_var(const std::string& str_path);

private:
    std::unordered_map<std::string, VslVar> var_map;
};

} // namespace vsl

#endif //VSL_TYPES_HPP

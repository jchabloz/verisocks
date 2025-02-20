/*
Copyright (c) 2025 Jérémie Chabloz

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

#ifndef VSL_TYPES_HPP
#define VSL_TYPES_HPP

#include "verilated.h"
#include "cJSON.h"
#include <string>
#include <any>
#include <unordered_map>

namespace vsl {

enum VslType {
    VSL_TYPE_SCALAR,        ///Scalar variable type
    VSL_TYPE_ARRAY,         ///Array variable type (dimension = 2)
    VSL_TYPE_MDARRAY,       ///Multi-dimensional array variable type (dimension > 2)
    VSL_TYPE_EVENT,         ///Event variable type
    VSL_TYPE_NOT_SUPPORTED, ///Not supported variable type
    VSL_TYPE_UNKNOWN        ///Unknown variable type
};

/**
 * @class VslVar
 * @brief Represents a variable in the Verisocks library.
 *
 * This class encapsulates a variable with its associated metadata and provides
 * methods to get and set its value.
 *
 * @param namep Pointer to the name of the variable.
 * @param datap Pointer to the data of the variable.
 * @param vltype Type of the variable as defined by VerilatedVarType.
 * @param type Type of the variable as defined by VslType.
 * @param dims Number of dimensions of the variable (default is 0).
 * @param depth Depth of the variable (default is 0).
 */
class VslVar {
public:

    /* Default constructor*/
    VslVar() = default;

    /* Default destructor */
    virtual ~VslVar() = default;

    /* Constructor */
    VslVar(const char* namep, std::any datap, VerilatedVarType vltype,
        VslType type, size_t dims, size_t width, size_t depth) : 
        namep {namep}, datap {datap}, vltype {vltype}, type {type},
        dims {dims}, width {width}, depth {depth} {};

    double get_value();
    double get_array_value(size_t index);
    int set_value(double value);
    int set_array_value(double value, size_t index);
    int set_array_variable_value(cJSON* p_obj);
    int add_value_to_msg(cJSON* p_msg, const char* key);
    int add_array_to_msg(cJSON* p_msg, const char* key);

    const char* get_name() { return namep; }
    const size_t get_dims() { return dims; }
    const size_t get_width() { return width; }
    const size_t get_depth() { return depth; }
    const VerilatedVarType get_vltype() { return vltype; }
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
    const bool has_var(std::string str_path) {
        if (var_map.count(str_path) > 0) return true;
        return false;
    }

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
    VslVar* get_var(std::string str_path) {
        auto search = var_map.find(str_path);
        if (search != var_map.end()) {
            return &var_map[str_path];
        }
        vs_log_mod_error("vsl_types",
            "Could not find variable %s in registered variables map",
            str_path.c_str());
        return nullptr;
    }

private:
    std::unordered_map<std::string, VslVar> var_map;
};

} // namespace vsl

#endif //VSL_TYPES_HPP
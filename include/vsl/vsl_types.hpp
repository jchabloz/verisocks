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
#include <unordered_map>
#include <string>
#include <any>

namespace vsl {

enum VslType {
    VSL_TYPE_SCALAR, ///Scalar variable type
    VSL_TYPE_ARRAY,  ///Array variable type
    VSL_TYPE_EVENT   ///Event variable type
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
    VslVar(const char* namep, std::any datap, VerilatedVarType vltype,
        VslType type, size_t dims, size_t depth) :
        namep(namep), datap(datap), type(type), vltype(vltype),
        dims(dims), depth(depth) {};
    VslVar(const char* namep, std::any datap, VerilatedVarType vltype,
        VslType type) :
        namep(namep), datap(datap), type(type), vltype(vltype) {};
    virtual ~VslVar() = default;

    double get_value();
    double get_array_value(size_t index);
    void set_value(double value);
    void set_array_value(double value, size_t index);
    int add_value_to_msg(cJSON* p_msg, const char* key);
    int add_array_to_msg(cJSON* p_msg, const char* key);


private:
    const char* namep;
    std::any datap;
    VerilatedVarType vltype;
    VslType type;
    size_t dims {0};
    size_t depth {0};
};

} // namespace vsl

#endif //VSL_TYPES_HPP
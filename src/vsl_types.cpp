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

#include "vs_logging.h"
#include "vsl/vsl_types.hpp"
#include "verilated.h"
#include <any>

namespace vsl{

template <typename T>
static inline T __get_value(std::any &datap) {
    return *(std::any_cast<T*>(datap));
}

template <typename T>
static inline T __get_array_value(std::any &datap, size_t index) {
    return std::any_cast<T*>(datap)[index];
}

template <typename T>
static inline void __set_value(std::any &datap, double value)
{
    *(std::any_cast<T*>(datap)) = static_cast<T>(value);
}

template <typename T>
static inline void __set_array_value(std::any &datap, double value, size_t index)
{
    std::any_cast<T*>(datap)[index] = static_cast<T>(value);
}

double VslVar::get_value() {
    switch (type) {
        case VSL_TYPE_SCALAR:
            switch (vltype) {
                case VLVT_UINT8:
                    return static_cast<double>(__get_value<uint8_t>(datap));
                case VLVT_UINT16:
                    return static_cast<double>(__get_value<uint16_t>(datap));
                case VLVT_UINT32:
                    return static_cast<double>(__get_value<uint32_t>(datap));
                case VLVT_UINT64:
                    return static_cast<double>(__get_value<uint64_t>(datap));
                case VLVT_REAL:
                    return __get_value<double>(datap);
                default:
                    return 0.0f;
            }
        case VSL_TYPE_PARAM:
            switch (vltype) {
                case VLVT_UINT8:
                    return static_cast<const double>(
						__get_value<const uint8_t>(datap));
                case VLVT_UINT16:
                    return static_cast<const double>(
						__get_value<const uint16_t>(datap));
                case VLVT_UINT32:
                    return static_cast<const double>(
						__get_value<const uint32_t>(datap));
                case VLVT_UINT64:
                    return static_cast<const double>(
						__get_value<const uint64_t>(datap));
                case VLVT_REAL:
                    return __get_value<const double>(datap);
                default:
                    return 0.0f;
            }
        case VSL_TYPE_EVENT:
            if (std::any_cast<VlEvent*>(datap)->isTriggered())
                return 1.0f;
            else
                return 0.0f;
        default:
            /* In case of a non-scalar variable, a value of 0.0 is returned by
            default - the check for the variable type should be performed
            beforehand */
            return 0.0f;
    }
}

double VslVar::get_array_value(size_t index) {
    switch (type) {
        case VSL_TYPE_ARRAY:
            switch (vltype) {
                case VLVT_UINT8:
                    return static_cast<double>(
                        __get_array_value<uint8_t>(datap, index));
                case VLVT_UINT16:
                    return static_cast<double>(
                        __get_array_value<uint16_t>(datap, index));
                case VLVT_UINT32:
                    return static_cast<double>(
                        __get_array_value<uint32_t>(datap, index));
                case VLVT_UINT64:
                    return static_cast<double>(
                        __get_array_value<uint64_t>(datap, index));
                case VLVT_REAL:
                    return __get_array_value<double>(datap, index);
                default:
                    return 0.0f;
            }
        default:
            /* In case of a non-array variable, a value of 0.0 is returned by
            default - the check for the variable type should be performed
            beforehand */
            return 0.0f;
    }
}

int VslVar::set_value(double value) {
    switch (type) {
        case VSL_TYPE_SCALAR:
            switch (vltype) {
                case VLVT_UINT8:
                    __set_value<uint8_t>(datap, value);
                    return 0;
                case VLVT_UINT16:
                    __set_value<uint16_t>(datap, value);
                    return 0;
                case VLVT_UINT32:
                    __set_value<uint32_t>(datap, value);
                    return 0;
                case VLVT_UINT64:
                    __set_value<uint64_t>(datap, value);
                    return 0;
                case VLVT_REAL:
                    *(std::any_cast<double*>(datap)) = value;
                    return 0;
                default:
                    vs_log_mod_error(
                        "vsl_types",
                        "Type not supported (yet) for scalar variable");
                    return -1;
            }
            break;
        case VSL_TYPE_EVENT:
            std::any_cast<VlEvent*>(datap)->fire();
            return 0;
        case VSL_TYPE_PARAM:
            vs_log_mod_error(
                "vsl_types", "Cannot set parameter value");
            return -1;
        default:
            vs_log_mod_error(
                "vsl_types", "Cannot set variable value (non-scalar)");
            return -1;
    }
}

int VslVar::set_array_value(double value, size_t index) {
    switch (type) {
        case VSL_TYPE_ARRAY:
            if (index > (depth - 1)) {
                vs_log_mod_error(
                    "vsl_types", "Index exceeds array depth");
                return -1;
            }
            switch (vltype) {
                case VLVT_UINT8:
                    __set_array_value<uint8_t>(datap, value, index);
                    return 0;
                case VLVT_UINT16:
                    __set_array_value<uint16_t>(datap, value, index);
                    return 0;
                case VLVT_UINT32:
                    __set_array_value<uint32_t>(datap, value, index);
                    return 0;
                case VLVT_UINT64:
                    __set_array_value<uint64_t>(datap, value, index);
                    return 0;
                case VLVT_REAL:
                    std::any_cast<double*>(datap)[index] = value;
                    return 0;
                default:
                    vs_log_mod_error(
                        "vsl_types", "Type not supported for array value");
                    return -1;
            }
        default:
            vs_log_mod_error(
                "vsl_types", "Cannot set value (not part of an array)");
            return -1;
    }
}

int VslVar::set_array_variable_value(cJSON* p_obj) {
    /* Detect if the variable is not an array */
    if (type != VSL_TYPE_ARRAY) {
        vs_log_mod_error(
            "vsl_types", "Variable is not an array as expected");
        return -1;
    }

    /* Detect if the JSON object is not an array object */
    if (!cJSON_IsArray(p_obj)) {
        vs_log_mod_error(
            "vsl_types", "Command field \"value\" should be an array");
        return -1;
    }

    /* Verify that the arrays sizes are corresponding */
    if (depth != (size_t) cJSON_GetArraySize(p_obj)) {
        vs_log_mod_error(
            "vsl_types",
            "Command field \"value\" should be an array of length %d",
            (int) depth);
        return -1;
    }

    cJSON *iterator;
    double value = 0.0f;
    size_t index = 0;
    cJSON_ArrayForEach(iterator, p_obj) {
        value = cJSON_GetNumberValue(iterator);
        set_array_value(value, index);
        index++;
    }
    return 0;
}

int VslVar::add_value_to_msg(cJSON* p_msg, const char* key) {
    cJSON* p_value = nullptr;
    switch (type) {
        case VSL_TYPE_SCALAR:
        case VSL_TYPE_PARAM:
        case VSL_TYPE_EVENT:
            switch (vltype) {
                case VLVT_UINT8:
                case VLVT_UINT16:
                case VLVT_UINT32:
                case VLVT_UINT64:
                case VLVT_REAL:
                    p_value = cJSON_AddNumberToObject(
                        p_msg, key, get_value());
                    if (p_value == nullptr) {return -1;}
                    return 0;
                default:
                    vs_log_mod_error(
                        "vsl_type",
                        "Type not supported for adding value to JSON message"
                    );
                    return -1;
            }
        default:
            vs_log_mod_error(
                "vsl_type",
                "Non-scalar value"
            );
            return -1;
    }
    return 0;
}

int VslVar::add_array_to_msg(cJSON* p_msg, const char* key) {
    cJSON* p_array = nullptr;
    size_t mem_index = 0;
    switch (type) {
        case VSL_TYPE_ARRAY:
            if (dims != 2) {
                vs_log_mod_error(
                    "vsl_type",
                    "Variable dimension is not as expected"
                );
                return -1;
            }
            p_array = cJSON_AddArrayToObject(p_msg, key);
            if (p_array == nullptr) {
                vs_log_mod_error("vsl_type", "Could not create cJSON array");
                return -1;
            }
            cJSON_bool retval;
            while (mem_index < depth) {
                retval = cJSON_AddItemToArray(
                    p_array,
                    cJSON_CreateNumber(get_array_value(mem_index))
                );
                if (1 != retval) {
                    vs_log_mod_error(
                        "vsl_type", "Error adding number to array");
                    return -1;
                }
                mem_index++;
            }
            return 0;
        default:
            vs_log_mod_error(
                "vsl_type",
                "Non-array value"
            );
            return -1;
    }
    return 0;
}

int VslVar::add_array_to_msg(cJSON* p_msg, const char* key,
    const VslArrayRange& range)
{
    cJSON* p_obj = nullptr;
    if (range.left == range.right) {
        p_obj = cJSON_AddNumberToObject(
            p_msg, key, get_array_value(range.left));
        if (p_obj == nullptr) {return -1;}
        return 0;
    }
    p_obj = cJSON_AddArrayToObject(p_msg, key);
    if (p_obj == nullptr) {
        vs_log_mod_error("vsl_type", "Could not create cJSON array");
        return -1;
    }
    cJSON_bool retval;
    size_t mem_index = range.right;
    while (mem_index != (range.left + range.incr)) {
        retval = cJSON_AddItemToArray(
            p_obj,
            cJSON_CreateNumber(get_array_value(mem_index))
        );
        if (1 != retval) {
            vs_log_mod_error(
                "vsl_type", "Error adding number to array");
            return -1;
        }
        mem_index += range.incr;
    }
    return 0;
}

VslVar* VslVarMap::get_var(const std::string& str_path) {
    auto search = var_map.find(str_path);
    if (search != var_map.end()) {
        return &var_map[str_path];
    }
    vs_log_mod_error("vsl_types",
        "Could not find variable %s in registered variables map",
        str_path.c_str());
    return nullptr;
}

} //namespace vsl
// EOF

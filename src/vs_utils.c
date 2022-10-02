/**
 * @file vs_utils.c
 * @author jchabloz (jeremie.chabloz@a3.epfl.ch)
 * @brief Utilities for Verisocks VPI
 * @version 0.1
 * @date 2022-09-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <math.h>
#include <string.h>
#include <iverilog/vpi_user.h>
#include "vs_logging.h"
#include "vs_utils.h"


typedef struct s_vs_time_def {
    int factor;
    char *name;
} vs_time_def_t;

static const vs_time_def_t TIME_DEF_TABLE[] = {
    {0, "s"},
    {-3, "ms"},
    {-6, "us"},
    {-9, "ns"},
    {-12, "ps"},
    {-15, "fs"},
    {0, NULL}
};

static PLI_INT32 get_time_factor(const char *time_unit)
{
    const vs_time_def_t *ptr_tdef = TIME_DEF_TABLE;
    while(ptr_tdef->name != NULL) {
        if (strcmp(ptr_tdef->name, time_unit) == 0) {
            return ptr_tdef->factor;
        }
        ptr_tdef++;
    }
    vs_log_mod_error("vs_utils", "Wrong time unit identifier %s", time_unit);
    return 0;
}

double vs_utils_time_to_double(s_vpi_time time, const char *time_unit)
{
    double time_factor;
    if (NULL == time_unit || strcmp("", time_unit) == 0) {
        time_factor = 0.0;
    } else {
        time_factor = (double) get_time_factor(time_unit);
    }
    double time_precision = (double) vpi_get(vpiTimePrecision, NULL);

    double time_value;
    if (vpiSimTime == time.type) {
	    PLI_UINT64 time_value_int =
            (PLI_UINT64) time.low + ((PLI_UINT64) time.high << 32u);
        time_value =
            time_value_int * pow(10.0, time_precision - time_factor);
    } else if (vpiScaledRealTime == time.type) {
        time_value =
            time.real * pow(10.0, time_precision - time_factor);
    } else {
        vs_log_mod_error("vs_utils",
            "Unknown or non-supported time type value %d", (int) time.type);
    }
    return time_value;
}

s_vpi_time vs_utils_double_to_time(double time_value, const char *time_unit)
{
    double time_factor;
    if (NULL == time_unit || strcmp("", time_unit) == 0) {
        time_factor = 0.0;
    } else {
        time_factor = (double) get_time_factor(time_unit);
    }
    double time_precision = (double) vpi_get(vpiTimePrecision, NULL);
    time_value *= pow(10.0, time_factor - time_precision);

    PLI_UINT64 time_int = (PLI_UINT64) time_value;

    s_vpi_time vpi_time;
    vpi_time.type = vpiSimTime;
    vpi_time.low = (PLI_INT32) (time_int & 0xffffffff);
    vpi_time.high = (PLI_INT32) (time_int >> 32u);
    vpi_time.real = 0.0;

    return vpi_time;
}

// typedef struct s_obj_value {
//     PLI_INT32 obj_type;
//     PLI_INT32 format;
// } obj_value_t;

// static const obj_value_t obj_value_format[] = {
//     {vpiNet,        vpiIntVal},
//     {vpiReg,        vpiIntVal},
//     {vpiIntegerVar, vpiIntVal},
//     {vpiRealVar,    vpiRealVal},
//     {vpiParameter,  vpiRealVal},
//     {vpiConstant,   vpiRealVal},
//     {NULL, NULL}
// };

// static PLI_INT32 vs_utils_get_format(PLI_INT32 obj_type)
// {
//     obj_value_t *ptr = &obj_value_format;
//     while (ptr->format != NULL) {
//         if (obj_type == ptr->obj_type) {
//             return ptr->format;
//         }
//     }
//     return -1;
// }

PLI_INT32 vs_utils_get_value(vpiHandle h_obj, s_vpi_value* p_value)
{
    PLI_INT32 obj_type = vpi_get(vpiType, h_obj);
    switch (obj_type) {
    case vpiNet:
        p_value->format = vpiIntVal;
        break;
    case vpiReg:
        p_value->format = vpiIntVal;
        break;
    case vpiIntegerVar:
        p_value->format = vpiIntVal;
        break;
    case vpiRealVar:
        p_value->format = vpiRealVal;
        break;
    case vpiParameter:
        p_value->format = vpiRealVal;
        break;
    case vpiConstant:
        p_value->format = vpiRealVal;
        break;
    default:
        vs_log_mod_error("vs_utils",
            "Type %d currently not supported", obj_type);
        return -1;
    }

    vpi_get_value(h_obj, p_value);
    return 0;
}

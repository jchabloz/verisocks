/**
 * @file vs_utils.h
 * @author jchabloz (jeremie.chabloz@a3.epfl.ch)
 * @brief Utilities for Verisocks VPI functions
 * @version 0.1
 * @date 2022-09-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef VS_UTILS_H
#define VS_UTILS_H

#include <iverilog/vpi_user.h>

/**
 * @brief Convert VPI time to a real (double) value
 * 
 * @param time s_vpi_time structure.
 * @param time_unit Time unit (e.g. "s", "us", etc.). If NULL or "", processed
 * as "s".
 * @return Converted time real value in defined unit
 */
double vs_utils_time_to_double(s_vpi_time time, const char *time_unit);

s_vpi_time vs_utils_double_to_time(double time_value, const char *time_unit);

#endif //VS_UTILS_H

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
 * @brief Convert VPI time struct to a real (double) value
 * 
 * @param time s_vpi_time time struct. Only supported type values are
 * vpiSimTime and vpiScaledRealTime.
 * @param time_unit Time unit desired (should be "s", "ms", "us", "ns", "ps" or
 * "fs", case sensitive). If NULL or "", the value is processed and returned in
 * seconds ("s").
 * @return Converted time real value in defined unit
 */
double vs_utils_time_to_double(s_vpi_time time, const char *time_unit);

/**
 * @brief Convert real (double) value to a VPI time struct
 * 
 * @param time_value Real time value.
 * @param time_unit Time unit associated with the provided real time value
 * (should be "s", "ms", "us", "ns", "ps" or "fs", case sensitive). If NULL or
 * "", the value is considered as being defined in seconds ("s").
 * @return s_vpi_time struct with vpiSimTime type
 */
s_vpi_time vs_utils_double_to_time(double time_value, const char *time_unit);

#endif //VS_UTILS_H

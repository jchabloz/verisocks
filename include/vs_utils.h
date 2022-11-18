/**
 * @file vs_utils.h
 * @author jchabloz
 * @brief Utilities for Verisocks VPI functions
 * @version 0.1
 * @date 2022-09-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef VS_UTILS_H
#define VS_UTILS_H

#include "vpi_config.h"
#include "cJSON.h"

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

/**
 * @brief Get the Verisocks interface format of choice to represent the value
 * for a given object.
 * 
 * @param h_obj Object handle
 * @return Format
 */
PLI_INT32 vs_utils_get_format(vpiHandle h_obj);

/**
 * @brief Get the value of an object. Format will be as returned by the
 * vs_utils_get_format function.
 * 
 * @param h_obj Object handle
 * @param p_value Pointer to an s_vpi_value struct that will be updated with
 * the value
 * @return 0 if successful, -1 in case of error.
 */
PLI_INT32 vs_utils_get_value(vpiHandle h_obj, s_vpi_value* p_value);

/**
 * @brief Compare two values
 * 
 * @warning Currently supports only vpiIntVal and vpiRealVal values
 * @param val1 First value
 * @param val2 Second value
 * @return Returns 0 if val1 and val2 are equivalent, 1 otherwise, -1 if there
 * is an error (e.g. type not supported)
 */
PLI_INT32 vs_utils_compare_values(s_vpi_value val1, s_vpi_value val2);

/**
 * @brief Set value from a VPI handle and a value
 * 
 * @param h_obj VPI object handle
 * @param value Value
 * @return 0 if successful, -1 in case of error
 */
PLI_INT32 vs_utils_set_value(vpiHandle h_obj, double value);

/**
 * @brief Add value to cJSON message object
 * 
 * @param value s_vpi_value struct containing format and value to add
 * @param p_msg Pointer to cJSON struct
 * @param key Key (string) to use in cJSON struct 
 * @return Returns 0 if successful, -1 in case of error
 */
PLI_INT32 vs_utils_add_value(s_vpi_value value, cJSON* p_msg, const char* key);

#endif //VS_UTILS_H

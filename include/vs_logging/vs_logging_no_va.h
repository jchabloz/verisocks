/**************************************************************************//**
 @file vs_logging_no_va.h
 @brief Verisocks logging macros for C preprocessor with variadic parameters
 support without VA_OPT

 @author Jérémie Chabloz
 @copyright Copyright (c) 2022-2025 Jérémie Chabloz Distributed under the MIT
 License. See file for details.
 ******************************************************************************/
/*
MIT License

Copyright (c) 2022-2025 Jérémie Chabloz

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

#ifndef VS_LOGGING_NO_VA_H
#define VS_LOGGING_NO_VA_H

#include <stdio.h>

/******************************************************************************/
/* Generic */
/******************************************************************************/
/** @brief Generic macro to log a message with module name
 *  @param LEVEL Level value
 *  @param MODNAME Module name
 *  @param fmt Formatted message
*/
#define VS_LOG_MODNAME(LEVEL, MODNAME, fmt, ...) \
    fprintf(stderr, LEVEL " [" MODNAME "]: " \
    fmt "\n", ##__VA_ARGS__)

/**
 * @brief Generic macro to log a VPI message
 * @param LEVEL Level value
 * @param fmt Formatted message
 */
#define VS_VPI_LOG(LEVEL, fmt, ...) \
    vpi_printf(LEVEL " [Verisocks]: " fmt "\n", ##__VA_ARGS__)

/**
 * @brief Generic macro to log a message
 * @param LEVEL Level value
 * @param fmt Formatted message
 */
#define VS_LOG(LEVEL, fmt, ...) \
    fprintf(stderr, LEVEL ": " \
    fmt "\n", ##__VA_ARGS__)

/******************************************************************************/
/* Debug-level macros */
/******************************************************************************/
#if (VS_LOG_LEVEL <= LEVEL_DEBUG)
/** @brief Logs a debug level message with module name
 *  @param modname Module name
 *  @param fmt Formatted message
*/
#define vs_log_mod_debug(modname, fmt, ...) \
    VS_LOG_MODNAME("DEBUG", modname, fmt, ##__VA_ARGS__)
/** @brief Logs a debug level message
 *  @param fmt Formatted message
*/
#define vs_log_debug(fmt, ...) \
    VS_LOG("DEBUG", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_debug(modname, fmt, ...)
#define vs_log_debug(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_DEBUG)
/** @brief Logs a VPI debug level message
 *  @param fmt Formatted message
 */
#define vs_vpi_log_debug(fmt, ...) \
    VS_VPI_LOG("DEBUG", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_debug(fmt, ...)
#endif

/******************************************************************************/
/* Info-level macros */
/******************************************************************************/
#if (VS_LOG_LEVEL <= LEVEL_INFO)
/** @brief Logs an info level message with module name
 *  @param modname Module name
 *  @param fmt Formatted message
*/
#define vs_log_mod_info(modname, fmt, ...) \
    VS_LOG_MODNAME("INFO", modname, fmt, ##__VA_ARGS__)
/** @brief Logs an info level message
 *  @param fmt Formatted message
*/
#define vs_log_info(fmt, ...) \
    VS_LOG("INFO", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_info(modname, fmt, ...)
#define vs_log_info(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_INFO)
/** @brief Logs a VPI info level message
 *  @param fmt Formatted message
*/
#define vs_vpi_log_info(fmt, ...) \
    VS_VPI_LOG("INFO", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_info(fmt, ...)
#endif

/******************************************************************************/
/* Warning-level macros */
/******************************************************************************/
#if (VS_LOG_LEVEL <= LEVEL_WARNING)
/** @brief Logs a warning level message with module name
 *  @param modname Module name
 *  @param fmt Formatted message
*/
#define vs_log_mod_warning(modname, fmt, ...) \
    VS_LOG_MODNAME("WARNING", modname, fmt, ##__VA_ARGS__)
/** @brief Logs a warning level message
 *  @param fmt Formatted message
*/
#define vs_log_warning(fmt, ...) \
    VS_LOG("WARNING", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_warning(modname, fmt, ...)
#define vs_log_warning(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_WARNING)
/** @brief Logs a VPI warning level message
 *  @param fmt Formatted message
*/
#define vs_vpi_log_warning(fmt, ...) \
    VS_VPI_LOG("WARNING", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_warning(fmt, ...)
#endif

/******************************************************************************/
/* Error-level macros */
/******************************************************************************/
#if (VS_LOG_LEVEL <= LEVEL_ERROR)
/** @brief Logs an error level message with module name
 *  @param modname Module name
 *  @param fmt Formatted message
*/
#define vs_log_mod_error(modname, fmt, ...) \
    VS_LOG_MODNAME("ERROR", modname, fmt, ##__VA_ARGS__)
/** @brief Logs an error level message
 *  @param fmt Formatted message
*/
#define vs_log_error(fmt, ...) \
    VS_LOG("ERROR", fmt, ##__VA_ARGS__)
#define vs_log_mod_perror(modname, val) \
    perror("ERROR [" modname "]: " val)
#define vs_log_perror(val) \
    perror("ERROR: " val)
#else
#define vs_log_mod_error(modname, fmt, ...)
#define vs_log_error(fmt, ...)
#define vs_log_mod_perror(modname, val)
#define vs_log_perror(val)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_ERROR)
/** @brief Logs a VPI error level message
 *  @param fmt Formatted message
*/
#define vs_vpi_log_error(fmt, ...) \
    VS_VPI_LOG("ERROR", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_error(fmt, ...)
#endif

/******************************************************************************/
/* Critical-level macros */
/******************************************************************************/
#if (VS_LOG_LEVEL <= LEVEL_CRITICAL)
/** @brief Logs a critical level message with module name
 *  @param modname Module name
 *  @param fmt Formatted message
*/
#define vs_log_mod_critical(modname, fmt, ...) \
    VS_LOG_MODNAME("CRITICAL", modname, fmt, ##__VA_ARGS__)
/** @brief Logs a critical level message
 *  @param fmt Formatted message
*/
#define vs_log_critical(fmt, ...) \
    VS_LOG("CRITICAL", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_critical(modname, fmt, ...)
#define vs_log_critical(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_CRITICAL)
/** @brief Logs a VPI critical level message
 *  @param fmt Formatted message
*/
#define vs_vpi_log_critical(fmt, ...) \
    VS_VPI_LOG("CRITICAL", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_critical(fmt, ...)
#endif

#endif //VS_LOGGING_NO_VA_H


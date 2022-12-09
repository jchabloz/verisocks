/**
 * @file vs_logging.h
 * @author jchabloz
 * @brief Verisocks logging macros
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef VS_LOGGING_H
#define VS_LOGGING_H

#include <stdio.h>

/* Define logging levels values */
#define LEVEL_DEBUG 10
#define LEVEL_INFO 20
#define LEVEL_WARNING 30
#define LEVEL_ERROR 40
#define LEVEL_CRITICAL 50

/* Define default logging level */
#ifndef VS_LOG_LEVEL
#define VS_LOG_LEVEL LEVEL_DEBUG
#endif

/* Define default VPI logging level */
#ifndef VS_VPI_LOG_LEVEL
#define VS_VPI_LOG_LEVEL LEVEL_DEBUG
#endif

/* Macros to verify if C preprocessor supports __VA_OPT__ */
#define PP_THIRD_ARG(a,b,c,...) c
#define VA_OPT_SUPPORTED_I(...) PP_THIRD_ARG(__VA_OPT__(,),true,false,)
#define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_I(?)

/******************************************************************************/
#ifdef VA_OPT_SUPPORTED
/******************************************************************************/
/* Generic */
#define VS_LOG_MODNAME(LEVEL, MODNAME, fmt, ...) \
    fprintf(stderr, LEVEL " [" MODNAME "]: " \
    fmt "\n" __VA_OPT__(,) __VA_ARGS__)

#define VS_VPI_LOG(LEVEL, fmt, ...) \
    vpi_printf(LEVEL " [Verisocks]: " fmt "\n" __VA_OPT__(,) __VA_ARGS__)

#define VS_LOG(LEVEL, fmt, ...) \
    fprintf(stderr, LEVEL ": " \
    fmt "\n" __VA_OPT__(,) __VA_ARGS__)

/* Debug-level macros */
#if (VS_LOG_LEVEL <= LEVEL_DEBUG)
#define vs_log_mod_debug(modname, fmt, ...) \
    VS_LOG_MODNAME("DEBUG", modname, fmt __VA_OPT__(,) __VA_ARGS__)
#define vs_log_debug(fmt, ...) \
    VS_LOG("DEBUG", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_log_mod_debug(modname, fmt, ...)
#define vs_log_debug(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_DEBUG)
#define vs_vpi_log_debug(fmt, ...) \
    VS_VPI_LOG("DEBUG", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_vpi_log_debug(fmt, ...)
#endif

/* Info-level macros */
#if (VS_LOG_LEVEL <= LEVEL_INFO)
#define vs_log_mod_info(modname, fmt, ...) \
    VS_LOG_MODNAME("INFO", modname, fmt __VA_OPT__(,) __VA_ARGS__)
#define vs_log_info(fmt, ...) \
    VS_LOG("INFO", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_log_mod_info(modname, fmt, ...)
#define vs_log_info(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_INFO)
#define vs_vpi_log_info(fmt, ...) \
    VS_VPI_LOG("INFO", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_vpi_log_info(fmt, ...)
#endif

/* Warning-level macros */
#if (VS_LOG_LEVEL <= LEVEL_WARNING)
#define vs_log_mod_warning(modname, fmt, ...) \
    VS_LOG_MODNAME("WARNING", modname, fmt __VA_OPT__(,) __VA_ARGS__)
#define vs_log_warning(fmt, ...) \
    VS_LOG("WARNING", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_log_mod_warning(modname, fmt, ...)
#define vs_log_warning(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_WARNING)
#define vs_vpi_log_warning(fmt, ...) \
    VS_VPI_LOG("WARNING", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_vpi_log_warning(fmt, ...)
#endif

/* Error-level macros */
#if (VS_LOG_LEVEL <= LEVEL_ERROR)
#define vs_log_mod_error(modname, fmt, ...) \
    VS_LOG_MODNAME("ERROR", modname, fmt __VA_OPT__(,) __VA_ARGS__)
#define vs_log_error(fmt, ...) \
    VS_LOG("ERROR", fmt __VA_OPT__(,) __VA_ARGS__)
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
#define vs_vpi_log_error(fmt, ...) \
    VS_VPI_LOG("ERROR", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_vpi_log_error(fmt, ...)
#endif

/* Critical-level macros */
#if (VS_LOG_LEVEL <= LEVEL_CRITICAL)
#define vs_log_mod_critical(modname, fmt, ...) \
    VS_LOG_MODNAME("CRITICAL", modname, fmt __VA_OPT__(,) __VA_ARGS__)
#define vs_log_critical(fmt, ...) \
    VS_LOG("CRITICAL", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_log_mod_critical(modname, fmt, ...)
#define vs_log_critical(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_CRITICAL)
#define vs_vpi_log_critical(fmt, ...) \
    VS_VPI_LOG("CRITICAL", fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define vs_vpi_log_critical(fmt, ...)
#endif

/******************************************************************************/
#else  // __VA_OPT__ not supported
/******************************************************************************/
/* Generic */
#define VS_LOG_MODNAME(LEVEL, MODNAME, fmt, ...) \
    fprintf(stderr, LEVEL " [" MODNAME "]: " \
    fmt "\n", ##__VA_ARGS__)

#define VS_VPI_LOG(LEVEL, fmt, ...) \
    vpi_printf(LEVEL " [Verisocks]: " fmt "\n", ##__VA_ARGS__)

#define VS_LOG(LEVEL, fmt, ...) \
    fprintf(stderr, LEVEL ": " \
    fmt "\n", ##__VA_ARGS__)

/* Debug-level macros */
#if (VS_LOG_LEVEL <= LEVEL_DEBUG)
#define vs_log_mod_debug(modname, fmt, ...) \
    VS_LOG_MODNAME("DEBUG", modname, fmt, ##__VA_ARGS__)
#define vs_log_debug(fmt, ...) \
    VS_LOG("DEBUG", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_debug(modname, fmt, ...)
#define vs_log_debug(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_DEBUG)
#define vs_vpi_log_debug(fmt, ...) \
    VS_VPI_LOG("DEBUG", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_debug(fmt, ...)
#endif

/* Info-level macros */
#if (VS_LOG_LEVEL <= LEVEL_INFO)
#define vs_log_mod_info(modname, fmt, ...) \
    VS_LOG_MODNAME("INFO", modname, fmt, ##__VA_ARGS__)
#define vs_log_info(fmt, ...) \
    VS_LOG("INFO", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_info(modname, fmt, ...)
#define vs_log_info(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_INFO)
#define vs_vpi_log_info(fmt, ...) \
    VS_VPI_LOG("INFO", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_info(fmt, ...)
#endif

/* Warning-level macros */
#if (VS_LOG_LEVEL <= LEVEL_WARNING)
#define vs_log_mod_warning(modname, fmt, ...) \
    VS_LOG_MODNAME("WARNING", modname, fmt, ##__VA_ARGS__)
#define vs_log_warning(fmt, ...) \
    VS_LOG("WARNING", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_warning(modname, fmt, ...)
#define vs_log_warning(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_WARNING)
#define vs_vpi_log_warning(fmt, ...) \
    VS_VPI_LOG("WARNING", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_warning(fmt, ...)
#endif

/* Error-level macros */
#if (VS_LOG_LEVEL <= LEVEL_ERROR)
#define vs_log_mod_error(modname, fmt, ...) \
    VS_LOG_MODNAME("ERROR", modname, fmt, ##__VA_ARGS__)
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
#define vs_vpi_log_error(fmt, ...) \
    VS_VPI_LOG("ERROR", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_error(fmt, ...)
#endif

/* Critical-level macros */
#if (VS_LOG_LEVEL <= LEVEL_CRITICAL)
#define vs_log_mod_critical(modname, fmt, ...) \
    VS_LOG_MODNAME("CRITICAL", modname, fmt, ##__VA_ARGS__)
#define vs_log_critical(fmt, ...) \
    VS_LOG("CRITICAL", fmt, ##__VA_ARGS__)
#else
#define vs_log_mod_critical(modname, fmt, ...)
#define vs_log_critical(fmt, ...)
#endif

#if (VS_VPI_LOG_LEVEL <= LEVEL_CRITICAL)
#define vs_vpi_log_critical(fmt, ...) \
    VS_VPI_LOG("CRITICAL", fmt, ##__VA_ARGS__)
#else
#define vs_vpi_log_critical(fmt, ...)
#endif


#endif


#endif //VS_LOGGING_H

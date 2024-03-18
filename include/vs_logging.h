/**************************************************************************//**
@file vs_logging.h
@author jchabloz
@brief Verisocks logging macros
@date 2024-03-17
******************************************************************************/
/*
MIT License

Copyright (c) 2022-2024 Jérémie Chabloz

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

#if VA_OPT_SUPPORTED
#include "vs_logging/vs_logging_va.h"
#else  // __VA_OPT__ not supported
#include "vs_logging/vs_logging_no_va.h"
#endif

#endif //VS_LOGGING_H


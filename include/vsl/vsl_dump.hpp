/*
MIT License

Copyright (c) 2026 Jérémie Chabloz

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

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

#ifndef VSL_DUMP_HPP
#define VSL_DUMP_HPP

#ifdef DUMP_FILE
#ifdef DUMP_FST
#include "verilated_fst_c.h"
#elifdef DUMP_VCD
#include "verilated_vcd_c.h"
#else
#error "At least one of DUMP_FST or DUMP_VCD macros shall be defined"
#endif
#else // DUMP_FILE undefined
#ifdef DUMP_FST
#define DUMP_FILE
#elifdef DUMP_VCD
#define DUMP_FILE
#endif
#endif

#ifdef DUMP_FILE
#ifndef DUMP_LEVELS
#define DUMP_LEVELS 99
#endif
#endif

#endif // VSL_DUMP_HPP
// EOF
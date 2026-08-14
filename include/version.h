/*
MIT License

Copyright (c) 2026 Jérémie Chabloz

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

#ifndef VERSION_H
#define VERSION_H

#define VERISOCKS_VERSION_MAJOR 1
#define VERISOCKS_VERSION_MINOR 7
#define VERISOCKS_VERSION_PATCH 0-dev

#define xstr(s) str(s)
#define str(s) #s
#define VERISOCKS_VERSION xstr(VERISOCKS_VERSION_MAJOR) "." \
xstr(VERISOCKS_VERSION_MINOR) "." xstr(VERISOCKS_VERSION_PATCH)

#endif // VERSION_H
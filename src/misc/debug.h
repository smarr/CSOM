#ifndef DEBUG_H_
#define DEBUG_H_

/*
 * $Id: debug.h 111 2007-09-18 09:21:40Z tobias.pape $
 *
Copyright (c) 2007 Michael Haupt, Tobias Pape
Software Architecture Group, Hasso Plattner Institute, Potsdam, Germany
http://www.hpi.uni-potsdam.de/swa/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>


#define fprintf_pass(f,x) \
    va_list ap; \
    va_start(ap, (x)); \
    (void)vfprintf((f), (x), ap); \
    va_end(ap); \
    fflush(f)


static inline void debug_print(const char* fmt, ...) {
    fprintf_pass(stderr, fmt);
}


static inline void debug_prefix(const char* prefix) {
    debug_print("%-6s ", prefix);
}


#define debug_pass(x) \
    va_list ap; \
    va_start(ap, (x)); \
    (void)vfprintf(stderr, (x), ap); \
    va_end(ap); \
    fflush(stderr)


static inline void debug_info(const char* fmt, ...) {
    #ifdef DEBUG
        debug_prefix("INFO:"); 
        debug_pass(fmt);
    #endif // DEBUG
}


static inline void debug_log(const char* fmt, ...) {
    #ifdef DEBUG
        debug_prefix("LOG:"); 
        debug_pass(fmt);    
    #endif // DEBUG
}


static inline void debug_warn(const char* fmt, ...) {
    debug_prefix("WARN:"); 
    debug_pass(fmt);
}


static inline void debug_error(const char* fmt, ...) {
    debug_prefix("ERROR:"); 
    debug_pass(fmt);
}


static inline void debug_dump(const char* fmt, ...) {
    debug_prefix("DUMP:"); 
    debug_pass(fmt);    
}


static inline void debug_trace(const char* fmt, ...) {
    debug_prefix("TRACE:"); 
    debug_pass(fmt);
}


#undef fprintf_pass
#undef debug_pass 


#endif // DEBUG_H_

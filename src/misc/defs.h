#ifndef DEFS_H_
#define DEFS_H_

/*
 * $Id: defs.h 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <stdint.h>
#include <stddef.h>


//
// error codes
//


#define ERR_SUCCESS        0x0
#define ERR_FAIL           0x1
#define ERR_NOMEM          0x2
#define ERR_PANIC          0xFFFF


//
// macro for freeing an array
//

#define FREE_ARRAY_ELEMENTS(arr,count) \
    for(int i = (count)-1; i >= 0; i--) \
        gc_free((arr)[i])

#define FREE_ARRAY(arr,count) \
    FREE_ARRAY_ELEMENTS(arr,count); \
    gc_free((arr))


/**
 * A String hashing inline function
 * Java-like; see http://mindprod.com/jgloss/hashcode.html
 */
static inline int64_t string_hash(const char* restrict string, size_t length) {
    uint64_t result = 0;

    for (size_t i = 0; i < length; i++) {
        result = 31 * result + string[i];
    }
    return result;
}


//
// platform-dependent includes
//


#include <platform.h>


#endif // DEFS_H_

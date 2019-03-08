/*
 * $Id: Core.c 229 2008-04-22 14:10:50Z tobias.pape $
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "Array.h"
#include "Block.h"
#include "Class.h"
#include "Double.h"
#include "Integer.h"
#include "Method.h"
#include "Object.h"
#include "Primitive.h"
#include "String.h"
#include "Symbol.h"
#include "System.h"

/* Lib initialization */
#pragma mark * Load-time execution *
/**
 * Both the following functions have to do with load/unload-time execution.
 * That is, anything in init/_init is being executed upon every loading of the
 * library, even if this very library is _not_ responsible for the class in
 * question.
 *
 * Use with care.
 */

// Library load initializer function declaration. 
// This is compiler/os-specific
#ifdef __GNUC__
void init(void) __attribute__((constructor));
void fini(void) __attribute__((destructor));
#else
void _init(void);
void _fini(void);
#pragma init _init
#pragma fini _fini
#endif // __GNUC__

// Library load initializer
#ifdef __GNUC__
void init(void) { ; /* noop */}
#else
void _init(void) { ; /* noop */ }
#endif // __GNUC__

// Library unload function
#ifdef __GNUC__
void fini(void) { ; /* noop */ }
#else
void _fini(void) { ; /* noop */ }
#endif // __GNUC__


// Classes supported by this lib.
static char* supported_classes[] = {
    "Array",
    "BigInteger",
    "Block", //this is Block1..3
    "Class",
    "Double",
    "Frame",
    "Integer",
    "Method",
    "Object",
    "Primitive",
    "String",
    "Symbol",
    "System",
    NULL
};


/*************************************************/
#pragma mark * Exported functions starting here  *
/*************************************************/

// returns, whether this lib is responsible for a specific class
bool supports_class(const char* name) {
    char **iter=supported_classes;
    while(*iter)
        if(strcmp(name, *iter++)==0)
            return true;
    return false;
}


/**
 * init_csp()
 *
 * the library initializer. It is not equal to the init/fini-pair, as for them, 
 * the init is executed upon every loading of the shared library.
 *
 * All work that needs to be done before the actual primitives are assigned
 * should be called from this function.
 */
static bool initialized = false;

void init_csp(void) {
    // Call init funcions.
    if (!initialized) {
        __System_init();
        __Integer_init();
        initialized = true;
    }
}


#ifndef VMSTRING_H_
#define VMSTRING_H_

/*
 * $Id: VMString.h 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <vmobjects/VMObject.h>

#pragma mark VTable definition

VTABLE(VMString) {
#define VMSTRING_VTABLE_FORMAT \
    VMOBJECT_VTABLE_FORMAT; \
    const size_t (*get_length)(void*); \
    const char* (*get_rawChars)(void*)
        
    VMSTRING_VTABLE_FORMAT;
};

#pragma mark class definition

#define VMSTRING_FORMAT \
    VMOBJECT_FORMAT; \
    size_t length; \
    char chars[0]


struct _VMString {
    VTABLE(VMString)* _vtable;

    VMSTRING_FORMAT;
};

#pragma mark class methods

pVMString VMString_new(const char* restrict chars, size_t length);
pVMString VMString_new_concat(pVMString a, pVMString b);

#pragma mark vtable initialization

VTABLE(VMString)* VMString_vtable(void);

#endif // VMSTRING_H_

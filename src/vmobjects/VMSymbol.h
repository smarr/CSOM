#ifndef VMSYMBOL_H_
#define VMSYMBOL_H_

/*
 * $Id: VMSymbol.h 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <misc/String.h>

#include <vmobjects/VMString.h>

#pragma mark VTable definition

VTABLE(VMSymbol) {
#define VMSYMBOL_VTABLE_FORMAT \
    VMSTRING_VTABLE_FORMAT; \
    const char* (*get_plain_string)(void*)
    
    VMSYMBOL_VTABLE_FORMAT;
};

#pragma mark class definition

#define VMSYMBOL_FORMAT \
   VMSTRING_FORMAT


struct _VMSymbol {
    VTABLE(VMSymbol)* _vtable;    
    VMSYMBOL_FORMAT;
};

#pragma mark class methods

pVMSymbol VMSymbol_new(pString restrict);

#pragma mark vtable initialization

VTABLE(VMSymbol)* VMSymbol_vtable(void);

#endif // VMSYMBOL_H_

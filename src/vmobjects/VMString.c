/*
 * $Id: VMString.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "VMString.h"

#include <memory/gc.h>

#include <string.h>


//
//  Class Methods (Starting with VMString_) 
//


pVMString VMString_new(const char* restrict chars) {
    pVMString result = (pVMString)gc_allocate_object(
        sizeof(VMString) + sizeof(char) * (strlen(chars) + 1));
    if(result) {
        result->_vtable = VMString_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, chars);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/**
 * Initialize a VMString
 */
void _VMString_init(void* _self, ...) {
    pVMString self = (pVMString)_self;
    SUPER(VMObject, self, init, 0);
    
    va_list args; 
    va_start(args, _self);
    const char* embed = va_arg(args, char*);
    va_end(args);
    
    strcpy(self->chars, embed);
}


//
//  Instance Methods (Starting with _VMString_) 
//


const char* _VMString_get_chars(void* _self) {
    return (const char*)(((pVMString)_self)->chars);
}


//
// The VTABLE-function
//


static VTABLE(VMString) _VMString_vtable;
bool VMString_vtable_inited = false;


VTABLE(VMString)* VMString_vtable(void) {
    if(! VMString_vtable_inited) {
        *((VTABLE(VMObject)*)&_VMString_vtable) = *VMObject_vtable();
        _VMString_vtable.init = METHOD(VMString, init);        
        _VMString_vtable.get_chars = METHOD(VMString, get_chars);
        
        VMString_vtable_inited = true;
    }
    return &_VMString_vtable;
}

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


pVMString VMString_new(const char* restrict chars, size_t length) {
    pVMString result = (pVMString)gc_allocate_object(
        sizeof(VMString) + sizeof(char) * (length + 1));
    if (result) {
        result->_vtable = VMString_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, chars, length);
        gc_end_uninterruptable_allocation();
    }
    return result;
}

pVMString VMString_new_concat(pVMString a, pVMString b) {
    size_t aLen = SEND(a, get_length);
    size_t bLen = SEND(b, get_length);

    pVMString result = (pVMString)gc_allocate_object(
        sizeof(VMString) + sizeof(char) * (aLen + bLen + 1));

    if (result) {
        result->_vtable = VMString_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, "", 0);
        gc_end_uninterruptable_allocation();

        // now, do the actual concatination work
        size_t i = 0;
        for (; i < aLen; i++) {
            result->chars[i] = a->chars[i];
        }

        size_t j = 0;
        for (; j < bLen; i++, j++) {
            result->chars[i] = b->chars[j];
        }

        result->length = aLen + bLen;
        result->chars[result->length] = '\0';
    }
    return result;
}

/**
 * Initialize a VMString
 */
void _VMString_init(void* _self, ...) {
    pVMString self = (pVMString)_self;
    SUPER(VMObject, self, init, (intptr_t) 0);
    
    va_list args; 
    va_start(args, _self);
    const char* embed = va_arg(args, char*);
    size_t length = va_arg(args, size_t);
    va_end(args);
    
    memcpy(self->chars, embed, length);
    self->chars[length] = '\0';
    self->length = length;
    self->hash = 0;
}


//
//  Instance Methods (Starting with _VMString_) 
//


const char* _VMString_get_rawChars(void* _self) {
    return (const char*)(((pVMString)_self)->chars);
}


const size_t _VMString_get_length(void* _self) {
    return (const size_t)(((pVMString)_self)->length);
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

        _VMString_vtable.get_length = METHOD(VMString, get_length);
        _VMString_vtable.get_rawChars = METHOD(VMString, get_rawChars);
        
        VMString_vtable_inited = true;
    }
    return &_VMString_vtable;
}

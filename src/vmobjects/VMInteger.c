/*
 * $Id: VMInteger.c 249 2008-04-28 08:17:23Z michael.haupt $
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

#include "VMInteger.h"

#include <memory/gc.h>


//
//  Class Methods (Starting with VMInteger_) 
//


/** 
 * Create a new VMInteger
 */
pVMInteger VMInteger_new(void) {
    pVMInteger result = (pVMInteger)gc_allocate_object(sizeof(VMInteger));
    if(result) {
        result->_vtable = VMInteger_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, (int64_t) 0);
        gc_end_uninterruptable_allocation();
    }
    return result;
}

/** 
 * Create a new VMInteger with initial value
 */
pVMInteger VMInteger_new_with(const int64_t integer) {
    pVMInteger result = (pVMInteger)gc_allocate_object(sizeof(VMInteger));
    if(result) {
        result->_vtable = VMInteger_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, integer);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/**
 * Initialize a VMInteger
 */
void _VMInteger_init(void* _self, ...) {
    pVMInteger self = (pVMInteger)_self;
    SUPER(VMObject, self, init, (intptr_t) 0);
    
    va_list args; va_start(args, _self);
    self->embedded_integer = va_arg(args, int64_t);
    self->hash = self->embedded_integer;
    va_end(args);
}

//
//  Instance Methods (Starting with _VMInteger_) 
//
//
int64_t _VMInteger_get_embedded_integer(void* _self) {
    pVMInteger self = (pVMInteger)_self;
    return self->embedded_integer;
}


void _VMInteger_set_embedded_integer(void* _self, const int64_t embedded) {
    pVMInteger self = (pVMInteger)_self;
    self->embedded_integer = embedded;
    self->hash = self->embedded_integer;
}


void _VMInteger_mark_references(void* _self) {
    pVMInteger self = (pVMInteger) _self;
    SUPER(VMObject, self, mark_references);
}


//
// The VTABLE-function
//
//
static VTABLE(VMInteger) _VMInteger_vtable;
bool VMInteger_vtable_inited = false;

VTABLE(VMInteger)* VMInteger_vtable(void) {
    if(! VMInteger_vtable_inited) {
        *((VTABLE(VMObject)*)&_VMInteger_vtable) = *VMObject_vtable();
        _VMInteger_vtable.init = METHOD(VMInteger, init);
        _VMInteger_vtable.get_embedded_integer =
            METHOD(VMInteger, get_embedded_integer);
        
        _VMInteger_vtable.mark_references = 
            METHOD(VMInteger, mark_references);

        VMInteger_vtable_inited = true;
    }
    return &_VMInteger_vtable;
}

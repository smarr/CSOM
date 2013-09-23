/*
 * $Id: VMBigInteger.c 210 2008-04-16 12:54:12Z michael.haupt $
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

#include "VMBigInteger.h"

#include <memory/gc.h>


//
//  Class Methods (Starting with VMBigInteger_) 
//

/** 
 *  Create a new VMBigInteger
 * 
 */
pVMBigInteger VMBigInteger_new(void) {
    pVMBigInteger result =
        (pVMBigInteger)gc_allocate_object(sizeof(VMBigInteger));
    if(result) {
        result->_vtable = VMBigInteger_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, 0);
        gc_end_uninterruptable_allocation();
    }
    return result;
}

/** 
 * Create a new VMBigInteger with initial value
 * 
 */
pVMBigInteger VMBigInteger_new_with(const int64_t biginteger) {
    pVMBigInteger result =
        (pVMBigInteger)gc_allocate_object(sizeof(VMBigInteger));
    if(result) {
        result->_vtable = VMBigInteger_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, biginteger);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/**
 *
 * Initialize a VMBigInteger
 *
 */
void _VMBigInteger_init(void* _self, ...) {
    pVMBigInteger self = (pVMBigInteger)_self;
    SUPER(VMObject, self, init, 0);
    
    va_list args; va_start(args, _self);
    self->embedded_biginteger = va_arg(args, int64_t);
    va_end(args);
}

//
//  Instance Methods (Starting with _VMBigInteger_) 
//
int64_t _VMBigInteger_get_embedded_biginteger(void* _self) {
    pVMBigInteger self = (pVMBigInteger)_self;
    return self->embedded_biginteger;
}


// embedded biginteger will be duplicated
void _VMBigInteger_set_embedded_biginteger(void* _self,
    const int64_t embedded
) {
    pVMBigInteger self = (pVMBigInteger)_self;
    self->embedded_biginteger = embedded;
}


void _VMBigInteger_mark_references(void* _self) {
    pVMBigInteger self = (pVMBigInteger) _self;
    SUPER(VMObject, self, mark_references);
}


//
// The VTABLE-function
//
//
static VTABLE(VMBigInteger) _VMBigInteger_vtable;
bool VMBigInteger_vtable_inited = false;

VTABLE(VMBigInteger)* VMBigInteger_vtable(void) {
    if(! VMBigInteger_vtable_inited) {
        *((VTABLE(VMObject)*)&_VMBigInteger_vtable) = *VMObject_vtable();
        _VMBigInteger_vtable.init = METHOD(VMBigInteger, init);        
        
        _VMBigInteger_vtable.get_embedded_biginteger =
            METHOD(VMBigInteger, get_embedded_biginteger);
        _VMBigInteger_vtable.set_embedded_biginteger =
            METHOD(VMBigInteger, set_embedded_biginteger);
			
		_VMBigInteger_vtable.mark_references = 
            METHOD(VMBigInteger, mark_references);
        
        VMBigInteger_vtable_inited = true;
    }
    return &_VMBigInteger_vtable;
}

/*
 * $Id: VMDouble.c 210 2008-04-16 12:54:12Z michael.haupt $
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

#include "VMDouble.h"

#include <memory/gc.h>

//
//  Class Methods (Starting with VMDouble_) 
//
//

/** 
 * Create a new VMDouble
 */
pVMDouble VMDouble_new(void) {
    pVMDouble result = (pVMDouble)gc_allocate_object(sizeof(VMDouble));
    if(result) {
        result->_vtable = VMDouble_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, (double) 0);
        gc_end_uninterruptable_allocation();
    }
    return result;
}

/** 
 * Create a new VMDouble with an initial value
 */
pVMDouble VMDouble_new_with(const double dble) {
    pVMDouble result = (pVMDouble)gc_allocate_object(sizeof(VMDouble));
    if(result) {
        result->_vtable = VMDouble_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, dble);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/**
 * Initialize a VMDouble
 */
void _VMDouble_init(void* _self, ...) {
    pVMDouble self = (pVMDouble)_self;
    
    SUPER(VMObject, self, init, (intptr_t) 0);
    
    va_list args; va_start(args, _self);
    self->embedded_double = va_arg(args, double);
    va_end(args);
}

//
//  Instance Methods (Starting with _VMDouble_) 
//
//
double _VMDouble_get_embedded_double(void* _self) {
    pVMDouble self = (pVMDouble)_self;
    return self->embedded_double;
}

void _VMDouble_mark_references(void* _self) {
    pVMDouble self = (pVMDouble) _self;
    SUPER(VMObject, self, mark_references);
}


//
// The VTABLE-function
//
//
static VTABLE(VMDouble) _VMDouble_vtable;
bool VMDouble_vtable_inited = false;

VTABLE(VMDouble)* VMDouble_vtable(void) {
    if(! VMDouble_vtable_inited) {
        *((VTABLE(VMObject)*)&_VMDouble_vtable) = *VMObject_vtable();
        _VMDouble_vtable.init                = METHOD(VMDouble, init);
        _VMDouble_vtable.get_embedded_double =
            METHOD(VMDouble, get_embedded_double);
        
        _VMDouble_vtable.mark_references = 
            METHOD(VMDouble, mark_references);
        
        VMDouble_vtable_inited = true;
    }
    return &_VMDouble_vtable;
}

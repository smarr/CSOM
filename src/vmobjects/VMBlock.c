/*
 * $Id: VMBlock.c 227 2008-04-21 15:21:14Z michael.haupt $
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
 
#include "VMBlock.h"
#include "VMEvaluationPrimitive.h"

#include <memory/gc.h>

#include <misc/debug.h>


//
//  Class Methods (Starting with VMBlock_) 
//


/**
 * Create a new VMBlock
 */
pVMBlock VMBlock_new(pVMMethod method, pVMFrame context) {
    pVMBlock result = (pVMBlock)gc_allocate_object(sizeof(VMBlock));
    if(result) {
        result->_vtable = VMBlock_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, method, context);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


pVMPrimitive VMBlock_get_evaluation_primitive(int64_t number_of_arguments) {
    return (pVMPrimitive)VMEvaluationPrimitive_new(number_of_arguments);
}


/**
 * Initialize a VMBlock
 */
void _VMBlock_init(void* _self, ...) {
    pVMBlock self = (pVMBlock)_self;
    SUPER(VMObject, self, init, SIZE_DIFF_VMOBJECT(VMBlock));
    
    va_list args;
    va_start(args, _self);
    self->method  = va_arg(args,pVMMethod);
    self->context = va_arg(args,pVMFrame);
    
    va_end(args);
}


//
//  Instance Methods (Starting with _VMArray_) 
//


pVMMethod _VMBlock_get_method(void* _self) {
    pVMBlock self = (pVMBlock)_self;
    return self->method;
}


pVMFrame _VMBlock_get_context(void* _self) {
    pVMBlock self = (pVMBlock)_self;
    // Get the context of this block
    return self->context;
}


void _VMBlock_mark_references(void* _self) {
    pVMBlock self = (pVMBlock) _self;
    gc_mark_object(self->method);
    gc_mark_object(self->context);
    SUPER(VMObject, self, mark_references);
}


//
// The VTABLE-function
//


static VTABLE(VMBlock) _VMBlock_vtable;
bool VMBlock_vtable_inited = false;


VTABLE(VMBlock)* VMBlock_vtable(void) {
    if(! VMBlock_vtable_inited) {
        *((VTABLE(VMObject)*)&_VMBlock_vtable) = *VMObject_vtable();
        _VMBlock_vtable.init            = METHOD(VMBlock, init);        
       
        _VMBlock_vtable.get_method  = METHOD(VMBlock, get_method);
        _VMBlock_vtable.get_context = METHOD(VMBlock, get_context);
        
        _VMBlock_vtable.mark_references = 
            METHOD(VMBlock, mark_references);
        
        
        VMBlock_vtable_inited = true;
    }
    return &_VMBlock_vtable;
}

/*
 * $Id: VMFrame.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "VMFrame.h"
#include "VMObject.h"
#include "VMMethod.h"
#include "VMClass.h"
#include "VMSymbol.h"
#include "VMInvokable.h"

#include <memory/gc.h>

#include <vm/Universe.h>

#include <interpreter/bytecodes.h>

#include <stddef.h>
#include <stdio.h>

//
//  Class Methods (Starting with VMFrame_) 
//
//

/**
 * Create a new VMFrame
 */
pVMFrame VMFrame_new(size_t length, pVMMethod method, pVMFrame context, pVMFrame previous_frame) {
    pVMFrame result = (pVMFrame)gc_allocate_object(
        sizeof(VMFrame) + sizeof(pVMObject) * length);
    if(result) {
        result->_vtable = VMFrame_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, length, method, context, previous_frame);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/**
 * Initialize a VMFrame
 */
void _VMFrame_init(void* _self, ...) {
    pVMFrame self = (pVMFrame)_self;
    
    va_list args;
    va_start(args, _self);
    
    SUPER(VMArray, _self, init, va_arg(args, size_t));
    self->method         = va_arg(args, pVMMethod);
    self->context        = va_arg(args, pVMFrame);
    self->previous_frame = va_arg(args, pVMFrame);
    
    va_end(args);
    self->local_offset   = 0;
    self->bytecode_index = 0;
    self->stack_pointer  = 0;
}


//
//  Instance Methods (Starting with _VMFrame_) 
//
//
pVMFrame _VMFrame_get_previous_frame(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    return self->previous_frame;
}


void _VMFrame_clear_previous_frame(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    self->previous_frame = (pVMFrame)nil_object;
}


bool _VMFrame_has_previous_frame(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    return (pVMObject)self->previous_frame != nil_object;
}


bool _VMFrame_is_bootstrap_frame(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    return !SEND(self, has_previous_frame);
}


pVMFrame _VMFrame_get_context(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    return self->context;
}


bool _VMFrame_has_context(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    return (pVMObject)self->context != nil_object;
}


pVMFrame _VMFrame_get_context_level(void* _self, int64_t level) {
    pVMFrame self = (pVMFrame)_self;
    pVMFrame current = self;
  
    // Iterate through the context chain until the given level is reached
    while(level > 0) {
        current = SEND(current, get_context);
        --level;
    }
    return current;
}


pVMFrame _VMFrame_get_outer_context(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    pVMFrame current = self;
    while(SEND(current, has_context))
        current = SEND(current, get_context);
    return current;
}


pVMMethod _VMFrame_get_method(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    return self->method;
}


pVMObject _VMFrame_pop(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    size_t sp = self->stack_pointer;
    self->stack_pointer = sp - 1;
    return SEND(self, get_indexable_field, sp);
}


void _VMFrame_push(void* _self, pVMObject value) {
    pVMFrame self = (pVMFrame)_self;
    size_t sp = self->stack_pointer + 1;
    self->stack_pointer = sp;
    SEND(self, set_indexable_field, sp, value);
}


void _VMFrame_reset_stack_pointer(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    // arguments are stored in front of local variables
    pVMMethod meth = SEND(self, get_method);
    size_t lo = SEND(meth, get_number_of_arguments);
    self->local_offset = lo;
  
    // Set the stack pointer to its initial value thereby clearing the stack
    size_t num_lo = SEND(meth, get_number_of_locals);
    self->stack_pointer = lo + num_lo - 1;
}


size_t _VMFrame_get_bytecode_index(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    return self->bytecode_index;
}


void _VMFrame_set_bytecode_index(void* _self, size_t index) {
    pVMFrame self = (pVMFrame)_self;
    self->bytecode_index = index;
}


pVMObject _VMFrame_get_stack_element(void* _self, size_t index) {
    pVMFrame self = (pVMFrame)_self;
    size_t sp = self->stack_pointer;
    return SEND(self, get_indexable_field, sp - index);
}


void _VMFrame_set_stack_element(void* _self, size_t index, pVMObject value) {
    pVMFrame self = (pVMFrame)_self;
    size_t sp = self->stack_pointer;
    SEND(self, set_indexable_field, sp - index, value);    
}


pVMObject _VMFrame_get_local(void* _self, size_t index, size_t context_level) {
    pVMFrame self = (pVMFrame)_self;
    pVMFrame context = SEND(self, get_context_level, context_level);
    size_t lo = context->local_offset;
    return SEND(context, get_indexable_field, lo + index);
}


void _VMFrame_set_local(void* _self, size_t index, size_t context_level,
    pVMObject value
) {
    pVMFrame self = (pVMFrame)_self;
    pVMFrame context = SEND(self, get_context_level, context_level);
    size_t lo = context->local_offset;
    SEND(context, set_indexable_field, lo + index, value);
}


size_t _VMFrame_argument_stack_index(void* _self, size_t index) {
    pVMFrame self = (pVMFrame)_self;
    pVMMethod meth = SEND(self, get_method);
    return SEND(meth, get_number_of_arguments) - index - 1;
}


pVMObject _VMFrame_get_argument(void* _self, size_t index, size_t context_level) {
    pVMFrame self = (pVMFrame)_self;
    // get the context
    pVMFrame context = SEND(self, get_context_level, context_level);
  
    // get the argument with the given index
    return SEND(context, get_indexable_field, index);
}


void _VMFrame_set_argument(void* _self, size_t index, size_t context_level,
    pVMObject value
) {
    pVMFrame self = (pVMFrame)_self;
    // get the context
    pVMFrame context = SEND(self, get_context_level, context_level);
  
    // set the argument with the given index to the given value
    SEND(context, set_indexable_field, index, value);
}


void _VMFrame_copy_arguments_from(void* _self, pVMFrame frame) {
    pVMFrame self = (pVMFrame)_self;
    // copy arguments from frame:
    // - arguments are at the top of the stack of frame.
    // - copy them into the argument area of the current frame
    pVMMethod meth = SEND(self, get_method);
    int64_t num_args = SEND(meth, get_number_of_arguments);
    for(size_t i=0; i < num_args; ++i) {
        pVMObject stackElem = SEND(frame, get_stack_element, num_args - 1 - i);
        SEND(self, set_indexable_field, i, stackElem);
    }
}


void _VMFrame_print_stack_trace(void* _self) {
    pVMFrame self = (pVMFrame)_self;
    
    // retrieve frame and method data
    
    // method
    pVMMethod method = SEND(self, get_method);    
    pVMSymbol methodSym = TSEND(VMInvokable, method, get_signature);
    
    // holding class
    pVMClass holder = TSEND(VMInvokable, method, get_holder);
    pVMSymbol holderSym = SEND(holder, get_name);
    
    // sending bytecode
    size_t bc_idx = self->bytecode_index;
    if(!SEND(self, is_bootstrap_frame)) 
        bc_idx -= 2; // length of SEND / SUPER_SEND
    uint8_t bc = SEND(method, get_bytecode, bc_idx);

    // current selector, if any
    const char* s_sel = "";
    if (bc == BC_SEND || bc == BC_SUPER_SEND) {
        pVMSymbol sel = (pVMSymbol)SEND(method, get_constant, bc_idx);
        s_sel = SEND(sel, get_rawChars);
    }
    
    debug_print("STACKTRACE:%20s>>%-20s%c@ %04d: %s%s\n",
                SEND(holderSym, get_rawChars),
                SEND(methodSym, get_rawChars),
                TSEND(VMInvokable, method, is_primitive) ? '*' : ' ',
                bc_idx,
                bytecodes_get_bytecode_name(bc),
                s_sel);
    
    // traverse contexts, if any
    if (SEND(self, has_previous_frame)) {
        SEND(self->previous_frame, print_stack_trace);
    }
}


size_t _VMFrame__get_offset(void* _self) {
    return (size_t)SIZE_DIFF_VMOBJECT(VMFrame);
}


void _VMFrame_mark_references(void* _self) {
    pVMFrame self = (pVMFrame) _self;
	gc_mark_object(self->previous_frame);
	gc_mark_object(self->context);
    gc_mark_object(self->method);
	SUPER(VMArray, self, mark_references);
}


//
// The VTABLE-function
//
//
static VTABLE(VMFrame) _VMFrame_vtable;
bool VMFrame_vtable_inited = false;

VTABLE(VMFrame)* VMFrame_vtable(void) {
    if(VMFrame_vtable_inited)
        return &_VMFrame_vtable;
       
    *((VTABLE(VMArray)*)&_VMFrame_vtable) = *VMArray_vtable();
    _VMFrame_vtable.init               = METHOD(VMFrame, init);        
    _VMFrame_vtable.get_previous_frame = METHOD(VMFrame, get_previous_frame);
    _VMFrame_vtable.clear_previous_frame = 
        METHOD(VMFrame, clear_previous_frame);
    _VMFrame_vtable.has_previous_frame =
        METHOD(VMFrame, has_previous_frame);
    _VMFrame_vtable.is_bootstrap_frame =
        METHOD(VMFrame, is_bootstrap_frame);
    _VMFrame_vtable.get_context = METHOD(VMFrame, get_context);
    _VMFrame_vtable.has_context = METHOD(VMFrame, has_context);
    _VMFrame_vtable.get_context_level = METHOD(VMFrame, get_context_level);
    _VMFrame_vtable.get_outer_context = METHOD(VMFrame, get_outer_context);
    _VMFrame_vtable.get_method = METHOD(VMFrame, get_method);
    _VMFrame_vtable.pop = METHOD(VMFrame, pop);
    _VMFrame_vtable.push = METHOD(VMFrame, push);
    _VMFrame_vtable.reset_stack_pointer = METHOD(VMFrame, reset_stack_pointer);
    _VMFrame_vtable.get_bytecode_index = METHOD(VMFrame, get_bytecode_index);
    _VMFrame_vtable.set_bytecode_index = METHOD(VMFrame, set_bytecode_index);
    _VMFrame_vtable.get_stack_element = METHOD(VMFrame, get_stack_element);
    _VMFrame_vtable.set_stack_element = METHOD(VMFrame, set_stack_element);
    _VMFrame_vtable.get_local = METHOD(VMFrame, get_local);
    _VMFrame_vtable.set_local = METHOD(VMFrame, set_local);
    _VMFrame_vtable.get_argument = METHOD(VMFrame, get_argument);
    _VMFrame_vtable.set_argument = METHOD(VMFrame, set_argument);
    _VMFrame_vtable.print_stack_trace = METHOD(VMFrame, print_stack_trace);
    _VMFrame_vtable.argument_stack_index = 
        METHOD(VMFrame, argument_stack_index);
    _VMFrame_vtable.copy_arguments_from = METHOD(VMFrame, copy_arguments_from);
    _VMFrame_vtable._get_offset = METHOD(VMFrame, _get_offset);
    
    _VMFrame_vtable.mark_references = 
        METHOD(VMFrame, mark_references);

    VMFrame_vtable_inited = true;

    return &_VMFrame_vtable;
}





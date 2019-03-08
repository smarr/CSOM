/*
 * $Id: VMMethod.c 227 2008-04-21 15:21:14Z michael.haupt $
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
 
#include "VMMethod.h"
#include "VMObject.h"
#include "VMSymbol.h"
#include "Signature.h"
#include "VMInteger.h"
#include "VMInvokable.h"

#include <memory/gc.h>

#include <vm/Universe.h>

#include <interpreter/bytecodes.h>
#include <interpreter/Interpreter.h>

#include <misc/debug.h>

#include <compiler/GenerationContexts.h>

#include <stdbool.h>
#include <stddef.h>


//
//  Class Methods (Starting with VMMethod_) 
//


/**
 * Create a new VMMethod
 */
pVMMethod VMMethod_new(size_t number_of_bytecodes, size_t number_of_constants,
                       size_t number_of_locals,
                       size_t max_number_of_stack_elements,
                       pVMSymbol signature) {
    pVMMethod result = (pVMMethod)gc_allocate_object(
        sizeof(VMMethod) + (sizeof(pVMObject) * number_of_constants) +
        (sizeof(uint8_t) * number_of_bytecodes));
    if(result) {
        result->_vtable = VMMethod_vtable();
		gc_start_uninterruptable_allocation();
        INIT(result, number_of_constants, number_of_bytecodes,
             number_of_locals, max_number_of_stack_elements, signature);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


pVMMethod VMMethod_assemble(method_generation_context* mgenc) {
    // create a method instance with the given number of bytecodes and literals
    size_t num_literals = SEND(mgenc->literals, size);
    size_t num_locals = SEND(mgenc->locals, size);

    pVMMethod meth = Universe_new_method(mgenc->signature, mgenc->bp,
        SEND(mgenc->literals, size), num_locals,
        method_genc_compute_stack_depth(mgenc));

    // copy literals into the method
    for(int i = 0; i < num_literals; i++) {
        pVMObject l = SEND(mgenc->literals, get, i);
        SEND(meth, set_indexable_field, i, l);
    }
    
    // copy bytecodes into method
    for(size_t i = 0; i < mgenc->bp; i++)
        SEND(meth, set_bytecode, i, mgenc->bytecode[i]);
    
    
    // return the method - the holder field is to be set later on!
    return meth;
}


/**
 * Initialize a VMMethod
 */
void _VMMethod_init(void* _self, ...) {
    pVMMethod self = (pVMMethod)_self;

    va_list args;
    va_start(args, _self);
    
    SUPER(VMArray, _self, init, va_arg(args,size_t));
    self->bytecodes_length = va_arg(args,size_t);
    self->number_of_locals = va_arg(args,size_t);
    self->maximum_number_of_stack_elements = va_arg(args,size_t);
    self->signature        = va_arg(args,pVMSymbol);
    
    va_end(args);
    
    self->number_of_arguments = Signature_get_number_of_arguments(self->signature);
}


/**
 *
 * Return the offset of the indexable Fields from "normal" fields
 *
 */
size_t _VMMethod__get_offset(void* _self) {
    return (size_t)SIZE_DIFF_VMOBJECT(VMMethod);
}


//
//  Instance Methods (Starting with _VMMethod_) 
//

int64_t _VMMethod_get_number_of_locals(void* _self) {
    pVMMethod self = (pVMMethod)_self;
    return self->number_of_locals;
}


int64_t _VMMethod_get_maximum_number_of_stack_elements(void* _self) {
    pVMMethod self = (pVMMethod)_self;
    return self->maximum_number_of_stack_elements;
}


void _VMMethod_set_holder_all(void* _self, pVMClass value) {
    pVMMethod self = (pVMMethod)_self;
    // Make sure all nested invokables have the same holder
    for(size_t i = 0; 
        i < SEND(self, get_number_of_indexable_fields);
        i++) {
        pVMObject o = SEND(self, get_indexable_field, i);
        
        if(SUPPORTS(o, VMInvokable))
            TSEND(VMInvokable, o, set_holder, value);
    }
}

pVMObject _VMMethod_get_constant(void* _self, size_t bytecode_index) {
    pVMMethod self = (pVMMethod)_self;
    uint8_t bc = SEND(self, get_bytecode, bytecode_index + 1);
    return SEND((pVMArray)self, get_indexable_field, bc);
}


int64_t _VMMethod_get_number_of_arguments(void* _self) {
    pVMMethod self = (pVMMethod)_self;
    return self->number_of_arguments;
}


int64_t _VMMethod_get_number_of_bytecodes(void* _self) {
    pVMMethod self = (pVMMethod)_self;
    return self->bytecodes_length;
}


uint8_t _VMMethod_get_bytecode(void* _self, size_t index) {
    pVMMethod self = (pVMMethod)_self;
    #ifdef DEBUG
        if(index >= self->bytecodes_length)
            Universe_error_exit("[get] Method Bytecode Index out of range.");
    #endif // DEBUG
    /*
     * Bytecodes start at end of the internal array,
     * thus, at offset + number_of_constants
     */
    return ((uint8_t*)&(self->fields[
        SEND(self, _get_offset) +
        SEND(self, get_number_of_indexable_fields)
    ]))[index];
}


void _VMMethod_set_bytecode(void* _self, size_t index, uint8_t value) {
    pVMMethod self = (pVMMethod)_self;
    #ifdef DEBUG
        if(index >= self->bytecodes_length)
            Universe_error_exit("[set] Method Bytecode Index out of range.");  
    #endif // DEBUG
    /*
     * Bytecodes start at end of the internal array,
     * thus, at offset + number_of_constants
     */        
    ((uint8_t*)&(self->fields[
        SEND(self, _get_offset) +
        SEND(self, get_number_of_indexable_fields)
    ]))[index] = value;
}


void _VMMethod_invoke_method(void* _self, pVMFrame frame) {
    pVMMethod self = (pVMMethod)_self;
    // Allocate and push a new frame on the interpreter stack
    pVMFrame frm = Interpreter_push_new_frame(self, (pVMFrame) nil_object);
    SEND(frm, copy_arguments_from, frame);
}


void _VMMethod_mark_references(void* _self) {
    pVMMethod self = (pVMMethod) _self;
    gc_mark_object(self->signature);
    gc_mark_object(self->holder); 
	SUPER(VMArray, self, mark_references);
}


//
// The VTABLE-function
//


static VTABLE(VMMethod) _VMMethod_vtable;
bool VMMethod_vtable_inited = false;


VTABLE(VMMethod)* VMMethod_vtable(void) {
    if(! VMMethod_vtable_inited) {
        *((VTABLE(VMArray)*)&_VMMethod_vtable) = *VMArray_vtable();
        
        ASSIGN_TRAIT(VMInvokable, VMMethod);

        _VMMethod_vtable.init = METHOD(VMMethod, init);        
        _VMMethod_vtable.get_number_of_locals =
            METHOD(VMMethod, get_number_of_locals);
        _VMMethod_vtable.get_maximum_number_of_stack_elements =
            METHOD(VMMethod, get_maximum_number_of_stack_elements);
        _VMMethod_vtable._get_offset = METHOD(VMMethod, _get_offset);
        _VMMethod_vtable.set_holder_all = METHOD(VMMethod, set_holder_all);
        _VMMethod_vtable.get_constant = METHOD(VMMethod, get_constant);
        _VMMethod_vtable.get_number_of_arguments =
            METHOD(VMMethod, get_number_of_arguments);
        _VMMethod_vtable.get_number_of_bytecodes =
            METHOD(VMMethod, get_number_of_bytecodes);
        _VMMethod_vtable.get_bytecode = METHOD(VMMethod, get_bytecode);
        _VMMethod_vtable.set_bytecode = METHOD(VMMethod, set_bytecode);
        _VMMethod_vtable.invoke_method = METHOD(VMMethod, invoke_method);
        
        _VMMethod_vtable.mark_references = 
            METHOD(VMMethod, mark_references);

        VMMethod_vtable_inited = true;
    }
    return &_VMMethod_vtable;
}

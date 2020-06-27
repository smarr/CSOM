/*
 * $Id: VMPrimitive.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "VMPrimitive.h"
#include "VMInvokable.h"
#include "VMSymbol.h"

#include <memory/gc.h>

#include <misc/debug.h>

#include <vm/Universe.h>

#include <stdbool.h>


void empty_routine(pVMObject prim, pVMFrame);


//
//  Class Methods (Starting with VMPrimitive_) 
//


/**
 * Create a new VMPrimitive
 */
pVMPrimitive VMPrimitive_new(pVMSymbol signature) {
    pVMPrimitive result = (pVMPrimitive)gc_allocate_object(sizeof(VMPrimitive));
    if(result) {
        result->_vtable = VMPrimitive_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, SIZE_DIFF_VMOBJECT(VMPrimitive), signature);
        gc_end_uninterruptable_allocation();
    }
    
    return result;
}


/**
 * Create a new VMPrimitive with an explicit empty-routine
 */
pVMPrimitive VMPrimitive_get_empty_primitive(pVMSymbol signature) {
    debug_info("\t\t'%s'*\n", signature->chars);
    pVMPrimitive prim = VMPrimitive_new(signature);
    prim->empty = true;
    prim->routine = empty_routine;
    
    // Return an empty primitive with the given signature
    return prim;
}


pVMPrimitive VMPrimitive_assemble(method_generation_context* mgenc) {
    return VMPrimitive_get_empty_primitive(mgenc->signature);
}


/** 
 * Initialize a VMPrimitive
 */
void _VMPrimitive_init(void* _self, ...) {
    pVMPrimitive self = (pVMPrimitive)_self;
    va_list args; va_start(args, _self);
   
    // call "super" constructor
    SUPER(VMObject, self, init, va_arg(args, size_t));
    
    //
    // Set the class of this primitive to be the universal primitive class
    // NOTE: this is the only branch of the object hierachy, which 
    //       does this in the init method!
    //
    SEND(self, set_class, primitive_class);
    
    // Set the signature of this primitive
    self->signature = va_arg(args, pVMSymbol);
    va_end(args);
    
    // set routine to NULL
    self->routine = NULL;
    self->empty = false;
}


void _VMPrimitive_free(void* _self) {
    pVMPrimitive self = (pVMPrimitive)_self;
    SUPER(VMObject, self, free);
}


//
//  Instance Primitive (Starting with _VMPrimitive_) 
//


bool _VMPrimitive_is_empty(void* _self) {
    pVMPrimitive self = (pVMPrimitive)_self;
    return self->empty;
}


void _VMPrimitive_set_routine(void* _self, routine_fn routine) {
    pVMPrimitive self = (pVMPrimitive)_self;
    self->routine = routine;
}


void empty_routine(pVMObject self, pVMFrame frame) {
    pVMSymbol sig = TSEND(VMInvokable, self, get_signature);
    debug_warn("undefined primitive %s called", SEND(sig, get_rawChars));
}


void _VMPrimitive_mark_references(void* _self) {
    pVMPrimitive self = (pVMPrimitive) _self;
	gc_mark_object(self->signature);
	gc_mark_object(self->holder);
	SUPER(VMObject, self, mark_references);
}


//
// The VTABLE-function
//


static VTABLE(VMPrimitive) _VMPrimitive_vtable;
bool VMPrimitive_vtable_inited = false;


VTABLE(VMPrimitive)* VMPrimitive_vtable(void) {
    if(! VMPrimitive_vtable_inited) {
        *((VTABLE(VMObject)*)&_VMPrimitive_vtable) = *VMObject_vtable();

        ASSIGN_TRAIT(VMInvokable, VMPrimitive);

        _VMPrimitive_vtable.init        = METHOD(VMPrimitive, init);
        _VMPrimitive_vtable.is_empty    = METHOD(VMPrimitive, is_empty);
        _VMPrimitive_vtable.set_routine = METHOD(VMPrimitive, set_routine);
        _VMPrimitive_vtable.free        = METHOD(VMPrimitive, free);
        
        _VMPrimitive_vtable.mark_references = 
            METHOD(VMPrimitive, mark_references);

        VMPrimitive_vtable_inited = true;
    }
    return &_VMPrimitive_vtable;
}

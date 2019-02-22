/*
 * $Id: VMObject.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "VMObject.h"
#include "VMSymbol.h"
#include "VMInvokable.h"
#include "VMClass.h"

#include <memory/gc.h>

#include <vm/Universe.h>

#include <interpreter/Interpreter.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>


// number of fields helper
static void set_number_of_fields(void* _self, intptr_t value);


//
//  Class Methods (Starting with VMObject_) 
//
//

/**
 * Create a new VMObject
 */

pVMObject VMObject_new(void) {
    return VMObject_new_num_fields(NUMBER_OF_OBJECT_FIELDS);
}

/**
 * Create a new VMObject with a specific number of fields
 */
pVMObject VMObject_new_num_fields(intptr_t number_of_fields) {
    size_t object_stub_size = 
        sizeof(VMObject) - 
        sizeof(pVMObject) * NUMBER_OF_OBJECT_FIELDS;
    pVMObject result = (pVMObject)gc_allocate_object(
        object_stub_size + sizeof(pVMObject) * number_of_fields);
    if(result) {
        result->_vtable = VMObject_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, number_of_fields);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/**
 * Initialize a VMObject
 */
void _VMObject_init(void* _self, ...) {
    pVMObject self = (pVMObject)_self;
    SUPER(OOObject, self, init);
    
    va_list args;
    va_start(args, _self);
    set_number_of_fields(self, va_arg(args, intptr_t));
    va_end(args);
}


void _VMObject_free(void* _self) {
    gc_free(_self);
}


//
//  Instance Methods (Starting with _VMObject_) 
//


pVMObject _VMObject_get_field(void* _self, int64_t index) {
    pVMObject self = (pVMObject)_self;
    // get the field with the given index
    return self->fields[index];
}


void _VMObject_set_field(void* _self, int64_t index, pVMObject value) {
    pVMObject self = (pVMObject)_self;
    // set the field with the given index to the given value
    self->fields[index] = value;
}


pVMClass _VMObject_get_class(void* _self) {
    pVMObject self = (pVMObject)_self;
    // get the class of this object
    return self->class;
}


void _VMObject_set_class(void* _self, pVMClass value) {
    pVMObject self = (pVMObject)_self;
    // set class.
    // this is equivalent to setting the Class Index Field
    self->class = value;
}


pVMSymbol _VMObject_get_field_name(void* _self, int64_t index) {
    pVMObject self = (pVMObject)_self;
    // get the name of the field with the given index
    return SEND(self->class, get_instance_field_name, index);
}


int64_t _VMObject_get_field_index(void* _self, pVMSymbol name) {
    pVMObject self = (pVMObject)_self;
    // get the index for the field with the given name
    return SEND(self->class, lookup_field_index, name);
}


intptr_t _VMObject_get_number_of_fields(void* _self) {
    pVMObject self = (pVMObject)_self;
    // get the number of fields in this object
    return self->num_of_fields;
}


// number of fields helper
static void set_number_of_fields(void* _self, intptr_t value) {
    pVMObject self = (pVMObject)_self;
    self->num_of_fields = value;
    
    // clear each and every field by putting nil into them
    for(int i = 0; i < value; i++) {
        SEND(self, set_field, i, nil_object);
    }
}


void _VMObject_send(void* _self, pVMSymbol selector, 
                    pVMObject* arguments, size_t argc) {
    pVMObject self = (pVMObject)_self;
    // push the receiver onto the stack
    pVMFrame frame = Interpreter_get_frame();
    SEND(frame, push, self);
    
    // push the arguments onto the stack
    for(int i = 0; i < argc; i++) {
          SEND(frame, push, arguments[i]);
    }
    // lookup the invokable
    pVMClass class= SEND(self, get_class);
    pVMObject invokable = SEND(class, lookup_invokable, selector);
    // invoke the invokable
    TSEND(VMInvokable, invokable, invoke, frame);
}


void VMObject_assert(bool value) {
    Universe_assert(value);
}


void _VMObject_mark_references(void* _self) {
    pVMObject self = (pVMObject) _self;
    gc_mark_object(self->class);
    for(int i = 0;i < SEND(self, get_number_of_fields); i++) {
        gc_mark_object(SEND(self, get_field, i));
    }
}


//
// The VTABLE-function
//


static VTABLE(VMObject) _VMObject_vtable;
bool VMObject_vtable_inited = false;


VTABLE(VMObject)* VMObject_vtable(void) {
    if(! VMObject_vtable_inited) {
        *((VTABLE(OOObject)*)&_VMObject_vtable) = *OOObject_vtable();
        _VMObject_vtable.init = METHOD(VMObject, init);        
        _VMObject_vtable.free = METHOD(VMObject, free);        
        _VMObject_vtable.get_class = METHOD(VMObject, get_class);
        _VMObject_vtable.set_class = METHOD(VMObject, set_class);
        _VMObject_vtable.get_field_name = METHOD(VMObject, get_field_name);
        _VMObject_vtable.get_field_index = METHOD(VMObject, get_field_index);
        _VMObject_vtable.get_number_of_fields =
            METHOD(VMObject, get_number_of_fields);
        _VMObject_vtable.send = METHOD(VMObject, send);
        _VMObject_vtable.get_field = METHOD(VMObject, get_field);
        _VMObject_vtable.set_field = METHOD(VMObject, set_field);

        _VMObject_vtable.mark_references = 
            METHOD(VMObject, mark_references);
        
        VMObject_vtable_inited = true;
    }
    return &_VMObject_vtable;
}

/*
 * $Id: VMArray.c 227 2008-04-21 15:21:14Z michael.haupt $
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

#include "VMArray.h"
#include "VMObject.h"

#include <vm/Universe.h>

#include <memory/gc.h>


//
//  Class Methods (Starting with VMArray_) 
//


/** 
 *  Create a new VMArray
 *
 */
pVMArray VMArray_new(size_t size) {
    pVMArray result = (pVMArray)gc_allocate_object(
        sizeof(VMArray) + sizeof(pVMObject) * size);
    if(result) {
        result->_vtable = VMArray_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, size);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/** 
 *
 * Initialize a VMArray
 *
 */
void _VMArray_init(void* _self, ...) {
    pVMArray self = (pVMArray)_self;
    
    va_list args;
    va_start(args, _self);
    size_t fields = SEND(self, _get_offset) + va_arg(args, size_t);
    va_end(args);
    SUPER(VMObject, _self, init, fields);
}


//
// Special instance Method: must be overridden by every subclass
//

/**
 *
 * Return the offset of the indexable Fields from "normal" fields
 *
 */
size_t _VMArray__get_offset(void* _self) {
    return (size_t)SIZE_DIFF_VMOBJECT(VMArray);
}


//
//  Instance Methods (Starting with _VMArray_) 
//


#define CHECK_INDEX \
    if(index >= SEND(self, get_number_of_indexable_fields) || index < 0) { \
        char* s = (char*)internal_allocate(128); \
        if(index >= SEND(self, get_number_of_indexable_fields)) \
            sprintf(s, "index too large in array access: %d", index); \
        else \
            sprintf(s, "index too small in array access: %d", index); \
        Universe_error_exit(s); \
    }


pVMObject _VMArray_get_indexable_field(void* _self, int64_t index) {
    pVMArray self = (pVMArray)_self;
    //CHECK_INDEX;
    if(index >= SEND(self, get_number_of_indexable_fields) || index < 0) {
        char* s = (char*)internal_allocate(128);
        if(index >= SEND(self, get_number_of_indexable_fields))
            sprintf(s, "index too large in array access: %lld", index);
        else
            sprintf(s, "index too small in array access: %lld", index);
        Universe_error_exit(s);
    }
    // get the indexable field with the given index
    return self->fields[index + SEND(self, _get_offset)];
}


void _VMArray_set_indexable_field(void* _self, int64_t index, pVMObject value) {
    pVMArray self = (pVMArray)_self;
    //CHECK_INDEX;
    if(index >= SEND(self, get_number_of_indexable_fields) || index < 0) {
        char* s = (char*)internal_allocate(128);
        if(index >= SEND(self, get_number_of_indexable_fields))
            sprintf(s, "index too large in array access: %lld", index);
        else
            sprintf(s, "index too small in array access: %lld", index);
        Universe_error_exit(s);
    }
    // set the indexable field with the given index to the given value
    self->fields[index + SEND(self, _get_offset)] = value;
}


int64_t _VMArray_get_number_of_indexable_fields(void* _self) {
    pVMArray self = (pVMArray)_self;
    // retrieve the number of indexable fields in this array
    return self->num_of_fields - SEND(self, _get_offset);
}


pVMArray _VMArray_copy_and_extend_with(void* _self, pVMObject value) {
    pVMArray self = (pVMArray)_self;
    size_t fields = SEND(self, get_number_of_indexable_fields);
    // allocate a new array which has one indexable field more than this array
    pVMArray result = Universe_new_array(fields + 1);
    // copy the indexable fields from this array to the new array
    SEND(self, copy_indexable_fields_to, result);
    // insert the given object as the last indexable field in the new array
    SEND(result, set_indexable_field, fields, value);
    // return the new array
    return result;
}


void _VMArray_copy_indexable_fields_to(void* _self, pVMArray destination) {
    pVMArray self = (pVMArray)_self;
    size_t fields = SEND(self, get_number_of_indexable_fields);
    // copy all indexable fields from this array to the destination array
    for(int i = 0; i < fields; i++) {
        pVMObject object = SEND(self, get_indexable_field, i);
        SEND(destination, set_indexable_field, i, object);
    }
}


void _VMArray_mark_references(void* _self) {
    pVMArray self = (pVMArray)_self;
	int64_t size_of_indexable_fields = SEND(self, get_number_of_indexable_fields);
	for(int i=0;i<size_of_indexable_fields;i++) {
		gc_mark_object(SEND(self, get_indexable_field, i));
	}
	SUPER(VMObject, self, mark_references);
}


//
// The VTABLE-function
//
static VTABLE(VMArray) _VMArray_vtable;
bool VMArray_vtable_inited = false;

VTABLE(VMArray)* VMArray_vtable(void) {
    if(! VMArray_vtable_inited) {
        *((VTABLE(VMObject)*)&_VMArray_vtable) = *VMObject_vtable();
        _VMArray_vtable.init = METHOD(VMArray, init);        

        _VMArray_vtable._get_offset =
            METHOD(VMArray, _get_offset);

        _VMArray_vtable.get_indexable_field    =
            METHOD(VMArray, get_indexable_field);
        _VMArray_vtable.set_indexable_field =
            METHOD(VMArray, set_indexable_field);
        _VMArray_vtable.get_number_of_indexable_fields =
            METHOD(VMArray, get_number_of_indexable_fields);
        _VMArray_vtable.copy_and_extend_with =
            METHOD(VMArray, copy_and_extend_with);
        _VMArray_vtable.copy_indexable_fields_to =
            METHOD(VMArray, copy_indexable_fields_to);
     
		_VMArray_vtable.mark_references = 
            METHOD(VMArray, mark_references);   

        VMArray_vtable_inited = true;
    }
    return &_VMArray_vtable;
}

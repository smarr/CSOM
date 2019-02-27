#ifndef VMOBJECT_H_
#define VMOBJECT_H_

/*
 * $Id: VMObject.h 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include <vmobjects/OOObject.h>

#include <misc/String.h>

#include <stdbool.h>

#pragma mark VM Allocation Helper


#pragma mark VTable definition

VTABLE(VMObject) {
#define VMOBJECT_VTABLE_FORMAT \
    OOOBJECT_VTABLE_FORMAT; \
    pVMClass  (*get_class)(void*); \
    void      (*set_class)(void*, pVMClass); \
    pVMSymbol (*get_field_name)(void*, int64_t); \
    int64_t   (*get_field_index)(void*, pVMSymbol); \
    intptr_t  (*get_number_of_fields)(void*); \
    int64_t   (*get_default_number_of_fields)(void*); \
    void      (*send)(void*, pVMSymbol, pVMObject*, size_t); \
    pVMObject (*get_field)(void*, int64_t); \
    void      (*set_field)(void*, int64_t, pVMObject); \
    void      (*mark_references)(void*)
    
    VMOBJECT_VTABLE_FORMAT;
};

#pragma mark class definition

#define VMOBJECT_FORMAT \
    OOOBJECT_FORMAT; \
    intptr_t   num_of_fields;\
    pVMClass   class; \
    pVMObject  fields[0]
    

/*
 * This is the number of fields visible to Smalltalk objects from the Smalltalk
 * side of the world. The sizes of the VMObject and OOObject structs are
 * subtracted. Moreover, 2 is subtracted to reflect the presence of the
 * "num_of_fields" field, and the class field which are not visible.
 */
#define NUMBER_OF_OBJECT_FIELDS (OBJECT_SIZE_DIFF(VMObject,OOObject)-2)


struct _VMObject {
    VTABLE(VMObject)* _vtable;
    VMOBJECT_FORMAT;
};

#pragma mark class methods

pVMObject VMObject_new(void);
pVMObject VMObject_new_num_fields(intptr_t);
void      VMObject_assert(bool);

#pragma mark vtable initialization

VTABLE(VMObject)* VMObject_vtable(void);

#endif // VMOBJECT_H_

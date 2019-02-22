#ifndef VMCLASS_H_
#define VMCLASS_H_

/*
 * $Id: VMClass.h 227 2008-04-21 15:21:14Z michael.haupt $
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
 
#include <vmobjects/VMObject.h>

#include <compiler/GenerationContexts.h>

#include <misc/Hashmap.h>

#include <stdbool.h>


#pragma mark VTable definition


VTABLE(VMClass) {
#define VMCLASS_VTABLE_FORMAT \
    VMOBJECT_VTABLE_FORMAT; \
    pVMClass  (*get_super_class)(void*); \
    void      (*set_super_class)(void*, pVMClass); \
    bool      (*has_super_class)(void*); \
    pVMSymbol (*get_name)(void*); \
    void      (*set_name)(void*, pVMSymbol); \
    pVMArray  (*get_instance_fields)(void*); \
    void      (*set_instance_fields)(void*, pVMArray); \
    pVMArray  (*get_instance_invokables)(void*); \
    void      (*set_instance_invokables)(void*, pVMArray); \
    int64_t   (*get_number_of_instance_invokables)(void*); \
    pVMObject (*get_instance_invokable)(void*, int64_t); \
    void      (*set_instance_invokable)(void*, int64_t, pVMObject); \
    pVMObject (*lookup_invokable)(void*, pVMSymbol); \
    int64_t   (*lookup_field_index)(void*, pVMSymbol); \
    bool      (*add_instance_invokable)(void*, pVMObject); \
    void      (*add_instance_primitive)(void*, pVMPrimitive, bool); \
    pVMSymbol (*get_instance_field_name)(void*, int64_t); \
    int64_t   (*get_number_of_instance_fields)(void*); \
    bool      (*has_primitives)(void*); \
    void      (*load_primitives)(void*,const pString*, size_t)
  
    VMCLASS_VTABLE_FORMAT;
};


#pragma mark class definition


#define CLASS_FORMAT \
    VMOBJECT_FORMAT; \
    pVMClass  super_class; \
    pVMSymbol name; \
    pVMArray  instance_fields; \
    pVMArray  instance_invokables


struct _VMClass {
    VTABLE(VMClass)* _vtable;
    CLASS_FORMAT;
};


#pragma mark class methods


pVMClass VMClass_new(void);
pVMClass VMClass_new_num_fields(intptr_t);

pVMClass VMClass_assemble(class_generation_context*);
void     VMClass_assemble_system_class(class_generation_context*, pVMClass);

void     VMClass_init_primitive_map(void);

#pragma mark vtable initialization


VTABLE(VMClass)* VMClass_vtable(void);


#endif // VMCLASS_H_

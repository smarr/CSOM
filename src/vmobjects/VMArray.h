#ifndef VMARRAY_H_
#define VMARRAY_H_

/*
 * $Id: VMArray.h 125 2007-11-08 21:40:09Z tobias.pape $
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

#pragma mark VTable definition

VTABLE(VMArray) {
#define VMARRAY_VTABLE_FORMAT \
    VMOBJECT_VTABLE_FORMAT; \
    size_t    (*_get_offset)(void*); \
    pVMObject (*get_indexable_field)(void*, int64_t); \
    void      (*set_indexable_field)(void*, int64_t, pVMObject); \
    int64_t   (*get_number_of_indexable_fields)(void*); \
    pVMArray  (*copy_and_extend_with)(void*, pVMObject); \
    void      (*copy_indexable_fields_to)(void*, pVMArray)
    
    VMARRAY_VTABLE_FORMAT;    
};

#pragma mark class definition

#define ARRAY_FORMAT \
    VMOBJECT_FORMAT

struct _VMArray {
    VTABLE(VMArray)* _vtable;       
    ARRAY_FORMAT;
};

#pragma mark class methods

pVMArray VMArray_new(size_t size);


#pragma mark vtable initialization

VTABLE(VMArray)* VMArray_vtable(void);


#endif // ARRAY_H_

#ifndef VMPRIMITIVE_H_
#define VMPRIMITIVE_H_

/*
 * $Id: VMPrimitive.h 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <stdbool.h>


#pragma mark VTable definition


VTABLE(VMPrimitive) {
#define VMPRIMITIVE_VTABLE_FORMAT \
    VMOBJECT_VTABLE_FORMAT; \
    bool (*is_empty)(void*); \
    void (*set_routine)(void*, routine_fn)

    VMPRIMITIVE_VTABLE_FORMAT;
};


#pragma mark class definition


#define PRIMITIVE_FORMAT \
    VMOBJECT_FORMAT; \
    pVMSymbol  signature; \
    pVMClass   holder; \
    routine_fn routine; \
    bool       empty


struct _VMPrimitive {
    VTABLE(VMPrimitive)* _vtable;   
    PRIMITIVE_FORMAT;
};


#pragma mark class methods


pVMPrimitive VMPrimitive_new(pVMSymbol signature);
pVMPrimitive VMPrimitive_get_empty_primitive(pVMSymbol signature);
pVMPrimitive VMPrimitive_assemble(method_generation_context* mgenc);


#pragma mark vtable initialization


VTABLE(VMPrimitive)* VMPrimitive_vtable(void);


#endif // VMPRIMITIVE_H_

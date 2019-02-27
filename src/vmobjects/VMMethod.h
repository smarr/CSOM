#ifndef VMMETHOD_H_
#define VMMETHOD_H_

/*
 * $Id: VMMethod.h 227 2008-04-21 15:21:14Z michael.haupt $
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
 
#include <stdint.h>
#include <stdbool.h>

#include <vmobjects/VMArray.h>

#include <compiler/GenerationContexts.h>


#pragma mark VTable definition


VTABLE(VMMethod) {
#define VMMETHOD_VTABLE_FORMAT \
    VMARRAY_VTABLE_FORMAT; \
    int64_t   (*get_number_of_locals)(void*); \
    int64_t   (*get_maximum_number_of_stack_elements)(void*); \
    void      (*set_holder_all)(void*, pVMClass); \
    pVMObject (*get_constant)(void*, size_t); \
    int64_t   (*get_number_of_arguments)(void*); \
    int64_t   (*get_number_of_bytecodes)(void*); \
    uint8_t   (*get_bytecode)(void*, size_t); \
    void      (*set_bytecode)(void*, size_t, uint8_t); \
    void      (*invoke_method)(void*, pVMFrame)

    VMMETHOD_VTABLE_FORMAT;
};


#pragma mark class definition


#define METHOD_FORMAT \
    ARRAY_FORMAT; \
    pVMSymbol  signature; \
    pVMClass   holder; \
    size_t     number_of_locals; \
    size_t     maximum_number_of_stack_elements; \
    size_t     bytecodes_length; \
    size_t     number_of_arguments

struct _VMMethod {
    VTABLE(VMMethod)* _vtable;
    METHOD_FORMAT;
};


#pragma mark class methods


pVMMethod VMMethod_new(size_t number_of_constants, size_t number_of_bytecodes,
                       size_t number_of_locals,
                       size_t max_number_of_stack_elements,
                       pVMSymbol signature);
pVMMethod VMMethod_assemble(method_generation_context* mgenc);


#pragma mark vtable initialization


VTABLE(VMMethod)* VMMethod_vtable(void);


#endif // VMMETHOD_H_

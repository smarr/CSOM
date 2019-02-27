#ifndef VMBLOCK_H_
#define VMBLOCK_H_

/*
 * $Id: VMBlock.h 227 2008-04-21 15:21:14Z michael.haupt $
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
#include <vmobjects/VMMethod.h>
#include <vmobjects/VMFrame.h>


#pragma mark VTable definition


VTABLE(VMBlock) {
#define VMBLOCK_VTABLE_FORMAT \
    VMOBJECT_VTABLE_FORMAT; \
    pVMMethod (*get_method)(void*); \
    pVMFrame  (*get_context)(void*)
    
    VMBLOCK_VTABLE_FORMAT;
};


#pragma mark class definition


#define VMBLOCK_FORMAT \
    VMOBJECT_FORMAT; \
    pVMMethod method; \
    pVMFrame  context


struct _VMBlock {
    VTABLE(VMBlock)* _vtable;    
    VMBLOCK_FORMAT;
};


#pragma mark class methods


pVMBlock VMBlock_new(pVMMethod method, pVMFrame context);
pVMPrimitive VMBlock_get_evaluation_primitive(int64_t number_of_arguments);


#pragma mark vtable initialization


VTABLE(VMBlock)* VMBlock_vtable(void);


#endif // VMBLOCK_H_

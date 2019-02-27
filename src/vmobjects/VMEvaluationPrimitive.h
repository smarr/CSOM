#ifndef VMEVALUATIONPRIMITIVE_H_
#define VMEVALUATIONPRIMITIVE_H_

/*
 * $Id: VMEvaluationPrimitive.h 227 2008-04-21 15:21:14Z michael.haupt $
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

#include <vmobjects/VMPrimitive.h>

#pragma mark VTable definition

VTABLE(VMEvaluationPrimitive) {
    
#define VMEVALUATIONPRIMITIVE_VTABLE_FORMAT \
    VMPRIMITIVE_VTABLE_FORMAT
    
    VMEVALUATIONPRIMITIVE_VTABLE_FORMAT;
};
    
#pragma mark class definition

#define EVALUATION_PRIMITIVE_FORMAT \
    PRIMITIVE_FORMAT; \
    pVMInteger number_of_arguments


struct _VMEvaluationPrimitive {
    VTABLE(VMEvaluationPrimitive)* _vtable;
    EVALUATION_PRIMITIVE_FORMAT;
};

#pragma mark class methods

pVMEvaluationPrimitive VMEvaluationPrimitive_new(int64_t argc);

#pragma mark vtable initialization

VTABLE(VMEvaluationPrimitive)* VMEvaluationPrimitive_vtable(void);


#endif // VMEVALUATIONPRIMITIVE_H_

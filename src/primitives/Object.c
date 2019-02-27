/*
 * $Id: Object.c 176 2008-03-19 10:21:59Z michael.haupt $
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
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMClass.h>
#include <vmobjects/VMInvokable.h>

#include <vm/Universe.h>


#include "Object.h"


void  _Object_equalequal(pVMObject object, pVMFrame frame) {
    pVMObject op1 = SEND(frame, pop);
    pVMObject op2 = SEND(frame, pop);
    
    SEND(frame, push, op1 == op2 ? true_object : false_object);
}


void  _Object_objectSize(pVMObject object, pVMFrame frame) {
    pVMObject self = SEND(frame, pop);
    SEND(frame, push, (pVMObject)Universe_new_integer(self->object_size));
}


void  _Object_hashcode(pVMObject object, pVMFrame frame) {
    pVMObject self = SEND(frame, pop);
    SEND(frame, push, (pVMObject)Universe_new_integer(self->hash));
}

void  _Object_inspect(pVMObject object, pVMFrame frame) {
    // NOT SUPPORTED
    SEND(frame, pop);
    SEND(frame, push, false_object);
}

void  _Object_halt(pVMObject object, pVMFrame frame) {
    // NOT SUPPORTED
    SEND(frame, pop);
    SEND(frame, push, false_object);
}

void  _Object_perform_(pVMObject object, pVMFrame frame) {
    pVMSymbol selector = (pVMSymbol)SEND(frame, pop);
    pVMObject self = SEND(frame, get_stack_element, 0);
    
    pVMClass class = SEND(self, get_class);
    pVMObject invokable = SEND(class, lookup_invokable, selector);
    
    TSEND(VMInvokable, invokable, invoke, frame);
}

void  _Object_perform_inSuperclass_(pVMObject object, pVMFrame frame) {
    pVMClass  class    = (pVMClass) SEND(frame, pop);
    pVMSymbol selector = (pVMSymbol)SEND(frame, pop);
    
    pVMObject invokable = SEND(class, lookup_invokable, selector);
    
    TSEND(VMInvokable, invokable, invoke, frame);
}

void  _Object_perform_withArguments_(pVMObject object, pVMFrame frame) {
    pVMArray  args     = (pVMArray) SEND(frame, pop);
    pVMSymbol selector = (pVMSymbol)SEND(frame, pop);
    pVMObject self = SEND(frame, get_stack_element, 0);
    
    size_t num_args = SEND(args, get_number_of_indexable_fields);
    for (size_t i = 0; i < num_args; i++) {
        pVMObject arg = SEND(args, get_indexable_field, i);
        SEND(frame, push, arg);
    }
    
    pVMClass  class = SEND(self, get_class);
    pVMObject invokable = SEND(class, lookup_invokable, selector);
    
    TSEND(VMInvokable, invokable, invoke, frame);
}

void  _Object_perform_withArguments_inSuperclass_(pVMObject object, pVMFrame frame) {
    pVMClass  class    = (pVMClass) SEND(frame, pop);
    pVMArray  args     = (pVMArray) SEND(frame, pop);
    pVMSymbol selector = (pVMSymbol)SEND(frame, pop);
    
    size_t num_args = SEND(args, get_number_of_indexable_fields);
    for (size_t i = 0; i < num_args; i++) {
        pVMObject arg = SEND(args, get_indexable_field, i);
        SEND(frame, push, arg);
    }
    
    pVMObject invokable = SEND(class, lookup_invokable, selector);
    
    TSEND(VMInvokable, invokable, invoke, frame);
}

void _Object_instVarAt_(pVMObject object, pVMFrame frame) {
    pVMInteger idx = (pVMInteger) SEND(frame, pop);
    pVMObject  self = SEND(frame, pop);
    
    int64_t field_idx = SEND(idx, get_embedded_integer) - 1;
    pVMObject value   = SEND(self, get_field, field_idx);
    
    SEND(frame, push, value);
}

void _Object_instVarAt_put_(pVMObject object, pVMFrame frame) {
    pVMObject  value = SEND(frame, pop);
    pVMInteger idx   = (pVMInteger) SEND(frame, pop);
    pVMObject  self  = SEND(frame, get_stack_element, 0);
    
    int64_t field_idx = SEND(idx, get_embedded_integer) - 1;

    
    SEND(self, set_field, field_idx, value);
}

void _Object_instVarNamed_(pVMObject object, pVMFrame frame) {
    pVMSymbol name = (pVMSymbol) SEND(frame, pop);
    pVMObject self = SEND(frame, pop);
    
    int64_t field_idx = SEND(self, get_field_index, name);
    pVMObject value   = SEND(self, get_field, field_idx);
    
    SEND(frame, push, value);
}

void  _Object_class(pVMObject object, pVMFrame frame) {
    pVMObject self = SEND(frame, pop);
    
    pVMClass cls = SEND(self, get_class);
    SEND(frame, push, (pVMObject) cls);
}

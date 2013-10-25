/*
 * $Id: VMInvokable.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "VMInvokable.h"
#include "VMMethod.h"
#include "VMPrimitive.h"
#include "VMEvaluationPrimitive.h"

#include "Signature.h"

#include <vm/Universe.h>


void _VMInvokable_invoke(void* _self, pVMFrame fra) {
    pVMInvokable self = (pVMInvokable)_self;
    // dispatch after actual type (primitive or method)
    if(TSEND(VMInvokable, self, is_primitive))
        // no SEND, due to routine is actually data.
        ((pVMPrimitive)self)->routine((pVMObject)self, fra);
    else
        SEND((pVMMethod)self, invoke_method, fra);
}


bool _VMInvokable_is_primitive(void* _self) {
    pVMInvokable self = (pVMInvokable)_self;
    return 
        (IS_A(self, VMPrimitive) || IS_A(self, VMEvaluationPrimitive));
}


pVMSymbol _VMInvokable_get_signature(void* _self) {
    return ((pVMInvokable)_self)->signature;
}


pVMClass _VMInvokable_get_holder(void* _self) {
    return ((pVMInvokable)_self)->holder;
}


void _VMInvokable_set_holder(void* _self, pVMClass cls) {
    ((pVMInvokable)_self)->holder = cls;
    if(!TSEND(VMInvokable, (pVMInvokable)_self, is_primitive))
        // if method, change holder subsequently
        SEND((pVMMethod)_self, set_holder_all, cls);
}


void _VMInvokable_mark_references(void* _self) {
    pVMInvokable self = (pVMInvokable) _self;
    SUPER(VMObject, self, mark_references);
}

//
// The VTABLE-function
//
//
static VTABLE(VMInvokable) _VMInvokable_vtable;
bool VMInvokable_vtable_inited = false;

VTABLE(VMInvokable)* VMInvokable_vtable(void) {
    if(!VMInvokable_vtable_inited) {
        /* per default, a trait does not have any trait */
        _VMInvokable_vtable._ttable       = NULL; 
        _VMInvokable_vtable.invoke        = METHOD(VMInvokable, invoke);
        _VMInvokable_vtable.is_primitive  = METHOD(VMInvokable, is_primitive);
        _VMInvokable_vtable.get_signature = METHOD(VMInvokable, get_signature);
        _VMInvokable_vtable.get_holder    = METHOD(VMInvokable, get_holder);
        _VMInvokable_vtable.set_holder    = METHOD(VMInvokable, set_holder);
        
        _VMInvokable_vtable.mark_references = 
            METHOD(VMInvokable, mark_references);
			
        VMInvokable_vtable_inited = true;
    }
    return &_VMInvokable_vtable;
}

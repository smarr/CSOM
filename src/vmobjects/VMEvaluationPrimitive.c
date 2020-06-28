/*
 * $Id: VMEvaluationPrimitive.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "VMEvaluationPrimitive.h"
#include "VMInvokable.h"
#include "VMBlock.h"
#include "VMFrame.h"

#include <memory/gc.h>

#include <interpreter/Interpreter.h>

#include <vm/Universe.h>

#include <stdbool.h>

#include <misc/debug.h>

//forward decl
void routine(pVMObject object, pVMFrame frame);
pVMSymbol compute_signature_string(int32_t argc);

//
//  Class Primitives (Starting with VMEvaluationPrimitive_) 
//
//

/** 
 * Create a new VMEvaluationPrimitive
 */
pVMEvaluationPrimitive VMEvaluationPrimitive_new(int64_t argc) {
    pVMEvaluationPrimitive result = (pVMEvaluationPrimitive)gc_allocate_object(
        sizeof(VMEvaluationPrimitive));
    if(result) {
        result->_vtable = VMEvaluationPrimitive_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, SIZE_DIFF_VMOBJECT(VMEvaluationPrimitive), argc);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/** 
 * Initialize a VMEvaluationPrimitive
 */
void _VMEvaluationPrimitive_init(void* _self, ...) {
    pVMEvaluationPrimitive self = (pVMEvaluationPrimitive)_self;

    // Set the signature of this primitive
    va_list args; va_start(args, _self);
        size_t fieldc = va_arg(args, size_t);
        int64_t argc = va_arg(args, int64_t);
    va_end(args);
    
    SUPER(VMPrimitive, self, init, fieldc, compute_signature_string((int32_t) argc));

    // set routine to the Evaluation routine
    SEND((pVMPrimitive)_self, set_routine, routine);
    
    self->empty = false;
    self->number_of_arguments = Universe_new_integer(argc);
}


void _VMEvaluationPrimitive_free(void* _self) {
    pVMEvaluationPrimitive self = (pVMEvaluationPrimitive)_self;
    if(self->number_of_arguments)
        SEND(self->number_of_arguments, free);
}


pVMSymbol compute_signature_string(int32_t argc) {
#define VALUE_S "value"
#define VALUE_LEN 5
#define WITH_S    "with:"
#define WITH_LEN (4+1)
#define COLON_S ":"
    
    char signature_string[1024];
    signature_string[0] = '\0';
    
    // Compute the signature string
    if (argc == 1) {
        strcat(signature_string, VALUE_S);
    } else {
        strcat(signature_string, VALUE_S COLON_S);
        --argc;
        while(--argc) 
            // Add extra value: selector elements if necessary
            strcat(signature_string, WITH_S);
    }

    // Return the signature string
    return Universe_symbol_for_cstr(signature_string);
}


// helper function: the evaluation routine for the primitive
void  routine(pVMObject object, pVMFrame frame) {
    pVMEvaluationPrimitive self = (pVMEvaluationPrimitive)object;
    // Get the block (the receiver) from the stack
    int64_t num_args = SEND(self->number_of_arguments, get_embedded_integer);
    pVMBlock block = (pVMBlock)SEND(frame, get_stack_element, num_args - 1);
    
    // Get the context of the block...
    pVMFrame context = SEND(block, get_context);
    
    // Push a new frame and set its context to be the one specified in the block
    pVMFrame new_frame = Interpreter_push_new_frame(SEND(block, get_method), context);
    SEND(new_frame, copy_arguments_from, frame);
}


void _VMEvaluationPrimitive_mark_references(void* _self) {
    pVMEvaluationPrimitive self = (pVMEvaluationPrimitive) _self;
    gc_mark_object(self->number_of_arguments);
	SUPER(VMPrimitive, self, mark_references);
}


//
// The VTABLE-function
//
//
static VTABLE(VMEvaluationPrimitive) _VMEvaluationPrimitive_vtable;
bool VMEvaluationPrimitive_vtable_inited = false;

VTABLE(VMEvaluationPrimitive)* VMEvaluationPrimitive_vtable(void) {
    if(! VMEvaluationPrimitive_vtable_inited) {
        *((VTABLE(VMPrimitive)*)&_VMEvaluationPrimitive_vtable) =
            (*VMPrimitive_vtable());
        _VMEvaluationPrimitive_vtable.init =
            METHOD(VMEvaluationPrimitive, init);
        _VMEvaluationPrimitive_vtable.free =
            METHOD(VMEvaluationPrimitive, free);
        
        _VMEvaluationPrimitive_vtable.mark_references = 
            METHOD(VMEvaluationPrimitive, mark_references);
        
        VMEvaluationPrimitive_vtable_inited = true;
    }
    return &_VMEvaluationPrimitive_vtable;
}


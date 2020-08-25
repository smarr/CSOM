/*
 * $Id: GenerationContexts.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "GenerationContexts.h"

#include <misc/debug.h>

#include <vmobjects/Signature.h>
#include <vmobjects/VMArray.h>
#include <vmobjects/VMSymbol.h>

#include <interpreter/bytecodes.h>

#include <vm/Universe.h>

#include <stdlib.h>
#include <stdbool.h>

// for memset, should go to gc
#include <string.h>


void class_genc_init(class_generation_context* cgenc) {
    cgenc->name = cgenc->super_name = NULL;
    cgenc->class_side = false;
    cgenc->instance_fields = List_new();
    cgenc->instance_methods = List_new();
    cgenc->class_fields = List_new();
    cgenc->class_methods = List_new();
}


void class_genc_release(class_generation_context* cgenc) {
    SEND(cgenc->instance_fields, free);
    SEND(cgenc->instance_methods, free);
    SEND(cgenc->class_fields, free);
    SEND(cgenc->class_methods, free);
}


void class_genc_set_instance_fields_of_super(class_generation_context* cgenc, pVMArray fields) {
  int64_t num = SEND(fields, get_number_of_indexable_fields);
  for (int64_t i = 0; i < num; i ++) {
    pVMSymbol field_name = (pVMSymbol)SEND(fields, get_indexable_field, i);
    SEND(cgenc->instance_fields, add, field_name);
  }
}


void class_genc_set_class_fields_of_super(class_generation_context* cgenc, pVMArray fields) {
  int64_t num = SEND(fields, get_number_of_indexable_fields);
  for (int64_t i = 0; i < num; i ++) {
    pVMSymbol field_name = (pVMSymbol)SEND(fields, get_indexable_field, i);
    SEND(cgenc->class_fields, add, field_name);
  }
}


void method_genc_init(method_generation_context* mgenc) {
    mgenc->signature = (pVMSymbol)NULL;
    mgenc->holder_genc = (class_generation_context*)NULL;
    mgenc->outer_genc = (method_generation_context*)NULL;
    mgenc->primitive = false;
    mgenc->block_method = false;
    mgenc->finished = false;
    mgenc->bp = 0;
    memset(mgenc->bytecode, 0, GEN_BC_SIZE);
    mgenc->arguments = List_new();
    mgenc->locals = List_new();
    mgenc->literals = List_new();
}


void method_genc_release(method_generation_context* mgenc) {
    SEND(mgenc->arguments, deep_free);
    SEND(mgenc->locals, deep_free);
    SEND(mgenc->literals, free);
}


int8_t method_genc_find_literal_index(
    method_generation_context* mgenc,
    pVMObject literal
) {
    return (int8_t) SEND(mgenc->literals, indexOf, literal);
}


bool method_genc_find_var(
    method_generation_context* mgenc,
    pString var,
    size_t* index,
    size_t* context,
    bool* is_argument
) {
    // Searching proceeds as follows:
    // 1. try to find var in local variables
    // 2. try to find var in arguments
    // 3. try 1+2 in surrounding context, if any
    // Return true on success, false otherwise.
    
    if((*index = SEND(mgenc->locals, indexOfString, var)) == -1) {
        if((*index = SEND(mgenc->arguments, indexOfString, var)) == -1) {
            if(!mgenc->outer_genc)
                return false;
            else {
                (*context)++;
                return method_genc_find_var(mgenc->outer_genc, var, index,
                    context, is_argument);
            }
        } else
            *is_argument = true;
    }
    
    return true;
}


bool method_genc_find_field(method_generation_context* mgenc,
    pString field
) {
    pList fields = mgenc->holder_genc->class_side ?
        mgenc->holder_genc->class_fields :
        mgenc->holder_genc->instance_fields;
    return SEND(fields, indexOf, Universe_symbol_for_str(field)) != -1;
}


uint8_t method_genc_compute_stack_depth(method_generation_context* mgenc) {
    uint8_t depth = 0;
    uint8_t max_depth = 0;
    int i = 0;
    while(i < mgenc->bp) {
        switch(mgenc->bytecode[i]) {
            case BC_HALT             :          i++;    break;
            case BC_DUP              : depth++; i++;    break;
            case BC_PUSH_LOCAL       :
            case BC_PUSH_ARGUMENT    : depth++; i += 3; break;
            case BC_PUSH_FIELD       :
            case BC_PUSH_BLOCK       :
            case BC_PUSH_CONSTANT    :
            case BC_PUSH_GLOBAL      : depth++; i += 2; break;
            case BC_POP              : depth--; i++;    break;
            case BC_POP_LOCAL        :
            case BC_POP_ARGUMENT     : depth--; i += 3; break;
            case BC_POP_FIELD        : depth--; i += 2; break;
            case BC_SEND             :
            case BC_SUPER_SEND       : {
                // these are special: they need to look at the number of
                // arguments (extractable from the signature)
                pVMSymbol sig =
                    SEND(mgenc->literals, get, mgenc->bytecode[i + 1]);
                depth -= Signature_get_number_of_arguments(sig);
                depth++; // return value
                i += 2;
                break;
            }
            case BC_RETURN_LOCAL     :
            case BC_RETURN_NON_LOCAL :          i++;    break;
            default                  :
                debug_error("Illegal bytecode %d.\n", mgenc->bytecode[i]);
                Universe_exit(1);
        }
        
        if(depth > max_depth)
            max_depth = depth;
    }
    
    return max_depth;
}

bool method_genc_has_bytecodes(method_generation_context* mgenc) {
    return mgenc->bp != 0;
}

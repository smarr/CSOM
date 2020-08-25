#ifndef GENERATIONCONTEXTS_H_
#define GENERATIONCONTEXTS_H_

/*
 * $Id: GenerationContexts.h 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <vmobjects/VMArray.h>
#include <vmobjects/VMSymbol.h>

#include <misc/List.h>

#include <stdbool.h>


typedef struct _class_generation_context {
    pVMSymbol name;
    pVMSymbol super_name;
    bool      class_side;
    pList     instance_fields;
    pList     instance_methods;
    pList     class_fields;
    pList     class_methods;
} class_generation_context;


#define GEN_BC_SIZE 1024

// declare forward
struct _method_generation_context;
typedef struct _method_generation_context method_generation_context;

struct _method_generation_context {
    class_generation_context*  holder_genc;
    method_generation_context* outer_genc;
    bool                       block_method;
    pVMSymbol                  signature;
    pList                      arguments;
    bool                       primitive;
    pList                      locals;
    pList                      literals;
    bool                       finished;
    uint32_t                   bp;
    uint8_t                    bytecode[GEN_BC_SIZE];
};


void class_genc_init(class_generation_context* cgenc);
void class_genc_release(class_generation_context* cgenc);
void class_genc_set_instance_fields_of_super(class_generation_context* cgenc, pVMArray fields);
void class_genc_set_class_fields_of_super(class_generation_context* cgenc, pVMArray fields);

void    method_genc_init(method_generation_context* mgenc);
void    method_genc_release(method_generation_context* mgenc);
int8_t  method_genc_find_literal_index(
    method_generation_context* mgenc,
    pVMObject literal
);
bool    method_genc_find_var(
    method_generation_context* mgenc,
    pString var,
    size_t* index,
    size_t* context,
    bool* is_argument
);
bool    method_genc_find_field(method_generation_context* mgenc, pString field);
uint8_t method_genc_compute_stack_depth(method_generation_context* mgenc);

bool    method_genc_has_bytecodes(method_generation_context* mgenc);

#endif // GENERATIONCONTEXTS_H_

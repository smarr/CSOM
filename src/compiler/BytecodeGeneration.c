/*
 * $Id: BytecodeGeneration.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "BytecodeGeneration.h"
#include "GenerationContexts.h"

#include <interpreter/bytecodes.h>

#include <vm/Universe.h>

#include <memory/gc.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_BC_SIZE(N) \
    if((N) > GEN_BC_SIZE) { \
        char* s = (char*)internal_allocate(1024); \
        sprintf(s, "Method %s>>%s is too long.", \
            mgenc->holder_genc->name->chars, \
            mgenc->signature->chars); \
        Universe_error_exit(s); \
    }

#define EMIT1(BC) \
    CHECK_BC_SIZE(mgenc->bp+1); \
    mgenc->bytecode[mgenc->bp++] = (BC)

#define EMIT2(BC, IDX) \
    CHECK_BC_SIZE(mgenc->bp+2); \
    mgenc->bytecode[mgenc->bp++] = (BC); \
    mgenc->bytecode[mgenc->bp++] = (IDX)

#define EMIT3(BC, IDX, CTX) \
    CHECK_BC_SIZE(mgenc->bp+3); \
    mgenc->bytecode[mgenc->bp++] = (BC); \
    mgenc->bytecode[mgenc->bp++] = (IDX); \
    mgenc->bytecode[mgenc->bp++] = (CTX)


void emit_HALT(method_generation_context* mgenc) {
    EMIT1(BC_HALT);
}


void emit_DUP(method_generation_context* mgenc) {
    EMIT1(BC_DUP);
}


void emit_PUSH_LOCAL(method_generation_context* mgenc, size_t idx, size_t ctx) {
    EMIT3(BC_PUSH_LOCAL, idx, ctx);
}


void emit_PUSH_ARGUMENT(method_generation_context* mgenc, size_t idx, size_t ctx) {
    EMIT3(BC_PUSH_ARGUMENT, idx, ctx);
}


void emit_PUSH_FIELD(method_generation_context* mgenc, pVMSymbol field) {
    EMIT2(BC_PUSH_FIELD, SEND(mgenc->literals, indexOf, field));
}


void emit_PUSH_BLOCK(method_generation_context* mgenc, pVMMethod block) {
    EMIT2(BC_PUSH_BLOCK, SEND(mgenc->literals, indexOf, block));
}


void emit_PUSH_CONSTANT(method_generation_context* mgenc, pVMObject cst) {
    EMIT2(BC_PUSH_CONSTANT, SEND(mgenc->literals, indexOf, cst));
}


void emit_PUSH_CONSTANT_String(
    method_generation_context* mgenc,
    pVMString str
) {
    const char* string = SEND(str, get_rawChars);
    size_t length = SEND(str, get_length);
    EMIT2(BC_PUSH_CONSTANT, SEND(mgenc->literals, indexOfStringLen, string, length));
}


void emit_PUSH_GLOBAL(method_generation_context* mgenc, pVMSymbol global) {
    EMIT2(BC_PUSH_GLOBAL, SEND(mgenc->literals, indexOf, global));
}


void emit_POP(method_generation_context* mgenc) {
    EMIT1(BC_POP);
}


void emit_POP_LOCAL(method_generation_context* mgenc, size_t idx, size_t ctx) {
    EMIT3(BC_POP_LOCAL, idx, ctx);
}


void emit_POP_ARGUMENT(method_generation_context* mgenc, size_t idx, size_t ctx) {
    EMIT3(BC_POP_ARGUMENT, idx, ctx);
}


void emit_POP_FIELD(method_generation_context* mgenc, pVMSymbol field) {
    EMIT2(BC_POP_FIELD, SEND(mgenc->literals, indexOf, field));
}


void emit_SEND(method_generation_context* mgenc, pVMSymbol msg) {
    EMIT2(BC_SEND, method_genc_find_literal_index(mgenc, (pVMObject)msg));
}


void emit_SUPER_SEND(method_generation_context* mgenc, pVMSymbol msg) {
    EMIT2(BC_SUPER_SEND, method_genc_find_literal_index(mgenc, (pVMObject)msg));
}


void emit_RETURN_LOCAL(method_generation_context* mgenc) {
    EMIT1(BC_RETURN_LOCAL);
}


void emit_RETURN_NON_LOCAL(method_generation_context* mgenc) {
    EMIT1(BC_RETURN_NON_LOCAL);
}

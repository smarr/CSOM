#ifndef BYTECODEGENERATION_H_
#define BYTECODEGENERATION_H_

/*
 * $Id: BytecodeGeneration.h 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <misc/String.h>

#include <vmobjects/VMObject.h>
#include <vmobjects/VMString.h>
#include <vmobjects/VMMethod.h>
#include <vmobjects/VMSymbol.h>

#include "GenerationContexts.h"


void emit_HALT(method_generation_context* mgenc);
void emit_DUP(method_generation_context* mgenc);
void emit_PUSH_LOCAL(method_generation_context* mgenc, size_t idx, size_t ctx);
void emit_PUSH_ARGUMENT(method_generation_context* mgenc, size_t idx, size_t ctx);
void emit_PUSH_FIELD(method_generation_context* mgenc, pVMSymbol field);
void emit_PUSH_BLOCK(method_generation_context* mgenc, pVMMethod block);
void emit_PUSH_CONSTANT(method_generation_context* mgenc, pVMObject cst);
void emit_PUSH_CONSTANT_String(method_generation_context* mgenc, pVMString str);
void emit_PUSH_GLOBAL(method_generation_context* mgenc, pVMSymbol global);
void emit_POP(method_generation_context* mgenc);
void emit_POP_LOCAL(method_generation_context* mgenc, size_t idx, size_t ctx);
void emit_POP_ARGUMENT(method_generation_context* mgenc, size_t idx, size_t ctx);
void emit_POP_FIELD(method_generation_context* mgenc, pVMSymbol field);
void emit_SEND(method_generation_context* mgenc, pVMSymbol msg);
void emit_SUPER_SEND(method_generation_context* mgenc, pVMSymbol msg);
void emit_RETURN_LOCAL(method_generation_context* mgenc);
void emit_RETURN_NON_LOCAL(method_generation_context* mgenc);


#endif // BYTECODEGENERATION_H_

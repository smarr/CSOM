/*
 Copyright (c) 2019 Stefan Marr <git@stefan-marr.de>

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
#include <vmobjects/VMMethod.h>
#include <vmobjects/VMSymbol.h>

#include <vm/Universe.h>

#include "Primitive.h"


void  _Primitive_holder(pVMObject object, pVMFrame frame) {
  pVMMethod self = (pVMMethod)SEND(frame, pop);
  SEND(frame, push, (pVMObject)self->holder);
}

void  _Primitive_signature(pVMObject object, pVMFrame frame) {
  pVMMethod self = (pVMMethod)SEND(frame, pop);
  SEND(frame, push, (pVMObject)self->signature);
}

void  _Primitive_invokeOn_with_(pVMObject object, pVMFrame frame) {
  printf("NOT YET IMPLEMENTED _Method_invokeOn_with_ ");
  Universe_exit(1);

  //  // REM: this is a clone with _Primitive::InvokeOn_With_
  //  VMArray* args  = static_cast<VMArray*>(frame->Pop());
  //  vm_oop_t rcvr  = static_cast<vm_oop_t>(frame->Pop());
  //  VMMethod* mthd = static_cast<VMMethod*>(frame->Pop());
  //
  //
  //  frame->Push(rcvr);
  //
  //  size_t num_args = args->GetNumberOfIndexableFields();
  //  for (size_t i = 0; i < num_args; i++) {
  //    vm_oop_t arg = args->GetIndexableField(i);
  //    frame->Push(arg);
  //  }
  //  mthd->Invoke(interp, frame);
}

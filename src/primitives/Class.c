/*
 * $Id: Class.c 116 2007-09-20 13:29:40Z tobias.pape $
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
#include <vmobjects/VMSymbol.h>

#include <vm/Universe.h>

#include "Class.h"


void  _Class_new(pVMObject object, pVMFrame frame) {
    pVMClass self = (pVMClass)SEND(frame, pop);
    SEND(frame, push, Universe_new_instance(self));
}

void  _Class_name(pVMObject object, pVMFrame frame) {
  pVMClass self = (pVMClass)SEND(frame, pop);
  pVMSymbol name = (pVMSymbol)SEND(self, get_name);
  SEND(frame, push, (pVMObject)name);
}

void  _Class_superclass(pVMObject object, pVMFrame frame) {
  pVMClass self = (pVMClass)SEND(frame, pop);
  pVMClass super_class = (pVMClass)SEND(self, get_super_class);
  SEND(frame, push, (pVMObject)super_class);
}

void  _Class_fields(pVMObject object, pVMFrame frame) {
  pVMClass self = (pVMClass)SEND(frame, pop);
  pVMArray fields = (pVMArray)SEND(self, get_instance_fields);
  SEND(frame, push, (pVMObject)fields);
}

void  _Class_methods(pVMObject object, pVMFrame frame) {
  pVMClass self = (pVMClass)SEND(frame, pop);
  pVMArray methods = (pVMArray)SEND(self, get_instance_invokables);
  SEND(frame, push, (pVMObject)methods);
}

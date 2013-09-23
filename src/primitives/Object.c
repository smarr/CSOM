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

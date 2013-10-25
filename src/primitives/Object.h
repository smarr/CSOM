#ifndef CORE_OBJECT_H_
#define CORE_OBJECT_H_

/*
 * $Id: Object.h 176 2008-03-19 10:21:59Z michael.haupt $
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

#include <vmobjects/OOObject.h>

void  _Object_equalequal(pVMObject object, pVMFrame frame);
void  _Object_objectSize(pVMObject object, pVMFrame frame);
void  _Object_hashcode(pVMObject object, pVMFrame frame);
void  _Object_inspect(pVMObject object, pVMFrame frame);
void  _Object_halt(pVMObject object, pVMFrame frame);

void  _Object_perform_(pVMObject object, pVMFrame frame);
void  _Object_perform_withArguments_(pVMObject object, pVMFrame frame);
void  _Object_perform_inSuperclass_(pVMObject object, pVMFrame frame);
void  _Object_perform_withArguments_inSuperclass_(pVMObject object, pVMFrame frame);
void  _Object_instVarAt_(pVMObject object, pVMFrame frame);
void  _Object_instVarAt_put_(pVMObject object, pVMFrame frame);
void  _Object_instVarNamed_(pVMObject object, pVMFrame frame);

void  _Object_class(pVMObject object, pVMFrame frame);

#endif // CORE_OBJECT_H_

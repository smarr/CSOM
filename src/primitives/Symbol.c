/*
 * $Id: Symbol.c 792 2009-04-06 08:07:33Z michael.haupt $
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
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMSymbol.h>

#include <vm/Universe.h>

#include "Symbol.h"


void  _Symbol_asString(pVMObject object, pVMFrame frame) {
    pVMSymbol sym = (pVMSymbol)SEND(frame, pop);
    const char* chars = SEND(sym, get_rawChars);
    const size_t length = SEND(sym, get_length);
  
    SEND(frame, push, (pVMObject) Universe_new_string_string(chars, length));
}


void _Symbol_equal(pVMObject object, pVMFrame frame) {
  pVMObject op1 = SEND(frame, pop);
  pVMSymbol op2 = (pVMSymbol)SEND(frame, pop);

  pVMClass op1_class = SEND(op1, get_class);

  if (op1_class == symbol_class) {
    if ((pVMSymbol) op1 == op2) {
      SEND(frame, push, true_object);
      return;
    }
  }

  SEND(frame, push, false_object);
}

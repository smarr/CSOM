#ifndef CORE_INTEGER_H_
#define CORE_INTEGER_H_

/*
 * $Id: Integer.h 792 2009-04-06 08:07:33Z michael.haupt $
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

void  _Integer_plus(pVMObject object, pVMFrame frame);
void  _Integer_minus(pVMObject object, pVMFrame frame);
void  _Integer_star(pVMObject object, pVMFrame frame);
void  _Integer_slash(pVMObject object, pVMFrame frame);
void  _Integer_slashslash(pVMObject object, pVMFrame frame);
void  _Integer_percent(pVMObject object, pVMFrame frame);
void  _Integer_and(pVMObject object, pVMFrame frame);
void  _Integer_equal(pVMObject object, pVMFrame frame);
void  _Integer_lessthan(pVMObject object, pVMFrame frame);
void  _Integer_asString(pVMObject object, pVMFrame frame);
void  _Integer_sqrt(pVMObject object, pVMFrame frame);
void  _Integer_atRandom(pVMObject object, pVMFrame frame);
void  _Integer_rem_(pVMObject object, pVMFrame frame);
void  _Integer_lessthanlessthan(pVMObject object, pVMFrame frame);
void  _Integer_greaterthangreaterthangreaterthan(pVMObject object, pVMFrame frame);
void  _Integer_bitXor_(pVMObject object, pVMFrame frame);
void  _Integer_as32BitSignedValue(pVMObject object, pVMFrame frame);
void  _Integer_as32BitUnsignedValue(pVMObject object, pVMFrame frame);
void  _Integer_equalequal(pVMObject object, pVMFrame frame);

void Integer_fromString_(pVMObject object, pVMFrame frame);

void __Integer_init(void);

#endif // CORE_INTEGER_H_

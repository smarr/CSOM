#ifndef CORE_DOUBLE_H_
#define CORE_DOUBLE_H_

/*
 * $Id: Double.h 792 2009-04-06 08:07:33Z michael.haupt $
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

void  _Double_plus(pVMObject object, pVMFrame frame);
void  _Double_minus(pVMObject object, pVMFrame frame);
void  _Double_star(pVMObject object, pVMFrame frame);
void  _Double_slashslash(pVMObject object, pVMFrame frame);
void  _Double_percent(pVMObject object, pVMFrame frame);
void  _Double_and(pVMObject object, pVMFrame frame);
void  _Double_bitXor_(pVMObject object, pVMFrame frame);
void  _Double_equal(pVMObject object, pVMFrame frame);
void  _Double_lessthan(pVMObject object, pVMFrame frame);
void  _Double_asString(pVMObject object, pVMFrame frame);
void  _Double_sqrt(pVMObject object, pVMFrame frame);
void  _Double_rount(pVMObject object, pVMFrame frame);
void  _Double_asInteger(pVMObject object, pVMFrame frame);
void  _Double_cos(pVMObject object, pVMFrame frame);
void  _Double_sin(pVMObject object, pVMFrame frame);

void  Double_PositiveInfinity(pVMObject object, pVMFrame frame);

#endif // CORE_DOUBLE_H_

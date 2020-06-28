/*
 * $Id: Double.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <stdio.h>
#include <math.h>

#include <memory/gc.h>

#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMDouble.h>
#include <vmobjects/VMInteger.h>

#include <vm/Universe.h>

#include "Double.h"


/*
 * This function coerces any right-hand parameter to a double, regardless of its
 * true nature. This is to make sure that all Double operations return Doubles.
 */
double coerce_double(pVMObject x) {
    if(IS_A(x, VMDouble))
        return SEND((pVMDouble)x, get_embedded_double);
    else if(IS_A(x, VMInteger))
        return (double)SEND((pVMInteger)x, get_embedded_integer);
    else
        Universe_error_exit("Attempt to apply Double operation to non-number.");
}


/*
 * The following standard functionality is performed in all arithmetic
 * operations: extract the right-hand operand by coercing it to a double, and
 * extract the left-hand operand as an immediate Double. Afterwards, left and
 * right are prepared for the operation.
 */
#define PREPARE_OPERANDS \
    double right = coerce_double(SEND(frame, pop)); \
    pVMDouble leftObj = (pVMDouble)SEND(frame, pop); \
    double left = SEND(leftObj, get_embedded_double)


void  _Double_plus(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    SEND(frame, push, (pVMObject)Universe_new_double(left + right));
}


void  _Double_minus(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    SEND(frame, push, (pVMObject)Universe_new_double(left - right));
}


void  _Double_star(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    SEND(frame, push, (pVMObject)Universe_new_double(left * right));
}


void  _Double_slashslash(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    SEND(frame, push, (pVMObject)Universe_new_double(left / right));
}


void  _Double_percent(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    SEND(frame, push, (pVMObject)Universe_new_double((double)
                                                     ((int64_t)left % 
                                                      (int64_t)right)));
}


void  _Double_and(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    SEND(frame, push, (pVMObject)Universe_new_double((double)
                                                     ((int64_t)left & 
                                                      (int64_t)right)));
}


void  _Double_bitXor_(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    SEND(frame, push, (pVMObject)Universe_new_double((double)
                                                     ((int64_t)left ^
                                                      (int64_t)right)));
}


/*
 * This function implements strict (bit-wise) equality and is therefore
 * inaccurate.
 */
void  _Double_equal(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    if(left == right)
        SEND(frame, push, true_object);
    else
        SEND(frame, push, false_object);
}


void  _Double_lessthan(pVMObject object, pVMFrame frame) {
    PREPARE_OPERANDS;
    if(left < right)
        SEND(frame, push, true_object);
    else
        SEND(frame, push, false_object);
}


void  _Double_asString(pVMObject object, pVMFrame frame) {
    pVMDouble self = (pVMDouble)SEND(frame, pop);

    // temporary storage for the number string
    // use c99 snprintf-goodie
    double dbl = SEND(self, get_embedded_double);
    char* strbuf = (char *)internal_allocate(snprintf(0, 0, "%.18g", dbl) +1);
    sprintf(strbuf, "%.18g", dbl);
    SEND(frame, push, (pVMObject) Universe_new_string_cstr(strbuf));

    internal_free(strbuf);
}


void _Double_sqrt(pVMObject object, pVMFrame frame) {
    pVMDouble self = (pVMDouble)SEND(frame, pop);
    pVMDouble result = Universe_new_double(sqrt(SEND(self, 
                                                     get_embedded_double)));
    SEND(frame, push, (pVMObject) result);
}


void _Double_round(pVMObject object, pVMFrame frame) {
    pVMDouble self = (pVMDouble)SEND(frame, pop);
    int64_t rounded = lround(SEND(self, get_embedded_double));

    SEND(frame, push, (pVMObject)Universe_new_integer((int64_t)rounded));
}


void _Double_asInteger(pVMObject object, pVMFrame frame) {
  pVMDouble self = (pVMDouble)SEND(frame, pop);
  double dbl = SEND(self, get_embedded_double);
#ifdef  __EMSCRIPTEN__
  // TODO: remove this hack, seems like an emscripten limitation
  //       the normal version bails out with an error about values
  //       not being representable
  SEND(frame, push, (pVMObject)Universe_new_integer((int32_t)dbl));
#else
  SEND(frame, push, (pVMObject)Universe_new_integer((int64_t)dbl));
#endif
}


void _Double_cos(pVMObject object, pVMFrame frame) {
  pVMDouble self = (pVMDouble)SEND(frame, pop);
  double result = cos(SEND(self, get_embedded_double));
  SEND(frame, push, (pVMObject)Universe_new_double(result));
}


void _Double_sin(pVMObject object, pVMFrame frame) {
  pVMDouble self = (pVMDouble)SEND(frame, pop);
  double result = sin(SEND(self, get_embedded_double));
  SEND(frame, push, (pVMObject)Universe_new_double(result));
}


void Double_PositiveInfinity(pVMObject object, pVMFrame frame) {
  SEND(frame, pop);
  SEND(frame, push, (pVMObject)Universe_new_double(INFINITY));
}

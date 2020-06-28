/*
 * $Id: Integer.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

#include <memory/gc.h>

#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMDouble.h>
#include <vmobjects/VMInteger.h>
#include <vmobjects/VMString.h>

#include <vm/Universe.h>

#include "Integer.h"


/*
 * This macro performs a coercion check to BigInteger and Double. Depending on
 * the right-hand operand, an Integer operation will have to be resent as a
 * BigInteger or Double operation (those types impose themselves on the result
 * of an Integer operation).
 */
#define CHECK_COERCION(obj,receiver,op) ({ \
    if(IS_A(rightObj, VMDouble)) { \
        _Integer__resendAsDouble( \
            object, (op), (receiver), (pVMDouble)(obj)); \
        return; \
    } \
})
    

//
// private functions for Integer
//


void _Integer__resendAsDouble(pVMObject object, const char* restrict operator,
    pVMInteger left, pVMDouble right
) {
    pVMDouble leftDouble =
        Universe_new_double((double)SEND(left, get_embedded_integer));
    pVMObject operands[] = { (pVMObject)right };
    pVMSymbol op = Universe_symbol_for_cstr(operator);
    SEND((pVMObject)leftDouble, send, op, operands, 1);
    SEND(op, free);
}


void __Integer_init(void) {
    srand((unsigned) time(NULL)) ;
}


//
// arithmetic operations
//


void  _Integer_plus(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "+");

    // Do operation:
    pVMInteger right = (pVMInteger)rightObj;
    
    int64_t result = (int64_t)SEND(left, get_embedded_integer) + 
        (int64_t)SEND(right, get_embedded_integer);
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_minus(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "-");

    // Do operation:
    pVMInteger right = (pVMInteger)rightObj;
    
    int64_t result = (int64_t)SEND(left, get_embedded_integer) - 
                     (int64_t)SEND(right, get_embedded_integer);
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_star(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "*");

    // Do operation:
    pVMInteger right = (pVMInteger)rightObj;
    
    int64_t result = (int64_t)SEND(left, get_embedded_integer) * 
                     (int64_t)SEND(right, get_embedded_integer);
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_slashslash(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "/");

    // Do operation:
    pVMInteger right = (pVMInteger)rightObj;
    
    double result = (double)SEND(left, get_embedded_integer) /
                    (double)SEND(right, get_embedded_integer);
    SEND(frame, push, (pVMObject)Universe_new_double(result));
}


void  _Integer_slash(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "/");

    // Do operation:
    pVMInteger right = (pVMInteger)rightObj;
    
    int64_t result = (int64_t)SEND(left, get_embedded_integer) / 
                     (int64_t)SEND(right, get_embedded_integer);
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_percent(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "%");

    // Do operation:
    pVMInteger right = (pVMInteger)rightObj;
    
    int64_t l = (int64_t)SEND(left, get_embedded_integer);
    int64_t r = (int64_t)SEND(right, get_embedded_integer);

    int64_t result = l % r;
    
    if ((result != 0) && ((result < 0) != (r < 0))) {
        result += r;
    }
    
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_and(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "&");

    // Do operation:
    pVMInteger right = (pVMInteger)rightObj;
    
    int64_t result = (int64_t)SEND(left, get_embedded_integer) & 
                    (int64_t)SEND(right, get_embedded_integer);
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}   


void  _Integer_equal(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "=");

    if(IS_A(rightObj, VMInteger)) {
        // Second operand was Integer:
        pVMInteger right = (pVMInteger)rightObj;
        
        if(SEND(left, get_embedded_integer) 
            == SEND(right, get_embedded_integer))
            SEND(frame, push, true_object);
        else
            SEND(frame, push, false_object);
    } else if(IS_A(rightObj, VMDouble)) {
        // Second operand was Double:
        pVMDouble right = (pVMDouble)rightObj;
        
        if((double)SEND(left, get_embedded_integer) 
            == SEND(right, get_embedded_double))
            SEND(frame, push, true_object);
        else
            SEND(frame, push, false_object);
    }
    else
        SEND(frame, push, false_object);
}


void  _Integer_lessthan(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);
    
    CHECK_COERCION(rightObj, left, "<");

    pVMInteger right = (pVMInteger)rightObj;
    
    if(SEND(left, get_embedded_integer) < SEND(right, get_embedded_integer))
        SEND(frame, push, true_object);
    else
        SEND(frame, push, false_object);
}


void  _Integer_asString(pVMObject object, pVMFrame frame) {
    pVMInteger self = (pVMInteger)SEND(frame, pop);

    // temporary storage for the number string
    // use c99 snprintf-goodie
    int64_t integer = SEND(self,  get_embedded_integer);
    char* strbuf = (char *)internal_allocate(snprintf(0, 0, "%lld", integer) +1);
    sprintf(strbuf, "%lld", integer);

    SEND(frame, push, (pVMObject) Universe_new_string_cstr(strbuf));
    internal_free(strbuf);    
}


void Integer_fromString_(pVMObject object, pVMFrame frame) {
    pVMString self = (pVMString)SEND(frame, pop);
    SEND(frame, pop);
    
    int64_t integer = atol(SEND(self, get_rawChars));
    
    SEND(frame, push, (pVMObject)Universe_new_integer(integer));
}


void  _Integer_sqrt(pVMObject object, pVMFrame frame) {
    pVMInteger self = (pVMInteger)SEND(frame, pop);
    double result = sqrt((double)SEND(self, get_embedded_integer));
    
    if (result == rint(result))
        SEND(frame, push, (pVMObject)Universe_new_integer(result));
    else
        SEND(frame, push, (pVMObject) Universe_new_double(result));
}


void  _Integer_atRandom(pVMObject object, pVMFrame frame) {
    pVMInteger self = (pVMInteger)SEND(frame, pop);
    int32_t result = (SEND(self, get_embedded_integer) * rand())%INT32_MAX;
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_rem_(pVMObject object, pVMFrame frame) {
  pVMObject rightObj = SEND(frame, pop);
  pVMInteger left = (pVMInteger)SEND(frame, pop);

  CHECK_COERCION(rightObj, left, "rem:");

  pVMInteger right = (pVMInteger)rightObj;

  int64_t r = SEND((pVMInteger)right, get_embedded_integer);
  int64_t l = SEND(left, get_embedded_integer);

  int64_t result = l - (l / r) * r;

  SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_lessthanlessthan(pVMObject object, pVMFrame frame) {
  pVMObject rightObj = SEND(frame, pop);
  pVMInteger left = (pVMInteger)SEND(frame, pop);

  CHECK_COERCION(rightObj, left, "<<");

  pVMInteger right = (pVMInteger)rightObj;

  int64_t r = SEND((pVMInteger)right, get_embedded_integer);
  int64_t l = SEND(left, get_embedded_integer);

  int64_t result = l << r;

  SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_greaterthangreaterthangreaterthan(pVMObject object, pVMFrame frame) {
  pVMObject rightObj = SEND(frame, pop);
  pVMInteger left = (pVMInteger)SEND(frame, pop);

  CHECK_COERCION(rightObj, left, ">>>");

  pVMInteger right = (pVMInteger)rightObj;

  int64_t r = SEND((pVMInteger)right, get_embedded_integer);
  int64_t l = SEND(left, get_embedded_integer);

  int64_t result = l >> r;

  SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_bitXor_(pVMObject object, pVMFrame frame) {
  pVMObject rightObj = SEND(frame, pop);
  pVMInteger left = (pVMInteger)SEND(frame, pop);

  CHECK_COERCION(rightObj, left, "bitXor:");

  pVMInteger right = (pVMInteger)rightObj;

  int64_t r = SEND((pVMInteger)right, get_embedded_integer);
  int64_t l = SEND(left, get_embedded_integer);

  int64_t result = l ^ r;

  SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_as32BitSignedValue(pVMObject object, pVMFrame frame) {
    pVMInteger self = (pVMInteger)SEND(frame, pop);
    int32_t result = (int32_t) (SEND(self, get_embedded_integer));
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_as32BitUnsignedValue(pVMObject object, pVMFrame frame) {
    pVMInteger self = (pVMInteger)SEND(frame, pop);
    uint32_t result = (uint32_t) (SEND(self, get_embedded_integer));
    SEND(frame, push, (pVMObject)Universe_new_integer(result));
}


void  _Integer_equalequal(pVMObject object, pVMFrame frame) {
    pVMObject rightObj = SEND(frame, pop);
    pVMInteger left = (pVMInteger)SEND(frame, pop);

    if (IS_A(rightObj, VMInteger)) {
        int64_t l = SEND(left, get_embedded_integer);
        pVMInteger right = (pVMInteger) rightObj;
        int64_t r = SEND(right, get_embedded_integer);
        SEND(frame, push, l == r ? true_object : false_object);
    } else {
        SEND(frame, push, false_object);
    }
}

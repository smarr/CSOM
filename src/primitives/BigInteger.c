/*
 * $Id: BigInteger.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <memory/gc.h>

#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMInteger.h>
#include <vmobjects/VMBigInteger.h>

#include <vm/Universe.h>


#include "BigInteger.h"

#define CHECK_BIGINT(object, result) ({ \
    /* Check second parameter type: */ \
    if(IS_A((object), VMInteger)) { \
        /* Second operand was Integer*/ \
        int32_t i = SEND((pVMInteger)(object), get_embedded_integer); \
        (result) = Universe_new_biginteger((int64_t)i); \
    } else \
        (result) = (pVMBigInteger)(object); \
})

#define PUSH_INT_OR_BIGINT(result) ({ \
    if(labs((result)) > INT32_MAX) \
        SEND(frame, push, (pVMObject)Universe_new_biginteger((result))); \
    else \
        SEND(frame, push, (pVMObject)Universe_new_integer((int32_t)(result))); \
})

void  _BigInteger_plus(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);
    
    // Do operation and perform conversion to Integer if required
    int64_t result =  SEND(left, get_embedded_biginteger) 
                    + SEND(right, get_embedded_biginteger);
    PUSH_INT_OR_BIGINT(result);
}


void  _BigInteger_minus(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);   
    
    // Do operation and perform conversion to Integer if required
    int64_t result =  SEND(left, get_embedded_biginteger) 
                    - SEND(right, get_embedded_biginteger);
    PUSH_INT_OR_BIGINT(result);
}


void  _BigInteger_star(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);
   
    // Do operation and perform conversion to Integer if required
    int64_t result =  SEND(left, get_embedded_biginteger) 
                    * SEND(right, get_embedded_biginteger);
    PUSH_INT_OR_BIGINT(result);
}


void  _BigInteger_slash(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);
    
    // Do operation and perform conversion to Integer if required
    int64_t result =  SEND(left, get_embedded_biginteger) 
                    / SEND(right, get_embedded_biginteger);
    PUSH_INT_OR_BIGINT(result);
}


void  _BigInteger_percent(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);
  
    // Do operation:
    pVMBigInteger b= 
        Universe_new_biginteger(SEND(left, get_embedded_biginteger) % 
                                SEND(right, get_embedded_biginteger));
    SEND(frame, push, (pVMObject) b);
}


void  _BigInteger_and(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);
    
    // Do operation:
    pVMBigInteger b= 
        Universe_new_biginteger(SEND(left, get_embedded_biginteger) & 
                                SEND(right, get_embedded_biginteger));
    SEND(frame, push, (pVMObject) b);
}


void  _BigInteger_equal(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);
    
    // Do operation:
    if(SEND(left, get_embedded_biginteger)  == 
       SEND(right, get_embedded_biginteger))
        SEND(frame, push, true_object);
    else
        SEND(frame, push, false_object);    
}


void  _BigInteger_lessthan(pVMObject object, pVMFrame frame) {
    pVMObject rightObj  = SEND(frame, pop);
    pVMBigInteger right = NULL;
    pVMBigInteger left  = (pVMBigInteger)SEND(frame, pop);
    
    CHECK_BIGINT(rightObj, right);
    
    // Do operation:
    if(SEND(left, get_embedded_biginteger)  < 
        SEND(right, get_embedded_biginteger))
        SEND(frame, push, true_object);
    else
        SEND(frame, push, false_object);    
}


void  _BigInteger_asString(pVMObject object, pVMFrame frame) {
    pVMBigInteger self = (pVMBigInteger)SEND(frame, pop);
    // temporary storage for the number string
    // use c99 snprintf-goodie
    int64_t bigint = SEND(self, get_embedded_biginteger);
    char* strbuf = (char*)internal_allocate(snprintf(0, 0, "%lld", bigint) +1);
    sprintf(strbuf, "%lld", bigint);
    SEND(frame, push, (pVMObject)Universe_new_string(strbuf));
    internal_free(strbuf);
}


void  _BigInteger_sqrt(pVMObject object, pVMFrame frame) {
    pVMBigInteger self = (pVMBigInteger)SEND(frame, pop);
    int64_t i = SEND(self, get_embedded_biginteger);
    SEND(frame, push, (pVMObject)Universe_new_double(sqrt((double)i)));
}

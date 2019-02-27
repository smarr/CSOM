/*
 * $Id: Array.c 223 2008-04-21 11:48:41Z michael.haupt $
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

#include <vmobjects/VMInteger.h>
#include <vmobjects/VMArray.h>
#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>

#include <vm/Universe.h>

#include "Array.h"


void _Array_at_(pVMObject object, pVMFrame frame) {
    pVMInteger index = (pVMInteger)SEND(frame, pop);
    pVMArray self = (pVMArray)SEND(frame, pop);
    int64_t i = SEND(index, get_embedded_integer);
    pVMObject elem = SEND((pVMArray)self, get_indexable_field, i - 1);
    SEND(frame, push, elem);
}


void _Array_at_put_(pVMObject object, pVMFrame frame) {
    pVMObject value = SEND(frame, pop);
    pVMInteger index = (pVMInteger)SEND(frame, pop);
    pVMArray self = (pVMArray)SEND(frame, get_stack_element, 0);
    int64_t i = SEND(index, get_embedded_integer);
    SEND(self, set_indexable_field, i - 1,  value);
}


void _Array_length(pVMObject object, pVMFrame frame) {
    pVMArray self = (pVMArray)SEND(frame, pop);
    pVMInteger new_int= 
        Universe_new_integer(SEND(self, get_number_of_indexable_fields));
    SEND(frame, push, (pVMObject)new_int);
}


void Array_new_(pVMObject object, pVMFrame frame) {
    pVMInteger length = (pVMInteger)SEND(frame, pop);
    pVMClass self __attribute__((unused)) = (pVMClass)SEND(frame, pop);        
    int64_t size = SEND(length, get_embedded_integer);
    SEND(frame, push, (pVMObject) Universe_new_array(size));
}

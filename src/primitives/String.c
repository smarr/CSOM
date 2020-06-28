/*
 * $Id: String.c 792 2009-04-06 08:07:33Z michael.haupt $
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
#include <stdlib.h>

#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMString.h>
#include <vmobjects/VMInteger.h>

#include <vm/Universe.h>

#include <memory/gc.h>

#include "String.h"

#include <string.h>
#include <ctype.h>


void  _String_concatenate_(pVMObject object, pVMFrame frame) {
    pVMString arg = (pVMString)SEND(frame, pop);
    pVMString self = (pVMString)SEND(frame, pop);

    SEND(frame, push, (pVMObject) Universe_new_string_concat(self, arg));
}


void  _String_asSymbol(pVMObject object, pVMFrame frame) {
    pVMString self = (pVMString)SEND(frame, pop);

    const char* chars = SEND(self, get_rawChars);
    size_t length     = SEND(self, get_length);

    SEND(frame, push, (pVMObject) Universe_symbol_for_chars(chars, length));
}


void  _String_hashcode(pVMObject object, pVMFrame frame) {
    pVMString self = (pVMString)SEND(frame, pop);
    if (self->hash == 0) {
        self->hash = string_hash(self->chars, self->length);
    }

    SEND(frame, push, (pVMObject)Universe_new_integer(self->hash));    
}


void  _String_length(pVMObject object, pVMFrame frame) {
    pVMString self = (pVMString)SEND(frame, pop);

    size_t length = SEND(self, get_length);

    SEND(frame, push, (pVMObject) Universe_new_integer(length));
}


void  _String_equal(pVMObject object, pVMFrame frame) {
    pVMObject op1 = SEND(frame, pop);
    pVMString op2 = (pVMString)SEND(frame, pop);

    pVMClass op1_class = SEND(op1, get_class);
    if ((op1_class == string_class) || op1_class == symbol_class) {
        size_t lenOp2 = SEND(op2, get_length);
        size_t lenOp1 = SEND((pVMString) op1, get_length);

        if (lenOp1 == lenOp2 && memcmp(SEND(op2, get_rawChars), SEND((pVMString)op1, get_rawChars), lenOp1) == 0) {
            SEND(frame, push, true_object);
            return;
        }
    }
    SEND(frame, push, false_object);
}


void  _String_primSubstringFrom_to_(pVMObject object, pVMFrame frame) {
    pVMInteger end = (pVMInteger)SEND(frame, pop);
    pVMInteger start = (pVMInteger)SEND(frame, pop);
    
    pVMString self = (pVMString)SEND(frame, pop);

    int64_t s = SEND(start, get_embedded_integer);
    int64_t e = SEND(end, get_embedded_integer);

    const char* string = SEND(self, get_rawChars) + s - 1;

    size_t l = e - s + 1;

    SEND(frame, push, (pVMObject) Universe_new_string_string(string, l));
}


void  _String_isWhiteSpace(pVMObject object, pVMFrame frame) {
    pVMString self = (pVMString)SEND(frame, pop);
    const char* string = SEND(self, get_rawChars);
    size_t length = SEND(self, get_length);

    for (size_t i = 0; i < length; i++) {
        if (!isspace(string[i])) {
            SEND(frame, push, false_object);
            return;
        }
    }

    if (length > 0) {
        SEND(frame, push, true_object);
    } else {
        SEND(frame, push, false_object);
    }
}


void  _String_isLetters(pVMObject object, pVMFrame frame) {
    pVMString self = (pVMString)SEND(frame, pop);
    const char* string = SEND(self, get_rawChars);
    size_t length = SEND(self, get_length);

    for (size_t i = 0; i < length; i++) {
        if (!isalpha(string[i])) {
            SEND(frame, push, false_object);
            return;
        }
    }

    if (length > 0) {
        SEND(frame, push, true_object);
    } else {
        SEND(frame, push, false_object);
    }
}


void  _String_isDigits(pVMObject object, pVMFrame frame) {
    pVMString self = (pVMString)SEND(frame, pop);
    const char* string = SEND(self, get_rawChars);
    size_t length = SEND(self, get_length);

    for (size_t i = 0; i < length; i++) {
        if (!isdigit(string[i])) {
            SEND(frame, push, false_object);
            return;
        }
    }

    if (length > 0) {
        SEND(frame, push, true_object);
    } else {
        SEND(frame, push, false_object);
    }
}

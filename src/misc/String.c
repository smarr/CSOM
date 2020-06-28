/*
 *  $Id: String.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "String.h"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include <misc/debug.h>

#include <memory/gc.h>

#include <vm/Universe.h>


int CString_compare(const char* restrict a, size_t lenA, const char* restrict b, size_t lenB) {
    size_t min_length = lenA < lenB ? lenA : lenB;
    int result = memcmp(a, b, min_length);
    if (result == 0) {
        return (int) lenA - (int) lenB;
    } else {
        return result;
    }
}


//
//  Class Methods (Starting with String_) 
//


/**
 * Allocate a new String
 */
pString String_new(const char* restrict cstring, const size_t length) {
    if(cstring==NULL)
        return NULL;
    
    pString result = (pString)internal_allocate(sizeof(String) + ((1 + length) * sizeof(char)));
    if (result) {
        result->_vtable = String_vtable();
        INIT(result, cstring, length);
    }    
    return result;
}


pString String_new_from(pString restrict string) {
    pString result = (pString)internal_allocate(
        sizeof(String) + ((1 + string->length) * sizeof(char)));
    if (result) {
        result->_vtable = String_vtable();
        INIT(result, string->chars, string->length);
    }    
    return result;    
}


pString String_new_concat(pString strA, pString strB) {
    return String_new_concat_str(strA, strB->chars, strB->length);
}


pString String_new_concat_str(pString strA, const char* restrict strB, size_t lengthB) {
    pString result = (pString) internal_allocate(
        sizeof(String) + ((strA->length + lengthB + 1) * sizeof(char)));
    if (result) {
        result->_vtable = String_vtable();
        // make sure we initialize the object completely
        INIT(result, "", 0);

        // now, do the actual concatination work
        size_t i = 0;
        for (; i < strA->length; i++) {
            result->chars[i] = strA->chars[i];
        }

        size_t j = 0;
        for (; j < lengthB; i++, j++) {
            result->chars[i] = strB[j];
        }
        
        result->length = strA->length + lengthB;
        result->chars[result->length] = '\0';
        result->hash = string_hash(result->chars, result->length);
    }
    return result;
}


/**
 * Set up the String object
 */
void _String_init(void* _self, ...) {
    pString self = (pString)_self;
    SUPER(OOObject, self, init);
    
    va_list args;
    va_start(args, _self);
    const char* restrict cstring = va_arg(args, char*);
    self->length = va_arg(args, size_t);
    va_end(args);

    for (size_t i = 0; i < self->length; i++) {
        self->chars[i] = cstring[i];
    }
    self->chars[self->length] = '\0';

    self->hash = string_hash(self->chars, self->length);
}


//
//  Instance Methods (Starting with _String_) 
//


void _String_free(void* self) {
    pString _self = (pString)self;
    SUPER(OOObject, _self, free);
}


size_t _String_length(void* _self) {
    pString self = (pString)_self;
    return self->length;
}


const char* _String_rawChars(void* _self) {
    pString self = (pString)_self;
    return (const char*)self->chars;
}


int _String_compareTo(void* _self, pString other) {
    pString self = (pString)_self;
    return CString_compare(self->chars, self->length, other->chars, other->length);
}


pString _String_concatChars(void* _self, const char* restrict chars, size_t length) {
    pString self = (pString) _self;
    pString result = String_new_concat_str(self, chars, length);
    return result;
}
/**
 * The following function is private and hence does not adhere to the naming
 * scheme used for methods.
 */

pString stringsep(char **inString, size_t *inLength, const char * restrict delim) {
    char *start = *inString;

    if (*inLength == 0) {
        return NULL; // end of string to tokenize
    }

    for (size_t i = 0; i < *inLength; i++) {
        for (register char* d=(char*)delim; *d; ++d) {
            if ((*inString)[i]==*d) {    // token built
                pString result = String_new(start, i);
                *inString = &(*inString)[i + 1];
                *inLength = *inLength - (i + 1);
                return result;
            }
        }
    }

    pString result = String_new(start, *inLength);
    *inLength = 0;
    return result;
}


pString _String_concat(void* _self, pString other) {
    pString self = (pString)_self;
    return String_new_concat(self, other);
}


intptr_t _String_indexOf(void* _self, pString pattern) {
    pString self = (pString)_self;
    char* pos = strstr(self->chars, pattern->chars);
    if (pos >= self->chars)
        return self->chars - pos;
    else
        //not found
        return -1;
}


intptr_t _String_indexOfChar(void* _self, char pattern) {
    pString self = (pString)_self;
    char* pos = memchr(self->chars, pattern, self->length);
    if (pos >= self->chars) {
        return pos - self->chars;
    } else {
        // not found
        return -1;
    }
}

static void *mem_r_chr(const void *s, int c, size_t n) {
    // macOS is apparently missing this one
    const char *start = s;
    const char *end = &start[n]; //one too much on purpose
    while (--end >= start) { //and this is why
        if (c == *end) {
            return (void *)end;
        }
    }
    return NULL;
}

intptr_t _String_lastIndexOfChar(void* _self, char pattern) {
    pString self = (pString)_self;
    char* pos = mem_r_chr(self->chars, pattern, self->length);
    if (pos >= self->chars)
        return pos - self->chars;
    else
        //not found
        return -1;
}


int _String_charAt(void* _self, size_t position) {
    pString self = (pString)_self;
    if (position <= self->length)
        return self->chars[position];
    else
        return -1;
}


pString _String_substring(void* _self, size_t start, size_t end) {
    pString self = (pString)_self;
    if ((start > self->length) || (end > self->length))
        // boundary excess
        return NULL;

    size_t new_length = end + 1 - start;

    char tmp[new_length + 1];
    memcpy(tmp, self->chars + start, new_length);
    
    return String_new(tmp, new_length);
}


pVMInteger _String_toInteger(void* _self) {
    pString self = (pString)_self;
    return Universe_new_integer((int64_t)strtoll(self->chars, NULL, 10));
}


pVMDouble _String_toDouble(void* _self) {
    pString self = (pString)_self;
    return Universe_new_double(strtod(self->chars, NULL));
}


pString* _String_tokenize(void* _self, size_t* length_of_result /* out param */,
    const char* restrict delimiters
) {
    pString self = (pString)_self;
    size_t selfLength = self->length;
    // count delimiters
    size_t delim_count = 0;
    for (size_t i = 0; i < selfLength; i++) {
        if (strchr(delimiters, self->chars[i])) {
            delim_count++;
        }
    }
    
    char* tempstring = self->chars;

    // resulting array of String pointers
    pString* result = 
        (pString*)internal_allocate(sizeof(pString) * (delim_count + 1));
    
    pString* ptr = result;
    pString part;
    size_t remainingLength = selfLength;
    do {
        part = stringsep(&tempstring, &remainingLength, delimiters);
        if (part != NULL) {
            *ptr = part;
            ptr++;
        }
    } while (part != NULL);

    *length_of_result = delim_count + 1;
    return result;    
}


//
// vtable initialisation
//


static VTABLE(String) _String_vtable;
bool String_vtable_inited = false;


VTABLE(String)* String_vtable(void) {
    if(!String_vtable_inited) {
        *((VTABLE(OOObject)*)&_String_vtable) = *OOObject_vtable();
        _String_vtable.free = METHOD(String, free);
        _String_vtable.init = METHOD(String, init);

        _String_vtable.length          = METHOD(String, length);
        _String_vtable.rawChars        = METHOD(String, rawChars);
        _String_vtable.compareTo       = METHOD(String, compareTo);
        _String_vtable.concat          = METHOD(String, concat);
        _String_vtable.concatChars     = METHOD(String, concatChars);
        _String_vtable.indexOf         = METHOD(String, indexOf);
        _String_vtable.indexOfChar     = METHOD(String, indexOfChar);
        _String_vtable.lastIndexOfChar = METHOD(String, lastIndexOfChar);
        _String_vtable.charAt          = METHOD(String, charAt);
        _String_vtable.substring       = METHOD(String, substring);
        _String_vtable.toInteger       = METHOD(String, toInteger);
        _String_vtable.toDouble        = METHOD(String, toDouble);
        _String_vtable.tokenize        = METHOD(String, tokenize);
        
        String_vtable_inited = true;
    }
    return &_String_vtable;
}

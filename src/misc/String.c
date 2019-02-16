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

#include <misc/debug.h>

#include <memory/gc.h>

#include <vm/Universe.h>

//
//  Class Methods (Starting with String_) 
//


/**
 * Allocate a new String
 */
pString String_new(const char* restrict cstring) {
    if(cstring==NULL)
        return NULL;
    
    pString result = (pString)internal_allocate(sizeof(String));
    if(result) {
        result->_vtable = String_vtable();
        INIT(result, cstring);        
    }    
    return result;
}


pString String_new_from(pString restrict string) {
    pString result = (pString)internal_allocate(sizeof(String));
    if(result) {
        result->_vtable = String_vtable();
        INIT(result, string->chars);        
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
    self->chars = internal_allocate_string(va_arg(args, char*));
    va_end(args);
    self->length = strlen(self->chars);
    self->hash = string_hash(self->chars);
}


//
//  Instance Methods (Starting with _String_) 
//


void _String_free(void* self) {
    pString _self = (pString)self;
    if(_self->chars)
        internal_free(_self->chars);
    SUPER(OOObject, _self, free);
}


size_t _String_length(void* _self) {
    pString self = (pString)_self;
    return self->length;
}


size_t _String_size(void* _self) {
    pString self = (pString)_self;
    return self->length + 1;
}


const char* _String_chars(void* _self) {
    pString self = (pString)_self;
    return (const char*)self->chars;
}


int _String_compareTo(void* _self, pString other) {
    pString self = (pString)_self;
    return strcmp(self->chars, other->chars);    
}


int _String_compareToChars(void* _self, const char* restrict other) {
    pString self = (pString)_self;
    return strcmp(self->chars, other);
}


/**
 * The following 2 functions are private and hence does not adhere to the naming
 * scheme used for methods.
 */
pString concatenate_stub(void* _self, const char* restrict other, size_t len) {
    pString self = (pString)_self;
    size_t new_length = self->length + len;
    
    char* new_chars = internal_allocate(new_length + 1);
    strcpy(new_chars, self->chars);
    strncat(new_chars, other, len);
    
    internal_free(self->chars);
    self->chars = new_chars;
    self->length = new_length;
    self->hash = string_hash(new_chars);
    
    return self;
}


/**
 * stringsep
 * a replacement for strsep, for systems without it
 */
#ifdef NEED_STRSEP
#define strsep stringsep
char *stringsep(char **instring, const char * restrict delim) {
    register char *run, *start = *instring;

    if (!*instring) return 0; // end of string to tokenize
    for (run=start; *run; ++run)
        for (register char* d=(char*)delim; *d; ++d)
            if (*run==*d) {	// token built
                *run='\0';
                *instring=run+1;
                return start;
            }
    *instring=NULL;
    return start;
}
#endif


pString _String_concat(void* _self, pString other) {
    pString self = (pString)_self;
    return concatenate_stub(self, other->chars, other->length);
}


pString _String_concatChars(void* _self, const char* restrict other) {
    pString self = (pString)_self;
    return concatenate_stub(self, other, strlen(other));
}


pString _String_concatChar(void* _self, char other) {
    pString self = (pString)_self;
    return concatenate_stub(self, &other, 1);
}


int _String_indexOf(void* _self, pString pattern) {
    pString self = (pString)_self;
    char* pos = strstr(self->chars, pattern->chars);
    if(pos >= self->chars)
        return self->chars - pos;
    else
        //not found
        return -1;
}


int _String_indexOfChar(void* _self, char pattern) {
    pString self = (pString)_self;
    char* pos = strchr(self->chars, pattern);
    if(pos >= self->chars)
        return pos - self->chars;
    else
        //not found
        return -1;
}


int _String_lastIndexOfChar(void* _self, char pattern) {
    pString self = (pString)_self;
    char* pos = strrchr(self->chars, pattern);
    if(pos >= self->chars)
        return pos - self->chars;
    else
        //not found
        return -1;
}


int _String_charAt(void* _self, size_t position) {
    pString self = (pString)_self;
    if(position <= self->length)
        return self->chars[position];
    else
        return -1;
}


pString _String_substring(void* _self, size_t start, size_t end) {
    pString self = (pString)_self;
    if((start > self->length) || (end > self->length))
        // boundary excess
        return NULL;
    size_t new_length = end + 1 - start;
    char tmp[new_length + 1];
    (strncpy(tmp, self->chars + start, new_length))[new_length] = '\0';
    
    return String_new(tmp);
}


pVMInteger _String_toInteger(void* _self) {
    pString self = (pString)_self;
    return Universe_new_integer((int64_t)strtoll(self->chars, NULL, 10));
}


pVMDouble _String_toDouble(void* _self) {
    pString self = (pString)_self;
    return Universe_new_double(strtod(self->chars, NULL));
}


pString* _String_tokenize(void* _self, size_t* length,
    const char* restrict delimiters
) {
    pString self = (pString)_self;
    // count delimiters
    size_t delim_count = 0;
    for(char* ptr = self->chars; *ptr; ptr++)
        if(strchr(delimiters, *ptr))
            delim_count++;
    
    //temporary string copy
    char* tempstring = internal_allocate_string(self->chars);
    char* tempstring_ptr = tempstring;
    
    //resulting array
    pString* result = 
        (pString*)internal_allocate(sizeof(pString) * (delim_count + 1));
    
    pString* ptr = result;
    char* sepr;
    do {
        sepr = strsep(&tempstring, delimiters);
        if(sepr != NULL)
            *ptr++ = String_new(sepr);
    } while(sepr != NULL);
    
    internal_free(tempstring_ptr);
    *length = delim_count + 1;
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
        _String_vtable.size            = METHOD(String, size);
        _String_vtable.chars           = METHOD(String, chars);
        _String_vtable.compareTo       = METHOD(String, compareTo);
        _String_vtable.compareToChars  = METHOD(String, compareToChars);
        _String_vtable.concat          = METHOD(String, concat);
        _String_vtable.concatChars     = METHOD(String, concatChars);
        _String_vtable.concatChar      = METHOD(String, concatChar);
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

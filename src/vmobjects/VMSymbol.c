/*
 * $Id: VMSymbol.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "VMSymbol.h"

#include <memory/gc.h>

#include <string.h>

//
//  Class Methods (Starting with VMSymbol_) 
//


/**
 * Create a new VMSymbol with an initial C-string
 */
pVMSymbol VMSymbol_new(pString restrict string) {
    pVMSymbol result = (pVMSymbol)gc_allocate_object(
        sizeof(VMSymbol) + sizeof(char) * (string->length + 1));
    if (result) {
        result->_vtable = VMSymbol_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, (char*) string->chars, string->length);
        gc_end_uninterruptable_allocation();
    }
    return result;
}


/**
 * Initialize a VMSymbol
 */
void _VMSymbol_init(void* _self, ...) {
    va_list args;
    va_start(args, _self);
    char* chars = va_arg(args, char*);
    size_t len  = va_arg(args, size_t);

    SUPER(VMString, _self, init, chars, len);

    va_end(args);
}


//
//  Instance Methods (Starting with _VMSymbol_) 
//


const char* _VMSymbol_get_plain_string(void* _self) {
    pVMSymbol self = (pVMSymbol)_self;
    char plain_string[1024];
    plain_string[0] = '\0';
    
    size_t l = self->length;
    for (size_t i = 0; i <= l; i++) {
        switch(self->chars[i]) {
            case '~':
                strcat(plain_string, "tilde");
                break;
            case '&':
                strcat(plain_string, "and");
                break;
            case '|':
                strcat(plain_string, "bar");
                break;
            case '*':
                strcat(plain_string, "star");
                break;
            case '/':
                strcat(plain_string, "slash");
                break;
            case '@':
                strcat(plain_string, "at");
                break;
            case '+':
                strcat(plain_string, "plus");
                break;
            case '-':
                strcat(plain_string, "minus");
                break;
            case '=':
                strcat(plain_string, "equal");
                break;     
            case '>':
                strcat(plain_string, "greaterthan");
                break;
            case '<':
                strcat(plain_string, "lessthan");
                break;
            case ',':
                strcat(plain_string, "comma");
                break;
            case '%':
                strcat(plain_string, "percent");
                break;
            case '\\':
                strcat(plain_string, "backslash");
                break;
            case ':':
        #ifdef EXPERIMENTAL
            case ' ':
        #endif
                strcat(plain_string, "_");
                break;
            default: {
                int64_t l = strlen(plain_string);
                plain_string[l] = self->chars[i];
                plain_string[l + 1] = '\0';
                break;
            }
        }
    }
    char* result = (char*)internal_allocate(strlen(plain_string) + 1);
    strcpy(result, plain_string);
    return (const char*)result;
}


//
// The VTABLE-function
//


static VTABLE(VMSymbol) _VMSymbol_vtable;
bool VMSymbol_vtable_inited = false;


VTABLE(VMSymbol)* VMSymbol_vtable(void) {
    if(! VMSymbol_vtable_inited) {
        *((VTABLE(VMString)*)&_VMSymbol_vtable) = *VMString_vtable();
        _VMSymbol_vtable.init             = METHOD(VMSymbol, init);        
        _VMSymbol_vtable.get_plain_string = METHOD(VMSymbol, get_plain_string);

        VMSymbol_vtable_inited = true;
    }
    return &_VMSymbol_vtable;
}


#ifndef STRING_H_
#define STRING_H_

/*
 *  $Id: String.h 168 2008-01-03 14:05:26Z tobias.pape $
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

#include <stdint.h>

#include <misc/defs.h>

#include <vmobjects/OOObject.h>


#pragma mark VTable definition


VTABLE(String) {
#define STRING_VTABLE_FORMAT \
    OOOBJECT_VTABLE_FORMAT; \
    size_t        (*length)(void*); \
    const char*   (*rawChars)(void*); \
    int           (*compareTo)(void*, pString other); \
    pString       (*concat)(void*, pString other); \
    pString       (*concatChars)(void*, const char* restrict other, size_t length); \
    intptr_t      (*indexOf)(void*, pString pattern); \
    intptr_t      (*indexOfChar)(void*, char pattern); \
    intptr_t      (*lastIndexOfChar)(void*, char pattern); \
    int           (*charAt)(void*, size_t position); \
    pString       (*substring)(void*, size_t start, size_t end); \
    pVMInteger    (*toInteger)(void*); \
    pVMDouble     (*toDouble)(void*); \
    pString*      (*tokenize)(void*, size_t* length_of_result, \
                              const char* restrict delimiters);
    
    STRING_VTABLE_FORMAT;
};


#pragma mark class definition


struct _String {
    VTABLE(String)* _vtable;    
#define STRING_FORMAT \
    OOOBJECT_FORMAT; \
    size_t length; \
    char chars[0]
    
    STRING_FORMAT;
}; 


//
//  forward declaration of String Class Methods goes here
//


#pragma mark class methods


pString String_new(const char* restrict cstring, const size_t length);
pString String_new_from(pString restrict string);
pString String_new_concat(pString, pString);
pString String_new_concat_str(pString restrict, const char* restrict, size_t);

int CString_compare(const char* restrict a, size_t lenA, const char* restrict b, size_t lenB);

#pragma mark vtable initialization


VTABLE(String)* String_vtable(void);


#endif // STRING_H_

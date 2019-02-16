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
    size_t        (*size)(void*); \
    const char*   (*chars)(void*); \
    int           (*compareTo)(void*, pString other); \
    int           (*compareToChars)(void*, const char* restrict other); \
    pString       (*concat)(void*, pString other); \
    pString       (*concatChars)(void*, const char* restrict other); \
    pString       (*concatChar)(void*, char other); \
    int           (*indexOf)(void*, pString pattern); \
    int           (*indexOfChar)(void*, char pattern); \
    int           (*lastIndexOfChar)(void*, char pattern); \
    int           (*charAt)(void*, size_t position); \
    pString       (*substring)(void*, size_t start, size_t end); \
    pVMInteger    (*toInteger)(void*); \
    pVMDouble     (*toDouble)(void*); \
    pString*      (*tokenize)(void*, size_t* length, \
                              const char* restrict delimiters);
    
    STRING_VTABLE_FORMAT;
};


#pragma mark class definition


struct _String {
    VTABLE(String)* _vtable;    
#define STRING_FORMAT \
    OOOBJECT_FORMAT; \
    char* chars; \
    size_t length
    
    STRING_FORMAT;
}; 


//
//  forward declaration of String Class Methods goes here
//


#pragma mark class methods


pString String_new(const char* restrict cstring);
pString String_new_from(pString restrict string);


#pragma mark vtable initialization


VTABLE(String)* String_vtable();


#endif // STRING_H_

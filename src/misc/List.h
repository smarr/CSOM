#ifndef LIST_H_
#define LIST_H_

/*
 * $Id: List.h 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <misc/defs.h>

#include <vmobjects/OOObject.h>


#pragma mark VTable definition


VTABLE(List) {
#define LIST_VTABLE_FORMAT \
    OOOBJECT_VTABLE_FORMAT; \
    void  (*add)(void*, void* ptr); \
    void  (*addString)(void*, pString str); \
    void  (*addIfAbsent)(void*, void* ptr); \
    void  (*addStringIfAbsent)(void*, pString str); \
    void  (*addAll)(void*, pList list); \
    void  (*clear)(void*); \
    void  (*deep_free)(void*); \
    size_t (*indexOf)(void*, void* ptr); \
    size_t (*indexOfString)(void*, pString str); \
    size_t (*indexOfStringLen)(void*, const char* restrict str, size_t length); \
    size_t (*size)(void*); \
    void* (*get)(void*, size_t index)
    
    LIST_VTABLE_FORMAT;
};


#pragma mark class definition


typedef struct _ListElem ListElem, *pListElem;

    
struct _List {
    VTABLE(List)* _vtable;
#define LIST_FORMAT \
    OOOBJECT_FORMAT; \
    size_t size; \
    pListElem head; \
    pListElem last

    LIST_FORMAT;
}; 


//
//  forward declaration of List Class Methods goes here
//


#pragma mark class methods


pList List_new(void);


#pragma mark vtable initialization


VTABLE(List)* List_vtable(void);


#endif // LIST_H_

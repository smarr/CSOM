#ifndef HASHMAP_H_
#define HASHMAP_H_

/*
 * $Id: Hashmap.h 168 2008-01-03 14:05:26Z tobias.pape $
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

#include <stdbool.h>


#pragma mark ** Hashmap Element


#pragma mark VTable definition


VTABLE(HashmapElem) {
#define HASHMAPELEM_VTABLE_FORMAT \
    OOOBJECT_VTABLE_FORMAT; \
    int64_t  (*key_hash)(void*); \
    bool     (*key_equal_to)(void*,void*)
        
    HASHMAPELEM_VTABLE_FORMAT;
};


#pragma mark class definition


struct _HashmapElem {
    VTABLE(HashmapElem)* _vtable;
#define HASHMAPELEM_FORMAT \
    OOOBJECT_FORMAT; \
    void* key; \
    void* value
    
    HASHMAPELEM_FORMAT;
};


#pragma mark class methods


pHashmapElem HashmapElem_new(void* key, void* value);


#pragma mark vtable initialization


VTABLE(HashmapElem)* HashmapElem_vtable(void);


#pragma mark ** Hashmap


#pragma mark VTable definition


//
// The hash map implemented here for VM-internal purposes provides two different
// ways for putting and getting elements. Keys are "ordinary" VM objects
// (i.e., instances of heirs of OOObject).
//


VTABLE(Hashmap) {
#define HASHMAP_VTABLE_FORMAT \
    OOOBJECT_VTABLE_FORMAT; \
    void  (*put)(void*, void* key, void* value); \
    void* (*get)(void*, void* key); \
    bool  (*contains_key)(void*, void* key); \
    void  (*clear)(void*); \
    void  (*rehash)(void*, size_t newSize)
                    
    HASHMAP_VTABLE_FORMAT;
};


#pragma mark class definition


#define HASHMAP_DEFAULT_SIZE 101


struct _Hashmap {
    VTABLE(Hashmap)* _vtable;
#define HASHMAP_FORMAT \
    OOOBJECT_FORMAT; \
    size_t size; \
    pHashmapElem* elems
            
    HASHMAP_FORMAT;
};


//
//  forward declaration of Hashmap Class Methods goes here
//


#pragma mark class methods


pHashmap Hashmap_new(void);


#pragma mark vtable initialization


VTABLE(Hashmap)* Hashmap_vtable(void);


#endif // HASHMAP_H_

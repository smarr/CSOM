#ifndef STRINGHASHMAP_H_
#define STRINGHASHMAP_H_

/*
 * $Id: StringHashmap.h 114 2007-09-19 09:45:37Z tobias.pape $
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
#include <misc/Hashmap.h>

#include <stdbool.h>


#pragma mark ** StringHashmap Element


#pragma mark VTable definition


VTABLE(StringHashmapElem) {
#define STRINGHASHMAPELEM_VTABLE_FORMAT \
    OOOBJECT_VTABLE_FORMAT; \
    int64_t  (*key_hash)(void*); \
    bool     (*key_equal_to)(void*,pString)

        
    STRINGHASHMAPELEM_VTABLE_FORMAT;
};


#pragma mark class definition


struct _StringHashmapElem {
    VTABLE(StringHashmapElem)* _vtable;
#define STRINGHASHMAPELEM_FORMAT \
    OOOBJECT_FORMAT; \
    pString key; \
    void* value
        
    STRINGHASHMAPELEM_FORMAT;
};


#pragma mark class methods


pHashmapElem StringHashmapElem_new(pString key, void *value);


#pragma mark vtable initialization


VTABLE(StringHashmapElem)* StringHashmapElem_vtable(void);


#pragma mark VTable definition


//
// The hash map implemented here for VM-internal purposes uses keys
// of time pString and values of arbitary type.
//


VTABLE(StringHashmap) {
#define STRINGHASHMAP_VTABLE_FORMAT \
    OOOBJECT_VTABLE_FORMAT; \
    void  (*put)(void*, pString key, void* value); \
    void* (*get)(void*, pString key); \
    bool  (*contains_key)(void*, pString key); \
    void  (*clear)(void*); \
    void  (*rehash)(void*, size_t newSize)
    
    STRINGHASHMAP_VTABLE_FORMAT;
};


#pragma mark class definition


struct _StringHashmap {
    VTABLE(StringHashmap)* _vtable;
#define STRINGHASHMAP_FORMAT \
    HASHMAP_FORMAT

    STRINGHASHMAP_FORMAT;
}; 


//
//  forward declaration of Hashmap Class Methods goes here
//


#pragma mark class methods


pStringHashmap StringHashmap_new(void);


#pragma mark vtable initialization

VTABLE(StringHashmap)* StringHashmap_vtable(void);


#endif // STRINGHASHMAP_H_

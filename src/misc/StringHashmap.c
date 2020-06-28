/*
 * $Id: StringHashmap.c 171 2008-01-10 08:28:50Z tobias.pape $
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

#include "StringHashmap.h"

#include <memory/gc.h>

#include <vm/Universe.h>

#include <string.h>

extern void*  primitiveGet(pHashmap self, void* key, size_t start);
extern bool   primitivePut(pHashmap self, pHashmapElem elem, size_t start);
extern size_t hashf(pHashmap self, size_t hashv);

void* _StringHashmap_get(void* _self, pString key);
void  _StringHashmap_put(void* _self, pString key, void* value);


/**
 * Compare two Keys using string comparison.
 * parameters key1 and key2 must be C-Strings
 */
bool _StringHashmapElem_key_equal_to(void* _self, pString other) {
    pStringHashmapElem self = (pStringHashmapElem)_self;
    return 0 == SEND(other, compareTo, self->key);
}


int64_t _StringHashmapElem_key_hash(void* _self) {
    pStringHashmapElem self = (pStringHashmapElem)_self;
    return self->key->hash;
}


pHashmapElem StringHashmapElem_new(pString k, void* v) {
    pStringHashmapElem result =
        (pStringHashmapElem)internal_allocate(sizeof(StringHashmapElem));
    if (result) {
        result->_vtable = StringHashmapElem_vtable();
        INIT(result, k, v);
    }
    return (pHashmapElem)result;
}


void _StringHashmapElem_init(void* _self, ...) {
    pStringHashmapElem self = (pStringHashmapElem)_self;
    va_list argp;
    va_start(argp, _self);
    pString k = va_arg(argp, pString);
    void* v = va_arg(argp, void*);
    SUPER(HashmapElem, self, init, k, v);
    va_end(argp);
}


void _StringHashmapElem_free(void* self) {
    internal_free(((pStringHashmapElem)self)->key);
    SUPER(OOObject, self, free);
}


//
//  Class Methods (Starting with StringHashmap_) 
//


/**
 * allocates a new StringHashmap
 */
pStringHashmap StringHashmap_new(void) {
    pStringHashmap result = (pStringHashmap)internal_allocate(sizeof(StringHashmap));
    if(result) {
        result->_vtable = StringHashmap_vtable();
        INIT(result);
    }    
    return result;    
}


/**
 * Set up the StringHashmap object
 */
void _StringHashmap_init(void* _self, ...) {
    pStringHashmap self = (pStringHashmap)_self;
    SUPER(Hashmap, self, init, HASHMAP_DEFAULT_SIZE);
}


//
//  Instance Methods (Starting with _StringHashmap_) 
//


void _StringHashmap_free(void* self) {
    pStringHashmap _self = (pStringHashmap)self;
    for(size_t i = 0; i < _self->size; i++) {
        if(_self->elems[i])
            // free the string.
            SEND(_self->elems[i], free);
    }
    internal_free(_self->elems);
    SUPER(OOObject, self, free);
}


/**
 *  Get Element from Map
 *  parameter key must be a C-String
 */
void* _StringHashmap_get(void* _self, pString key) {
    pStringHashmap self = (pStringHashmap)_self;
    size_t hash = key->hash;
    return primitiveGet((pHashmap)self, key, hashf((pHashmap)self, hash));
}


/**
 *  Put element into Map
 *  parameter key must be a C-String
 */
void _StringHashmap_put(void* _self, pString key, void* value) {
    pStringHashmap self = (pStringHashmap)_self;
    pHashmapElem elem = (pHashmapElem) StringHashmapElem_new(key, value);
    if(!primitivePut((pHashmap)self, elem, SEND(elem, key_hash)))
        // something is terribly wrong now, bail out
        Universe_error_exit("Hash map overflow after rehashing.\n");
}


//
// vtable initialisation
//


static VTABLE(StringHashmapElem) _StringHashmapElem_vtable;
bool StringHashmapElem_vtable_inited = false;


VTABLE(StringHashmapElem)* StringHashmapElem_vtable(void) {
    if(!StringHashmapElem_vtable_inited) {
        *((VTABLE(HashmapElem)*)&_StringHashmapElem_vtable) =
            *HashmapElem_vtable();
        _StringHashmapElem_vtable.free =
            METHOD(StringHashmapElem, free);
        _StringHashmapElem_vtable.init =
            METHOD(StringHashmapElem, init);
        _StringHashmapElem_vtable.key_equal_to =
            METHOD(StringHashmapElem, key_equal_to);
        _StringHashmapElem_vtable.key_hash =
            METHOD(StringHashmapElem, key_hash);            
        StringHashmapElem_vtable_inited = true;
    }
    return &_StringHashmapElem_vtable;
}

static VTABLE(StringHashmap) _StringHashmap_vtable;
bool StringHashmap_vtable_inited = false;

VTABLE(StringHashmap)* StringHashmap_vtable(void) {
    if(!StringHashmap_vtable_inited) {
        *((VTABLE(Hashmap)*)&_StringHashmap_vtable) = *Hashmap_vtable();
        _StringHashmap_vtable.free = METHOD(StringHashmap, free);
        _StringHashmap_vtable.init = METHOD(StringHashmap, init);
        _StringHashmap_vtable.get  = METHOD(StringHashmap, get);
        _StringHashmap_vtable.put  = METHOD(StringHashmap, put);
        StringHashmap_vtable_inited = true;
    }
    return &_StringHashmap_vtable;
}

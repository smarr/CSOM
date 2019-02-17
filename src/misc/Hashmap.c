/*
 * $Id: Hashmap.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "Hashmap.h"

#include <misc/debug.h>

#include <memory/gc.h>

#include <vm/Universe.h>


//
// HashmapElem: class for representing hash map elements
//
bool _HashmapElem_key_equal_to(void* _self, void* other) {
    pHashmapElem self = (pHashmapElem)_self;
    return self->key == other;
}


int64_t _HashmapElem_key_hash(void* _self) {
    pHashmapElem self = (pHashmapElem)_self;
    return ((pOOObject)self->key)->hash;
}


pHashmapElem HashmapElem_new(void* k, void* v) {
    pHashmapElem result = (pHashmapElem)internal_allocate(sizeof(HashmapElem));
    if(result) {
        result->_vtable = HashmapElem_vtable();
        INIT(result, (pOOObject)k, (pOOObject)v);
    }
    return result;
}


void _HashmapElem_init(void* _self, ...) {
    pHashmapElem self = (pHashmapElem)_self;
    SUPER(OOObject, self, init);

    va_list argp;
    va_start(argp, _self);
    self->key = va_arg(argp, void*);
    self->value = va_arg(argp, void*);
    va_end(argp);
}


void _HashmapElem_free(void* _self) {
    SUPER(OOObject, _self, free);
}

//
// Hashmap class
//
// The hash map implemented here uses a very simple collision handling scheme:
// in case a collision occurs, the next entry is tried, then the next, and so
// forth. This obviously is not the best implementation in terms of performance,
// but it keeps the implementation very simple.
//


//
//  Class Methods (Starting with Hashmap_) 
//


/**
 * Allocate a new Hashmap
 */
pHashmap Hashmap_new(void) {
    pHashmap result = (pHashmap)internal_allocate(sizeof(Hashmap));
    if(result) {
        result->_vtable = Hashmap_vtable();
        INIT(result, HASHMAP_DEFAULT_SIZE);
    }    
    return result;    
}


/**
 * Set up the Hashmap object
 */
void _Hashmap_init(void* _self, ...) {
    pHashmap self = (pHashmap)_self;
    SUPER(OOObject, self, init);

    va_list argp;
    va_start(argp, _self);
    self->size = va_arg(argp, size_t);
    va_end(argp);
    self->elems = 
        (pHashmapElem*)internal_allocate(self->size * sizeof(pHashmapElem));
    SEND(self, clear);
}


//
//  Instance Methods (Starting with _Hashmap_) 
//


void _Hashmap_free(void* _self) {
    SEND((pHashmap)_self, clear);
    internal_free(((pHashmap)_self)->elems);
    SUPER(OOObject, _self, free);
}


/**
 * Slot computation from hash value.
 */
size_t hashf(pHashmap self, size_t hashv) {
    return hashv % self->size;
}


void* primitiveGet(pHashmap self, void* key, size_t start) {
    size_t slot = start;
    do {
        if(!self->elems[slot]) // NULL entry -> key not found
            return NULL;
        if(SEND(self->elems[slot], key_equal_to, key))
            return self->elems[slot]->value;
        slot++;
        if(slot == self->size)
            slot = 0;
    } while(slot != start);
    return NULL;
}


/**
 * Get an element from a hash map
 */
void* _Hashmap_get(void* _self, void* key) {
    pHashmap self = (pHashmap)_self;
    return primitiveGet(self,
        key, hashf(self, ((pOOObject)key)->hash));
}


//
// rehashing helper array with prime numbers <= 2000
//


size_t nprimes = 303;


static size_t primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
    53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
    223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293,
    307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389,
    397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479,
    487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587,
    593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673,
    677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773,
    787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881,
    883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991,
    997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
    1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
    1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249,
    1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
    1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439,
    1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
    1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601,
    1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693,
    1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783,
    1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877,
    1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987,
    1993, 1997, 1999
};


/**
 * return the next prime larger than or equal to k, or k itself if the array is
 * exhausted
 */
size_t nextPrimeOrNumber(size_t k) {
    for(int i = 0; i < nprimes; i++)
        if(primes[i] >= k)
            return primes[i];
    return k;
}


//
// helpers for putting an element into the hash map: return true on success,
// false otherwise ("private", hence not visible in the vtable)
//


bool attemptPut(void* _self, pHashmapElem elem, size_t start) {
    pHashmap self = (pHashmap)_self;
    size_t slot = start;
    do {
        if(self->elems[slot]) {
            slot++;
            if(slot == self->size) // wrap around at end of elems array
                slot = 0;
        } else {
            self->elems[slot] = elem;
            return true;
        }
    } while(slot != start);
    return false;
}


bool primitivePut(pHashmap self, pHashmapElem elem, size_t hashv) {
    if(!attemptPut(self, elem, hashf(self, hashv))) {
        // no free slot found - at least double the size of the elems array and
        // rehash
        // Note: the slot has to be re-computed
        SEND(self, rehash, nextPrimeOrNumber(self->size * 2 + 1));
        return attemptPut(self, elem, hashf(self, hashv));
    }
    return true;
}


/**
 *  Put an element into a hash map
 */
void _Hashmap_put(void* _self, void* key, void* value) {
    pHashmap self = (pHashmap)_self;
    pHashmapElem elem = HashmapElem_new(key, value);
    if(!primitivePut(self, elem, SEND(elem, key_hash)))
        // something is terribly wrong now, bail out
        Universe_error_exit("Hash map overflow after rehashing.\n");
}


void _Hashmap_rehash(void* _self, size_t newSize) {
    pHashmap self = (pHashmap)_self;
    size_t oldSize = self->size;
    pHashmapElem* oldElems = self->elems;
    self->size = newSize;
    self->elems = (pHashmapElem*)internal_allocate(
        self->size * sizeof(pHashmapElem));
    SEND(self, clear);
    
    for(size_t i = 0; i < oldSize; i++)
        if(oldElems[i]) {
            if(!attemptPut(self, oldElems[i],
                hashf(self, SEND(oldElems[i], key_hash))))
                // not good, can't help
                Universe_error_exit("Hashmap overflow while rehashing.\n");
            else
                oldElems[i] = NULL;
        }
            
    internal_free(oldElems);
}


/**
 *  Check existence of a given key in a hash map
 */
bool _Hashmap_contains_key(void* _self, void* key) {
    pHashmap self = (pHashmap)_self;
    return SEND(self, get, key) ? true : false;
}


/**
 *  Clear a hash map
 */
void _Hashmap_clear(void* _self) {
    pHashmap self = (pHashmap)_self;
    for(size_t i = 0; i < self->size; i++) {
        if (self->elems[i])
            internal_free(self->elems[i]);
        self->elems[i] = NULL;
    }
}


//
// vtable initialisation
//


static VTABLE(HashmapElem) _HashmapElem_vtable;
bool HashmapElem_vtable_inited = false;


VTABLE(HashmapElem)* HashmapElem_vtable(void) {
    if(!HashmapElem_vtable_inited) {
        *((VTABLE(OOObject)*)&_HashmapElem_vtable) = *OOObject_vtable();
        _HashmapElem_vtable.free         = METHOD(HashmapElem, free);
        _HashmapElem_vtable.init         = METHOD(HashmapElem, init);
        _HashmapElem_vtable.key_equal_to = METHOD(HashmapElem, key_equal_to);
        _HashmapElem_vtable.key_hash     = METHOD(HashmapElem, key_hash);            
        HashmapElem_vtable_inited = true;
    }
    return &_HashmapElem_vtable;
}


static VTABLE(Hashmap) _Hashmap_vtable;
bool Hashmap_vtable_inited = false;


VTABLE(Hashmap)* Hashmap_vtable(void) {
    if(!Hashmap_vtable_inited) {
        *((VTABLE(OOObject)*)&_Hashmap_vtable) = *OOObject_vtable();
        _Hashmap_vtable.free         = METHOD(Hashmap, free);
        _Hashmap_vtable.init         = METHOD(Hashmap, init);
        _Hashmap_vtable.get          = METHOD(Hashmap, get);
        _Hashmap_vtable.put          = METHOD(Hashmap, put);
        _Hashmap_vtable.clear        = METHOD(Hashmap, clear);
        _Hashmap_vtable.contains_key = METHOD(Hashmap, contains_key);
        _Hashmap_vtable.rehash       = METHOD(Hashmap, rehash);
        Hashmap_vtable_inited = true;
    }
    return &_Hashmap_vtable;
}

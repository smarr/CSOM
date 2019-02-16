/*
 * $Id: OOObject.c 176 2008-03-19 10:21:59Z michael.haupt $
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

#include <stddef.h>
#include <stdbool.h>

#include <memory/gc.h>

#include "OOObject.h"


/*
 * Initialize an Object
 */
void _OOObject_init(void* _self, ...) {
    pOOObject self = (pOOObject)_self;
    // top level Object.
    // set the hashcode
    self->hash = (intptr_t)self;
}


//
//  Instance Methods (Starting with _OOObject_) 
//


void _OOObject_free(void* _self) {
    internal_free(_self);
}


intptr_t _OOObject_object_size(void* _self) {
    return ((pOOObject)_self)->object_size;
}


// the VTable
static VTABLE(OOObject) _OOObject_vtable; 
bool   OOObject_vtable_inited = false;


VTABLE(OOObject)* OOObject_vtable(void) {
    if(! OOObject_vtable_inited) {
        _OOObject_vtable._ttable     = NULL; /* no traits present */
        _OOObject_vtable.free        = METHOD(OOObject, free);
        _OOObject_vtable.init        = METHOD(OOObject, init);
        _OOObject_vtable.object_size = METHOD(OOObject, object_size);
        OOObject_vtable_inited = true;
    }
    return &_OOObject_vtable;
}


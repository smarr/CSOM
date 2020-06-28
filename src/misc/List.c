/*
 * $Id: List.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <stdlib.h>
#include "List.h"

#include <vm/Universe.h>
#include <misc/debug.h>

#include <memory/gc.h>


void   _List_add(void* _self, void* ptr);
void   _List_free(void* _self);
void   _List_deep_free(void* _self);
void   _List_addAll(void* _self, pList list);

void   _List_clear(void* _self);
size_t _List_indexOf(void* _self, void* ptr);
size_t _List_size(void* _self);
void*  _List_get(void* _self, size_t index);

void   _List_init(void* _self, ...);


//
// ListElem 
//


VTABLE(ListElem)* ListElem_vtable(void);


VTABLE(ListElem) {
    OOOBJECT_VTABLE_FORMAT;
};


struct _ListElem {
    VTABLE(ListElem)* _vtable;
    OOOBJECT_FORMAT;
    void* data;
    pListElem next;
};


pListElem ListElem_new(void* data) {
    pListElem result = (pListElem)internal_allocate(sizeof(ListElem));
    if(result) {
        result->_vtable = ListElem_vtable();
        INIT(result, data);
    }
    return result;
}


void _ListElem_init(void* _self, ...) {
    pListElem self = (pListElem)_self;
    va_list argp;
    va_start(argp, _self);
    self->data = va_arg(argp, void*);
    va_end(argp);
    self->next = NULL;
}


void _ListElem_free(void* _self) {
    SUPER(OOObject, _self, free);
}


//
//  Class Methods (Starting with List_) 
//


/**
 * Allocate a new List
 */
pList List_new(void) {
    pList result = (pList)internal_allocate(sizeof(List));
    if(result) {
        result->_vtable = List_vtable();
        INIT(result);        
    }    
    return result;    
}


/**
 * Set up the List object
 */
void _List_init(void* _self, ...) {
    pList self = (pList)_self;
    SUPER(OOObject, self, init);
    self->size = 0;
    self->head = self->last = NULL;
}


//
//  Instance Methods (Starting with _List_) 
//


/**
 * add an element to a list
 */
void _List_add(void* _self, void* ptr) {
    pList self = (pList)_self;
    pListElem elem = ListElem_new(ptr);
    if(!self->head) { // list head is empty (i.e., add for the first time)
        self->size = 1;
        self->head = self->last = elem;
    } else { // append to the end using last
        self->size++;
        self->last->next = elem;
        self->last = elem;
    }
}


void _List_addString(void* _self, pString string) {
    pList self = (pList)_self;
    pString managedString = String_new(string->chars, string->length);
    SEND(self, add, managedString);
}


void _List_addIfAbsent(void* _self, void* ptr) {
    pList self = (pList)_self;
    if(SEND(self, indexOf, ptr) == -1)
        SEND(self, add, ptr);
}


void _List_addStringIfAbsent(void* _self, pString string) {
    pList self = (pList)_self;
    if (SEND(self, indexOfString, string) == -1) {
        SEND(self, addString, string);
    }
}


/**
 * free a list
 */
void _List_free(void* _self) {
    pList self = (pList)_self;
    if(self->head) {
        pListElem elem = self->head;
        for(pListElem elem2 = elem; elem2; elem=elem2) {
            elem2 = elem->next;
            SEND(elem, free);
        }
    }
    SUPER(OOObject, self, free);
}


/**
 * frees a list and its content as well
 */
void _List_deep_free(void* _self) {
    pList self = (pList)_self;
    if(self->head) {
        pListElem elem = self->head;
        for(pListElem elem2 = elem; elem2; elem=elem2) {
            elem2 = elem->next;
            SEND((pOOObject)(elem->data), free);
            SEND(elem, free);
        }
    }
    internal_free(self);
}


/**
 * reset the list by unreferencing its still allocated content
 */
void _List_clear(void* _self) {
    pList self = (pList)_self;
    if(self->head) {
        self->size = 0;
        self->head = self->last = NULL;
    }
}


size_t _List_indexOf(void* _self, void* ptr) {
    pList self = (pList)_self;
    pListElem elem = self->head;
    for(size_t result = 0; elem; result++, elem = elem->next)
        if(elem->data == ptr)
            return result;
    return -1;
}


size_t _List_indexOfString(void* _self, pString string) {
    pList self = (pList)_self;
    pListElem elem = self->head;
    size_t hash = string->hash;

    for (size_t result = 0; elem; result++, elem = elem->next) {
        pString dataString = (pString) elem->data;
        if (dataString->hash == hash &&
            (CString_compare(string->chars, string->length,
                             SEND(dataString, rawChars), SEND(dataString, length)) == 0)) {
            return result;
        }
    }
    return -1;
}


size_t _List_indexOfStringLen(void* _self, const char* restrict string, size_t length) {
    pList self = (pList)_self;
    pListElem elem = self->head;
    for (size_t result = 0; elem; result++, elem = elem->next) {
        pString dataString = (pString) elem->data;
        if (CString_compare(string, length,
                            SEND(dataString, rawChars), SEND(dataString, length)) == 0) {
            return result;
        }
    }
    return -1;
}


size_t _List_size(void* _self) {
    pList self = (pList)_self;
    return self->size;
}


void* _List_get(void* _self, size_t index) {
    pList self = (pList)_self;
    pListElem elem = self->head;
    while(index-- && elem)
        elem = elem->next;
    if(elem)
        return elem->data;
    else {
        Universe_error_exit("Error in list traversal: illegal index.\n");
        return NULL;
    }
}


void _List_addAll(void* _self, pList list) {
    pList self = (pList)_self;
    for(pListElem elem = list->head; elem; elem = elem->next)
        SEND(self, add, elem->data);
}


//
// vtable initialisation
//


static VTABLE(ListElem) _ListElem_vtable;
bool ListElem_vtable_inited = false;


VTABLE(ListElem)* ListElem_vtable(void) {
    if(!ListElem_vtable_inited) {
        *((VTABLE(OOObject)*)&_ListElem_vtable) = *OOObject_vtable();
        _ListElem_vtable.free = METHOD(ListElem, free);
        _ListElem_vtable.init = METHOD(ListElem, init);
        ListElem_vtable_inited = true;
    }
    return &_ListElem_vtable;
}


static VTABLE(List) _List_vtable;
bool List_vtable_inited = false;


VTABLE(List)* List_vtable(void) {
    if(!List_vtable_inited) {
        *((VTABLE(OOObject)*)&_List_vtable) = *OOObject_vtable();
        _List_vtable.free               = METHOD(List, free);
        _List_vtable.init               = METHOD(List, init);
        _List_vtable.add                = METHOD(List, add);
        _List_vtable.addString          = METHOD(List, addString);
        _List_vtable.addIfAbsent        = METHOD(List, addIfAbsent);
        _List_vtable.addStringIfAbsent  = METHOD(List, addStringIfAbsent);
        _List_vtable.addAll             = METHOD(List, addAll);
        _List_vtable.clear              = METHOD(List, clear);
        _List_vtable.deep_free          = METHOD(List, deep_free);
        _List_vtable.indexOf            = METHOD(List, indexOf);
        _List_vtable.indexOfString      = METHOD(List, indexOfString);
        _List_vtable.indexOfStringLen   = METHOD(List, indexOfStringLen);
        _List_vtable.size               = METHOD(List, size);
        _List_vtable.get                = METHOD(List, get);
        List_vtable_inited = true;
    }
    return &_List_vtable;
}

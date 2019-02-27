#ifndef OOOBJECT_H_
#define OOOBJECT_H_

/*
 * $Id: OOObject.h 227 2008-04-21 15:21:14Z michael.haupt $
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
#include <stddef.h>
#include <stdarg.h>
#include <misc/defs.h>


#pragma mark **** Object Macros.  ******


/** 
 * The SEND macro. 
 * It is used for general sending of messages to objects.
 */
#define SEND(O,M,...) \
    ({ typeof(O) _O = (O); \
    (_O->_vtable->M(_O , ##__VA_ARGS__)); \
    })


/** 
 *  The TSEND macro.
 *  It is used for sending messages to an object's trait
 */
#ifdef EXPERIMENTAL
#define TSEND(TRAIT,O,M,...) \
    ({ typeof(O) _O = (O); \
    (((VTABLE(TRAIT)*)(_O->_vtable))->M((TRAIT *)_O , ##__VA_ARGS__)); \
    })
#else
#define TSEND(TRAIT,O,M,...) \
    ({ typeof(O) _O = (O); \
    (((VTABLE(TRAIT)*)(_O->_vtable->_ttable))->M((TRAIT*)_O , ##__VA_ARGS__)); \
    })
#endif // EXPERIMENTAL


/** 
 * The ASSIGN_TRAIT helper macro.
 * It is used for assigning a trait t to a VTable C
 */
// using GNU ({ ... }) extension.
#ifdef EXPERIMENTAL
#define ASSIGN_TRAIT(T,C) ({ \
      VTABLE(OOObject)* _vt = (VTABLE(OOObject)*)&_##C##_vtable; \
      while(_vt->_ttable) { \
         _vt=((VTABLE(OOObject)*)_vt)->_ttable; \
      } \
       _vt->_ttable=T##_vtable(); \
    })
#else
#define ASSIGN_TRAIT(T,C) \
    ((_##C##_vtable._ttable)=T##_vtable())
#endif // EXPERIMENTAL


/** 
 * The VTable Helper Macros
 * Useful for simpler VTable Defintition
 */
#define VTABLE(CLASS) \
    struct _##CLASS##_vtable_layout


/**
 * The Method Helper.
 * used to define instance-methods in VTables
 */
#define METHOD(C,M) _##C##_##M


#pragma mark typing


/**
 *  Every _init shall call it's SuperConstructor,
 *  p.e. SUPER(ASTNode, self, init);
 */
#define SUPER(class,object,msg, ...) \
    ((class##_vtable())->msg((object) , ##__VA_ARGS__))


#define IS_A(object,class) \
    (((class *)object)->_vtable == class##_vtable())


#ifdef EXPERIMENTAL
#define SUPPORTS(O,TRAIT) ({ \
    VTABLE(OOObject)* _vt = (VTABLE(OOObject)*)((O)->_vtable); \
    bool found = false; \
    while(_vt->_ttable) { \
        if(_vt->_ttable == TRAIT##_vtable()) \
            found = true; \
             _vt=((VTABLE(OOObject)*)_vt)->_ttable; \
         } \
    found; \
})
#else
#define SUPPORTS(O,TRAIT) \
    ((VTABLE(TRAIT)*)((VTABLE(OOObject)*)((O)->_vtable)->_ttable) == \
        TRAIT##_vtable())
#endif // EXPERIMENTAL


#define INIT(O, ...) \
    SEND((pOOObject)(O),init , ##__VA_ARGS__)


#pragma mark Class type accessing


#include <vmobjects/objectformats.h>


#pragma mark Object Defintitions


VTABLE(OOObject) {
#define OOOBJECT_VTABLE_FORMAT \
    void*   _ttable; \
    void    (*init)(void* _self, ...); \
    void    (*free)(void* _self); \
    intptr_t (*object_size)(void* _self)
        
    OOOBJECT_VTABLE_FORMAT;
};


struct _OOObject {
    VTABLE(OOObject)* _vtable;
    
#define OOOBJECT_FORMAT \
    intptr_t object_size; \
    intptr_t gc_field; \
    intptr_t hash

    OOOBJECT_FORMAT;
};


VTABLE(OOObject)* OOObject_vtable(void);


//
// macros for object size computations
//


#define OBJECT_SIZE_DIFF(SUB, SUPER) \
    (sizeof(SUB)/sizeof(void*)-sizeof(SUPER)/sizeof(void*))
    
    
#define SIZE_DIFF_VMOBJECT(SUB) ((size_t)(OBJECT_SIZE_DIFF(SUB,VMObject)))


#endif // OOOBJECT_H_

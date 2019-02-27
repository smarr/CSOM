#ifndef OBJECTFORMATS_H_
#define OBJECTFORMATS_H_

/*
 * $Id: objectformats.h 229 2008-04-22 14:10:50Z tobias.pape $
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

typedef struct _OOObject OOObject, *pOOObject;
	typedef struct _String String, *pString;
    typedef struct _List List, *pList;
    typedef struct _HashmapElem HashmapElem, *pHashmapElem;
        typedef struct _StringHashmapElem StringHashmapElem,
            *pStringHashmapElem;
    typedef struct _Hashmap Hashmap, *pHashmap;
        typedef struct _StringHashmap StringHashmap, *pStringHashmap;
    typedef struct _VMObject VMObject, *pVMObject;
        typedef struct _VMArray VMArray, *pVMArray;
            typedef struct _VMMethod VMMethod, *pVMMethod;
            typedef struct _VMFrame VMFrame, *pVMFrame;
        typedef struct _VMBlock VMBlock,  *pVMBlock;
        typedef struct _VMClass VMClass, *pVMClass;
        typedef struct _VMDouble VMDouble, *pVMDouble;
        typedef struct _VMInteger VMInteger, *pVMInteger;
        typedef struct _VMPrimitive VMPrimitive, *pVMPrimitive;
            typedef struct _VMEvaluationPrimitive VMEvaluationPrimitive,
                *pVMEvaluationPrimitive;
        typedef struct _VMString VMString, *pVMString;
            typedef struct _VMSymbol VMSymbol, *pVMSymbol;
        
        typedef struct _VMInvokable VMInvokable, *pVMInvokable;

typedef struct _free_list_entry free_list_entry, *pFree_list_entry;


/** 
 * typedef for Loaded primitives... 
 */
#include <stdbool.h>
typedef void (*routine_fn)(pVMObject, pVMFrame);
typedef bool (*supports_class_fn)(const char*);
typedef void (*init_csp_fn)(void);

#endif // OBJECTFORMATS_H_



#ifndef GC_H_
#define GC_H_

/*
 * $Id: gc.h 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include <vmobjects/VMObject.h>
#include <compiler/GenerationContexts.h>

#include <stdlib.h>


/*
 * macro for padding - only word-aligned memory must be allocated
 */
#define PAD_BYTES(N) ((sizeof(void*) - ((N) % sizeof(void*))) % sizeof(void*))


/*
 * The heap size can be set on VM startup. The argument passed to this function
 * represents the dedicated heap size in MB.
 */
void gc_set_heap_size(uint32_t heap_size);


void gc_mark_object(void* _self);


void gc_collect(void);
void gc_start_uninterruptable_allocation(void);
void gc_end_uninterruptable_allocation(void);


void*  gc_allocate(size_t size);
void*  gc_allocate_object(size_t size);
char*  gc_allocate_string(const char* restrict str);
void   gc_free(void* ptr);


void gc_stat(void);

void gc_initialize(void);
void gc_finalize(void);


void*  internal_allocate(size_t size);
void   internal_free(void* ptr);


#endif // GC_H_



/*
 * $Id: gc.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
 
#include <vm/Universe.h>


#include "gc.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


#include <misc/Hashmap.h>


#include <vmobjects/objectformats.h>
#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMMethod.h>
#include <vmobjects/VMBlock.h>
#include <vmobjects/VMPrimitive.h>
#include <vmobjects/VMEvaluationPrimitive.h>
#include <vmobjects/VMClass.h>
#include <vmobjects/VMString.h>
#include <vmobjects/VMDouble.h>


#include <interpreter/Interpreter.h>
#include <compiler/GenerationContexts.h>


/*
 * free_list_entries are used to manage the space freed after the sweep phase.
 * these entries contain their own size and a reference of the entry next to them
 */
struct _free_list_entry {
    free_list_entry* next;
    size_t size;
};


/*
 * heap used by gc_allocate
 */
void* object_space = NULL;


/*
 * the size of the heap used by gc_allocate (standard: 1 MB)
 */
intptr_t OBJECT_SPACE_SIZE = 1048576;


/*
 * will be adjusted when gc is initialized
 */
intptr_t BUFFERSIZE_FOR_UNINTERRUPTABLE = 50000;


/*
 * reference to the first entry in the free_list
 */
free_list_entry* first_free_entry = NULL;


/*
 * if this counter equals 0, only then it is safe to collect the
 * garbage. The counter is increased during initializations of
 * VMObjects and the generation of classes
 */
int uninterruptable_counter = 0;


size_t size_of_free_heap = 0;


//
// values for GC statistics
//


uint32_t num_collections; // the number of collections performed
uint32_t num_live;        // number of live objects (per collection)
uint32_t spc_live;        // space consumed by live objects (per collection)
uint32_t num_freed;       // number of freed objects (per collection)
uint32_t spc_freed;       // freed space (per collection)
uint32_t num_alloc;       // number of allocated objects (since last collection)
uint32_t spc_alloc;       // allocated space (since last collection)


//
// forward declarations
//


void gc_merge_free_spaces(void);
void gc_initialize(void);


void init_stat(void);
void init_collect_stat(void);
void collect_stat(void);
void reset_alloc_stat(void);


//
// implementations
//


void gc_set_heap_size(uint32_t heap_size) {
    // this can only be done before the heap is initialised - afterwards, it
    // yields an error message and terminates the VM
    if (object_space != NULL) {
        Universe_error_exit("attempt to change heap size after initialisation");
    }
    OBJECT_SPACE_SIZE = 1024 * 1024 * heap_size;
}


void gc_mark_reachable_objects() {
    // get globals
    pHashmap globals = (pHashmap) Universe_get_globals_dictionary();
    pHashmapElem elem = NULL;
    
    // iterate over the globals to mark all of them
    for(size_t i = 0; i < globals->size; i++) {
        elem = (pHashmapElem) globals->elems[i];
        if (elem != NULL) {
            gc_mark_object(elem->key);
            gc_mark_object(elem->value);
        }
    }
        
    // Get the current frame and mark it.
    // Since marking is done recursively, this automatically
    // marks the whole stack
    pVMFrame current_frame = (pVMFrame) Interpreter_get_frame();
    if (current_frame != NULL) {
        gc_mark_object(current_frame);
    }
}


/**
 *  check whether the object is inside the managed heap
 *  if it isn't, there is no VMObject to mark, otherwise
 *  check whether the object is already marked.
 *  if the self is an unmarked object inside the heap, then
 *  it is told to 'mark_references', recursively using this
 *  function for all its references.
 */
void gc_mark_object(void* _self) {
    pVMObject self = (pVMObject) _self;
    if (   ((void*) self >= (void*)  object_space) 
        && ((void*) self <= (void*) ((intptr_t) object_space + OBJECT_SPACE_SIZE)))
    {
        if (self->gc_field != 1) {
            // mark self before recursively marking contained references
            self->gc_field = 1;
            num_live++;
            spc_live += self->object_size;
            SEND(self, mark_references);
        }
    }
}


/**
 * For debugging - the layout of the heap is shown.
 * This function uses 
 * - '[size]' for free space,
 * - '-xx-' for marked objects, and
 * - '-size-' for unmarked objects.
 * The output is also aligned to improve readability.
 */
void gc_show_memory() {
    pVMObject pointer = object_space;
    free_list_entry* current_entry = first_free_entry;
    intptr_t object_size = 0;
    intptr_t object_aligner = 0;
    int line_count = 2;
    
    fprintf(stderr,"\n########\n# SHOW #\n########\n1 ");
    
    do {
        while (((void*)pointer > (void*)current_entry->next) && (current_entry->next != NULL)) {
            current_entry = current_entry->next;
        }
        
        if ((   (void*)current_entry->next == (void*)pointer) 
            || ((void*)current_entry == (void*)pointer)) 
        {
            if ((void*)current_entry == (void*)pointer) {
                object_size = current_entry->size;
            } else {
                free_list_entry* next = current_entry->next;
                object_size = next->size;
            }
            fprintf(stderr, "[%ld]", object_size);
        } else {
            pVMObject object = (pVMObject) pointer;
            
            object_size = SEND(object, object_size);
            
            // is this object marked or not?
            if (object->gc_field == 1) {
                fprintf(stderr,"-xx-");
            } else {
                pVMSymbol class_name = SEND(SEND(object, get_class), get_name);
                fprintf(stderr,"|%ld %s %p", object_size, SEND(class_name, get_plain_string), object);
            }
        }
        // aligns the output by inserting a line break after 36 objects
        object_aligner++;
        if (object_aligner == 36) {
            fprintf(stderr,"\n%d ", line_count++);
            object_aligner = 0;
        }
        pointer = (void*)((intptr_t)pointer + object_size);
    } while ((void*)pointer < (void*)((intptr_t) object_space + OBJECT_SPACE_SIZE));
}


void gc_collect() {
    num_collections++;
    init_collect_stat();
    
    if(gc_verbosity > 2) {
        fprintf(stderr, "-- pre-collection heap dump --\n");
        gc_show_memory();
    }
    
    gc_mark_reachable_objects();
    //gc_show_memory();
    pVMObject pointer = object_space;
    free_list_entry* current_entry = first_free_entry;
    size_t object_size = 0;

    do {
        // we need to find the last free entry before the pointer
        // whose 'next' lies behind or is the pointer
        while (((void*)pointer > (void*)current_entry->next) && (current_entry->next != NULL)) {
            current_entry = current_entry->next;
        }
        
        if (((void*)current_entry->next == (void*)pointer) || ((void*)current_entry == (void*)pointer)) {
            // in case the pointer is the part of the free_list:
            if ((void*)current_entry == (void*)pointer) {
                object_size = current_entry->size;
            } else {
                free_list_entry* next = current_entry->next;
                object_size = next->size;
            }
            //fprintf(stderr,"[%d]",object_size);
            // nothing else to be done here
        } else {
            // in this case the pointer is a VMObject
            pVMObject object = (pVMObject) pointer;         
            object_size = SEND(object, object_size);
            
            // is this object marked or not?
            if (object->gc_field == 1) {
                // remove the marking
                object->gc_field = 0;
            } else {
                num_freed++;
                spc_freed += object_size;
                
                // add new entry containing this object to the free_list
                SEND(object, free);
                memset(object, 0, object_size);
                free_list_entry* new_entry = (free_list_entry*) pointer;
                new_entry->size = object_size;

                // if the new entry lies before the first entry,
                // adjust the pointer to the first one
                if (new_entry < first_free_entry) {
                    new_entry->next = first_free_entry;
                    first_free_entry = new_entry;
                    current_entry = new_entry;
                } else {
                    // insert the newly created entry right after the current entry
                    new_entry->next = current_entry->next;
                    current_entry->next = new_entry;
                }
            }
        }
        // set the pointer to the next object in the heap
        pointer = (void*)((intptr_t)pointer + object_size);

    } while ((void*)pointer < (void*)((intptr_t)object_space + OBJECT_SPACE_SIZE));

    // combine free_entries, which are next to each other
    gc_merge_free_spaces();
    
    if(gc_verbosity > 1)
        collect_stat();
    if(gc_verbosity > 2) {
        fprintf(stderr, "-- post-collection heap dump --\n");
        gc_show_memory();
    }
    
    reset_alloc_stat();
}


void* gc_allocate(size_t size) { 
    if(size == 0) return NULL;
    
    if(size < sizeof(struct _free_list_entry)) {
        return internal_allocate(size);
    }
    
    // start garbage collection if the free heap has less
    // than BUFFERSIZE_FOR_UNINTERRUPTABLE Bytes and this
    // allocation is interruptable
    if ((size_of_free_heap <= BUFFERSIZE_FOR_UNINTERRUPTABLE)
        && (uninterruptable_counter <= 0)) {
        gc_collect();
    }
    void* result = NULL;
    
    // initialize variables to search through the free_list
    free_list_entry* entry = first_free_entry;
    free_list_entry* before_entry = NULL;

    // don't look for the perfect match, but for the first-fit
    while (! ((entry->size == size) 
               || (entry->next == NULL) 
               || (entry->size >= (size + sizeof(struct _free_list_entry))))) { 
        before_entry = entry;
        entry = entry->next;
    }
    
    // did we find a perfect fit?
    // if so, we simply remove this entry from the list
    if (entry->size == size) {
        if (entry == first_free_entry) { 
            // first one fitted - adjust the 'first-entry' pointer
            first_free_entry = entry->next;
        } else {
            // simply remove the reference to the found entry
            before_entry->next = entry->next;
        } // entry fitted
        result = entry;
        
    } else {
        // did we find an entry big enough for the request and a new
        // free_entry?
        if (entry->size >= (size + sizeof(struct _free_list_entry))) {
            // save data from found entry
            size_t old_entry_size = entry->size;
            free_list_entry* old_next = entry->next;
            
            result = entry;
            // create new entry and assign data
            free_list_entry* replace_entry =  (free_list_entry*) ((intptr_t)entry + size);
            
            replace_entry->size = old_entry_size - size;
            replace_entry->next = old_next;
            if (entry == first_free_entry) {
                first_free_entry = replace_entry;
            } else {
                before_entry->next = replace_entry;
            }
        } else {
            // no space was left
            // running the GC here will most certainly result in data loss!
            fprintf(stderr,"Not enough heap! Data loss is possible\n");
            fprintf(stderr, "FREE-Size: %zd, uninterruptable_counter: %d\n",
                size_of_free_heap, uninterruptable_counter);
            
            exit(ERR_FAIL);
        }
    }
           
    if(!result) {
        fprintf(stderr, "Failed to allocate %ld bytes. Panic.\n", size);
        Universe_exit(-1);
    }
    memset(result, 0, size);
    
    // update the available size
    size_of_free_heap -= size;
    return result;
}


void* gc_allocate_object(size_t size) {
    size_t aligned_size = size + PAD_BYTES(size);
    void* o = gc_allocate(aligned_size);
    if(o)
        ((pOOObject)o)->object_size = aligned_size;
    num_alloc++;
    spc_alloc += aligned_size;
    return o;
}


/**
 * this function must not do anything, since the heap management
 * is done inside gc_collect.
 * However, it is called upon by all VMObjects.
 */
void gc_free(void* ptr) {
    // do nothing when called for an object inside the object_space
    if ((   ptr < (void*)  object_space) 
        || (ptr >= (void*) ((intptr_t)object_space + OBJECT_SPACE_SIZE))) 
    {
        internal_free(ptr);
    }
}


// free entries which are next to each other are merged into one entry
void gc_merge_free_spaces() {
    free_list_entry* entry = first_free_entry;
    free_list_entry* entry_to_append = NULL;
    size_t new_size = 0;
    free_list_entry* new_next = NULL;
    
    size_of_free_heap = 0;

    while (entry->next != NULL) {
        if (((intptr_t)entry + (intptr_t)(entry->size)) == (intptr_t)(entry->next)) {
            entry_to_append = entry->next;
            new_size = entry->size + entry_to_append->size;
            new_next = entry_to_append->next;
            
            memset(entry_to_append, 0, entry_to_append->size);
            entry->next = new_next;
            entry->size = new_size;
        } else {
            size_of_free_heap += entry->size;   
            entry = entry->next;
        }
    }
    if (entry->next == NULL) {
        size_of_free_heap += entry->size;   
    } else {
        fprintf(stderr, "Missed last free_entry of GC\n");
        Universe_exit(-1);
    }
}


void* internal_allocate(size_t size) {
    if(size == 0)
        return NULL;
    void* result = malloc(size);
    if(!result) {
        debug_error("Failed to allocate %ld bytes. Panic.\n", size);
        Universe_exit(-1);
    }
    memset(result, 0, size);
    return result;
}


void internal_free(void* ptr) {
    free(ptr);
}


/**
 * Sets up the heap and the free_list managing the free entries
 * inside the heap.
 */
void gc_initialize() { 
    // Buffersize is adjusted to the size of the heap (10%)
    BUFFERSIZE_FOR_UNINTERRUPTABLE = (intptr_t) (OBJECT_SPACE_SIZE * 0.1);

    // allocation of the heap
    object_space = malloc(OBJECT_SPACE_SIZE);
    if (!object_space) {
        fprintf(stderr, "Failed to allocate the initial %ld bytes for the GC. Panic.\n",
                OBJECT_SPACE_SIZE);
        Universe_exit(-1);
    } 
    memset(object_space, 0, OBJECT_SPACE_SIZE);
    size_of_free_heap = OBJECT_SPACE_SIZE;
    
    // initialize free_list by creating the first
    // entry, which contains the whole object_space
    first_free_entry = (free_list_entry*) object_space;
    first_free_entry->size = OBJECT_SPACE_SIZE;
    first_free_entry->next = NULL;
    
    // initialise statistical counters
    init_stat();
}

void gc_finalize() {
    free(object_space);
    object_space = NULL;
}


/**
 * if this counter is gt zero, an object initialization is
 * in progress. This means, that most certainlt, there are
 * objects, which are not reachable by the rootset, therefore,
 * the garbage collection would result in data loss and must
 * not be started!
 */
void gc_start_uninterruptable_allocation() {
    uninterruptable_counter++;
}


/**
 * only if the counter reaches zero, it is safe to start the
 * garbage collection.
 */
void gc_end_uninterruptable_allocation() {
    uninterruptable_counter--;
}


//
// functions for GC statistics and debugging output
//


/*
 * initialise statistics
 */
void init_stat(void) {
    num_collections = 0;

    // these two need to be initially set here, as they have to be preserved
    // across collections and cannot be reset in init_collect_stat()
    num_alloc = 0;
    spc_alloc = 0;
}


/*
 * initialise per-collection statistics
 */
void init_collect_stat(void) {
    // num_alloc and spc_alloc are not initialised here - they are reset after
    // the collection in reset_alloc_stat()
    num_live = 0;
    spc_live = 0;
    num_freed = 0;
    spc_freed = 0;
}


/*
 * reset allocation statistics
 */
void reset_alloc_stat(void) {
    num_alloc = 0;
    spc_alloc = 0;
}


#define _KB(B) (B/1024)
#define _MB(B) ((double)B/(1024.0*1024.0))


/*
 * Output garbage collection statistics. This function is called at the end of
 * a VM run.
 */
void gc_stat(void) {
    fprintf(stderr, "-- GC statistics --\n");
    fprintf(stderr, "* heap size %ld B (%ld kB, %.2f MB)\n",
        OBJECT_SPACE_SIZE, _KB(OBJECT_SPACE_SIZE), _MB(OBJECT_SPACE_SIZE));
    fprintf(stderr, "* performed %d collections\n", num_collections);
}


/*
 * output per-collection statistics
 */
void collect_stat(void) {
    fprintf(stderr, "\n[GC %d, %d alloc (%d kB), %d live (%d kB), %d freed "\
        "(%d kB)]\n",
        num_collections, num_alloc, _KB(spc_alloc), num_live, _KB(spc_live),
        num_freed, _KB(spc_freed));
}

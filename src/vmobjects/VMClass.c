/*
 * $Id: VMClass.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "VMClass.h"
#include "VMInvokable.h"
#include "VMMethod.h"
#include "VMSymbol.h"
#include "VMArray.h"
#include "VMPrimitive.h"

#include <memory/gc.h>

#include <misc/debug.h>
#include <misc/Hashmap.h>
#include <misc/StringHashmap.h>

#include <vm/Universe.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef CSOM_WIN
/**
 * Emualting the dl-interface with win32 means
 */
#   define WIN32_LEAN_AND_MEAN
#   define dlerror()   "Load Error"
#   define dlsym       GetProcAddress
#   define DL_LOADMODE NULL, LOAD_WITH_ALTERED_SEARCH_PATH
#   define dlopen      LoadLibraryEx
#   define dlclose     CloseHandle
#   include <windows.h>
#else
#   include <dlfcn.h>
#endif

// private
int64_t number_of_super_instance_fields(void* _self);

//
//  Class Methods (Starting with VMClass_) 
//


/**
 * Create a new VMClass
 */
pVMClass VMClass_new(void) {
    return VMClass_new_num_fields(SIZE_DIFF_VMOBJECT(VMClass) - 1);
}


/**
 * Create a new VMClass with a specific number of fields
 */
pVMClass VMClass_new_num_fields(intptr_t number_of_fields) {
    // calculate Class size without fields
    size_t class_stub_size = sizeof(VMClass) - 
                             sizeof(pVMObject) * 
                            (SIZE_DIFF_VMOBJECT(VMClass) - 1);
    pVMClass result = 
        (pVMClass)gc_allocate_object(class_stub_size +
                              sizeof(pVMObject) * number_of_fields);

    if(result) {
        result->_vtable = VMClass_vtable();
        gc_start_uninterruptable_allocation();
        INIT(result, number_of_fields);
        gc_end_uninterruptable_allocation();
    }
    
    return result;
}


pVMClass VMClass_assemble(class_generation_context* cgc) {
    // build class class name
    const char* cgc_name = SEND(cgc->name, get_rawChars);
    size_t cgc_name_len = SEND(cgc->name, get_length);
    char* ccname =
        (char*)internal_allocate(cgc_name_len + 6 + 1); // 6: " class"
    strcpy(ccname, cgc_name);
    strcat(ccname, " class");
    pVMSymbol ccname_sym = Universe_symbol_for_cstr(ccname);
    internal_free(ccname);
    
    // Load the super class
    pVMClass super_class = Universe_load_class(cgc->super_name);
    
    // Allocate the class of the resulting class
    pVMClass result_class = Universe_new_class(metaclass_class);

    // Initialize the class of the resulting class
    SEND(result_class, set_instance_fields,
         Universe_new_array_list(cgc->class_fields));
    SEND(result_class, set_instance_invokables,
         Universe_new_array_list(cgc->class_methods));
    SEND(result_class, set_name, ccname_sym);
    pVMClass super_mclass = SEND(super_class, get_class);
    SEND(result_class, set_super_class, super_mclass);
    
    // Allocate the resulting class
    pVMClass result = Universe_new_class(result_class);
    
    // Initialize the resulting class
    SEND(result, set_instance_fields,
         Universe_new_array_list(cgc->instance_fields));
    SEND(result, set_instance_invokables,
         Universe_new_array_list(cgc->instance_methods));
    SEND(result, set_name, cgc->name);
    SEND(result, set_super_class, super_class);
    
    return result;
}


void VMClass_assemble_system_class(class_generation_context* cgc,
                                   pVMClass system_class
) {
    //System classes have only to be filed with fields and methods.
        
    // instance-bound
    SEND(system_class, set_instance_invokables, 
         Universe_new_array_list(cgc->instance_methods));
    SEND(system_class, set_instance_fields,
         Universe_new_array_list(cgc->instance_fields));
    // class-bound == class-instance-bound 
    pVMClass super_mclass = SEND(system_class, get_class);
    SEND(super_mclass, set_instance_invokables,
         Universe_new_array_list(cgc->class_methods));
    SEND(super_mclass, set_instance_fields,
         Universe_new_array_list(cgc->class_fields));
}


/**
 * Initialize a VMClass
 */
void _VMClass_init(void* _self, ...) {
    pVMClass self = (pVMClass)_self;    
    va_list args;
    va_start(args, _self);
    SUPER(VMObject, self, init, va_arg(args, intptr_t));
    va_end(args);
}


void _VMClass_free(void* self) {
    SUPER(VMObject, self, free);
}


//
//  Instance Methods (Starting with _VMClass_) 
//
pVMClass _VMClass_get_super_class(void* _self) {
    pVMClass self = (pVMClass)_self;
    // get the super class
    return self->super_class;
}


void _VMClass_set_super_class(void* _self, pVMClass value) {
    pVMClass self = (pVMClass)_self;
    // set the super class
    self->super_class = value;
}


bool _VMClass_has_super_class(void* _self) {
    pVMClass self = (pVMClass)_self;
    // check whether or not this class has a super class
    return (pVMObject)self->super_class != nil_object;
}


pVMSymbol _VMClass_get_name(void* _self) {
    pVMClass self = (pVMClass)_self;
    // get the name of this class
    return self->name;
}


void _VMClass_set_name(void* _self, pVMSymbol value) {
    pVMClass self = (pVMClass)_self;
    // set the name of this class by writing to the field with name index
    self->name = value;
}


pVMArray _VMClass_get_instance_fields(void* _self) {
    pVMClass self = (pVMClass)_self;
    // get the instance fields
    return self->instance_fields;
}


void _VMClass_set_instance_fields(void* _self, pVMArray value) {
    pVMClass self = (pVMClass)_self;
    // set the instance fields
    self->instance_fields = value;
}


pVMArray _VMClass_get_instance_invokables(void* _self) {
    pVMClass self = (pVMClass)_self;
    // get the instance invokables
    return self->instance_invokables;
}


void _VMClass_set_instance_invokables(void* _self, pVMArray value) {
    pVMClass self = (pVMClass)_self;
    // set the instance invokables 
    self->instance_invokables = value;
    
    // make sure this class is the holder of all invokables in the array
    for(int i = 0; i < SEND(self, get_number_of_instance_invokables); i++) {
        pVMObject inv = SEND(self, get_instance_invokable, i);
        TSEND(VMInvokable, inv, set_holder, self);
    }
}


int64_t _VMClass_get_number_of_instance_invokables(void* _self) {
    pVMClass self = (pVMClass)_self;
    // return the number of instance invokables in this class
    pVMArray arr = SEND(self, get_instance_invokables);
    return SEND(arr, get_number_of_indexable_fields);
}


pVMObject _VMClass_get_instance_invokable(void* _self, int64_t index) {
    pVMClass self = (pVMClass)_self;
    // get the instance invokable with the given index
    pVMArray arr = SEND(self, get_instance_invokables);
    return SEND(arr, get_indexable_field, index);
}


void _VMClass_set_instance_invokable(void* _self, int64_t idx, pVMObject value) {
    pVMClass self = (pVMClass)_self;
    // set this class as the holder of the given invokable
    TSEND(VMInvokable, value,  set_holder, self);
    // set the instance method with the given index to the given value
    pVMArray arr = SEND(self, get_instance_invokables);
    SEND(arr, set_indexable_field, idx, value);
}


pVMObject _VMClass_lookup_invokable(void* _self, pVMSymbol signature) {
    pVMClass self = (pVMClass)_self;
    pVMObject invokable = NULL;
    // lookup invokable with given signature in array of instance invokables
    for(int i = 0; i < SEND(self, get_number_of_instance_invokables); i++) {
      // get the next invokable in the instance invokable array
      invokable = SEND(self, get_instance_invokable, i);
      // return the invokable if the signature matches
      if(TSEND(VMInvokable, invokable, get_signature) == signature)
          return invokable;
    }    
    // traverse the super class chain by calling lookup on the super class
    if(SEND(self, has_super_class)) {
        invokable = SEND(self->super_class, lookup_invokable, signature);
        if(invokable)
            return invokable;
    }
    // invokable not found
    return NULL;
}


int64_t _VMClass_lookup_field_index(void* _self, pVMSymbol field_name) {
    pVMClass self = (pVMClass)_self;
    // lookup field with given name in array of instance fields
    for(int64_t i = SEND(self, get_number_of_instance_fields) - 1; i >= 0; i--) {
      // return the current index if the name matches
      if(field_name == SEND(self, get_instance_field_name, i))
        return i;
    }
    // field not found
    return -1;
}


bool _VMClass_add_instance_invokable(void* _self, pVMObject value) {
    pVMClass self = (pVMClass)_self;
    // add the given invokable to the array of instance invokables
    for(int i = 0; i < SEND(self, get_number_of_instance_invokables); i++) {
        // get the next invokable in the instance invokable array
        pVMObject invokable = SEND(self, get_instance_invokable, i);
        // replace the invokable with the given one if the signature matches
        if(TSEND(VMInvokable, invokable, get_signature) 
           == TSEND(VMInvokable, value, get_signature)) {
            SEND(self, set_instance_invokable, i, value);
            return false;
        }
    }
    // append the given method to the array of instance methods
    self->instance_invokables = 
        SEND(self->instance_invokables, copy_and_extend_with, value);
    return true;
}


void _VMClass_add_instance_primitive(void* _self, pVMPrimitive value, bool warn) {
    pVMClass self = (pVMClass)_self;
    if(SEND(self, add_instance_invokable, (pVMObject)value) && warn) {
        pVMSymbol sym = TSEND(VMInvokable, value, get_signature);
        debug_warn("Primitive %s is not in class definition for class %s.\n",
                   sym->chars,
                   self->name->chars);
    }
}


pVMSymbol _VMClass_get_instance_field_name(void* _self, int64_t index) {
    pVMClass self = (pVMClass)_self;
    // get the name of the instance field with the given index
    if(index >= number_of_super_instance_fields(self)) {
        // adjust the index to account for fields defined in the super class
        index -= number_of_super_instance_fields(self);
        // return the symbol representing the instance fields name
        return 
            (pVMSymbol)SEND(self->instance_fields, get_indexable_field, index);
    } else {
        // ask the super class to return the name of the instance field
        return SEND(self->super_class, get_instance_field_name, index);
    }
}


int64_t _VMClass_get_number_of_instance_fields(void* _self) {
    pVMClass self = (pVMClass)_self;
    // get the total number of instance fields in this class
    return
        SEND(self->instance_fields, get_number_of_indexable_fields) +
        number_of_super_instance_fields(self);
}


bool _VMClass_has_primitives(void* _self) {
    pVMClass self = (pVMClass)_self;
    // lookup invokable with given signature in array of instance invokables
    for(int i = 0; i < SEND(self, get_number_of_instance_invokables); i++) {
      // get the next invokable in the instance invokable array
        pVMObject inv = SEND(self, get_instance_invokable, i);
        if(TSEND(VMInvokable, inv, is_primitive))
            return true;
    }
    return false;  
}


int64_t number_of_super_instance_fields(void* _self) {
    pVMClass self = (pVMClass)_self;
    /*
     * get the total number of instance fields defined in super classes
     */
    if(SEND(self, has_super_class))
        return SEND(self->super_class, get_number_of_instance_fields);
    else
        return 0;
}


#pragma mark primitive loading helpers


/*
 * these functions are used solely in VMClass: load_primitives. 
 */

/**
 * generate the string containing the path to a primitive library which may be 
 * located at the classpath given.
 */
pString gen_loadstring(const pString restrict cp, const char* cname, size_t cnameLen) {
    pString loadstring = String_new_from(cp);
    pString old = loadstring;
    loadstring = SEND(old, concatChars, file_separator, strlen(file_separator));
    internal_free(old);

    old = loadstring;
    loadstring = SEND(old, concatChars, cname, cnameLen);
    internal_free(old);

    old = loadstring;
    loadstring = SEND(loadstring, concatChars, shared_extension, strlen(shared_extension));
    internal_free(old);
    
    return loadstring;
}


/**
 *  generate the string containing the path to a SOMCore which may be located
 *  at the classpath given.
 *
 */
pString gen_core_loadstring(const pString restrict cp) {
    #define S_CORE "SOMCore"
    pString result = gen_loadstring(cp, S_CORE, strlen(S_CORE));
    return result;
}


/**
 * load the given library, return the handle
 *
 */
void* load_lib(const pString restrict path) {

    #if !defined(CSOM_WIN)
        #ifdef DEBUG
            #define    DL_LOADMODE RTLD_NOW
        #else
            #define    DL_LOADMODE RTLD_LAZY
        #endif // DEBUG
    #endif
    
    // static handle. will be returned
    static void* handle = NULL;
    
    // try load lib
    if ((handle = dlopen(SEND(path, rawChars), DL_LOADMODE))) {
        // found
        return handle;
    } else {
        printf("dlopen failed with: %s\n", dlerror());
        return NULL;
    }
}


/**
 * check, whether the lib referenced by the handle supports the class given
 */
bool is_responsible(void* handle, const char* class) {
    // function handler
    supports_class_fn supports_class = NULL;

    supports_class = (supports_class_fn)dlsym(handle, "supports_class");

    if(!supports_class) {
        debug_error(dlerror());
        Universe_error_exit("Library doesn't have expected format");
    }
    
    // test class responsibility
    return (*supports_class)(class);
}


/*
 * Format definitions for Primitive naming scheme.
 */
#define CLASS_METHOD_FORMAT_S "%s_%s"
// as in AClass_aClassMethod
#define INSTANCE_METHOD_FORMAT_S "_%s_%s"
// as in _AClass_anInstanceMethod

static pStringHashmap primitive_map = NULL;

void VMClass_init_primitive_map() {
    primitive_map = StringHashmap_new();
}

/**
 * set the routines for primitive marked invokables of the given class
 */
void set_primitives(pVMClass class, void* handle, const char* cname,
    const char* restrict format, pVMClass target) {
    pVMPrimitive the_primitive;
    routine_fn   routine=NULL;
    
    // iterate invokables
    for(int i = 0; i < SEND(class, get_number_of_instance_invokables); i++) {
        the_primitive = (pVMPrimitive)SEND(class, get_instance_invokable, i);
        
        if(TSEND(VMInvokable, the_primitive, is_primitive)) {

            // we have a primitive to load
            // get it's selector
            pVMSymbol sig = TSEND(VMInvokable, the_primitive, get_signature);
            const char* selector = SEND(sig, get_plain_string);

            { //string block
                char symbol[strlen(cname) + strlen(selector) + 2 + 1];
                                                                //2 for 2x '_'
                size_t length = sprintf(symbol, format, cname, selector);
                pString symbolString = String_new(symbol, length);

                bool loaded = (bool) SEND(primitive_map, get, symbolString);


                if (loaded) {
                    // we already loaded this symbol
                    continue;
                }

                // try loading the primitive
                routine = (routine_fn)dlsym(handle, symbol);
                SEND(primitive_map, put, symbolString, (void*)true);
            }
            
            if(!routine && class == target) {
                debug_error("could not load primitive '%s' for class %s\n"
                            "ERR: routine not in library\n",
                            selector,
                            cname);
                Universe_exit(ERR_FAIL);
            }

            if (routine && class != target) {
                // need new invokable in the target class, because we are loading
                // the primitive for a method defined in a superclass
                the_primitive = VMPrimitive_get_empty_primitive(sig);
                SEND(target, add_instance_primitive, the_primitive, false);
            }

            if (routine) {
                SEND(the_primitive, set_routine, routine);
                the_primitive->empty = false;
            }

            internal_free((void*) selector);
        }
    }
}

static void init_lib(void* dlhandle) {
    // ...and is responsible:
    // explicitely call the libirary initializer
    void* init_csp = dlsym(dlhandle, "init_csp");
    if (init_csp) {
        ((init_csp_fn)init_csp)();
    } else {
        dlclose(dlhandle);
        Universe_error_exit("Library does not define the init_csp() initializer.");
    }
}

/**
 * Load all primitives for the class given _and_ its metaclass
 */
void _VMClass_load_primitives(void* _self, const pString* cp, size_t cp_count) {
    pVMClass self = (pVMClass)_self;
    // the library handle
    void* dlhandle = NULL;
    // cached object properties
    const char* cname = SEND(self->name, get_rawChars);
    size_t cnameLen = SEND(self->name, get_length);

    // iterate the classpathes
    for (size_t i = 0; (i < cp_count) && !dlhandle; i++) {

        // check the core library
        pString loadstring = gen_core_loadstring(cp[i]);
        dlhandle = load_lib(loadstring);
        SEND(loadstring, free);
        if (dlhandle && is_responsible(dlhandle, cname)) {
            // the core library is found and responsible
            init_lib(dlhandle);
            break;
        }
        
        // the core library is not found or respondible, 
        // continue w/ class file
        loadstring = gen_loadstring(cp[i], cname, cnameLen);
        dlhandle = load_lib(loadstring);
        SEND(loadstring, free);
        if (dlhandle) {
            // the class library was found...
            if (is_responsible(dlhandle, cname)) {
                init_lib(dlhandle);
                break;
            } else {
                // ... but says not responsible, but have to
                // close it
                dlclose(dlhandle);
                Universe_error_exit("Library claims no responsiblity, "
                                    "but musn't!");        
            }
        }// continue checking the next class path
    }

    // finished cycling,
    // check if a lib was found.
    if(!dlhandle) {
        debug_error("load failure: %s\n", dlerror());
        debug_error("could not load primitive library for %s\n", cname);
        Universe_exit(ERR_FAIL);
    }
    // do the actual loading for both class and metaclass
    set_primitives(self, dlhandle, cname, INSTANCE_METHOD_FORMAT_S, self);
    set_primitives(self->class, dlhandle, cname, CLASS_METHOD_FORMAT_S, self->class);

    // do load
    pVMClass clazz = self;
    while (SEND(clazz, has_super_class)) {
        clazz = SEND(clazz, get_super_class);
        set_primitives(clazz, dlhandle, cname, INSTANCE_METHOD_FORMAT_S, self);
    }

}


void _VMClass_mark_references(void* _self) {
    pVMClass self = (pVMClass) _self;
    gc_mark_object(self->super_class);
    gc_mark_object(self->name);
    gc_mark_object(self->instance_fields);
    gc_mark_object(self->instance_invokables);
    SUPER(VMObject, self, mark_references);
}


//
// The VTABLE-function
//
//
static VTABLE(VMClass) _VMClass_vtable;
bool VMClass_vtable_inited = false;

VTABLE(VMClass)* VMClass_vtable(void) {
    if(VMClass_vtable_inited) 
        return &_VMClass_vtable;

    *((VTABLE(VMObject)*)&_VMClass_vtable) = *VMObject_vtable();
    _VMClass_vtable.init            = METHOD(VMClass, init);        
    _VMClass_vtable.free            = METHOD(VMClass, free);
    _VMClass_vtable.get_super_class = METHOD(VMClass, get_super_class);
    _VMClass_vtable.set_super_class = METHOD(VMClass, set_super_class);
    _VMClass_vtable.has_super_class = METHOD(VMClass, has_super_class);
    _VMClass_vtable.get_name        = METHOD(VMClass, get_name);
    _VMClass_vtable.set_name        = METHOD(VMClass, set_name);
    _VMClass_vtable.get_instance_fields = METHOD(VMClass, get_instance_fields);
    _VMClass_vtable.set_instance_fields = METHOD(VMClass, set_instance_fields);
    _VMClass_vtable.get_instance_invokables = 
        METHOD(VMClass, get_instance_invokables);
    _VMClass_vtable.set_instance_invokables = 
        METHOD(VMClass, set_instance_invokables);
    _VMClass_vtable.get_number_of_instance_invokables =
        METHOD(VMClass, get_number_of_instance_invokables);
    _VMClass_vtable.get_instance_invokable = 
        METHOD(VMClass, get_instance_invokable);
    _VMClass_vtable.set_instance_invokable =
        METHOD(VMClass, set_instance_invokable);
    _VMClass_vtable.lookup_invokable = METHOD(VMClass, lookup_invokable);
    _VMClass_vtable.lookup_field_index = METHOD(VMClass, lookup_field_index);
    _VMClass_vtable.add_instance_invokable = 
        METHOD(VMClass, add_instance_invokable);
    _VMClass_vtable.add_instance_primitive =
        METHOD(VMClass, add_instance_primitive);
    _VMClass_vtable.get_instance_field_name =
        METHOD(VMClass, get_instance_field_name);
    _VMClass_vtable.get_number_of_instance_fields =
        METHOD(VMClass, get_number_of_instance_fields);
    _VMClass_vtable.has_primitives    = METHOD(VMClass, has_primitives);
    _VMClass_vtable.load_primitives   = METHOD(VMClass, load_primitives);
    
    _VMClass_vtable.mark_references = 
        METHOD(VMClass, mark_references);

    VMClass_vtable_inited = true;
    
    return &_VMClass_vtable;
}

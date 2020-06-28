#ifndef UNIVERSE_H_
#define UNIVERSE_H_

/*
 * $Id: Universe.h 792 2009-04-06 08:07:33Z michael.haupt $
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
#include <misc/List.h>
#include <misc/debug.h>

#include <vmobjects/OOObject.h>

#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  Global objects  */
extern pVMObject nil_object;
extern pVMObject true_object;
extern pVMObject false_object;
  
extern pVMClass object_class;
extern pVMClass class_class;
extern pVMClass metaclass_class;
  
extern pVMClass nil_class;
extern pVMClass integer_class;
extern pVMClass array_class;
extern pVMClass method_class;
extern pVMClass symbol_class;
extern pVMClass primitive_class;
extern pVMClass string_class;
extern pVMClass system_class;
extern pVMClass block_class;
extern pVMClass double_class;

extern pVMClass true_class;
extern pVMClass false_class;

extern pVMSymbol doesNotUnderstand_sym;
extern pVMSymbol unknownGlobal_sym;
extern pVMSymbol escapedBlock_sym;
extern pVMSymbol run_sym;


// for runtime debug
extern short dump_bytecodes;
extern short gc_verbosity;
 
 
void          Universe_exit(int)                        __attribute__((noreturn));
void          Universe_error_exit(const char* restrict) __attribute__((noreturn));

void          Universe_assert(bool);

pVMSymbol     Universe_symbol_for_str(pString restrict);
pVMSymbol     Universe_symbol_for_chars(const char* restrict, size_t);
pVMSymbol     Universe_symbol_for_cstr(const char* restrict);

pVMArray      Universe_new_array(int64_t);
pVMArray      Universe_new_array_list(pList list);
pVMArray      Universe_new_array_from_argv(int, const char**);
pVMBlock      Universe_new_block(pVMMethod, pVMFrame, int64_t);
pVMClass      Universe_new_class(pVMClass);
pVMFrame      Universe_new_frame(pVMFrame, pVMMethod, pVMFrame);
pVMMethod     Universe_new_method(pVMSymbol, size_t, size_t, size_t, size_t);
pVMObject     Universe_new_instance(pVMClass);
pVMInteger    Universe_new_integer(int64_t);
pVMDouble     Universe_new_double(double);
pVMClass      Universe_new_metaclass_class(void);

pVMString     Universe_new_string_cstr(const char*);
pVMString     Universe_new_string_str(pString);
pVMString     Universe_new_string_string(const char* restrict, size_t);
pVMString     Universe_new_string_concat(pVMString, pVMString);

pVMSymbol     Universe_new_symbol(pString);
pVMClass      Universe_new_system_class(void);

void          Universe_initialize_system_class(pVMClass, pVMClass, const char*);

pHashmap      Universe_get_globals_dictionary(void);
pVMObject     Universe_get_global(pVMSymbol);
void          Universe_set_global(pVMSymbol, pVMObject);
bool          Universe_has_global(pVMSymbol);

pVMClass      Universe_get_block_class(void);
pVMClass      Universe_get_block_class_with_args(int64_t);

pVMClass      Universe_load_class(pVMSymbol);
void          Universe_load_system_class(pVMClass);
pVMClass      Universe_load_class_basic(pVMSymbol, pVMClass);
pVMClass      Universe_load_shell_class(const char*);

const char**  Universe_handle_arguments(int* vm_argc, int argc,
                                        const char** argv);

void          Universe_set_classpath(const char* classpath);
void          Universe_start(int argc, const char** argv);
pVMObject     Universe_interpret(const char* class_name, const char* method_name);
void          Universe_destruct(void);

#endif // UNIVERSE_H_

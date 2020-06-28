/*
 * $Id: Universe.c 792 2009-04-06 08:07:33Z michael.haupt $
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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
 
#include "Universe.h"
#include "Shell.h"

#include <memory/gc.h>

#include <misc/Hashmap.h>
#include <misc/List.h>

#include <vmobjects/VMFrame.h>
#include <vmobjects/VMMethod.h>
#include <vmobjects/VMArray.h>
#include <vmobjects/VMInvokable.h>
#include <vmobjects/VMBlock.h>
#include <vmobjects/VMInteger.h>
#include <vmobjects/VMDouble.h>
#include <vmobjects/VMString.h>
#include <vmobjects/Symboltable.h>

#include <compiler/SourcecodeCompiler.h>
#include <compiler/Disassembler.h>

#include <interpreter/bytecodes.h>
#include <interpreter/Interpreter.h>

#include <misc/debug.h>

// Here we go:
// externally refenced variables:
pVMObject nil_object;
pVMObject true_object;
pVMObject false_object;

pVMClass object_class;
pVMClass class_class;
pVMClass metaclass_class;

pVMClass nil_class;
pVMClass integer_class;
pVMClass array_class;
pVMClass method_class;
pVMClass symbol_class;
pVMClass primitive_class;
pVMClass string_class;
pVMClass system_class;
pVMClass block_class;
pVMClass double_class;

pVMClass true_class;
pVMClass false_class;

pVMSymbol doesNotUnderstand_sym;
pVMSymbol unknownGlobal_sym;
pVMSymbol escapedBlock_sym;
pVMSymbol run_sym;


//
// values for debug output verbosity levels (controllable via the command line)
//


short dump_bytecodes;
short gc_verbosity;


// private helper functions
static int  setup_class_path(pString cp);
static int  add_class_path(const pString restrict cp);
static void print_usage_and_exit(const char* executable);
static int  get_path_class_ext(pString **tokens, pString arg);

//file-local variables
static pString* class_path=NULL;
static size_t cp_count=0;
static pHashmap globals_dictionary=NULL;

///////////////////////////////////
#pragma mark private hepler functions

static int get_path_class_ext(pString **tokens, pString arg) {   
#define EXT_TOKENS 2    

    // index last path/file separator
    intptr_t fp_index = SEND(arg, lastIndexOfChar, *file_separator);
    // index of filename/suffix separator
    intptr_t ssep_index = SEND(arg, lastIndexOfChar, '.');
    
    if(fp_index < 0) { //no new path
        return ERR_FAIL;    
    } else {
        // copy path token
        // alloc result String array
        *tokens = (pString *)internal_allocate(EXT_TOKENS * sizeof(pString));

        (*tokens)[0] = SEND(arg, substring, 0, fp_index - 1);
    }
    
    // copy filename to 2nd slot
    // strip suffix if present
    ssep_index = ( (ssep_index >= 0)  && (ssep_index > fp_index)) ?
                 (ssep_index - 1):
                 SEND(arg, length);
    // copy substring
    (*tokens)[1] = SEND(arg, substring, fp_index + 1, ssep_index);
    
    return ERR_SUCCESS;
}


/*
 *  setup 'path_separator'-separated Classpathes
 *  from cp
 */
static int setup_class_path(const pString restrict cp) {
    /*
     * affected globals:
     * affected file globals: class_path, cp_count
     */
    
    size_t count;
    if ((class_path = SEND(cp, tokenize, &count, path_separator))) {
        cp_count = count;
        SEND(cp, free);
        return ERR_SUCCESS;
    } else
        return ERR_NOMEM;
}


/**
 * add cp to the class_path vector
 */
static int add_class_path(const pString restrict cp) {
    /*
     * affected    globals:
     * affected file globals: class_path, cp_count
     */
    
    // resize class_path-vector
    class_path = realloc(class_path, (cp_count + 1) * sizeof(pString));
    if(!class_path) 
        return ERR_NOMEM;
    // copy it and increment
    class_path[cp_count++] = cp;
    
    return ERR_SUCCESS;    
}


static void print_usage_and_exit(const char* executable) {
    // print the usage
    fprintf(stderr, "Usage: %s [-options] [args...]\n\n", executable);
    fprintf(stderr, "where options include:\n");
    fprintf(stderr, "    -cp <directories separated by %s>\n", path_separator);
    fprintf(stderr, "        set search path for application classes\n");
    fprintf(stderr, "    -d  enable disassembling (twice for tracing)\n");
    fprintf(stderr, "    -g  enable garbage collection details:\n" \
                    "        1x - print statistics when VM shuts down\n" \
                    "        2x - print statistics upon each collection\n" \
                    "        3x - print statistics and dump heap upon each " \
                    "collection\n");
    fprintf(stderr, "    -Hx set the heap size to x MB (default: 1 MB)\n");
    fprintf(stderr, "    -h  show this help\n");
    // exit
    Universe_exit(ERR_SUCCESS);
}

////////////////

#pragma mark Extern Callable Functions

void Universe_exit(int err) {
    if(gc_verbosity > 0)
        gc_stat();
    Universe_destruct();
    exit(err);
}


void Universe_error_exit(const char* restrict err) {
    debug_error("Runtime error: %s\n", err);
    Universe_exit(ERR_FAIL);
}


void Universe_set_classpath(const char* classpath) {
    setup_class_path(String_new(classpath, strlen(classpath)));
}


const char** Universe_handle_arguments(
    int* vm_argc, int argc, const char** argv
) {
    int vm_arg_start = 0;
    *vm_argc = 0;
    
    dump_bytecodes = 0;
    gc_verbosity = 0;
    
    // iterate over arguments (argv[>0])
    for(int i = 1; i < argc ; i++) {
        if(strcmp(argv[i], "-cp") == 0) { // Classpath
            if((argc == i+1) // no CP given
                 || class_path // CP already set up.
                 ) 
                print_usage_and_exit(argv[0]);
            
            // setup & skip class path
            Universe_set_classpath(argv[++i]);

        } else if(strcmp(argv[i], "-d") == 0) { // Dump  bytecode
            dump_bytecodes++;
        } else if(strcmp(argv[i], "-g") == 0) {
            gc_verbosity++;
        } else if(argv[i][0] == '-' && argv[i][1] == 'H') {
            int heap_size = atoi(argv[i] + 2);
            gc_set_heap_size(heap_size);
        } else if((strcmp(argv[i], "-h") == 0) ||
            (strcmp(argv[i], "--help") == 0)
        ) {
            print_usage_and_exit(argv[0]);
        } else {
            // set "VM arguments start here" value
            if(vm_arg_start == 0)
                vm_arg_start = i;
            (*vm_argc)++;
            
            // copy remaining args for VM                        
            // temp vector for path
            pString* ext_path_tokens = NULL;
            pString tmp_string = String_new(argv[i], strlen(argv[i]));
            if(get_path_class_ext(&ext_path_tokens, tmp_string) ==
               ERR_SUCCESS
            ) {
                // got additional path, so add it
                add_class_path(ext_path_tokens[0]);
                                
                // copy filename back to args
                argv[i] = strndup(SEND(ext_path_tokens[1], rawChars), SEND(ext_path_tokens[1], length));
                SEND(ext_path_tokens[1], free);

                // guaranteed not to be used/referenced any more
                internal_free(ext_path_tokens);
            } else {
                argv[i] = strdup(argv[i]);
            }
            // will not be referenced, because copied
            SEND(tmp_string, free);
        }
    }

    // Add local dir to class_path
    add_class_path(String_new(".", 1));
    
    return argv + vm_arg_start;
}


static pVMObject initialize_object_system() {
    /*
     * affected file globals: globals_dictionary
     */
    VMClass_init_primitive_map();

    // setup the Hashmap for all globals
    globals_dictionary = Hashmap_new();
    // init the Symboltable
    Symbol_table_init();

    ///////////////////////////////////
    //     allocate the nil object
    nil_object = VMObject_new();

    // allocate the Metaclass classes
    metaclass_class = Universe_new_metaclass_class();
    // allocate the rest of the system classes
    object_class    = Universe_new_system_class();
    nil_class       = Universe_new_system_class();
    class_class     = Universe_new_system_class();
    array_class     = Universe_new_system_class();
    symbol_class    = Universe_new_system_class();
    method_class    = Universe_new_system_class();
    integer_class   = Universe_new_system_class();
    primitive_class = Universe_new_system_class();
    string_class    = Universe_new_system_class();
    double_class    = Universe_new_system_class();

    // setup the class reference for the nil object
    SEND(nil_object, set_class, nil_class);

    // initialize the system classes.
    Universe_initialize_system_class(object_class, NULL, "Object");
    Universe_initialize_system_class(class_class, object_class, "Class");
    Universe_initialize_system_class(metaclass_class, class_class, "Metaclass");
    Universe_initialize_system_class(nil_class, object_class, "Nil");
    Universe_initialize_system_class(array_class, object_class, "Array");
    Universe_initialize_system_class(method_class, array_class, "Method");
    Universe_initialize_system_class(integer_class, object_class, "Integer");
    Universe_initialize_system_class(primitive_class, object_class, "Primitive");
    Universe_initialize_system_class(string_class, object_class, "String");
    Universe_initialize_system_class(symbol_class, string_class, "Symbol");
    Universe_initialize_system_class(double_class, object_class, "Double");

    // load methods and fields into the system classes
    Universe_load_system_class(object_class);
    Universe_load_system_class(class_class);
    Universe_load_system_class(metaclass_class);
    Universe_load_system_class(nil_class);
    Universe_load_system_class(array_class);
    Universe_load_system_class(method_class);
    Universe_load_system_class(symbol_class);
    Universe_load_system_class(integer_class);
    Universe_load_system_class(primitive_class);
    Universe_load_system_class(string_class);
    Universe_load_system_class(double_class);

    // load the generic block class
    block_class = Universe_load_class(Universe_symbol_for_cstr("Block"));

    // setup the true and false objects
    pVMSymbol trueSymbol = Universe_symbol_for_cstr("True");
    true_class  = Universe_load_class(trueSymbol);
    true_object = Universe_new_instance(true_class);

    pVMSymbol falseSymbol = Universe_symbol_for_cstr("False");
    false_class  = Universe_load_class(falseSymbol);
    false_object = Universe_new_instance(false_class);

    // load the system class and create an instance of it
    system_class = Universe_load_class(Universe_symbol_for_cstr("System"));
    pVMObject system_object = Universe_new_instance(system_class);

    // put special objects and classes into the dictionary of globals
    Universe_set_global(Universe_symbol_for_cstr("nil"), nil_object);
    Universe_set_global(Universe_symbol_for_cstr("true"), true_object);
    Universe_set_global(Universe_symbol_for_cstr("false"), false_object);
    Universe_set_global(Universe_symbol_for_cstr("system"), system_object);
    Universe_set_global(Universe_symbol_for_cstr("System"),
                        (pVMObject)system_class);
    Universe_set_global(Universe_symbol_for_cstr("Block"),
                        (pVMObject)block_class);

    Universe_set_global(trueSymbol,  (pVMObject) true_class);
    Universe_set_global(falseSymbol, (pVMObject) false_class);

    // initialize symbols required by the interpreter
    doesNotUnderstand_sym = Universe_symbol_for_cstr("doesNotUnderstand:arguments:");
    unknownGlobal_sym = Universe_symbol_for_cstr("unknownGlobal:");
    escapedBlock_sym = Universe_symbol_for_cstr("escapedBlock:");
    run_sym = Universe_symbol_for_cstr("run:");

    return system_object;
}


// create a fake bootstrap method to simplify later frame traversal
static pVMMethod create_bootstrap_method() {
    pVMMethod bootstrap_method =
        Universe_new_method(Universe_symbol_for_cstr("bootstrap"), 1, 0, 0, 2);
    SEND(bootstrap_method, set_bytecode, 0, BC_HALT);
    TSEND(VMInvokable, bootstrap_method, set_holder, system_class);
    return bootstrap_method;
}


pVMObject Universe_interpret(const char* class_name, const char* method_name) {
    gc_initialize();
    initialize_object_system();
    pVMMethod bootstrap_method = create_bootstrap_method();

    // lookup the class and method
    pVMClass class = Universe_load_class(Universe_symbol_for_cstr(class_name));
    pVMObject method = (pVMObject)SEND(SEND(class, get_class), lookup_invokable,
                                       Universe_symbol_for_cstr(method_name));

    Universe_assert(method != NULL);

    // create a fake bootstrap frame with the system object on the stack
    Interpreter_initialize(nil_object);
    pVMFrame bootstrap_frame = Interpreter_push_new_frame(bootstrap_method, (pVMFrame) nil_object);
    SEND(bootstrap_frame, push, (pVMObject)class);

    // invoke the method on the class object
    TSEND(VMInvokable, method, invoke, bootstrap_frame);

    // start the interpreter
    Interpreter_start();

    return SEND(bootstrap_frame, pop);
}


void Universe_start(int argc, const char** argv) {
    gc_initialize();
    pVMObject system_object = initialize_object_system();
    pVMMethod bootstrap_method = create_bootstrap_method();

    // start the shell if no filename is given
    if(argc == 0) {
      Shell_set_bootstrap_method(bootstrap_method);
      Shell_start();
      return;
    }

    /* only trace bootstrap if the number of cmd-line "-d"s is > 2 */
    int trace = 2 - dump_bytecodes;
    if(!(trace > 0)) dump_bytecodes = 1;
    
    // convert the arguments into an array
    pVMArray arguments_array = Universe_new_array_from_argv(argc, argv);
        
    // create a fake bootstrap frame with the system object on the stack
    Interpreter_initialize(nil_object);
    pVMFrame bootstrap_frame = Interpreter_push_new_frame(bootstrap_method, (pVMFrame) nil_object);
    SEND(bootstrap_frame, push, system_object);
    SEND(bootstrap_frame, push, (pVMObject)arguments_array);
            
    // lookup the initialize invokable on the system class
    pVMObject initialize =
        (pVMObject)SEND(system_class, lookup_invokable,
                        Universe_symbol_for_cstr("initialize:"));
        
    // invoke the initialize invokable
    TSEND(VMInvokable, initialize, invoke, bootstrap_frame);

    // reset "-d" indicator
    if(!(trace>0)) dump_bytecodes = 2 - trace;
    
    // start the interpreter
    Interpreter_start();
    
    gc_finalize();
}


/**
 * Universe Destructor
 * Free "member" variables
 */    
void Universe_destruct(void) {
    /*
     * affected    globals:
     * affected file globals: class_path, globals_dictionary
     */

    Symbol_table_destruct();
    if(globals_dictionary)
        SEND(globals_dictionary, free);
    
    for(size_t i = 0; i < cp_count; i++)
        SEND(class_path[i], free);
    internal_free(class_path);
}


void Universe_assert(bool value) {
    if(!value)
      // for now we just print something whenever an assertion fails
      debug_error("Assertion failed!\n");
}


pVMSymbol Universe_symbol_for_str(pString restrict string) {
    // Lookup the symbol in the symbol table
    pVMSymbol result = Symbol_table_lookup(string);
    
    // return found or newly cerated symbol
    return result ? : Universe_new_symbol(string); //new
}


pVMSymbol Universe_symbol_for_chars(const char* restrict chars, size_t length) {
    return Universe_symbol_for_str(String_new(chars, length));
}


pVMSymbol Universe_symbol_for_cstr(const char* restrict chars) {
    return Universe_symbol_for_chars(chars, strlen(chars));
}


pVMArray Universe_new_array(int64_t size) {
    // Allocate a new array and set its class to be the array class
    pVMArray result = VMArray_new((size_t)size);
    SEND((pVMObject)result, set_class, array_class);
    
    // Return the freshly allocated array
    return result;
}


pVMArray Universe_new_array_list(pList list) {
    // Allocate a new array with the same length as the list
    size_t size = SEND(list, size);
    pVMArray result = Universe_new_array(size);
    
    if(result) {
        // Copy all elements from the list into the array
        for(size_t i = 0; i < size; i++) {
            pVMObject elem =  (pVMObject)SEND(list, get, i);
            SEND(result, set_indexable_field, i, elem);
        }
    }    
    // Return the allocated and initialized array
    return result;
}


pVMArray Universe_new_array_from_argv(int argc, const char** argv) {
    // allocate a new array with the same length as the string array
    pVMArray result = Universe_new_array(argc);
    // copy all elements from the string array into the array

    for(int i = 0; i < argc; i++)
        SEND(result, set_indexable_field, i,
            (pVMObject)Universe_new_string_cstr(argv[i]));

    // return the allocated and initialized array
    return result;
}


pVMBlock Universe_new_block(pVMMethod method, pVMFrame context, int64_t arguments) {
    // Allocate a new block and set its class to be the block class
    pVMBlock result = VMBlock_new(method, context);
    SEND((pVMObject)result, set_class,
         Universe_get_block_class_with_args(arguments));
    
    // Return the freshly allocated block
    return result;
}


pVMClass Universe_new_class(pVMClass class_of_class) {
    // Allocate a new class and set its class to be the given class class
    intptr_t num_fields = SEND(class_of_class, get_number_of_instance_fields);
    pVMClass result;
    
    if(num_fields) //this is a normal class as class class
        result = VMClass_new_num_fields(num_fields);        
    else // ths class_of_class has no fields
        result = VMClass_new();

    SEND((pVMObject)result, set_class, class_of_class);
    
    // Return the freshly allocated class
    return result;
}


pVMFrame Universe_new_frame(pVMFrame previous_frame, pVMMethod method, pVMFrame context) {
    // Compute the maximum number of stack locations (including arguments,
    // locals and extra buffer to support doesNotUnderstand) and set the number
    // of indexable fields accordingly
    // + 3 for the use by #doesNotUnderstand and #escapedBlock
    int64_t length = SEND(method, get_number_of_arguments) +
                     SEND(method, get_number_of_locals) +
                     SEND(method, get_maximum_number_of_stack_elements) + 3;
    
    // Allocate a new frame and set its class to be the frame class
    pVMFrame result = VMFrame_new(length, method, context, previous_frame);
    SEND((pVMObject)result, set_class, (pVMClass)nil_object);
    
    // Reset the stack pointer and the bytecode index
    SEND(result, reset_stack_pointer);
    SEND(result, set_bytecode_index, 0);
    
    // Return the freshly allocated frame
    return result;
}


pVMMethod Universe_new_method(pVMSymbol signature, size_t number_of_bytecodes,
                              size_t number_of_constants, size_t number_of_locals,
                              size_t max_number_of_stack_elements) {
    // Allocate a new method and set its class to be the method class
    pVMMethod result = VMMethod_new(number_of_bytecodes, number_of_constants,
                                    number_of_locals,
                                    max_number_of_stack_elements, signature);
    SEND((pVMObject)result, set_class, method_class);
    
    // Return the freshly allocated method
    return result;
}


pVMObject Universe_new_instance(pVMClass instance_class) {
    // Allocate a new instance and set its class to be the given class
    pVMObject result = VMObject_new_num_fields(
        SEND(instance_class, get_number_of_instance_fields));
    SEND(result, set_class, instance_class);
    
    // Return the freshly allocated instance
    return result;
}


pVMInteger Universe_new_integer(int64_t value) {
    // Allocate a new integer and set its class to be the integer class
    pVMInteger result = VMInteger_new_with(value);
    SEND((pVMObject)result, set_class, integer_class);
    
    // Return the freshly allocated integer
    return result;
}


pVMDouble Universe_new_double(double value) {
    // Allocate a new integer and set its class to be the double class
    pVMDouble result = VMDouble_new_with(value);
    SEND((pVMObject)result, set_class, double_class);
    
    // Return the freshly allocated double
    return result;
}


pVMClass Universe_new_metaclass_class(void) {
    // Allocate the metaclass classes
    pVMClass result = VMClass_new();
    SEND((pVMObject)result, set_class, VMClass_new());
    
    // Setup the metaclass hierarchy
    pVMObject mclass = (pVMObject)SEND((pVMObject)result, get_class);
    SEND(mclass, set_class, result);
    
    // Return the freshly allocated metaclass class
    return result;
}


pVMString Universe_new_string_cstr(const char* cstring) {
    return Universe_new_string_string(cstring, strlen(cstring));
}


pVMString Universe_new_string_str(pString str) {
    return Universe_new_string_string(str->chars, str->length);
}


pVMString Universe_new_string_string(const char* restrict string, size_t length) {
    // Allocate a new string and set its class to be the string class
    pVMString result = VMString_new(string, length);
    SEND((pVMObject)result, set_class, string_class);

    return result;
}


pVMString Universe_new_string_concat(pVMString a, pVMString b) {
    pVMString result = VMString_new_concat(a, b);
    SEND((pVMObject)result, set_class, string_class);

    return result;
}


pVMSymbol Universe_new_symbol(pString string) {
    // Allocate a new symbol and set its class to be the symbol class
    pVMSymbol result = VMSymbol_new(string);
    SEND((pVMObject)result, set_class, symbol_class);
    
    // Insert the new symbol into the symbol table
    Symbol_table_insert(result);
    
    // Return the freshly allocated symbol
    return result;
}


pVMClass Universe_new_system_class(void) {
    // Allocate the new system class
    pVMClass system_class = VMClass_new();
    
    // Setup the metaclass hierarchy
    SEND((pVMObject)system_class, set_class, VMClass_new());
    pVMObject mclass = (pVMObject)SEND((pVMObject)system_class, get_class);
    SEND(mclass, set_class, metaclass_class);
    
    // Return the freshly allocated system class
    return system_class;
}


void Universe_initialize_system_class(pVMClass system_class,
    pVMClass super_class, const char* name
) {
    // Initialize the superclass hierarchy
    if(super_class != NULL) {
        SEND(system_class, set_super_class, super_class);
        pVMClass sys_class_class = SEND((pVMObject)system_class, get_class);
        pVMClass super_class_class = SEND((pVMObject) super_class, get_class);
        SEND(sys_class_class, set_super_class, super_class_class);
    } else {
        pVMClass sys_class_class = SEND((pVMObject)system_class, get_class);
        SEND(sys_class_class, set_super_class, class_class);
    }

    pVMClass sys_class_class = SEND((pVMObject)system_class, get_class);

    // Initialize the array of instance fields
    SEND(system_class, set_instance_fields, Universe_new_array(0));
    SEND(sys_class_class, set_instance_fields, Universe_new_array(0));

    // Initialize the array of instance invokables
    SEND(system_class, set_instance_invokables, Universe_new_array(0));
    SEND(sys_class_class, set_instance_invokables, Universe_new_array(0));
    
    // Initialize the name of the system class
    SEND(system_class, set_name, Universe_symbol_for_cstr(name));
    char* class_class_name =
        (char*)internal_allocate(strlen(name) + 6 + 1); // 6: " class"
    strcpy(class_class_name, name);
    strcat(class_class_name, " class");
    SEND(sys_class_class, set_name, Universe_symbol_for_cstr(class_class_name));
    internal_free(class_class_name);
    
    // Insert the system class into the dictionary of globals
    Universe_set_global(SEND(system_class, get_name), (pVMObject)system_class);
}


pHashmap Universe_get_globals_dictionary() {
    return globals_dictionary;
}


pVMObject Universe_get_global(pVMSymbol name) {
    // Return the global with the given name if it's in the dictionary of
    // globals
    if(Universe_has_global(name))
        return (pVMObject)SEND(globals_dictionary, get, name);
    
    // Global not found
    return NULL;
}


void Universe_set_global(pVMSymbol name, pVMObject value) {
    // Insert the given value into the dictionary of globals
    SEND(globals_dictionary, put, name, value);
}


bool Universe_has_global(pVMSymbol name) {
    // Returns if the Universe has a value for the global of the given name
    return SEND(globals_dictionary, contains_key, name);
}


pVMClass Universe_get_block_class(void) {
    return block_class;
}


pVMClass Universe_get_block_class_with_args(int64_t number_of_arguments) {
    // Compute the name of the block class with the given number of arguments
    char block_name[7];
    Universe_assert(number_of_arguments <10); // buffer overflow otherwise
    sprintf(block_name, "Block%lld", number_of_arguments);
    pVMSymbol name = Universe_symbol_for_cstr(block_name);
    
    // Lookup the specific block class in the dictionary of globals and return
    // it
    if(Universe_has_global(name))
        return (pVMClass)Universe_get_global(name);

    // Get the block class for blocks with the given number of arguments
    pVMClass result = Universe_load_class_basic(name, NULL);
    
    // Add the appropriate value primitive to the block class
    SEND(result, add_instance_primitive,
         VMBlock_get_evaluation_primitive(number_of_arguments), true);
    
    // Insert the block class into the dictionary of globals
    Universe_set_global(name, (pVMObject)result);
    
    // Return the loaded block class
    return result;
}


pVMClass Universe_load_class(pVMSymbol name) {
    // Check if the requested class is already in the dictionary of globals
    if (Universe_has_global(name)) 
        return (pVMClass)Universe_get_global(name);
    
    // Load the class
    pVMClass result = Universe_load_class_basic(name, NULL);
        
    // we fail silently, it is not fatal that loading a class failed
    if (!result) {
		return (pVMClass) nil_object;
    }

    // Load primitives (if necessary) and return the resulting class
    if (SEND(result, has_primitives) || SEND(result->class, has_primitives)) 
        SEND(result, load_primitives, class_path, cp_count);

    // Insert the class into the dictionary of globals
    Universe_set_global(name, (pVMObject)result);

    return result;
}


void Universe_load_system_class(pVMClass system_class) {
    // Load the system class
    pVMClass result =
        Universe_load_class_basic(SEND(system_class, get_name), system_class);
    
    // check class loading.
    if(!result) {
        pVMSymbol cname = SEND(system_class, get_name);
        debug_error("can't load system class:\t%s", SEND(cname, get_rawChars));
        Universe_exit(ERR_FAIL);
    }

    // Load primitives (if necessary) 
    if(SEND(result, has_primitives) || SEND(result->class, has_primitives)) 
        SEND(result, load_primitives, class_path, cp_count);
}


pVMClass Universe_load_class_basic(pVMSymbol name, pVMClass system_class) {
    debug_log("Universe_load_class_basic %s cp_count %zd\n", SEND(name, get_rawChars), cp_count);

    pVMClass result;
    // Try loading the class from all different paths
    for(int i = 0; i < cp_count; i++) {
        // Load the class from a file and return the loaded class
        result = SourcecodeCompiler_compile_class(SEND(class_path[i], rawChars),
                                                  SEND(class_path[i], length),
                                                  SEND(name, get_rawChars),
                                                  SEND(name, get_length),
                                                  system_class);
        if(result) {
            if(dump_bytecodes) {
                Disassembler_dump(SEND(result, get_class));
                Disassembler_dump(result);
            }
            // found class in class_path, so return it
            return result; 
        }
    }
    // The class could not be found.
    return NULL;
}


pVMClass Universe_load_shell_class(const char* stmt) {
    // Load the class from a stream and return the loaded class
    pVMClass result = SourcecodeCompiler_compile_class_string(stmt, NULL);
    if(dump_bytecodes)
        Disassembler_dump(result);
    return result;    
}

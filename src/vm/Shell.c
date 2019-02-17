/*
 * $Id: Shell.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "Shell.h"

#include <vm/Universe.h>

#include <vmobjects/VMMethod.h>
#include <vmobjects/VMClass.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMInvokable.h>

#include <interpreter/Interpreter.h>

#include <memory/gc.h>

#include <stdbool.h>
#include <string.h>

// maximal length of an input line from the shell
#define INPUT_MAX_SIZE BUFSIZ

// some constants for assembling Smalltalk code
#define SHELL_PREFIX "Shell_Class_"
#define SHELL_PART_1 " = (run: it = ( | tmp | tmp := ("
#define SHELL_PART_2 "). 'it = ' print. ^tmp println) )"


/*
 * The bootstrapping method, 
 * class variable.
 */
static pVMMethod bootstrap_method;


/**
 * Setter for bootstrap_method
 */
void Shell_set_bootstrap_method(pVMMethod bsm) {
    bootstrap_method = bsm;
}

/**
 * Getter for bootstrap_method
 */
pVMMethod Shell_get_bootstrap_method(void) {
    return bootstrap_method;
}


/**
 * Start the Shell
 */
void Shell_start() {
#define QUIT_CMD "system exit"
#define QUIT_CMD_L 11 + 1
    // the statement to evaluate
    char      stmt[INPUT_MAX_SIZE];
    size_t    bytecode_index, counter = 0;
    pVMFrame  current_frame;
    pVMClass  runClass;
    pVMObject it = nil_object; // last evaluation result.

    printf("SOM Shell. Type \"" QUIT_CMD "\" to exit.\n");

    // Create a fake bootstrap frame
    Interpreter_initialize(nil_object);
    current_frame = Interpreter_push_new_frame(Shell_get_bootstrap_method(),
                                               (pVMFrame) nil_object);
    // Remember the first bytecode index, e.g. index of the halt instruction
    bytecode_index = SEND(current_frame, get_bytecode_index);

    /**
     * Main Shell Loop
     */
    while(true) {
        // initialize empty strings
        char*  statement;
        char   inp[INPUT_MAX_SIZE];
        size_t in_len = 0;

        
        printf("---> ");
    
        // Read a statement from the keyboard
        // and avoid buffer overflow.
        do {
            if(!feof(stdin))
                inp[in_len++] = fgetc(stdin);
            else {
                if(in_len <= 1) {
                    strcpy(inp, QUIT_CMD);
                    in_len = QUIT_CMD_L + 1;
                }
                break;
            }
        } while((in_len < INPUT_MAX_SIZE) && (inp[in_len - 1] != '\n'));
        // undo last increment
        in_len--;
        
        if(!(in_len - 1)) // line empty
            continue;
        
        // strip newline.
        inp[in_len] = '\0';
        
        // Generate a temporary class with a run method
        sprintf(stmt, "%s%zd%s", SHELL_PREFIX, counter++, SHELL_PART_1);
        statement = (char*)internal_allocate(
                strlen(stmt) +
                strlen(inp) +
                strlen(SHELL_PART_2) +
                1
            );
        strcpy(statement, stmt);
        strcat(statement, inp);
        strcat(statement, SHELL_PART_2);

        // Compile and load the newly generated class
        if(! (runClass = Universe_load_shell_class(statement))) {
            debug_error("can't compile statement.");
            internal_free(statement);
            continue;            
        }
                    
        current_frame = Interpreter_get_frame();

        // Go back, so we will evaluate the bootstrap frames halt
        // instruction again
        SEND(current_frame, set_bytecode_index, bytecode_index);
        
        // Create and push a new instance of our class on the stack
        SEND(current_frame, push, Universe_new_instance(runClass));
        
        // Push the old value of "it" on the stack
        SEND(current_frame, push, it);
        
        // Lookup the run: method
        pVMObject initialize = 
            SEND(runClass, lookup_invokable, run_sym);
        
        // Invoke the run method
        TSEND(VMInvokable, initialize, invoke, current_frame);
        
        // Start the Interpreter
        Interpreter_start();
        
        // Save the result of the run method
        it = SEND(current_frame, pop);
        internal_free(statement);
    }
}

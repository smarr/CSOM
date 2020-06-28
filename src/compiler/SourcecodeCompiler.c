/*
 * $Id: SourcecodeCompiler.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "SourcecodeCompiler.h"
#include "Parser.h"
#include "GenerationContexts.h"

#include <memory/gc.h>

#include <vmobjects/VMClass.h>
#include <vmobjects/VMSymbol.h>

#include <stdio.h>

/* private functions */
static void show_compilation_error(const char* filename,
                                   const char* restrict message){
    if(filename) 
        debug_error("Compilation of %s.som failed\n", filename);
    else
        debug_error("Compilation failed\n");
    debug_error("\t%s\n\n", message);
}


// precondition: Parser is inited!
static pVMClass compile(Lexer* l, pVMClass system_class) {
    // context for class generation
    class_generation_context cgc;
    class_genc_init(&cgc);
    
    /*
     * the resulting class 
     * prepopulated, in case it is a system class
     */
    pVMClass result = system_class;   

    // parse class into 
    Parser_classdef(l, &cgc);
    
    // compile the class
    if(system_class == NULL)
        result = VMClass_assemble(&cgc);
    else
        VMClass_assemble_system_class(&cgc, result);
    
    // 
    // TODO: either case, we should check for errors
    // // Show the compilation error and return null
    // show_compilation_error(filename, e);
    // return NULL
    //  
    
    class_genc_release(&cgc);
    debug_log("Compilation Finished.\n");            
       
    // Return the freshly parsed class
    return result;
}


pVMClass SourcecodeCompiler_compile_class_string(const char* stream,     
                                                 pVMClass system_class) {
    Lexer* l = Parser_init_string(stream);
    pVMClass class = compile(l, system_class);
    internal_free(l);
    return class;
}


pVMClass SourcecodeCompiler_compile_class(const char* path,
                                          size_t pathLength,
                                          // ^^^ without file_separator!
                                          const char* filename,
                                          size_t filenameLength,
                                          pVMClass system_class) {
    pVMClass result = system_class;

    // filename to be created
    //const char* path_c = SEND(path, chars);
    char* fname = (char*)internal_allocate(
            pathLength +
            strlen(file_separator) +
            filenameLength +
            4 + // ".som"
            1
        );
    strncpy(fname, path, pathLength);
    strcat(fname, file_separator);
    strncat(fname, filename, filenameLength);
    strcat(fname, ".som");

    if (access(fname, F_OK & R_OK) == -1) {
        //file not found or not readable       
        debug_info("Unable to open specified classpath, trying next one.\n");
        debug_info("\t\tFile: %s\n", fname);
        
        // name no longer needed.
        internal_free(fname);
        return NULL;
    }
    
    debug_info("Loading Class from file: %s\n", fname);
    
    FILE* stream = fopen(fname, "r");
    // we want linebuffering
    setlinebuf(stream);
    
    // name no longer needed.
    internal_free(fname);

    // start compiling
    Lexer* l = Parser_init(stream, filename);
    result = compile(l, system_class);
    internal_free(l);
    fclose(stream);
    
    // Make sure the filename matches the class name
    pVMSymbol cname = SEND(result, get_name);

    if (CString_compare(filename, filenameLength,
                        SEND(cname, get_rawChars), SEND(cname, get_length)) != 0)  {
        // Show the compilation error and return null
        show_compilation_error(filename, 
                               "File name does not match class name");
        return NULL;        
    }
        
    // Return the freshly parsed class
    return result;
}

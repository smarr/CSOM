/*
 * $Id: System.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include <stdio.h>

#include <sys/time.h>

#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMString.h>
#include <vmobjects/VMInteger.h>

#include <vm/Universe.h>

#include <memory/gc.h>

#include "System.h"

//file variable
struct timeval _System_start_time = { 0, 0 };

void  _System_global_(pVMObject object, pVMFrame frame) {
    pVMSymbol arg = (pVMSymbol)SEND(frame, pop);
    pVMObject self __attribute__((unused))= SEND(frame, pop);
    pVMObject result = Universe_get_global(arg);
    
    SEND(frame, push, result?
                      result:nil_object);    
}


void  _System_global_put_(pVMObject object, pVMFrame frame) {
    pVMObject value = SEND(frame, pop);
    pVMSymbol arg = (pVMSymbol)SEND(frame, pop);
    Universe_set_global(arg, value);
}


void _System_hasGlobal_(pVMObject object, pVMFrame frame) {
    pVMSymbol arg = (pVMSymbol)SEND(frame, pop);
    SEND(frame, pop);

  if (Universe_has_global(arg)) {
    SEND(frame, push, true_object);
  } else {
    SEND(frame, push, false_object);
  }
}


void  _System_load_(pVMObject object, pVMFrame frame) {
    pVMSymbol arg = (pVMSymbol)SEND(frame, pop);
    pVMObject self __attribute__((unused)) = SEND(frame, pop);
    pVMClass result = Universe_load_class(arg);
    SEND(frame, push, result? (pVMObject)result:
                              nil_object);
   
}


void  _System_exit_(pVMObject object, pVMFrame frame) {
    pVMInteger err = (pVMInteger)SEND(frame, pop);
    int64_t err_no = SEND(err, get_embedded_integer);

    if (err_no != ERR_SUCCESS)
        SEND(frame, print_stack_trace);    
    Universe_exit((int32_t)err_no);
}


void  _System_printString_(pVMObject object, pVMFrame frame) {
    pVMString arg = (pVMString)SEND(frame, pop);
    printf("%s", SEND(arg, get_rawChars));
    fflush(stdout);
}


void  _System_printNewline(pVMObject object, pVMFrame frame) {
    printf("\n");
    fflush(stdout);
}


void  _System_time(pVMObject object, pVMFrame frame) {
    pVMObject self __attribute__((unused)) = SEND(frame, pop);
    struct timeval now;
    gettimeofday(&now, NULL);
    long long diff = 
        ((now.tv_sec - _System_start_time.tv_sec) * 1000) + //seconds
        ((now.tv_usec - _System_start_time.tv_usec) / 1000); // µseconds
    SEND(frame, push, (pVMObject)Universe_new_integer((int32_t)diff));
}

void  _System_ticks(pVMObject object, pVMFrame frame) {
    SEND(frame, pop);
    struct timeval now;
    gettimeofday(&now, NULL);
    
    int64_t ticks = ((now.tv_sec - _System_start_time.tv_sec) * 1000 * 1000) + //seconds
                    ((now.tv_usec - _System_start_time.tv_usec)); //µseconds
    SEND(frame, push, (pVMObject)Universe_new_integer(ticks));
}


void _System_fullGC(pVMObject object, pVMFrame frame) {
    SEND(frame, pop);
    gc_collect();
    SEND(frame, push, true_object);
}

void __System_init(void) {
    gettimeofday(&_System_start_time , NULL);
}

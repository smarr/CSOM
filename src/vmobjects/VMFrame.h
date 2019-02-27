#ifndef VMFRAME_H_
#define VMFRAME_H_

/*
 * $Id: VMFrame.h 227 2008-04-21 15:21:14Z michael.haupt $
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
 
#include <vmobjects/VMArray.h>
#include <vmobjects/VMInteger.h>

/**
 * Frame layout:
 *
 * +-----------------+
 * | Arguments       | 0 
 * +-----------------+
 * | Local Variables | <-- local_offset
 * +-----------------+
 * | Stack           | <-- stack_pointer
 * | ...             |
 * +-----------------+
 */

#pragma mark VTable definition

VTABLE(VMFrame) {

#define VMFRAME_VTABLE_FORMAT \
    VMARRAY_VTABLE_FORMAT; \
    pVMFrame  (*get_previous_frame)(void*); \
    void      (*clear_previous_frame)(void*); \
    bool      (*has_previous_frame)(void*); \
    bool      (*is_bootstrap_frame)(void*); \
    pVMFrame  (*get_context)(void*); \
    bool      (*has_context)(void*); \
    pVMFrame  (*get_context_level)(void*, int64_t); \
    pVMFrame  (*get_outer_context)(void*); \
    pVMMethod (*get_method)(void*); \
    pVMObject (*pop)(void*); \
    void      (*push)(void*, pVMObject); \
    void      (*reset_stack_pointer)(void*); \
    size_t    (*get_bytecode_index)(void*); \
    void      (*set_bytecode_index)(void*, size_t); \
    pVMObject (*get_stack_element)(void*, size_t); \
    void      (*set_stack_element)(void*, size_t, pVMObject); \
    pVMObject (*get_local)(void*, size_t, size_t); \
    void      (*set_local)(void*, size_t, size_t, pVMObject); \
    pVMObject (*get_argument)(void*, size_t, size_t); \
    void      (*set_argument)(void*, size_t, size_t, pVMObject); \
    void      (*print_stack_trace)(void*); \
    size_t    (*argument_stack_index)(void* frame, size_t index); \
    void      (*copy_arguments_from)(void* self, pVMFrame frame)
    
    VMFRAME_VTABLE_FORMAT;
};

#pragma mark class definition

#define FRAME_FORMAT \
    ARRAY_FORMAT; \
    pVMFrame   previous_frame; \
    pVMFrame   context; \
    pVMMethod  method; \
    size_t     stack_pointer; \
    size_t     bytecode_index; \
    size_t     local_offset

struct _VMFrame {
    VTABLE(VMFrame)* _vtable;
    FRAME_FORMAT;
};

#pragma mark class methods

pVMFrame VMFrame_new(size_t length, pVMMethod method, pVMFrame context, pVMFrame previous_frame);

#pragma mark vtable initialization

VTABLE(VMFrame)* VMFrame_vtable(void);

#endif // VMFRAME_H_

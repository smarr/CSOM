/*
 * $Id: Disassembler.c 792 2009-04-06 08:07:33Z michael.haupt $
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
#include <string.h>

#include "Disassembler.h"

#include <vm/Universe.h>

#include <interpreter/bytecodes.h>
#include <interpreter/Interpreter.h>

#include <vmobjects/VMArray.h>
#include <vmobjects/VMBlock.h>
#include <vmobjects/VMClass.h>
#include <vmobjects/VMDouble.h>
#include <vmobjects/VMEvaluationPrimitive.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMInteger.h>
#include <vmobjects/VMInvokable.h>
#include <vmobjects/VMMethod.h>
#include <vmobjects/VMObject.h>
#include <vmobjects/VMPrimitive.h>
#include <vmobjects/VMString.h>
#include <vmobjects/VMSymbol.h>
#include <vmobjects/Signature.h>

/** 
 * Dispatch an object to its content and write out
 */
static inline void _Disassembler_dispatch(pVMObject o) {
    //dispatch
    // can't switch() objects, so:
    if(!o) return; // NULL isn't interesting.
    else if(o == nil_object)
        debug_print("{Nil}");
    else if(o == true_object)
        debug_print("{True}");
    else if(o == false_object)
        debug_print("{False}"); 
    else if((pVMClass)o == system_class)
        debug_print("{System Class object}");
    else if((pVMClass)o == block_class)
        debug_print("{Block Class object}");
    else if(o == Universe_get_global(Universe_symbol_for_cstr("system")))
        debug_print("{System}");
    else {
        pVMClass c = SEND(o, get_class);
        if(c == string_class) {
            debug_print("\"%s\"", SEND((pVMString)o, get_rawChars));
        } else if(c == double_class)
            debug_print("%g", SEND((pVMDouble)o, get_embedded_double));
        else if(c == integer_class)
            debug_print("%lld", SEND((pVMInteger)o, get_embedded_integer));
        else if(c == symbol_class) {
            debug_print("#%s", SEND((pVMSymbol)o, get_rawChars));
        } else
            debug_print("address: %p", (void*)o);
    }
}

/**
 * Dump a class and all subsequent methods.
 */
void Disassembler_dump(pVMClass class) {
    for (int i = 0; i < SEND(class, get_number_of_instance_invokables); i++) {
        pVMObject inv = SEND(class, get_instance_invokable, i);
        // output header and skip if the Invokable is a Primitive
        pVMSymbol sig = TSEND(VMInvokable, inv, get_signature);
        const char* sig_s = SEND(sig, get_rawChars);
        pVMSymbol cname = SEND(class, get_name);
        const char* cname_s = SEND(cname, get_rawChars);
        debug_dump("%s>>%s = ", cname_s, sig_s);
        if (TSEND(VMInvokable, inv, is_primitive)) {
            debug_print("<primitive>\n");
            continue;
        }
        // output actual method
        Disassembler_dump_method((pVMMethod)inv, "\t");
    }
}


/**
 * Bytecode Index Accessor macros
 */
#define BC_0 SEND(method, get_bytecode,bc_idx)
#define BC_1 SEND(method, get_bytecode,bc_idx+1)
#define BC_2 SEND(method, get_bytecode,bc_idx+2)


/**
 * Dump all Bytecode of a method.
 */
void Disassembler_dump_method(pVMMethod method, const char* indent) {
    debug_print("(\n");
    {   // output stack information
        int64_t locals =SEND(method, get_number_of_locals);
        int64_t max_stack = SEND(method, get_maximum_number_of_stack_elements);
        debug_dump("%s<%d locals, %d stack>\n", indent, locals, max_stack);
    }
    // output bytecodes
    for(size_t bc_idx = 0; 
        bc_idx < SEND(method, get_number_of_bytecodes); 
        bc_idx += bytecodes_get_bytecode_length(
            SEND(method, get_bytecode, bc_idx))
    ) {
        // the bytecode.
        uint8_t bytecode = BC_0;
        // indent, bytecode index, bytecode mnemonic
        debug_dump("%s%4d:%s  ", indent, bc_idx,
            bytecodes_get_bytecode_name(bytecode));
        // parameters (if any)
        if(bytecodes_get_bytecode_length(bytecode) == 1) {
            debug_print("\n");
            continue;
        }
        switch(bytecode) {
            case BC_PUSH_LOCAL:
                debug_print("local: %d, context: %d\n", BC_1, BC_2); break;
            case BC_PUSH_ARGUMENT:
                debug_print("argument: %d, context %d\n", BC_1, BC_2); break;
            case BC_PUSH_FIELD:{
                pVMSymbol name = (pVMSymbol)SEND(method, get_constant, bc_idx);
                debug_print("(index: %d) field: %s\n", BC_1,
                    SEND(name, get_rawChars));
                break;
            }
            case BC_PUSH_BLOCK: {
                char nindent[strlen(indent)+1+1];
                debug_print("block: (index: %d) ", BC_1);
                sprintf(nindent, "%s\t", indent);
                Disassembler_dump_method(
                    (pVMMethod)SEND(method, get_constant, bc_idx), nindent);
                break;
            }            
            case BC_PUSH_CONSTANT: {
                pVMObject constant = SEND(method, get_constant, bc_idx);
                pVMClass class = SEND(constant, get_class);
                pVMSymbol cname = SEND(class, get_name);
                debug_print("(index: %d) value: (%s) ", 
                            BC_1, SEND(cname, get_rawChars));
                _Disassembler_dispatch(constant); debug_print("\n");
                break;
            }
            case BC_PUSH_GLOBAL: {
                pVMSymbol name = (pVMSymbol)SEND(method, get_constant, bc_idx);
                debug_print("(index: %d) value: %s\n", BC_1,
                            SEND(name, get_rawChars));
                break;
            }
            case BC_POP_LOCAL:
                debug_print("local: %d, context: %d\n", BC_1, BC_2);
                break;
            case BC_POP_ARGUMENT:
                debug_print("argument: %d, context: %d\n", BC_1, BC_2);
                break;
            case BC_POP_FIELD: {
                pVMSymbol name = (pVMSymbol)SEND(method, get_constant, bc_idx);
                debug_print("(index: %d) field: %s\n", BC_1,
                    SEND(name, get_rawChars));
                break;
            }
            case BC_SEND: {
                pVMSymbol name = (pVMSymbol)SEND(method, get_constant, bc_idx);
                debug_print("(index: %d) signature: %s\n", BC_1,
                    SEND(name, get_rawChars));
                break;
            }
            case BC_SUPER_SEND: {
                pVMSymbol name = (pVMSymbol)SEND(method, get_constant, bc_idx);
                debug_print("(index: %d) signature: %s\n", BC_1,
                    SEND(name, get_rawChars));
                break;
            }
            default:
                debug_print("<incorrect bytecode>\n");
        }
    }
    debug_dump("%s)\n", indent);
}

/**
 * Dump bytecode from the frame running
 */
void Disassembler_dump_bytecode(pVMFrame frame, pVMMethod method, size_t bc_idx) {
    static long long indentc = 0;
    static char      ikind   = '@';
    uint8_t          bc      = BC_0;
    pVMClass         class   = TSEND(VMInvokable, method, get_holder);
    
    // Determine Context: Class or Block?
    if(IS_A(class, VMClass)) {
        pVMSymbol cname = SEND(class, get_name);
        pVMSymbol sig = TSEND(VMInvokable, method, get_signature);
        
        debug_trace("%20s>>%-20s% 10lld %c %04d: %s\t",
                    SEND(cname, get_rawChars), SEND(sig, get_rawChars),
                    indentc, ikind, bc_idx,
                    bytecodes_get_bytecode_name(bc));        
    } else {
        pVMSymbol sig = TSEND(VMInvokable, method, get_signature);
        debug_trace("%-42s% 10lld %c %04d: %s\t", 
                    SEND(sig, get_rawChars),
                    indentc, ikind, bc_idx,
                    bytecodes_get_bytecode_name(bc));
    }
    // reset send indicator
    if(ikind != '@') ikind = '@';
    
    switch(bc) {
        case BC_HALT: {
            debug_print("<halting>\n\n\n");
            break;
        }
        case BC_DUP: {
            pVMObject o = SEND(frame, get_stack_element, 0);
            if(o) {
                pVMClass c = SEND(o, get_class);
                pVMSymbol cname = SEND(c, get_name);
                debug_print("<to dup: (%s) ", SEND(cname, get_rawChars));
                //dispatch
                _Disassembler_dispatch(o);
            } else
                debug_print("<to dup: address: %p", (void*)o);
            debug_print(">\n");                        
            break;
        }
        case BC_PUSH_LOCAL: {
            uint8_t bc1 = BC_1, bc2 = BC_2;
            pVMObject o = SEND(frame, get_local, bc1, bc2);
            pVMClass c = SEND(o, get_class);
            pVMSymbol cname = SEND(c, get_name);
            debug_print("local: %d, context: %d <(%s) ", 
                        BC_1, BC_2, SEND(cname, get_rawChars));
            //dispatch
            _Disassembler_dispatch(o);
            debug_print(">\n");                        
            break;
        }
        case BC_PUSH_ARGUMENT: {
            uint8_t bc1 = BC_1, bc2 = BC_2;
            pVMObject o = SEND(frame, get_argument, bc1, bc2);
            debug_print("argument: %d, context: %d", bc1, bc2);
            if(IS_A(class, VMClass)) {
                pVMClass c = SEND(o, get_class);
                pVMSymbol cname = SEND(c, get_name);
                debug_print("<(%s) ", SEND(cname, get_rawChars));
                //dispatch
                _Disassembler_dispatch(o);                
                debug_print(">");                        
            }            
            debug_print("\n");
            break;
        }
        case BC_PUSH_FIELD: {
            pVMFrame ctxt = SEND(frame, get_outer_context);
            pVMObject arg = SEND(ctxt, get_argument, 0, 0);
            pVMSymbol name = (pVMSymbol)SEND(method, get_constant, bc_idx);
            int64_t field_index = SEND((pVMObject)arg, get_field_index, name);
           
            pVMObject o = SEND(arg, get_field, field_index);
            pVMClass c = SEND(o, get_class);
            pVMSymbol cname = SEND(c, get_name);

            debug_print("(index: %d) field: %s <(%s) ", BC_1,
                        SEND(name, get_rawChars), SEND(cname, get_rawChars));
            //dispatch
            _Disassembler_dispatch(o);                
            debug_print(">\n");                        
            break;
        }
        case BC_PUSH_BLOCK: {
            debug_print("block: (index: %d) ", BC_1);
            pVMMethod meth = (pVMMethod)SEND(method, get_constant, bc_idx);
            Disassembler_dump_method(meth, "$");
            break;
        }
        case BC_PUSH_CONSTANT: {
            pVMObject constant = SEND(method, get_constant, bc_idx);
            pVMClass c = SEND(constant, get_class);
            pVMSymbol cname = SEND(c, get_name);
            debug_print("(index: %d) value: (%s) ", BC_1, 
                        SEND(cname, get_rawChars));
            _Disassembler_dispatch(constant);
            debug_print("\n");
            break;
        }
        case BC_PUSH_GLOBAL: {
            pVMSymbol   name = (pVMSymbol)SEND(method, get_constant, bc_idx);
            pVMObject   o = Universe_get_global(name);
            pVMSymbol   cname;
            const char* c_cname;
            if(o) {
                pVMClass c = SEND(o, get_class);
                cname = SEND(c, get_name);
                c_cname = SEND(cname, get_rawChars);
            } else
                c_cname = "NULL";
            
            debug_print("(index: %d)value: %s <(%s) ", BC_1,
                        SEND(name, get_rawChars), c_cname);
            _Disassembler_dispatch(o);
            debug_print(">\n");
            break;
        }
        case BC_POP: {
            size_t sp = frame->stack_pointer;
            pVMObject o = SEND((pVMArray)frame, get_indexable_field, sp);
            pVMClass c = SEND(o, get_class);
            pVMSymbol cname = SEND(c, get_name);
            debug_print("popped <(%s) ", SEND(cname, get_rawChars));
            //dispatch
            _Disassembler_dispatch(o);
            debug_print(">\n");                        
            break;            
        }            
        case BC_POP_LOCAL: {
            size_t sp = frame->stack_pointer;
            pVMObject o = SEND((pVMArray)frame, get_indexable_field, sp);
            pVMClass c = SEND(o, get_class);
            pVMSymbol cname = SEND(c, get_name);
            debug_print("popped local: %d, context: %d <(%s) ", BC_1, BC_2,
                        SEND(cname, get_rawChars));
            //dispatch
            _Disassembler_dispatch(o);            
            debug_print(">\n");                        
            break;            
        }
        case BC_POP_ARGUMENT: {
            size_t sp = frame->stack_pointer;
            pVMObject o = SEND((pVMArray)frame, get_indexable_field, sp);
            pVMClass c = SEND(o, get_class);
            pVMSymbol cname = SEND(c, get_name);
            debug_print("argument: %d, context: %d <(%s) ", BC_1, BC_2,
                        SEND(cname, get_rawChars));
            //dispatch
            _Disassembler_dispatch(o);
            debug_print(">\n");                        
            break;
        }
        case BC_POP_FIELD: {
            size_t sp = frame->stack_pointer;
            pVMObject o = SEND((pVMArray)frame, get_indexable_field, sp);
            pVMSymbol name = (pVMSymbol)SEND(method, get_constant, bc_idx);
            pVMClass c = SEND(o, get_class);
            pVMSymbol cname = SEND(c, get_name);
            debug_print("(index: %d) field: %s <(%s) ",  BC_1,
                        SEND(name, get_rawChars),
                        SEND(cname, get_rawChars));
            _Disassembler_dispatch(o);
            debug_print(">\n");                        
            break;
        }
        case BC_SUPER_SEND:
        case BC_SEND: {
            pVMSymbol sel = (pVMSymbol)SEND(method, get_constant, bc_idx);

            debug_print("(index: %d) signature: %s (", BC_1,
                        SEND(sel, get_rawChars));
            //handle primitives, they don't increase call-depth
            pVMObject elem = SEND(Interpreter_get_frame(), get_stack_element,
                                  Signature_get_number_of_arguments(sel));
            pVMClass elemClass = SEND(elem, get_class);
            pVMObject inv =  SEND(elemClass, lookup_invokable, sel);
            
            if(inv && TSEND(VMInvokable, inv, is_primitive)) 
                debug_print("*)\n");
            else {
                debug_print("\n");    
                indentc++; ikind='>'; // visual
            }
                break;
        }            
        case BC_RETURN_LOCAL:
        case BC_RETURN_NON_LOCAL: {
            debug_print(")\n");
            indentc--; ikind='<'; //visual
            break;
        }
        default:
            debug_print("<incorrect bytecode>\n");
            break;
    }
}

// EOF: diassembler.c


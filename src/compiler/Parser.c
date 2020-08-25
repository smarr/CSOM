/*
 * $Id: Parser.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "Lexer.h"
#include "Parser.h"

#include "GenerationContexts.h"
#include "BytecodeGeneration.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <misc/defs.h>

#include <memory/gc.h>

#include <vmobjects/VMClass.h>
#include <vmobjects/VMString.h>


/***** Grammar Documentation *****
This parser accepts valid programs adhering to the following grammar. Comments
and white space are not dealt with in the grammar. Names of non-terminals begin
with a lower-case letter; terminals, with an upper-case one.

classdef =
    Identifier Equal superclass
    instanceFields method*
    ( Separator classFields method* )?
    EndTerm

superclass =
    Identifier? NewTerm

instanceFields =
    ( Or Identifier* Or )?

classFields
    ( Or Identifier* Or )?

method =
    pattern Equal ( Primitive | methodBlock )

pattern =
    unaryPattern | keywordPattern | binaryPattern

unaryPattern =
    unarySelector

binaryPattern =
    binarySelector argument

keywordPattern =
    ( keyword argument )+

methodBlock =
    NewTerm blockContents EndTerm

unarySelector =
    identifier

binarySelector =
    Or | Comma | Minus | Equal | Not | And | Star | Div | Mod | Plus | More |
    Less | At | Per | OperatorSequence

identifier =
    Primitive | Identifier

keyword =
    Keyword

argument =
    variable

blockContents =
    ( Or locals Or )?
    blockBody

locals =
    variable*

blockBody =
      Exit result
    | expression ( Period blockBody )?

result =
    expression Period?

expression =
    assignation | evaluation

assignation =
    assignments evaluation

assignments =
    assignment+

assignment =
    variable Assign

evaluation =
    primary messages?

primary =
    variable | nestedTerm | nestedBlock | literal

variable =
    identifier

messages =
      unaryMessage+ binaryMessage* keywordMessage?
    | binaryMessage+ keywordMessage?
    | keywordMessage

unaryMessage =
    unarySelector

binaryMessage =
    binarySelector binaryOperand

binaryOperand =
    primary unaryMessage*

keywordMessage =
    ( keyword formula )+

formula =
    binaryOperand binaryMessage*

nestedTerm =
    NewTerm expression EndTerm

literal =
    literalSymbol | literalString | literalNumber

literalNumber =
    negativeDecimal | literalDecimal

literalDecimal =
    literalInteger

negativeDecimal =
    Minus literalInteger

literalInteger =
    Integer

literalSymbol =
    Pound ( string | selector )

literalString =
    string

selector =
    binarySelector | keywordSelector | unarySelector

keywordSelector =
    Keyword | KeywordSequence

string =
    STString

nestedBlock =
    NewBlock blockPattern? blockContents EndBlock

blockPattern =
    blockArguments Or

blockArguments =
    ( Colon argument )+

***** End of Grammar Documentation *****/


//
// prototypes and definitions
//


#pragma mark intern Prototypes


static bool symIn(Lexer* l, Symbol* ss);
static bool accept(Lexer* l, Symbol s);
static bool acceptOneOf(Lexer* l, Symbol* ss);
static bool expect(Lexer* l, Symbol s);
static bool expectOneOf(Lexer* l, Symbol* ss);
static void superclass(Lexer* l, class_generation_context* cgenc);
static void instanceFields(Lexer* l, class_generation_context* cgenc);
static void classFields(Lexer* l, class_generation_context* cgenc);
static void method(Lexer* l, method_generation_context* mgenc);
static void primitiveBlock(Lexer* l);
static void pattern(Lexer* l, method_generation_context* mgenc);
static void unaryPattern(Lexer* l, method_generation_context* mgenc);
static void binaryPattern(Lexer* l, method_generation_context* mgenc);
static void keywordPattern(Lexer* l, method_generation_context* mgenc);
static void methodBlock(Lexer* l, method_generation_context* mgenc);
static pVMSymbol unarySelector(Lexer* l);
static pVMSymbol binarySelector(Lexer* l);
static pString identifier(Lexer* l);
static pString keyword(Lexer* l);
static pString argument(Lexer* l);
static void blockContents(Lexer* l, method_generation_context* mgenc);
static void locals(Lexer* l, method_generation_context* mgenc);
static void blockBody(Lexer* l, method_generation_context* mgenc, bool seen_period);
static void result(Lexer* l, method_generation_context* mgenc);
static void expression(Lexer* l, method_generation_context* mgenc);
static void assignation(Lexer* lexer, method_generation_context* mgenc);
static void assignments(Lexer* lexer, method_generation_context* mgenc, pList l);
static pString assignment(Lexer* l, method_generation_context* mgenc);
static void evaluation(Lexer* l, method_generation_context* mgenc);
static bool primary(Lexer* l, method_generation_context* mgenc);
static pString variable(Lexer* l);
static void messages(Lexer* l, method_generation_context* mgenc, bool super);
static void unaryMessage(Lexer* l, method_generation_context* mgenc, bool super);
static void binaryMessage(Lexer* l, method_generation_context* mgenc, bool super);
static bool binaryOperand(Lexer* l, method_generation_context* mgenc);
static void keywordMessage(Lexer* l, method_generation_context* mgenc, bool super);
static void formula(Lexer* l, method_generation_context* mgenc);
static void nestedTerm(Lexer* l, method_generation_context* mgenc);
static void literal(Lexer* l, method_generation_context* mgenc);
static pVMObject literalArray(Lexer* l);
static pVMObject literalNumber(Lexer* l);
static pVMObject literalDecimal(Lexer* l, bool negateValue);
static pVMObject negativeDecimal(Lexer* l);
static pVMObject literalInteger(Lexer* l, bool negateValue);
static pVMObject literalDouble(Lexer* l, bool negateValue);
static pVMObject literalSymbol(Lexer* l);
static pVMObject literalString(Lexer* l);
static pVMSymbol selector(Lexer* l);
static pVMSymbol keywordSelector(Lexer* l);
static pString string(Lexer* l);
static void nestedBlock(Lexer* l, method_generation_context* mgenc);
static void blockPattern(Lexer* l, method_generation_context* mgenc);
static void blockArguments(Lexer* l, method_generation_context* mgenc);


Lexer* Parser_init(const FILE* fp, const char* fname) {
    Lexer* l = Lexer_create(fp, fname);
    Lexer_get_sym(l);
    return l;
}


Lexer* Parser_init_string(const char* stream) {
    Lexer* l = Lexer_from_string(stream);
    Lexer_get_sym(l);
    return l;
}


#pragma mark Parser helpers


bool symIn(Lexer* l, Symbol* ss) {
    while(*ss)
        if(*ss++ == l->sym)
            return true;
    return false;
}


bool accept(Lexer* l, Symbol s) {
    if (l->sym == s) {
        Lexer_get_sym(l);
        return true;
    }
    return false;
}


bool acceptOneOf(Lexer* l, Symbol* ss) {
    if(symIn(l, ss)) {
        Lexer_get_sym(l);
        return true;
    }
    return false;
}


#define _PRINTABLE_SYM (l->sym == Integer || l->sym >= STString)


bool expect(Lexer* l, Symbol s) {
    if(accept(l, s))
        return true;
    fprintf(stderr, "Error parsing %s:%d: unexpected symbol. Expected %s, but found %s",
            l->file_name, l->line_num, symnames[s], symnames[l->sym]);
    if(_PRINTABLE_SYM)
        fprintf(stderr, " (%s)", l->_text);
    fprintf(stderr, ": %s\n", l->buf);
    return false;
}


bool expectOneOf(Lexer* l, Symbol* ss) {
    if(acceptOneOf(l, ss))
        return true;
    fprintf(stderr, "Error parsing %s:%d: unexpected symbol. Expected one of ",
            l->file_name, l->line_num);
    while(*ss)
        fprintf(stderr, "%s, ", symnames[*ss++]);
    fprintf(stderr, "but found %s", symnames[l->sym]);
    if(_PRINTABLE_SYM)
        fprintf(stderr, " (%s)", l->_text);
    fprintf(stderr, ": %s\n", l->buf);
    return false;
}


#pragma mark helper functions for pushing / popping variables


void gen_push_variable(method_generation_context* mgenc, pString var) {
    // The purpose of this function is to find out whether the variable to be
    // pushed on the stack is a local variable, argument, or object field. This
    // is done by examining all available lexical contexts, starting with the
    // innermost (i.e., the one represented by mgenc).
    
    size_t index = 0;
    size_t context = 0;
    bool is_argument = false;
    if (method_genc_find_var(mgenc, var, &index, &context, &is_argument))
        if (is_argument)
            emit_PUSH_ARGUMENT(mgenc, index, context);
        else
            emit_PUSH_LOCAL(mgenc, index, context);
    else if(method_genc_find_field(mgenc, var)) {
        pVMSymbol fieldName = Universe_symbol_for_str(var);
        SEND(mgenc->literals, addIfAbsent, fieldName);
        emit_PUSH_FIELD(mgenc, fieldName);
    } else {
        pVMSymbol global = Universe_symbol_for_str(var);
        SEND(mgenc->literals, addIfAbsent, global);
        emit_PUSH_GLOBAL(mgenc, global);
    }
}


void gen_pop_variable(method_generation_context* mgenc, pString var) {
    // The purpose of this function is to find out whether the variable to be
    // popped off the stack is a local variable, argument, or object field. This
    // is done by examining all available lexical contexts, starting with the
    // innermost (i.e., the one represented by mgenc).
    
    size_t index = 0;
    size_t context = 0;
    bool is_argument = false;
    if (method_genc_find_var(mgenc, var, &index, &context, &is_argument)) {
        if (is_argument) {
            emit_POP_ARGUMENT(mgenc, index, context);
        } else {
            emit_POP_LOCAL(mgenc, index, context);
        }
    } else {
        emit_POP_FIELD(mgenc, Universe_symbol_for_str(var));
    }
}


//
// grammar
//


static Symbol singleOpSyms[] = {
    Not, And, Or, Star, Div, Mod, Plus, Equal, More, Less, Comma, At, Per, NONE
};


static Symbol binaryOpSyms[] = {
    Or, Comma, Minus, Equal, Not, And, Or, Star, Div, Mod, Plus, Equal,
    More, Less, Comma, At, Per, NONE
};


static Symbol keywordSelectorSyms[] = { Keyword, KeywordSequence };


static pString selfStr;
static pString superStr;
static pString blockSelfStr;


void Parser_init_constants() {
    selfStr = String_new("self", strlen("self"));
    superStr = String_new("super", strlen("super"));
    blockSelfStr = String_new("$block self", strlen("$block self"));
}


#pragma mark Parser grammar


void Parser_classdef(Lexer* l, class_generation_context* cgenc) {
    cgenc->name = Universe_symbol_for_chars(l->_text, l->_textLength);
    Lexer_consumed_text(l);

    expect(l, Identifier);
    
    expect(l, Equal);

    superclass(l, cgenc);

    expect(l, NewTerm);
    instanceFields(l, cgenc);
    while(l->sym == Identifier || l->sym == Keyword || l->sym == OperatorSequence ||
        symIn(l, binaryOpSyms)
    ) {
        method_generation_context mgenc;
        method_genc_init(&mgenc);
        mgenc.holder_genc = cgenc;
        
        // each method has a self variable
        SEND(mgenc.arguments, addString, selfStr);
        
        method(l, &mgenc);
        
        if(mgenc.primitive)
            SEND(cgenc->instance_methods, add, VMPrimitive_assemble(&mgenc));
        else
            SEND(cgenc->instance_methods, add, VMMethod_assemble(&mgenc));
        
        method_genc_release(&mgenc);
    }
    
    if(accept(l, Separator)) {
        cgenc->class_side = true;
        classFields(l, cgenc);
        while(l->sym == Identifier || l->sym == Keyword || l->sym == OperatorSequence ||
            symIn(l, binaryOpSyms)
        ) {
            method_generation_context mgenc;
            method_genc_init(&mgenc);
            mgenc.holder_genc = cgenc;
            
            // each method has a self variable
            SEND(mgenc.arguments, addString, selfStr);
            
            method(l, &mgenc);
            
            if(mgenc.primitive)
                SEND(cgenc->class_methods, add, VMPrimitive_assemble(&mgenc));
            else
                SEND(cgenc->class_methods, add, VMMethod_assemble(&mgenc));
            
            method_genc_release(&mgenc);
        }    
    }
    expect(l, EndTerm);
}


void superclass(Lexer* l, class_generation_context* cgenc) {
    if(l->sym == Identifier) {
        cgenc->super_name = Universe_symbol_for_chars(l->_text, l->_textLength);
        Lexer_consumed_text(l);

        accept(l, Identifier);
    } else
        cgenc->super_name = Universe_symbol_for_cstr("Object");

    // Load the super class, if it is not nil (break the dependency cycle)
    if (cgenc->super_name != Universe_symbol_for_cstr("nil")) {
        pVMClass super_class = Universe_load_class(cgenc->super_name);
        class_genc_set_instance_fields_of_super(cgenc, SEND(super_class, get_instance_fields));
        class_genc_set_class_fields_of_super(cgenc, SEND(SEND(super_class, get_class), get_instance_fields));
    } else {
        // we hardcode here the field names for Class
        // SOM doesn't make them accessible normally anymore.
        // On the language level, we only have the primitives.
        // But to keep the implementation simple and allow classes to have
        // fields, we add them back in here.

        // We do this when the super_name is nil, because that means the class
        // is Object, and we want to add the fields there on the class-side
        // since Object class superclass = Class.
        // We avoid here any kind of dynamic solution to avoid further complexity.
        // However, that makes it static, it is going to make it harder to
        // change the definition of Class and Object

        // There are exactly 4 fields
        Universe_assert(4 == SIZE_DIFF_VMOBJECT(VMClass));

        const char* field_names[] = {
            "superClass", "name", "instanceFields", "instanceInvokables"};

        pVMArray class_fields = Universe_new_array_from_argv(4, field_names);
        class_genc_set_class_fields_of_super(cgenc, class_fields);
    }
}


void instanceFields(Lexer* l, class_generation_context* cgenc) {
    if(accept(l, Or)) {
        while(l->sym == Identifier) {
            pString var = variable(l);
            SEND(cgenc->instance_fields, add, Universe_symbol_for_str(var));
            internal_free(var);
        }
        expect(l, Or);
    }
}


void classFields(Lexer* l, class_generation_context* cgenc) {
    if(accept(l, Or)) {
        while(l->sym == Identifier) {
            pString var = variable(l);
            SEND(cgenc->class_fields, add, Universe_symbol_for_str(var));
            internal_free(var);
        }
        expect(l, Or);
    }
}


void method(Lexer* l, method_generation_context* mgenc) {
    pattern(l, mgenc);
    
    expect(l, Equal);
    if(l->sym == Primitive) {
        mgenc->primitive = true;
        primitiveBlock(l);
    } else
        methodBlock(l, mgenc);
}


void primitiveBlock(Lexer* l) {
    expect(l, Primitive);
}


void pattern(Lexer* l, method_generation_context* mgenc) {
    switch(l->sym) {
        case Identifier: 
            unaryPattern(l, mgenc);
            break;
        case Keyword: 
            keywordPattern(l, mgenc);
            break;
        default: 
            binaryPattern(l, mgenc);
            break;
    }
}


void unaryPattern(Lexer* l, method_generation_context* mgenc) {
    mgenc->signature = unarySelector(l);
}


void binaryPattern(Lexer* l, method_generation_context* mgenc) {
    mgenc->signature = binarySelector(l);
    pString arg = argument(l);
    SEND(mgenc->arguments, addStringIfAbsent, arg);
    internal_free(arg);
}


void keywordPattern(Lexer* l, method_generation_context* mgenc) {
    pString kw = String_new("", 0);
    do {
        pString key = keyword(l);
        pString oldKw = kw;
        kw = SEND(kw, concat, key);
        SEND(oldKw, free);
        SEND(key, free);
        pString arg = argument(l);
        SEND(mgenc->arguments, addStringIfAbsent, arg);
        SEND(arg, free);
    } while (l->sym == Keyword);
    
    mgenc->signature = Universe_symbol_for_str(kw);
    SEND(kw, free);
}


void methodBlock(Lexer* l, method_generation_context* mgenc) {
    expect(l, NewTerm);
    blockContents(l, mgenc);
    
    // if no return has been generated so far, we can be sure there was no .
    // terminating the last expression, so the last expression's value must be
    // popped off the stack and a ^self be generated
    if(!mgenc->finished) {
        emit_POP(mgenc);
        emit_PUSH_ARGUMENT(mgenc, 0, 0);
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    }
    
    expect(l, EndTerm);
}


pVMSymbol unarySelector(Lexer* l) {
    pString id = identifier(l);
    pVMSymbol result = Universe_symbol_for_str(id);
    internal_free(id);
    return result;
}


pVMSymbol binarySelector(Lexer* l) {
    pVMSymbol symb = Universe_symbol_for_chars(l->_text, l->_textLength);
    Lexer_consumed_text(l);
    
    if(accept(l, Or))
        ;
    else if(accept(l, Comma))
        ;
    else if(accept(l, Minus))
        ;
    else if(accept(l, Equal))
        ;
    else if(acceptOneOf(l, singleOpSyms))
        ;
    else if(accept(l, OperatorSequence))
        ;
    else
        expect(l, NONE);
    
    return symb;
}


pString identifier(Lexer* l) {
    pString s = Lexer_get_text(l);
    if(accept(l, Primitive))
        ; // text is set
    else
        expect(l, Identifier);
    
    return s;
}


pString keyword(Lexer* l) {
    pString s = Lexer_get_text(l);
    expect(l, Keyword);
    
    return s;
}


pString argument(Lexer* l) {
    return variable(l);
}


void blockContents(Lexer* l, method_generation_context* mgenc) {
    if(accept(l, Or)) {
        locals(l, mgenc);
        expect(l, Or);
    }
    blockBody(l, mgenc, false);
}


void locals(Lexer* l, method_generation_context* mgenc) {
    while(l->sym == Identifier) {
        pString var = variable(l);
        SEND(mgenc->locals, addStringIfAbsent, var);
        internal_free(var);
    }
}


void blockBody(Lexer* l, method_generation_context* mgenc, bool seen_period) {
    if(accept(l, Exit))
        result(l, mgenc);
    else if(l->sym == EndBlock) {
        if (seen_period) {
            // a POP has been generated which must be elided (blocks always
            // return the value of the last expression, regardless of whether it
            // was terminated with a . or not)
            mgenc->bp--;
        }
        if (mgenc->block_method && !method_genc_has_bytecodes(mgenc)) {
            pVMSymbol nilSym = Universe_symbol_for_cstr("nil");
            SEND(mgenc->literals, addIfAbsent, nilSym);
            emit_PUSH_GLOBAL(mgenc, nilSym);
        }
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    } else if(l->sym == EndTerm) {
        // it does not matter whether a period has been seen, as the end of
        // the method has been found (EndTerm) - so it is safe to emit a
        // "return self"
        emit_PUSH_ARGUMENT(mgenc, 0, 0);
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    } else {
        expression(l, mgenc);
        if(accept(l, Period)) {
            emit_POP(mgenc);
            blockBody(l, mgenc, true);
        }
    }
}


void result(Lexer* l, method_generation_context* mgenc) {
    expression(l, mgenc);
    if(mgenc->block_method)
        emit_RETURN_NON_LOCAL(mgenc);
    else
        emit_RETURN_LOCAL(mgenc);
    mgenc->finished = true;
    accept(l, Period);
}


void expression(Lexer* l, method_generation_context* mgenc) {
    Lexer_peek(l);
    if(l->nextSym == Assign)
        assignation(l, mgenc);
    else
        evaluation(l, mgenc);
}


void assignation(Lexer* lexer, method_generation_context* mgenc) {
    pList l = List_new();
    
    assignments(lexer, mgenc, l);
    evaluation(lexer, mgenc);
    
    int i;
    for(i = 0; i < SEND(l, size); i++)
        emit_DUP(mgenc);
    for(i = 0; i < SEND(l, size); i++) {
        pString s = (pString)SEND(l, get, i);
        gen_pop_variable(mgenc, s);
    }
    
    SEND(l, deep_free);
}


void assignments(Lexer* lexer, method_generation_context* mgenc, pList l) {
    if(lexer->sym == Identifier) {
        SEND(l, add, assignment(lexer, mgenc));
        Lexer_peek(lexer);
        if(lexer->nextSym == Assign)
            assignments(lexer, mgenc, l);
    }
}


pString assignment(Lexer* l, method_generation_context* mgenc) {
    pString v = variable(l);
    pVMSymbol var = Universe_symbol_for_str(v);
    SEND(mgenc->literals, addIfAbsent, var);
    
    expect(l, Assign);

    return v;
}


void evaluation(Lexer* l, method_generation_context* mgenc) {
    bool super = primary(l, mgenc);
    if(l->sym == Identifier || l->sym == Keyword || l->sym == OperatorSequence ||
        symIn(l, binaryOpSyms)
    ) {       
        messages(l, mgenc, super);
    }
}


bool primary(Lexer* l, method_generation_context* mgenc) {
    bool super = false;
    switch(l->sym) {
        case Identifier: {
            pString var = variable(l);
            if (SEND(var, compareTo, superStr) == 0) {
                super = true;
                // sends to super push self as the receiver
                gen_push_variable(mgenc, selfStr);
            } else {
                gen_push_variable(mgenc, var);
            }
            
            internal_free(var);
            break;
        }
        case NewTerm:
            nestedTerm(l, mgenc);
            break;
        case NewBlock: {
            method_generation_context bgenc;
            method_genc_init(&bgenc);
            bgenc.block_method = true;
            bgenc.holder_genc = mgenc->holder_genc;
            bgenc.outer_genc = mgenc;
            
            nestedBlock(l, &bgenc);
            
            pVMMethod block_method = VMMethod_assemble(&bgenc);
            SEND(mgenc->literals, add, block_method);
            emit_PUSH_BLOCK(mgenc, block_method);
            
            method_genc_release(&bgenc);
            break;
        }
        default: {
            literal(l, mgenc);
            break;
        }
    }

    return super;
}


pString variable(Lexer* l) {
    return identifier(l);
}


void messages(Lexer* l, method_generation_context* mgenc, bool super) {
    if(l->sym == Identifier) {
        do {
            // only the first message in a sequence can be a super send
            unaryMessage(l, mgenc, super);
            super = false;
        } while (l->sym == Identifier);
        
        while (l->sym == OperatorSequence || symIn(l, binaryOpSyms)) {
            binaryMessage(l, mgenc, false);
        }
        
        if (l->sym == Keyword) {
            keywordMessage(l, mgenc, false);
        }
    } else if (l->sym == OperatorSequence || symIn(l, binaryOpSyms)) {
        do {
            // only the first message in a sequence can be a super send
            binaryMessage(l, mgenc, super);
            super = false;
        } while(l->sym == OperatorSequence || symIn(l, binaryOpSyms));
        
        if (l->sym == Keyword) {
            keywordMessage(l, mgenc, false);
        }
    } else {
        keywordMessage(l, mgenc, super);
    }
}


void unaryMessage(Lexer* l, method_generation_context* mgenc, bool super) {
    pVMSymbol msg = unarySelector(l);
    SEND(mgenc->literals, addIfAbsent, msg);
    if (super) {
        emit_SUPER_SEND(mgenc, msg);
    } else {
        emit_SEND(mgenc, msg);
    }
}


void binaryMessage(Lexer* l, method_generation_context* mgenc, bool super) {
    pVMSymbol msg = binarySelector(l);
    SEND(mgenc->literals, addIfAbsent, msg);
    
    binaryOperand(l, mgenc);
    
    if (super) {
        emit_SUPER_SEND(mgenc, msg);
    } else {
        emit_SEND(mgenc, msg);
    }
}


bool binaryOperand(Lexer* l, method_generation_context* mgenc) {
    bool super = primary(l, mgenc);
    
    while (l->sym == Identifier) {
        unaryMessage(l, mgenc, super);
        super = false;
    }

    return super;
}


void keywordMessage(Lexer* l, method_generation_context* mgenc, bool super) {
    pString kw = String_new("", 0);
    do {
        pString key = keyword(l);
        kw = SEND(kw, concat, key);
        SEND(key, free);
        formula(l, mgenc);
    } while(l->sym == Keyword);
    
    pVMSymbol msg = Universe_symbol_for_str(kw);
    SEND(kw, free);
    SEND(mgenc->literals, addIfAbsent, msg);
    
    if(super)
        emit_SUPER_SEND(mgenc, msg);
    else
        emit_SEND(mgenc, msg);
}


void formula(Lexer* l, method_generation_context* mgenc) {
    bool super = binaryOperand(l, mgenc);
    
    // only the first message in a sequence can be a super send
    if (l->sym == OperatorSequence || symIn(l, binaryOpSyms)) {
        binaryMessage(l, mgenc, super);
    }

    while(l->sym == OperatorSequence || symIn(l, binaryOpSyms)) {
        binaryMessage(l, mgenc, false);
    }
}


void nestedTerm(Lexer* l, method_generation_context* mgenc) {
    expect(l, NewTerm);
    expression(l, mgenc);
    expect(l, EndTerm);
}

static pVMObject get_object_for_current_literal(Lexer* l) {
    pVMObject literal_obj;
    switch(l->sym) {
        case Pound:
            Lexer_peek_if_necessary(l);
            if (l->nextSym == NewTerm) {
                literal_obj = literalArray(l);
            } else {
                literal_obj = literalSymbol(l);
            }
            break;
        case STString:
            literal_obj = literalString(l);
            break;
        default:
            literal_obj = literalNumber(l);
            break;
    }
    return literal_obj;
}

void literal(Lexer* l, method_generation_context* mgenc) {
    pVMObject literal_obj = get_object_for_current_literal(l);

    SEND(mgenc->literals, addIfAbsent, literal_obj);
    emit_PUSH_CONSTANT(mgenc, literal_obj);
}


pVMObject literalNumber(Lexer* l) {
    if (l->sym == Minus)
        return negativeDecimal(l);
    else
        return literalDecimal(l, false);
}


pVMObject literalDecimal(Lexer* l, bool negateValue) {
    if (l->sym == Integer) {
            return literalInteger(l, negateValue);
    } else {
        Universe_assert(l->sym == Double);
        return literalDouble(l, negateValue);
    }
}


pVMObject negativeDecimal(Lexer* l) {
    expect(l, Minus);
    return literalDecimal(l, true);
}


pVMObject literalInteger(Lexer* l, bool negateValue) {
    int64_t i = strtoll(l->_text, NULL, 10);
    Lexer_consumed_text(l);

    expect(l, Integer);

    if (negateValue) {
        i = 0 - i;
    }
    return (pVMObject)Universe_new_integer(i);
}


pVMObject literalDouble(Lexer* l, bool negateValue) {
    double d = strtod(l->_text, NULL);
    Lexer_consumed_text(l);

    if (negateValue) {
        d = 0 - d;
    }
    expect(l, Double);
    return (pVMObject) Universe_new_double(d);
}

pVMObject literalSymbol(Lexer* l) {
    pVMSymbol symb;
    expect(l, Pound);
    if (l->sym == STString) {
        pString s = string(l);
        symb = Universe_symbol_for_str(s);
        internal_free(s);
    } else
        symb = selector(l);
    
    return (pVMObject)symb;
}


pVMObject literalString(Lexer* l) {
    pString s = string(l);
    pVMString str = Universe_new_string_str(s);
    internal_free(s);

    return (pVMObject)str;
}


pVMObject literalArray(Lexer* l) {
    pList literal_values = List_new();

    expect(l, Pound);
    expect(l, NewTerm);

    while (l->sym != EndTerm) {
        SEND(literal_values, add, get_object_for_current_literal(l));
    }

    expect(l, EndTerm);

    pVMArray arr = Universe_new_array_list(literal_values);

    SEND(literal_values, free);

    return (pVMObject)arr;
}


pVMSymbol selector(Lexer* l) {
    if(l->sym == OperatorSequence || symIn(l, singleOpSyms))
        return binarySelector(l);
    else if(l->sym == Keyword || l->sym == KeywordSequence)
        return keywordSelector(l);
    else
        return unarySelector(l);
}


pVMSymbol keywordSelector(Lexer* l) {
    pString s = Lexer_get_text(l);
    expectOneOf(l, keywordSelectorSyms);
    pVMSymbol symb = Universe_symbol_for_str(s);
    internal_free(s);

    return symb;
}


pString string(Lexer* l) {
    pString s = Lexer_get_text(l);
    expect(l, STString);
    return s; // literal strings are at most BUFSIZ long
}


void nestedBlock(Lexer* l, method_generation_context* mgenc) {
#define BLOCK_METHOD   "$block method"
#define BLOCK_METHOD_L (13)
    SEND(mgenc->arguments, addStringIfAbsent, blockSelfStr);
    
    expect(l, NewBlock);
    if(l->sym == Colon)
        blockPattern(l, mgenc);
    
    // generate Block signature
    size_t arg_size = SEND(mgenc->arguments, size);
    char* block_sig = (char*)internal_allocate(BLOCK_METHOD_L + arg_size);
    strcpy(block_sig, BLOCK_METHOD);
    memset(block_sig + BLOCK_METHOD_L, ':', arg_size - 1);

    mgenc->signature = Universe_symbol_for_cstr(block_sig);
    internal_free(block_sig);
    
    blockContents(l, mgenc);
    
    // if no return has been generated, we can be sure that the last expression
    // in the block was not terminated by ., and can generate a return
    if (!mgenc->finished) {
        if (!method_genc_has_bytecodes(mgenc)) {
            pVMSymbol nilSym = Universe_symbol_for_cstr("nil");
            SEND(mgenc->literals, addIfAbsent, nilSym);
            emit_PUSH_GLOBAL(mgenc, nilSym);
        }
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    }
    
    expect(l, EndBlock);
}


void blockPattern(Lexer* l, method_generation_context* mgenc) {
    blockArguments(l, mgenc);
    expect(l, Or);
}


void blockArguments(Lexer* l, method_generation_context* mgenc) {
    do {
        expect(l, Colon);
        pString arg = argument(l);
        SEND(mgenc->arguments, addStringIfAbsent, arg);
        internal_free(arg);
    } while(l->sym == Colon);
}



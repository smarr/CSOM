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

#include "Parser.h"

#include "GenerationContexts.h"
#include "BytecodeGeneration.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <misc/defs.h>

#include <memory/gc.h>

#include <vmobjects/VMString.h>


/***** Grammar Documentation *****
This parser accepts valid programs adhering to the following grammar. Comments
and white space are not dealt with in the grammar. Names of non-terminals begin
with a lower-case letter; terminals, with an upper-case one.

classdef =
    Identifier Equal Identifier? NewTerm
    instanceFields method*
    ( Separator classFields method* )?
    EndTerm

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


#pragma mark Enums


typedef enum {
    NONE, Integer, Double, Not, And, Or, Star, Div, Mod, Plus,
    Minus, Equal, More, Less, Comma, At, Per, NewBlock,
    EndBlock, Colon, Period, Exit, Assign, NewTerm, EndTerm, Pound,
    Primitive, Separator, STString, Identifier, Keyword, KeywordSequence,
    OperatorSequence
} Symbol;


static char* symnames[] = {
    "NONE", "Integer", "Double", "Not", "And", "Or", "Star", "Div", "Mod", "Plus",
    "Minus", "Equal", "More", "Less", "Comma", "At", "Per", "NewBlock",
    "EndBlock", "Colon", "Period", "Exit", "Assign", "NewTerm", "EndTerm",
    "Pound", "Primitive", "Separator", "STString", "Identifier", "Keyword",
    "KeywordSequence", "OperatorSequence"
};


#pragma mark intern Prototypes


static int fillbuffer(void);
static bool eob(void);
static void skipWhiteSpace(void);
static void skipComment(void);
static void getsym(void);
static void peek(void);
static bool symIn(Symbol* ss);
static bool accept(Symbol s);
static bool acceptOneOf(Symbol* ss);
static bool expect(Symbol s);
static bool expectOneOf(Symbol* ss);
static void SingleOperator(void);
static void classdef(void);
static void instanceFields(class_generation_context* cgenc);
static void classFields(class_generation_context* cgenc);
static void method(method_generation_context* mgenc);
static void primitiveBlock(void);
static void pattern(method_generation_context* mgenc);
static void unaryPattern(method_generation_context* mgenc);
static void binaryPattern(method_generation_context* mgenc);
static void keywordPattern(method_generation_context* mgenc);
static void methodBlock(method_generation_context* mgenc);
static pVMSymbol unarySelector(void);
static pVMSymbol binarySelector(void);
static char* identifier(void);
static pString keyword(void);
static char* argument(void);
static void blockContents(method_generation_context* mgenc);
static void locals(method_generation_context* mgenc);
static void blockBody(method_generation_context* mgenc, bool seen_period);
static void result(method_generation_context* mgenc);
static void expression(method_generation_context* mgenc);
static void assignation(method_generation_context* mgenc);
static void assignments(method_generation_context* mgenc, pList l);
static pString assignment(method_generation_context* mgenc);
static void evaluation(method_generation_context* mgenc);
static void primary(method_generation_context* mgenc, bool* super);
static char* variable(void);
static void messages(method_generation_context* mgenc, bool super);
static void unaryMessage(method_generation_context* mgenc, bool super);
static void binaryMessage(method_generation_context* mgenc, bool super);
static void binaryOperand(method_generation_context* mgenc, bool* super);
static void keywordMessage(method_generation_context* mgenc, bool super);
static void formula(method_generation_context* mgenc);
static void nestedTerm(method_generation_context* mgenc);
static void literal(method_generation_context* mgenc);
static pVMObject literalArray(void);
static pVMObject literalNumber(void);
static pVMObject literalDecimal(bool);
static pVMObject negativeDecimal(void);
static pVMObject literalInteger(bool negateValue);
static pVMObject literalDouble(bool negateValue);
static pVMObject literalSymbol(void);
static pVMObject literalString(void);
static pVMSymbol selector(void);
static pVMSymbol keywordSelector(void);
static char* string(void);
static void nestedBlock(method_generation_context* mgenc);
static void blockPattern(method_generation_context* mgenc);
static void blockArguments(method_generation_context* mgenc);


//
// internal data
//


#pragma mark File local variables


static FILE* infile;

static Symbol sym;
static char symc;
static char text[BUFSIZ];

static bool peekDone;
static Symbol nextSym;
static char nextSymc;
static char nextText[BUFSIZ];


//
// input buffer
//


static char buf[BUFSIZ];
static int bufp;


#pragma mark Stream handling


void Parser_init(const FILE* fp) {
    infile = (FILE*)fp;
    sym = NONE;
    peekDone = false;
    nextSym = NONE;
    *buf = 0;
    bufp = 0;
    getsym();
}


void Parser_init_string(const char* stream) {
    infile = NULL;
    sym = NONE;
    peekDone = false;
    nextSym = NONE;
    strncpy(buf, stream, BUFSIZ);
    bufp = 0;
    getsym();
}


#define _BC (buf[bufp])
#define EOB (buf[bufp]==0)


int fillbuffer(void) {
    if(!infile) // string stream
        return -1;
    int p = 0;
    do {
        if(!feof(infile))
            buf[p] = fgetc(infile);
        else
            buf[p] = '\n';
    } while(buf[p++] != '\n');
    buf[p - 1] = 0;
    bufp = 0;
    return p;
}


//
// basic lexing
//


#pragma mark Lexer functions


void skipWhiteSpace(void) {
    while(isblank(_BC)) {
        bufp++;
        while (EOB) {
            if (fillbuffer() == -1) {
                return;
            }
        }
    }
}


void skipComment(void) {
    if(_BC == '"') {
        do {
            bufp++;
            while (EOB) {
                if (fillbuffer() == -1) {
                    return;
                }
            }
        } while(_BC != '"');
        bufp++;
    }
}


#define _ISOP(C) \
    ((C) == '~' || (C) == '&' || (C) == '|' || (C) == '*' || (C) == '/' || \
     (C) == '\\' || (C) == '+' || (C) == '=' || (C) == '>' || (C) == '<' || \
     (C) == ',' || (C) == '@' || (C) == '%' || (C) == '-')
#define _MATCH(C, S) \
    if(_BC == (C)) { sym = (S); symc = _BC; sprintf(text, "%c", _BC); bufp++;}
#define SEPARATOR "----"
#define PRIMITIVE "primitive"


void lexNumber() {
    sym = Integer;
    symc = 0;
    char* t = text;

    bool saw_decimal_mark = false;

    do {
        *t++ = buf[bufp++];
        char char_arr[3];
        char_arr[0] = buf[bufp + 0];
        char_arr[1] = buf[bufp + 1];
        char_arr[2] = buf[bufp + 2];

        if (!saw_decimal_mark &&
            '.' == _BC &&
            buf[bufp + 1] != 0 &&
            isdigit(buf[bufp + 1])) {
            sym = Double;
            saw_decimal_mark = true;
            *t++ = buf[bufp++];
        }
    } while(isdigit(_BC));
    *t = 0;
}

void lexEscapeChar(char** t) {
    char current = buf[++bufp];

    switch (current) {
        case 't': *(*t)++ = current; break;
        case 'b': *(*t)++ = current; break;
        case 'n': *(*t)++ = current; break;
        case 'r': *(*t)++ = current; break;
        case 'f': *(*t)++ = current; break;
        case '\'': *(*t)++ = current; break;
        case '\\': *(*t)++ = current; break;
    }
}

void lexStringChar(char** t) {
    char current = buf[++bufp];

    if (current == '\\') {
        lexEscapeChar(t);
    } else {
        *(*t)++ = current;
    }
}

void lexString() {
    sym = STString;
    symc = 0;
    char* t = text;
    do {
      lexStringChar(&t);
        while (EOB) {
            if (fillbuffer() == -1) {
                return;
            }
        }
    } while(_BC != '\'');
    bufp++;
    *--t = 0;
}

void lexOperator() {
    if(_ISOP(buf[bufp + 1])) {
        sym = OperatorSequence;
        symc = 0;
        char* t = text;
        while(_ISOP(_BC))
            *t++ = buf[bufp++];
        *t = 0;
    }
    else _MATCH('~', Not)
    else _MATCH('&', And)
    else _MATCH('|', Or)
    else _MATCH('*', Star)
    else _MATCH('/', Div)
    else _MATCH('\\', Mod)
    else _MATCH('+', Plus)
    else _MATCH('=', Equal)
    else _MATCH('>', More)
    else _MATCH('<', Less)
    else _MATCH(',', Comma)
    else _MATCH('@', At)
    else _MATCH('%', Per)
    else _MATCH('-', Minus)
}

void getsym(void) {
    if(peekDone) {
        peekDone = false;
        sym = nextSym;
        symc = nextSymc;
        strcpy(text, nextText);
        return;
    }

    do {
        if(EOB)
            fillbuffer();
        skipWhiteSpace();
        skipComment();
    } while((EOB || isblank(_BC) || _BC == '"') && infile);
    
    if(_BC == '\'') {
        lexString();
    }
    else _MATCH('[', NewBlock)
    else _MATCH(']', EndBlock)
    else if(_BC == ':') {
        if(buf[bufp+1] == '=') {
            bufp += 2;
            sym = Assign;
            symc = 0;
            sprintf(text, ":=");
        } else {
            bufp++;
            sym = Colon;
            symc = ':';
            sprintf(text, ":");
        }
    }
    else _MATCH('(', NewTerm)
    else _MATCH(')', EndTerm)
    else _MATCH('#', Pound)
    else _MATCH('^', Exit)
    else _MATCH('.', Period)
    else if(_BC == '-') {
        if(!strncmp(buf + bufp, SEPARATOR, strlen(SEPARATOR))) {
            char* t = text;
            while(_BC == '-')
                *t++ = buf[bufp++];
            *t = 0;
            sym = Separator;
        } else {
            lexOperator();
        }
    }
    else if(_ISOP(_BC)) {
        lexOperator();
    }
    else if(!strncmp(buf + bufp, PRIMITIVE, strlen(PRIMITIVE))) {
        bufp += strlen(PRIMITIVE);
        sym = Primitive;
        symc = 0;
        sprintf(text, PRIMITIVE);
    }
    else if(isalpha(_BC)) {
        char* t = text;
        symc = 0;
        while(isalpha(_BC) || isdigit(_BC) || _BC == '_')
            *t++ = buf[bufp++];
        sym = Identifier;
        if(buf[bufp] == ':') {
            sym = Keyword;
            bufp++;
            *t++ = ':';
            if(isalpha(_BC)) {
                sym = KeywordSequence;
                while(isalpha(_BC) || _BC == ':')
                    *t++ = buf[bufp++];
            }
        }
        *t = 0;
    }
    else if(isdigit(_BC)) {
        lexNumber();
    }
    else {
        sym = NONE;
        symc = _BC;
        sprintf(text, "%c", _BC);
    }
}


void peek(void) {
    Symbol saveSym = sym;
    char saveSymc = symc;
    char saveText[256];
    strcpy(saveText, text);
    if(peekDone)
        fprintf(stderr, "Cannot peek twice!\n");
    getsym();
    nextSym = sym;
    nextSymc = symc;
    strcpy(nextText, text);
    sym = saveSym;
    symc = saveSymc;
    strcpy(text, saveText);
    peekDone = true;
}


void peekIfNecessary() {
    if (!peekDone) {
        peek();
    }
}


//
// parsing
//


#pragma mark Parser helpers


bool symIn(Symbol* ss) {
    while(*ss)
        if(*ss++ == sym)
            return true;
    return false;
}


bool accept(Symbol s) {
    if(sym == s) {
        getsym();
        return true;
    }
    return false;
}


bool acceptOneOf(Symbol* ss) {
    if(symIn(ss)) {
        getsym();
        return true;
    }
    return false;
}


#define _PRINTABLE_SYM (sym == Integer || sym >= STString)


bool expect(Symbol s) {
    if(accept(s))
        return true;
    fprintf(stderr, "Error: unexpected symbol. Expected %s, but found %s", 
            symnames[s], symnames[sym]);
    if(_PRINTABLE_SYM)
        fprintf(stderr, " (%s)", text);
    fprintf(stderr, ": %s\n", buf);
    return false;
}


bool expectOneOf(Symbol* ss) {
    if(acceptOneOf(ss))
        return true;
    fprintf(stderr, "Error: unexpected symbol. Expected one of ");
    while(*ss)
        fprintf(stderr, "%s, ", symnames[*ss++]);
    fprintf(stderr, "but found %s", symnames[sym]);
    if(_PRINTABLE_SYM)
        fprintf(stderr, " (%s)", text);
    fprintf(stderr, ": %s\n", buf);
    return false;
}


#pragma mark helper functions for pushing / popping variables


void gen_push_variable(method_generation_context* mgenc, const char* var) {
    // The purpose of this function is to find out whether the variable to be
    // pushed on the stack is a local variable, argument, or object field. This
    // is done by examining all available lexical contexts, starting with the
    // innermost (i.e., the one represented by mgenc).
    
    int index = 0;
    int context = 0;
    bool is_argument = false;
    if(method_genc_find_var(mgenc, var, &index, &context, &is_argument))
        if(is_argument)
            emit_PUSH_ARGUMENT(mgenc, index, context);
        else
            emit_PUSH_LOCAL(mgenc, index, context);
    else if(method_genc_find_field(mgenc, var)) {
        pVMSymbol fieldName = Universe_symbol_for(var);
        SEND(mgenc->literals, addIfAbsent, fieldName);
        emit_PUSH_FIELD(mgenc, fieldName);
    } else {
        pVMSymbol global = Universe_symbol_for(var);
        SEND(mgenc->literals, addIfAbsent, global);
        emit_PUSH_GLOBAL(mgenc, global);
    }
}


void gen_pop_variable(method_generation_context* mgenc, const char* var) {
    // The purpose of this function is to find out whether the variable to be
    // popped off the stack is a local variable, argument, or object field. This
    // is done by examining all available lexical contexts, starting with the
    // innermost (i.e., the one represented by mgenc).
    
    int index = 0;
    int context = 0;
    bool is_argument = false;
    if(method_genc_find_var(mgenc, var, &index, &context, &is_argument))
        if(is_argument)
            emit_POP_ARGUMENT(mgenc, index, context);
        else
            emit_POP_LOCAL(mgenc, index, context);
    else
        emit_POP_FIELD(mgenc, Universe_symbol_for(var));
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


#pragma mark Parser grammar


void Parser_classdef(class_generation_context* cgenc) {
    cgenc->name = Universe_symbol_for(text);
    expect(Identifier);
    
    expect(Equal);
    
    if(sym == Identifier) {
        cgenc->super_name = Universe_symbol_for(text);
        accept(Identifier);
    } else
        cgenc->super_name = Universe_symbol_for("Object");
    
    expect(NewTerm);
    instanceFields(cgenc);
    while(sym == Identifier || sym == Keyword || sym == OperatorSequence ||
        symIn(binaryOpSyms)
    ) {
        method_generation_context mgenc;
        method_genc_init(&mgenc);
        mgenc.holder_genc = cgenc;
        
        // each method has a self variable
        SEND(mgenc.arguments, addCString, "self");
        
        method(&mgenc);
        
        if(mgenc.primitive)
            SEND(cgenc->instance_methods, add, VMPrimitive_assemble(&mgenc));
        else
            SEND(cgenc->instance_methods, add, VMMethod_assemble(&mgenc));
        
        method_genc_release(&mgenc);
    }
    
    if(accept(Separator)) {
        cgenc->class_side = true;
        classFields(cgenc);
        while(sym == Identifier || sym == Keyword || sym == OperatorSequence ||
            symIn(binaryOpSyms)
        ) {
            method_generation_context mgenc;
            method_genc_init(&mgenc);
            mgenc.holder_genc = cgenc;
            
            // each method has a self variable
            SEND(mgenc.arguments, addCString, "self");
            
            method(&mgenc);
            
            if(mgenc.primitive)
                SEND(cgenc->class_methods, add, VMPrimitive_assemble(&mgenc));
            else
                SEND(cgenc->class_methods, add, VMMethod_assemble(&mgenc));
            
            method_genc_release(&mgenc);
        }    
    }
    expect(EndTerm);
}


void instanceFields(class_generation_context* cgenc) {
    if(accept(Or)) {
        while(sym == Identifier) {
            char* var = variable();
            SEND(cgenc->instance_fields, add, Universe_symbol_for(var));
            internal_free(var);
        }
        expect(Or);
    }
}


void classFields(class_generation_context* cgenc) {
    if(accept(Or)) {
        while(sym == Identifier) {
            char* var = variable();
            SEND(cgenc->class_fields, add, Universe_symbol_for(var));
            internal_free(var);
        }
        expect(Or);
    }
}


void method(method_generation_context* mgenc) {
    pattern(mgenc);
    
    expect(Equal);
    if(sym == Primitive) {
        mgenc->primitive = true;
        primitiveBlock();
    } else
        methodBlock(mgenc);
}


void primitiveBlock(void) {
    expect(Primitive);
}


void pattern(method_generation_context* mgenc) {
    switch(sym) {
        case Identifier: 
            unaryPattern(mgenc);
            break;
        case Keyword: 
            keywordPattern(mgenc);
            break;
        default: 
            binaryPattern(mgenc);
            break;
    }
}


void unaryPattern(method_generation_context* mgenc) {
    mgenc->signature = unarySelector();
}


void binaryPattern(method_generation_context* mgenc) {
    mgenc->signature = binarySelector();
    char* arg = argument();
    SEND(mgenc->arguments, addCStringIfAbsent, arg);
    internal_free(arg);
}


void keywordPattern(method_generation_context* mgenc) {
    pString kw = String_new("");
    do {
        pString key = keyword();
        kw = SEND(kw, concat, key);
        SEND(key, free);
        char* arg = argument();
        SEND(mgenc->arguments, addCStringIfAbsent, arg);
        internal_free(arg);
    } while(sym == Keyword);
    
    mgenc->signature = Universe_symbol_for(SEND(kw, chars));
    SEND(kw, free);
}


void methodBlock(method_generation_context* mgenc) {
    expect(NewTerm);
    blockContents(mgenc);
    
    // if no return has been generated so far, we can be sure there was no .
    // terminating the last expression, so the last expression's value must be
    // popped off the stack and a ^self be generated
    if(!mgenc->finished) {
        emit_POP(mgenc);
        emit_PUSH_ARGUMENT(mgenc, 0, 0);
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    }
    
    expect(EndTerm);
}


pVMSymbol unarySelector(void) {
    const char* id = identifier();
    pVMSymbol result = Universe_symbol_for(id);
    internal_free((void*) id);
    return result;
}


pVMSymbol binarySelector(void) {
    pVMSymbol symb = Universe_symbol_for(text);
    
    if(accept(Or))
        ;
    else if(accept(Comma))
        ;
    else if(accept(Minus))
        ;
    else if(accept(Equal))
        ;
    else if(acceptOneOf(singleOpSyms))
        ;
    else if(accept(OperatorSequence))
        ;
    else
        expect(NONE);
    
    return symb;
}


char* identifier(void) {
    char* s = internal_allocate_string(text);
    if(accept(Primitive))
        ; // text is set
    else
        expect(Identifier);
    
    return s;
}


pString keyword(void) {
    pString s = String_new(text);
    expect(Keyword);
    
    return s;
}


char* argument(void) {
    return variable();
}


void blockContents(method_generation_context* mgenc) {
    if(accept(Or)) {
        locals(mgenc);
        expect(Or);
    }
    blockBody(mgenc, false);
}


void locals(method_generation_context* mgenc) {
    while(sym == Identifier) {
        char* var = variable();
        SEND(mgenc->locals, addCStringIfAbsent, var);
        internal_free(var);
    }
}


void blockBody(method_generation_context* mgenc, bool seen_period) {
    if(accept(Exit))
        result(mgenc);
    else if(sym == EndBlock) {
        if(seen_period)
            // a POP has been generated which must be elided (blocks always
            // return the value of the last expression, regardless of whether it
            // was terminated with a . or not)
            mgenc->bp--;
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    } else if(sym == EndTerm) {
        // it does not matter whether a period has been seen, as the end of
        // the method has been found (EndTerm) - so it is safe to emit a
        // "return self"
        emit_PUSH_ARGUMENT(mgenc, 0, 0);
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    } else {
        expression(mgenc);
        if(accept(Period)) {
            emit_POP(mgenc);
            blockBody(mgenc, true);
        }
    }
}


void result(method_generation_context* mgenc) {
    expression(mgenc);
    if(mgenc->block_method)
        emit_RETURN_NON_LOCAL(mgenc);
    else
        emit_RETURN_LOCAL(mgenc);
    mgenc->finished = true;
    accept(Period);
}


void expression(method_generation_context* mgenc) {
    peek();
    if(nextSym == Assign)
        assignation(mgenc);
    else
        evaluation(mgenc);
}


void assignation(method_generation_context* mgenc) {
    pList l = List_new();
    
    assignments(mgenc, l);
    evaluation(mgenc);
    
    int i;
    for(i = 0; i < SEND(l, size); i++)
        emit_DUP(mgenc);
    for(i = 0; i < SEND(l, size); i++) {
        pString s = (pString)SEND(l, get, i);
        gen_pop_variable(mgenc, SEND(s, chars));
    }
    
    SEND(l, deep_free);
}


void assignments(method_generation_context* mgenc, pList l) {
    if(sym == Identifier) {
        SEND(l, add, assignment(mgenc));
        peek();
        if(nextSym == Assign)
            assignments(mgenc, l);
    }
}


pString assignment(method_generation_context* mgenc) {
    char* v = variable();
    pVMSymbol var = Universe_symbol_for(v);
    SEND(mgenc->literals, addIfAbsent, var);
    
    expect(Assign);
    
    pString vString = String_new(v);
    internal_free(v);
    return vString;
}


void evaluation(method_generation_context* mgenc) {
    bool super;
    primary(mgenc, &super);
    if(sym == Identifier || sym == Keyword || sym == OperatorSequence ||
        symIn(binaryOpSyms)
    ) {       
        messages(mgenc, super);
    }
}


void primary(method_generation_context* mgenc, bool* super) {
    *super = false;
    switch(sym) {
        case Identifier: {
            char* var = variable();
            if(strcmp(var, "super") == 0) {
                *super = true;
                // sends to super push self as the receiver
                gen_push_variable(mgenc, "self");
            } else {
                gen_push_variable(mgenc, var);
            }
            
            internal_free(var);
            break;
        }
        case NewTerm:
            nestedTerm(mgenc);
            break;
        case NewBlock: {
            method_generation_context bgenc;
            method_genc_init(&bgenc);
            bgenc.block_method = true;
            bgenc.holder_genc = mgenc->holder_genc;
            bgenc.outer_genc = mgenc;
            
            nestedBlock(&bgenc);
            
            pVMMethod block_method = VMMethod_assemble(&bgenc);
            SEND(mgenc->literals, add, block_method);
            emit_PUSH_BLOCK(mgenc, block_method);
            
            method_genc_release(&bgenc);
            break;
        }
        default:
            literal(mgenc);
            break;
    }
}


char* variable(void) {
    return identifier();
}


void messages(method_generation_context* mgenc, bool super) {
    if(sym == Identifier) {
        do {
            // only the first message in a sequence can be a super send
            unaryMessage(mgenc, super);
            super = false;
        } while(sym == Identifier);
        
        while(sym == OperatorSequence || symIn(binaryOpSyms)) {
            binaryMessage(mgenc, false);
        }
        
        if(sym == Keyword) {
            keywordMessage(mgenc, false);
        }
    } else if(sym == OperatorSequence || symIn(binaryOpSyms)) {
        do {
            // only the first message in a sequence can be a super send
            binaryMessage(mgenc, super);
            super = false;
        } while(sym == OperatorSequence || symIn(binaryOpSyms));
        
        if(sym == Keyword) {
            keywordMessage(mgenc, false);
        }
    } else
        keywordMessage(mgenc, super);
}


void unaryMessage(method_generation_context* mgenc, bool super) {
    pVMSymbol msg = unarySelector();
    SEND(mgenc->literals, addIfAbsent, msg);
    if(super)
        emit_SUPER_SEND(mgenc, msg);
    else
        emit_SEND(mgenc, msg);
}


void binaryMessage(method_generation_context* mgenc, bool super) {
    pVMSymbol msg = binarySelector();
    SEND(mgenc->literals, addIfAbsent, msg);
    
    bool tmp_bool = false;
    binaryOperand(mgenc, &tmp_bool);
    
    if(super)
        emit_SUPER_SEND(mgenc, msg);
    else
        emit_SEND(mgenc, msg);
}


void binaryOperand(method_generation_context* mgenc, bool* super) {
    primary(mgenc, super);
    
    while(sym == Identifier)
        unaryMessage(mgenc, *super);
}


void keywordMessage(method_generation_context* mgenc, bool super) {
    pString kw = String_new("");
    do {
        pString key = keyword();
        kw = SEND(kw, concat, key);
        SEND(key, free);
        formula(mgenc);
    } while(sym == Keyword);
    
    pVMSymbol msg = Universe_symbol_for(SEND(kw, chars));
    SEND(kw, free);
    SEND(mgenc->literals, addIfAbsent, msg);
    
    if(super)
        emit_SUPER_SEND(mgenc, msg);
    else
        emit_SEND(mgenc, msg);
}


void formula(method_generation_context* mgenc) {
    bool super;
    binaryOperand(mgenc, &super);
    
    // only the first message in a sequence can be a super send
    if(sym == OperatorSequence || symIn(binaryOpSyms))
        binaryMessage(mgenc, super);
    while(sym == OperatorSequence || symIn(binaryOpSyms))
        binaryMessage(mgenc, false);
}


void nestedTerm(method_generation_context* mgenc) {
    expect(NewTerm);
    expression(mgenc);
    expect(EndTerm);
}

static pVMObject get_object_for_current_literal() {
    pVMObject literal_obj;
    switch(sym) {
        case Pound:
            peekIfNecessary();
            if (nextSym == NewTerm) {
                literal_obj = literalArray();
            } else {
                literal_obj = literalSymbol();
            }
            break;
        case STString:
            literal_obj = literalString();
            break;
        default:
            literal_obj = literalNumber();
            break;
    }
    return literal_obj;
}

void literal(method_generation_context* mgenc) {
    pVMObject literal_obj = get_object_for_current_literal();

    SEND(mgenc->literals, addIfAbsent, literal_obj);
    emit_PUSH_CONSTANT(mgenc, literal_obj);
}


pVMObject literalNumber() {
    if (sym == Minus)
        return negativeDecimal();
    else
        return literalDecimal(false);
}


pVMObject literalDecimal(bool negateValue) {
    if (sym == Integer) {
            return literalInteger(negateValue);
    } else {
        Universe_assert(sym == Double);
        return literalDouble(negateValue);
    }
}


pVMObject negativeDecimal(void) {
    expect(Minus);
    return literalDecimal(true);
}


pVMObject literalInteger(bool negateValue) {
    int64_t i = strtoll(text, NULL, 10);
    expect(Integer);

    if (negateValue) {
        i = 0 - i;
    }
    return (pVMObject)Universe_new_integer(i);
}


pVMObject literalDouble(bool negateValue) {
    double d = strtod(text, NULL);
    if (negateValue) {
        d = 0 - d;
    }
    expect(Double);
    return (pVMObject) Universe_new_double(d);
}

pVMObject literalSymbol() {
    pVMSymbol symb;
    expect(Pound);
    if(sym == STString) {
        char* s = string();
        symb = Universe_symbol_for(s);
        internal_free(s);
    } else
        symb = selector();
    
    return (pVMObject)symb;
}


pVMObject literalString() {
    char* s = string();
    pVMString str = Universe_new_string(s);
    internal_free(s);
    
    return (pVMObject)str;
}


pVMObject literalArray() {
    pList literal_values = List_new();

    expect(Pound);
    expect(NewTerm);

    while (sym != EndTerm) {
        SEND(literal_values, add, get_object_for_current_literal());
    }

    expect(EndTerm);

    pVMArray arr = Universe_new_array_list(literal_values);

    SEND(literal_values, free);

    return (pVMObject)arr;
}


pVMSymbol selector(void) {
    if(sym == OperatorSequence || symIn(singleOpSyms))
        return binarySelector();
    else if(sym == Keyword || sym == KeywordSequence)
        return keywordSelector();
    else
        return unarySelector();
}


pVMSymbol keywordSelector(void) {
    char* s = internal_allocate_string(text);
    expectOneOf(keywordSelectorSyms);
    pVMSymbol symb = Universe_symbol_for(s);
    internal_free(s);
    return symb;
}


char* string(void) {
    char* s = internal_allocate_string(text);
    expect(STString);
    return s; // literal strings are at most BUFSIZ long
}


void nestedBlock(method_generation_context* mgenc) {
#define BLOCK_METHOD   "$block method"
#define BLOCK_METHOD_L (13)
    SEND(mgenc->arguments, addCStringIfAbsent, "$block self");
    
    expect(NewBlock);
    if(sym == Colon)
        blockPattern(mgenc);
    
    // generate Block signature
    size_t arg_size = SEND(mgenc->arguments, size);
    char* block_sig = (char*)internal_allocate(BLOCK_METHOD_L + arg_size);
    strcpy(block_sig, BLOCK_METHOD);
    memset(block_sig + BLOCK_METHOD_L, ':', arg_size - 1);

    mgenc->signature = Universe_symbol_for(block_sig);
    internal_free(block_sig);
    
    blockContents(mgenc);
    
    // if no return has been generated, we can be sure that the last expression
    // in the block was not terminated by ., and can generate a return
    if(!mgenc->finished) {
        emit_RETURN_LOCAL(mgenc);
        mgenc->finished = true;
    }
    
    expect(EndBlock);
}


void blockPattern(method_generation_context* mgenc) {
    blockArguments(mgenc);
    expect(Or);
}


void blockArguments(method_generation_context* mgenc) {
    do {
        expect(Colon);
        char* arg = argument();
        SEND(mgenc->arguments, addCStringIfAbsent, arg);
        internal_free(arg);
    } while(sym == Colon);
}



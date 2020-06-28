/*
 * $Id: Symboltable.c 792 2009-04-06 08:07:33Z michael.haupt $
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
 
#include "Symboltable.h"

#include "VMSymbol.h"
#include "VMString.h"

#include <misc/StringHashmap.h>

// class variable
static pStringHashmap symtab;

pVMSymbol Symbol_table_lookup(pString restrict string) {
    return (pVMSymbol)SEND(symtab, get, string);
}


void Symbol_table_insert(pVMSymbol symbol) {
    pString key = String_new(SEND(symbol, get_rawChars), SEND(symbol, get_length));
    SEND(symtab, put, key, symbol);
}


void Symbol_table_init(void) {
    symtab = StringHashmap_new();
}


void Symbol_table_destruct(void) {
    if (symtab) {
        SEND(symtab, free);
    }
}


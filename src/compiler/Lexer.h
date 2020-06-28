#ifndef LEXER_H_
#define LEXER_H_

/*
 Copyright (c) 2019 Stefan Marr <git@stefan-marr.de>

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

#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

#include <misc/String.h>


typedef enum {
  NONE, Integer, Double, Not, And, Or, Star, Div, Mod, Plus,
  Minus, Equal, More, Less, Comma, At, Per, NewBlock,
  EndBlock, Colon, Period, Exit, Assign, NewTerm, EndTerm, Pound,
  Primitive, Separator, STString, Identifier, Keyword, KeywordSequence,
  OperatorSequence
} Symbol;


extern const char* const symnames[];

typedef struct {
  FILE* infile;
  const char* file_name;

  int line_num;
  Symbol sym;
  char symc;
  char _text[BUFSIZ];
  size_t _textLength;

  bool peekDone;
  Symbol nextSym;
  char nextSymc;
  char _nextText[BUFSIZ];
  size_t _nextTextLength;

  char buf[BUFSIZ];
  int bufp;
} Lexer;

Lexer* Lexer_create(const FILE* fp, const char* fname);
Lexer* Lexer_from_string(const char* stream);

Symbol Lexer_get_sym(Lexer* l);
pString Lexer_get_text(Lexer* l);
void Lexer_consumed_text(Lexer* l);

void Lexer_peek(Lexer* l);
void Lexer_peek_if_necessary(Lexer* l);

#define _ISOP(C) \
  ((C) == '~'  || (C) == '&' || (C) == '|' || (C) == '*' || (C) == '/' || \
   (C) == '\\' || (C) == '+' || (C) == '=' || (C) == '>' || (C) == '<' || \
   (C) == ','  || (C) == '@' || (C) == '%' || (C) == '-')


#endif // LEXER_H_

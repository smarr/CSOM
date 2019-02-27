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

#include "Lexer.h"

#include <string.h>

#include <memory/gc.h>

const char* const symnames[] = {
  "NONE", "Integer", "Double", "Not", "And", "Or", "Star", "Div", "Mod", "Plus",
  "Minus", "Equal", "More", "Less", "Comma", "At", "Per", "NewBlock",
  "EndBlock", "Colon", "Period", "Exit", "Assign", "NewTerm", "EndTerm",
  "Pound", "Primitive", "Separator", "STString", "Identifier", "Keyword",
  "KeywordSequence", "OperatorSequence"
};

Lexer* Lexer_create(const FILE* fp, const char* file_name) {
  Lexer* lexer = internal_allocate(sizeof(Lexer));
  lexer->infile = (FILE*)fp;
  lexer->file_name = file_name;
  lexer->line_num = 0;
  lexer->sym = NONE;
  lexer->symc = 0;
  lexer->text[0] = 0;
  lexer->peekDone = false;
  lexer->nextSym = NONE;
  lexer->nextSymc = 0;
  lexer->nextText[0] = 0;
  lexer->buf[0] = 0;
  lexer->bufp = 0;
  return lexer;
}

Lexer* Lexer_from_string(const char* stream) {
  Lexer* lexer = Lexer_create(NULL, NULL);
  strncpy(lexer->buf, stream, BUFSIZ);
  lexer->file_name = "<string>";
  return lexer;
}

#define _BC (l->buf[l->bufp])
#define EOB (l->buf[l->bufp]==0)
#define _MATCH(C, S) \
  if(_BC == (C)) { l->sym = (S); l->symc = _BC; sprintf(l->text, "%c", _BC); l->bufp++;}

#define SEPARATOR "----"
#define PRIMITIVE "primitive"

int fillbuffer(Lexer* const l) {
  if (!l->infile) // string stream
    return -1;
  int p = 0;
  do {
    if(!feof(l->infile))
      l->buf[p] = fgetc(l->infile);
    else
      l->buf[p] = '\n';
  } while(l->buf[p++] != '\n');
  l->buf[p - 1] = 0;
  l->bufp = 0;
  l->line_num += 1;
  return p;
}

void skipWhiteSpace(Lexer* l) {
  while (isblank(_BC)) {
    if (_BC == '\n') {
      l->line_num += 1;
    }

    l->bufp++;
    while (EOB) {
      if (fillbuffer(l) == -1) {
        return;
      }
    }
  }
}

void skipComment(Lexer* l) {
  if(_BC == '"') {
    do {
      if (_BC == '\n') {
        l->line_num += 1;
      }
      l->bufp++;
      while (EOB) {
        if (fillbuffer(l) == -1) {
          return;
        }
      }
    } while(_BC != '"');
    l->bufp++;
  }
}

void lexNumber(Lexer* l) {
  l->sym = Integer;
  l->symc = 0;
  char* t = l->text;

  bool saw_decimal_mark = false;

  do {
    *t++ = l->buf[l->bufp++];
    char char_arr[3];
    char_arr[0] = l->buf[l->bufp + 0];
    char_arr[1] = l->buf[l->bufp + 1];
    char_arr[2] = l->buf[l->bufp + 2];

    if (!saw_decimal_mark &&
        '.' == _BC &&
        l->buf[l->bufp + 1] != 0 &&
        isdigit(l->buf[l->bufp + 1])) {
      l->sym = Double;
      saw_decimal_mark = true;
      *t++ = l->buf[l->bufp++];
    }
  } while(isdigit(_BC));
  *t = 0;
}

void lexEscapeChar(Lexer* l, char** t) {
  char current = _BC;

  switch (current) {
    case 't': *(*t)++ = '\t'; break;
    case 'b': *(*t)++ = '\b'; break;
    case 'n': *(*t)++ = '\n'; break;
    case 'r': *(*t)++ = '\r'; break;
    case 'f': *(*t)++ = '\f'; break;
    case '\'': *(*t)++ = '\''; break;
    case '\\': *(*t)++ = '\\'; break;
  }
  l->bufp++;
}

void lexStringChar(Lexer* l, char** t) {
  char current = _BC;

  if (current == '\\') {
    l->bufp++;
    lexEscapeChar(l, t);
  } else {
    *(*t)++ = current;
    l->bufp++;
  }
}

void lexString(Lexer* l) {
  l->sym = STString;
  l->symc = 0;
  char* t = l->text;
  l->bufp++;

  while(_BC != '\'') {
    if (_BC == '\n') {
      l->line_num += 1;
    }
    lexStringChar(l, &t);
    while (EOB) {
      if (fillbuffer(l) == -1) {
        return;
      }
    }
  }

  *t = 0;
  l->bufp++;
}

void lexOperator(Lexer* l) {
  if(_ISOP(l->buf[l->bufp + 1])) {
    l->sym = OperatorSequence;
    l->symc = 0;
    char* t = l->text;
    while(_ISOP(_BC))
      *t++ = l->buf[l->bufp++];
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
  else _MATCH('-', Minus);
}

Symbol Lexer_get_sym(Lexer* l) {
  if (l->peekDone) {
    l->peekDone = false;
    l->sym = l->nextSym;
    l->symc = l->nextSymc;
    strcpy(l->text, l->nextText);
    return l->sym;
  }

  do {
    if (EOB)
      fillbuffer(l);
    skipWhiteSpace(l);
    skipComment(l);
  } while((EOB || isblank(_BC) || _BC == '"') && l->infile);

  if(_BC == '\'') {
    lexString(l);
  }
  else _MATCH('[', NewBlock)
  else _MATCH(']', EndBlock)
  else if(_BC == ':') {
    if(l->buf[l->bufp+1] == '=') {
      l->bufp += 2;
      l->sym = Assign;
      l->symc = 0;
      sprintf(l->text, ":=");
    } else {
      l->bufp++;
      l->sym = Colon;
      l->symc = ':';
      sprintf(l->text, ":");
    }
  }
  else _MATCH('(', NewTerm)
  else _MATCH(')', EndTerm)
  else _MATCH('#', Pound)
  else _MATCH('^', Exit)
  else _MATCH('.', Period)
  else if(_BC == '-') {
    if(!strncmp(l->buf + l->bufp, SEPARATOR, strlen(SEPARATOR))) {
      char* t = l->text;
      while(_BC == '-')
        *t++ = l->buf[l->bufp++];
      *t = 0;
      l->sym = Separator;
    } else {
      lexOperator(l);
    }
  }
  else if(_ISOP(_BC)) {
    lexOperator(l);
  }
  else if(!strncmp(l->buf + l->bufp, PRIMITIVE, strlen(PRIMITIVE))) {
    l->bufp += strlen(PRIMITIVE);
    l->sym = Primitive;
    l->symc = 0;
    sprintf(l->text, PRIMITIVE);
  }
  else if(isalpha(_BC)) {
    char* t = l->text;
    l->symc = 0;
    while(isalpha(_BC) || isdigit(_BC) || _BC == '_')
      *t++ = l->buf[l->bufp++];
    l->sym = Identifier;
    if(l->buf[l->bufp] == ':') {
      l->sym = Keyword;
      l->bufp++;
      *t++ = ':';
      if(isalpha(_BC)) {
        l->sym = KeywordSequence;
        while(isalpha(_BC) || _BC == ':')
          *t++ = l->buf[l->bufp++];
      }
    }
    *t = 0;
  }
  else if(isdigit(_BC)) {
    lexNumber(l);
  }
  else {
    l->sym = NONE;
    l->symc = _BC;
    sprintf(l->text, "%c", _BC);
  }
  return l->sym;
}

void Lexer_peek(Lexer* l) {
  Symbol saveSym = l->sym;
  char saveSymc = l->symc;
  char saveText[256];
  strcpy(saveText, l->text);
  if (l->peekDone)
    fprintf(stderr, "Cannot peek twice!\n");
  Lexer_get_sym(l);
  l->nextSym = l->sym;
  l->nextSymc = l->symc;
  strcpy(l->nextText, l->text);
  l->sym = saveSym;
  l->symc = saveSymc;
  strcpy(l->text, saveText);
  l->peekDone = true;
}

void Lexer_peek_if_necessary(Lexer* l) {
  if (!l->peekDone) {
    Lexer_peek(l);
  }
}

#undef _BC
#undef EOB

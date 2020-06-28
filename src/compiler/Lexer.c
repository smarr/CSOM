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

#include <misc/debug.h>
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
  lexer->_text[0] = 0;
  lexer->_textLength = 0;
  lexer->peekDone = false;
  lexer->nextSym = NONE;
  lexer->nextSymc = 0;
  lexer->_nextText[0] = 0;
  lexer->_nextTextLength = 0;
  lexer->buf[0] = 0;
  lexer->bufp = 0;
  return lexer;
}

Lexer* Lexer_from_string(const char* stream) {
  Lexer* lexer = Lexer_create(NULL, NULL);
  memcpy(lexer->buf, stream, BUFSIZ);
  lexer->file_name = "<string>";
  return lexer;
}

pString Lexer_get_text(Lexer* l) {
  pString result = String_new(l->_text, l->_textLength);

  l->_text[0] = 0;
  l->_textLength = 0;

  return result;
}

void Lexer_consumed_text(Lexer* l) {
  l->_text[0] = 0;
  l->_textLength = 0;
}

#define _BC (l->buf[l->bufp])
#define EOB ((l->bufp >= BUFSIZ) || (l->buf[l->bufp]==0))
#define _MATCH(C, S) \
  if(_BC == (C)) { l->sym = (S); l->symc = _BC; l->_textLength = sprintf(l->_text, "%c", _BC); incBufP(l);}

#define SEPARATOR "----"
#define PRIMITIVE "primitive"

static void incBufP(Lexer* const l) {
  l->bufp++;
  if (l->bufp >= BUFSIZ) {
    debug_error("Lexer failed to read file. Run out of file reading buffer space.");
    exit(ERR_FAIL);
  }
}

int fillbuffer(Lexer* const l) {
  if (!l->infile) // string stream
    return -1;
  int p = 0;
  do {
    if (p >= BUFSIZ) {
      debug_error("Lexer failed to read file. Run out of file reading buffer space.");
      exit(ERR_FAIL);
    }
    
    if (!feof(l->infile)) {
      l->buf[p] = fgetc(l->infile);
    } else {
      l->buf[p] = '\n';
    }
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

    incBufP(l);
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
      incBufP(l);
      while (EOB) {
        if (fillbuffer(l) == -1) {
          return;
        }
      }
    } while(_BC != '"');
    incBufP(l);
  }
}

void addChar(Lexer* l, char character) {
  if (l->_textLength >= BUFSIZ) {
    debug_error("%s:%d: Element too long. Doesn't fit into text buffer.", l->file_name, l->line_num);
    exit(ERR_FAIL);
  }

  l->_text[l->_textLength] = character;
  l->_textLength++;
}

void terminateText(Lexer* l) {
  l->_text[l->_textLength] = '\0';
}

void lexNumber(Lexer* l) {
  l->sym = Integer;
  l->symc = 0;

  bool saw_decimal_mark = false;

  do {
    addChar(l, l->buf[l->bufp]);
    incBufP(l);

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
      addChar(l, l->buf[l->bufp]);
      incBufP(l);
    }
  } while(isdigit(_BC));
}

void lexEscapeChar(Lexer* l) {
  char current = _BC;

  switch (current) {
    case 't': addChar(l, '\t'); break;
    case 'b': addChar(l, '\b'); break;
    case 'n': addChar(l, '\n'); break;
    case 'r': addChar(l, '\r'); break;
    case 'f': addChar(l, '\f'); break;
    case '0': addChar(l, '\0'); break;
    case '\'': addChar(l, '\''); break;
    case '\\': addChar(l, '\\'); break;
  }
  incBufP(l);
}

void lexStringChar(Lexer* l) {
  char current = _BC;

  if (current == '\\') {
    incBufP(l);
    lexEscapeChar(l);
  } else {
    addChar(l, current);
    incBufP(l);
  }
}

void lexString(Lexer* l) {
  l->sym = STString;
  l->symc = 0;
  incBufP(l);

  while(_BC != '\'') {
    while (EOB) {
      if (fillbuffer(l) == -1) {
        return;
      }
      addChar(l, '\n');
    }

    if (_BC == '\n') {
      l->line_num += 1;
    } else if (_BC != '\'') {
      lexStringChar(l);
    }
  }

  incBufP(l);
}

void lexOperator(Lexer* l) {
  if(_ISOP(l->buf[l->bufp + 1])) {
    l->sym = OperatorSequence;
    l->symc = 0;

    while(_ISOP(_BC)) {
      addChar(l, _BC);
      incBufP(l);
    }
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
    memcpy(l->_text, l->_nextText, l->_nextTextLength);
    l->_textLength = l->_nextTextLength;
    terminateText(l);

    return l->sym;
  }

  Lexer_consumed_text(l);

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
      incBufP(l);
      incBufP(l);

      l->sym = Assign;
      l->symc = 0;
      l->_textLength = sprintf(l->_text, ":=");
    } else {
      incBufP(l);

      l->sym = Colon;
      l->symc = ':';
      l->_textLength = sprintf(l->_text, ":");
    }
  }
  else _MATCH('(', NewTerm)
  else _MATCH(')', EndTerm)
  else _MATCH('#', Pound)
  else _MATCH('^', Exit)
  else _MATCH('.', Period)
  else if(_BC == '-') {
    if(!strncmp(l->buf + l->bufp, SEPARATOR, strlen(SEPARATOR))) {

      while(_BC == '-') {
        addChar(l, _BC);
        incBufP(l);
      }
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
    l->_textLength = sprintf(l->_text, PRIMITIVE);
  }
  else if(isalpha(_BC)) {
    l->symc = 0;
    while(isalpha(_BC) || isdigit(_BC) || _BC == '_') {
      addChar(l, _BC);
      incBufP(l);
    }
    l->sym = Identifier;
    if (_BC == ':') {
      l->sym = Keyword;
      incBufP(l);
      addChar(l, ':');
      if (isalpha(_BC)) {
        l->sym = KeywordSequence;
        while (isalpha(_BC) || _BC == ':') {
          addChar(l, _BC);
          incBufP(l);
        }
      }
    }
  }
  else if(isdigit(_BC)) {
    lexNumber(l);
  }
  else {
    l->sym = NONE;
    l->symc = _BC;
    l->_textLength = sprintf(l->_text, "%c", _BC);
  }

  terminateText(l);
  return l->sym;
}

void Lexer_peek(Lexer* l) {
  Symbol saveSym = l->sym;
  char saveSymc = l->symc;
  char saveText[BUFSIZ];
  memcpy(saveText, l->_text, l->_textLength);
  size_t saveTextLength = l->_textLength;
  saveText[saveTextLength] = '\0';

  if (l->peekDone) {
    fprintf(stderr, "Cannot peek twice!\n");
  }

  Lexer_get_sym(l);
  l->nextSym = l->sym;
  l->nextSymc = l->symc;

  memcpy(l->_nextText, l->_text, l->_textLength);
  l->_nextTextLength = l->_textLength;
  l->_nextText[l->_textLength] = '\0';

  l->sym = saveSym;
  l->symc = saveSymc;
  memcpy(l->_text, saveText, saveTextLength);
  l->_textLength = saveTextLength;
  terminateText(l);

  l->peekDone = true;
}

void Lexer_peek_if_necessary(Lexer* l) {
  if (!l->peekDone) {
    Lexer_peek(l);
  }
}

#undef _BC
#undef EOB

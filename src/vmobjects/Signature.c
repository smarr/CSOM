/*
 * $Id: Signature.c 792 2009-04-06 08:07:33Z michael.haupt $
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

#include "Signature.h"

#include <stdlib.h>
#include <string.h>

#include <misc/debug.h>
#include <misc/String.h>

int Signature_get_number_of_arguments(pVMSymbol sig) {
    // check default binaries
    if (Signature_is_binary(sig)) {
        return 2;
    } else {
        const char* str = SEND(sig, get_rawChars);
        size_t length = SEND(sig, get_length);

        // colons in str
        int num_colons = 0;
        
        // search the str
        for (int i = 0 ; i < length; i++) {
            if (str[i] == ':') {
                // additional colon found
                num_colons++;
            }
        }
        
        // The number of arguments is equal to the number of colons plus one
        // (->> SELF)
        return num_colons + 1;        
    }    
}


bool Signature_is_binary(pVMSymbol sig) {
    const char* sigstr = SEND(sig, get_rawChars);
    size_t length = SEND(sig, get_length);

    if (length < 1) {
        return false;
    }

    switch (sigstr[0]) {
            case '~' :
            case '&' :
            case '|' :
            case '*' :
            case '/' :
            case '@' :
            case '+' :
            case '-' :
            case '=' :
            case '>' :
            case '<' :
            case ',' :
            case '%' :
            case '\\':        
                return true;
            default: break;
    }
    return false;
}



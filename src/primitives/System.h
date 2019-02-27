#ifndef CORE_SYSTEM_H_
#define CORE_SYSTEM_H_

/*
 * $Id: System.h 109 2007-09-17 20:39:52Z tobias.pape $
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

#include <vmobjects/OOObject.h>
#include <time.h>

void _System_global_(pVMObject object, pVMFrame frame);
void _System_global_put_(pVMObject object, pVMFrame frame);
void _System_hasGlobal_(pVMObject object, pVMFrame frame);
void _System_load_(pVMObject object, pVMFrame frame);
void _System_exit_(pVMObject object, pVMFrame frame);
void _System_printString_(pVMObject object, pVMFrame frame);
void _System_printNewline(pVMObject object, pVMFrame frame);
void _System_time(pVMObject object, pVMFrame frame);
void _System_ticks(pVMObject object, pVMFrame frame);
void _System_fullGC(pVMObject object, pVMFrame frame);

void __System_init(void);
extern struct timeval _System_start_time;

#endif // CORE_SYSTEM_H_

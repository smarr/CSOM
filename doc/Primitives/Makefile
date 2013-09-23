#!/usr/bin/env make

#
# $Id: Makefile 95 2007-09-10 09:48:20Z tobias.pape $
#
# Copyright (c) 2007 Michael Haupt, Tobias Pape
# Software Architecture Group, Hasso Plattner Institute, Potsdam, Germany
# http://www.hpi.uni-potsdam.de/swa/
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


#configure here.

CLASSNAME :=Hello

SRC_DIR		=../../src
INCLUDES	=-I$(SRC_DIR)
LIBS			=
OSTOOL_NAME:=ostool.exe
SOM2LIB		=$(SRC_DIR)/primitives/som2lib.rb


# do not change anything beneath this line
#==============================================================================

## we need c99!
CC			=gcc
CFLAGS		=-Wno-endif-labels -std=gnu99 $(DBG_FLAGS) $(OFLAGS) $(INCLUDES)
LDFLAGS		=

INSTALL		=install
OSTOOL		:=$(SRC_DIR)/$(OSTOOL_NAME)

.SUFFIXES: .som .pic.o

.som.c:
	$(SOM2LIB) $*.som

.c.pic.o:
	$(CC) $(CFLAGS) -fPIC -g -c $< -o $*.pic.o

all:	PRIMITIVE

PRIMITIVE: $(CLASSNAME).pic.o
	@echo Linking $(CLASSNAME)
	$(CC) $(LDFLAGS) `$(OSTOOL) l "$(CLASSNAME)"` \
		-o `$(OSTOOL) s "$(CLASSNAME).som"`  $(CLASSNAME).pic.o $(LIBS)
	@touch PRIMITIVE
	@echo Done.

$(OSTOOL):
	$(MAKE) -C $(INCLUDES) $(OSTOOL_NAME)
#!/usr/bin/env make -f

#
#  Makefile by Tobias Pape
#
# $Id: windows.make 407 2008-05-20 12:41:10Z stefan.marr $
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

## we need c99!

CC			=gcc
CFLAGS		=$(COMPILER_ARCH) -Wno-endif-labels -std=gnu99 $(DBG_FLAGS) $(INCLUDES)
LDFLAGS		=$(COMPILER_ARCH) $(LIBRARIES)

INSTALL		=install

CSOM_LIBS	=
CORE_LIBS	=-lm

CSOM_NAME	=CSOM
CORE_NAME	=SOMCore

############ global stuff -- overridden by ../Makefile

ROOT_DIR	?= $(PWD)/..
SRC_DIR		?= $(ROOT_DIR)/src
BUILD_DIR   ?= $(ROOT_DIR)/build
DEST_DIR	?= $(ROOT_DIR)/build.out

ST_DIR		?= $(ROOT_DIR)/Smalltalk
EX_DIR		?= $(ROOT_DIR)/Examples
TEST_DIR	?= $(ROOT_DIR)/TestSuite

############# "component" directories


COMPILER_DIR 	= $(SRC_DIR)/compiler
INTERPRETER_DIR = $(SRC_DIR)/interpreter
MEMORY_DIR 		= $(SRC_DIR)/memory
MISC_DIR 		= $(SRC_DIR)/misc
VM_DIR 			= $(SRC_DIR)/vm
VMOBJECTS_DIR 	= $(SRC_DIR)/vmobjects

COMPILER_SRC	= $(wildcard $(COMPILER_DIR)/*.c)
COMPILER_OBJ	= $(COMPILER_SRC:.c=.o)
INTERPRETER_SRC	= $(wildcard $(INTERPRETER_DIR)/*.c)
INTERPRETER_OBJ	= $(INTERPRETER_SRC:.c=.o)
MEMORY_SRC		= $(wildcard $(MEMORY_DIR)/*.c)
MEMORY_OBJ		= $(MEMORY_SRC:.c=.o)
MISC_SRC		= $(wildcard $(MISC_DIR)/*.c)
MISC_OBJ		= $(MISC_SRC:.c=.o)
VM_SRC			= $(wildcard $(VM_DIR)/*.c)
VM_OBJ			= $(VM_SRC:.c=.o)
VMOBJECTS_SRC	= $(wildcard $(VMOBJECTS_DIR)/*.c)
VMOBJECTS_OBJ	= $(VMOBJECTS_SRC:.c=.o)

MAIN_SRC		= $(SRC_DIR)/main.c
MAIN_OBJ		= $(SRC_DIR)/main.o

############# primitives location etc.

PRIMITIVES_DIR	= $(SRC_DIR)/primitives
PRIMITIVES_SRC	= $(wildcard $(PRIMITIVES_DIR)/*.c)
PRIMITIVES_OBJ	= $(PRIMITIVES_SRC:.c=.pic.o)

############# include path

INCLUDES		=-I$(SRC_DIR)
LIBRARIES		=-L$(ROOT_DIR)

##############
############## Collections.

CSOM_OBJ		=  $(MEMORY_OBJ) $(MISC_OBJ) $(VMOBJECTS_OBJ) \
				$(COMPILER_OBJ) $(INTERPRETER_OBJ) $(VM_OBJ) 

OBJECTS			= $(MAIN_OBJ) $(CSOM_OBJ) $(PRIMITIVES_OBJ)

SOURCES			=  $(COMPILER_SRC) $(INTERPRETER_SRC) $(MEMORY_SRC) \
				$(MISC_SRC) $(VM_SRC) $(VMOBJECTS_SRC)  \
				$(PRIMITIVES_SRC) $(MAIN_SRC)

############# Things to clean

CLEAN			= $(OBJECTS) \
				$(DIST_DIR) $(DEST_DIR) CORE
############# Tools

OSTOOL			= $(BUILD_DIR)/ostool.exe

#
#
#
#  metarules
#

.SUFFIXES: .pic.o

.PHONY: clean clobber test

all: $(OSTOOL) \
	$(SRC_DIR)/platform.h \
	$(CSOM_NAME).exe \
	$(CSOM_NAME).dll \
	CORE


debug : DBG_FLAGS=-DDEBUG -g
debug: all

profiling : DBG_FLAGS=-g -pg
profiling : LDFLAGS+=-pg
profiling: all


.c.pic.o:
	$(CC) $(CFLAGS) -g -c $< -o $*.pic.o


clean:
	rm -Rf $(CLEAN)


clobber: $(OSTOOL) clean
	rm -f `$(OSTOOL) x "$(CSOM_NAME)"` \
		$(CSOM_NAME).dll $(CSOM_NAME).lib $(CORE_NAME).lib \
		$(ST_DIR)/`$(OSTOOL) s "$(CORE_NAME)"`
	rm -f $(OSTOOL)
	rm -f $(SRC_DIR)/platform.h

$(OSTOOL): $(BUILD_DIR)/ostool.c
	$(CC) -g -Wno-endif-labels -o $(OSTOOL) $(BUILD_DIR)/ostool.c

$(SRC_DIR)/platform.h: $(OSTOOL)
	@($(OSTOOL) i >$(SRC_DIR)/platform.h)

#
#
#
# product rules
#

$(CSOM_NAME).exe: $(CSOM_NAME).dll $(MAIN_OBJ)
	@echo Linking $(CSOM_NAME) loader
	$(CC) $(LDFLAGS) `$(OSTOOL) l` \
		-o `$(OSTOOL) x "$(CSOM_NAME)"` \
		`$(OSTOOL) c o` $(MAIN_OBJ) \
		`$(OSTOOL) c l` -l$(CSOM_NAME)

$(CSOM_NAME).dll: $(CSOM_OBJ)
	@echo Linking $(CSOM_NAME) Dynamic Library
	$(CC) $(LDFLAGS) `$(OSTOOL) l "$(CSOM_NAME)"` \
		-o $(CSOM_NAME).dll \
		`$(OSTOOL) c o` $(CSOM_OBJ) \
		`$(OSTOOL) c l` $(CSOM_LIBS) 
	@echo CSOM done.

CORE: $(CSOM_NAME).dll $(PRIMITIVES_OBJ)
	@echo Linking SOMCore lib
	$(CC) $(LDFLAGS) `$(OSTOOL) l "$(CORE_NAME)"` \
		-o `$(OSTOOL) s "$(CORE_NAME)"` \
		`$(OSTOOL) c o` $(PRIMITIVES_OBJ) \
		`$(OSTOOL) c l` $(CORE_LIBS) -l$(CSOM_NAME)
	mv `$(OSTOOL) s "$(CORE_NAME)"` $(ST_DIR)
	@touch CORE
	@echo SOMCore done.

install: all
	@echo installing CSOM into build
	$(INSTALL) -d $(DEST_DIR)
	$(INSTALL) `$(OSTOOL) x "$(CSOM_NAME)"` $(DEST_DIR)
	$(INSTALL) $(CSOM_NAME).dll $(DEST_DIR)
	@echo CSOM.
	cp -Rpf $(ST_DIR) $(EX_DIR) $(TEST_DIR)  $(DEST_DIR)
	@echo Library.
	@echo done.

#
# test: run the standard test suite
#
test: install
	@(cd $(DEST_DIR); \
	./$(CSOM_NAME).exe -cp Smalltalk TestSuite\\TestHarness.som;)

#
# bench: run the benchmarks
#
bench: install
	@(cd $(DEST_DIR); \
	./$(CSOM_NAME).exe -cp Smalltalk Examples\\Benchmarks\\All.som;)

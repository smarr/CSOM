#!/usr/bin/env make -f

#
#  Makefile by Tobias Pape
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

ifeq ($(CC),)
	CC		= gcc
endif

OPT_FLAGS	=-O3
CFLAGS		=$(COMPILER_ARCH) -Wno-endif-labels -std=gnu99 $(DBG_FLAGS) $(OPT_FLAGS) $(INCLUDES)
LDFLAGS		=$(COMPILER_ARCH) $(LIBRARIES)

INSTALL		=install

CSOM_LIBS	=-ldl
CORE_LIBS	=-lm

CSOM_NAME	=CSOM
SOM_NAME	=SOM
CORE_NAME	=$(SOM_NAME)Core

############ global stuff -- overridden by ../Makefile

PREFIX		?= /usr/local

ROOT_DIR	?= $(PWD)/..
SRC_DIR		?= $(ROOT_DIR)/src
BUILD_DIR   ?= $(ROOT_DIR)/build

DEST_SHARE	?= $(PREFIX)/share/$(SOM_NAME)
DEST_BIN	?= $(PREFIX)/bin
DEST_LIB	?= $(PREFIX)/lib/$(SOM_NAME)


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
VM_SRC			= $(wildcard $(VM_DIR)/*.c) $(SRC_DIR)/main.c
VM_OBJ			= $(VM_SRC:.c=.o)
VMOBJECTS_SRC	= $(wildcard $(VMOBJECTS_DIR)/*.c)
VMOBJECTS_OBJ	= $(VMOBJECTS_SRC:.c=.o)

############# primitives location etc.

PRIMITIVES_DIR	= $(SRC_DIR)/primitives
PRIMITIVES_SRC	= $(wildcard $(PRIMITIVES_DIR)/*.c)
PRIMITIVES_OBJ	= $(PRIMITIVES_SRC:.c=.pic.o)

############# unit tests

UNITTEST_DIR	= $(ROOT_DIR)/tests
UNITTEST_SRC	= $(wildcard $(UNITTEST_DIR)/*.c)
UNITTEST_OBJ	= $(UNITTEST_SRC:.c=.o)

############# include path

INCLUDES		=-I$(SRC_DIR)
LIBRARIES       =

##############
############## Collections.

CSOM_OBJ		=  $(MEMORY_OBJ) $(MISC_OBJ) $(VMOBJECTS_OBJ) \
					$(COMPILER_OBJ) $(INTERPRETER_OBJ) $(VM_OBJ)
OBJECTS			= $(CSOM_OBJ) $(PRIMITIVES_OBJ)

SOURCES			=  $(COMPILER_SRC) $(INTERPRETER_SRC) $(MEMORY_SRC) \
					$(MISC_SRC) $(VM_SRC) $(VMOBJECTS_SRC)  \
					$(PRIMITIVES_SRC)

############# Things to clean

CLEAN			= $(OBJECTS) CORE $(SRC_DIR)/unittest

############# Tools

OSTOOL			= $(BUILD_DIR)/ostool.exe

#
#
#
#  metarules
#

.SUFFIXES: .pic.o

.PHONY: clean clobber test

all: $(OSTOOL) $(SRC_DIR)/platform.h CORE $(SRC_DIR)/CSOM


debug : OPT_FLAGS=-O0
debug : DBG_FLAGS=-DDEBUG -g3
debug: all

profiling : DBG_FLAGS=-g -pg
profiling : LDFLAGS+=-pg
profiling: all

# emscripten : DBG_FLAGS=-g
emscripten : EMBED_FILES+=--embed-file Smalltalk --embed-file Examples --embed-file TestSuite
emscripten : EXP_FN=-s EXPORTED_FUNCTIONS="['_strcmp']" -s ENVIRONMENT=node -s EXPORT_ALL=1 -s LINKABLE=1 -s WASM=1 -s EXIT_RUNTIME=1 -s TOTAL_MEMORY=150MB
emscripten : MAIN_MODULE=-s MAIN_MODULE=1 $(EMBED_FILES) $(EXP_FN)
emscripten : SIDE_MODULE=-s SIDE_MODULE=1 $(EXP_FN)
emscripten : CC=emcc
emscripten : LDFLAGS+=
emscripten : DEF_EMSCRIPTEN=-D__EMSCRIPTEN__
emscripten: all

em-awfy : AWFY_ROOT?=../../benchmarks/SOM
em-awfy : EMBED_FILES+=--embed-file $(AWFY_ROOT)
em-awfy: emscripten


.c.pic.o:
	$(CC) $(CFLAGS) -fPIC -c $< -o $*.pic.o


clean:
	rm -Rf $(CLEAN)


clobber: $(OSTOOL) clean
	rm -f `$(OSTOOL) x "$(CSOM_NAME)"` $(ST_DIR)/`$(OSTOOL) s "$(CORE_NAME)"`
	rm -f $(OSTOOL)
	rm -f CSOM.js CSOM.wasm Smalltalk/SOMCore.wasm
	rm -f $(SRC_DIR)/platform.h

$(OSTOOL): $(BUILD_DIR)/ostool.c
	# using cc directly to avoid using emcc for emscripten
	cc -g -Wno-endif-labels -o $(OSTOOL) $(DEF_EMSCRIPTEN) $(BUILD_DIR)/ostool.c

$(SRC_DIR)/platform.h: $(OSTOOL)
	@($(OSTOOL) i >$(SRC_DIR)/platform.h)

core-lib/.gitignore:
	git submodule update --init

#
#
#
# product rules
#


$(SRC_DIR)/CSOM: $(CSOM_OBJ) CORE
	@echo Linking CSOM
	$(CC) $(MAIN_MODULE) $(DBG_FLAGS) $(LDFLAGS) `$(OSTOOL) l`\
		-o `$(OSTOOL) x "$(CSOM_NAME)"` \
		$(CSOM_OBJ) $(CSOM_LIBS)
	@echo CSOM done.

CORE: core-lib/.gitignore $(PRIMITIVES_OBJ)
	@echo Linking SOMCore lib
	$(CC) $(SIDE_MODULE) $(DBG_FLAGS) $(LDFLAGS) `$(OSTOOL) l "$(CORE_NAME)"` \
		-o `$(OSTOOL) s "$(CORE_NAME)"`\
		$(PRIMITIVES_OBJ) $(CORE_LIBS)
	mv `$(OSTOOL) s "$(CORE_NAME)"` $(ST_DIR)
	@touch CORE
	@echo SOMCore done.

install: all
	@echo installing CSOM into build
	$(INSTALL) -d $(DEST_SHARE) $(DEST_BIN) $(DEST_LIB)
	$(INSTALL) `$(OSTOOL) x "$(CSOM_NAME)"` $(DEST_BIN)
	@echo CSOM.
	cp -R $(ST_DIR) $(DEST_LIB)
	@echo Library.
	cp -R $(EX_DIR) $(TEST_DIR) $(DEST_SHARE)
	@echo shared components.
	@echo done.

uninstall:
	@echo removing Library and shared Components
	rm -Rf $(DEST_SHARE) $(DEST_LIB)
	@echo removing CSOM
	rm -Rf $(DEST_BIN)/`$(OSTOOL) x "$(CSOM_NAME)"`
	@echo done.

#
# test: run the standard test suite
#
test: all $(SRC_DIR)/unittest
	$(SRC_DIR)/unittest
	@(./CSOM -cp Smalltalk TestSuite/TestHarness.som;)

$(SRC_DIR)/unittest: all $(UNITTEST_OBJ)
	echo $(UNITTEST_DIR)
	echo $(UNITTEST_OBJ)
	$(CC) $(DBG_FLAGS) $(LDFLAGS) `$(OSTOOL) l` \
	  -o $(SRC_DIR)/unittest $(filter-out $(SRC_DIR)/main.o,$(CSOM_OBJ)) $(CSOM_LIBS) $(UNITTEST_OBJ)

#
# bench: run the benchmarks
#
bench: all
	@(./CSOM -cp Smalltalk Examples/Benchmarks/All.som;)

bench-gc: all
	@(./CSOM -g -cp Smalltalk Examples/Benchmarks/All.som;)

ddbench: all
	@(./CSOM -d -d -cp Smalltalk Examples/Benchmarks/All.som;)

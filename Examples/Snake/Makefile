#!/usr/bin/env make

TARGET		=Terminal


################ ONLY CHANGE STUFF BELOW IF YOU REALLY HAVE ###################


CC			=gcc
CFLAGS		=-Wno-endif-labels -std=gnu99 $(DBG_FLAGS) $(INCLUDES)
LDFLAGS		=$(LIBRARIES)

INSTALL		=install

############ global stuff -- overridden by ../Makefile

ROOT_DIR	?= $(PWD)/../..
BUILD_DIR   ?= $(ROOT_DIR)/build
SRC_DIR		?= $(ROOT_DIR)/src

ST_DIR		?= $(ROOT_DIR)/Smalltalk
EX_DIR		?= $(ROOT_DIR)/Examples
TEST_DIR	?= $(ROOT_DIR)/TestSuite

############# include path

INCLUDES		=-I$(SRC_DIR)
LIBRARIES		=-L$(ROOT_DIR)

ifneq ($(OS),)
# only Windows has OS predefined.
	LIBS		=-lCSOM
endif

OSTOOL			= $(BUILD_DIR)/ostool.exe

#
#
#
#  metarules
#

.SUFFIXES: .csp

.PHONY: clean

all: $(TARGET).csp

debug : DBG_FLAGS=-DDEBUG -g
debug: all

profiling : DBG_FLAGS=-g -pg
profiling : LDFLAGS+=-pg
profiling: all

clean:
	rm -Rf $(TARGET).csp

.c.csp:
	$(CC) $(CFLAGS) $(LDFLAGS) `$(OSTOOL) l $*`\
		-o `$(OSTOOL) s $*` $< $(LIBS)

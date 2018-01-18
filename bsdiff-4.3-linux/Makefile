# ------------------------------------------------------------------
# This file is part of bzip2/libbzip2, a program and library for
# lossless, block-sorting data compression.
#
# bzip2/libbzip2 version 1.0.6 of 6 September 2010
# Copyright (C) 1996-2010 Julian Seward <jseward@bzip.org>
#
# Please read the WARNING, DISCLAIMER and PATENTS sections in the 
# README file.
#
# This program is released under the terms of the license contained
# in the file LICENSE.
# ------------------------------------------------------------------
CC=gcc
CFLAGS=-Wall -O2 -I../bzip2-1.0.6 
LIBS=-L../bzip2-1.0.6 -lbz2

# Where you want it installed when you do 'make install'
DPRG=bsdiff
DOBJS= xmd5.o \
      bsdiff.o

PPRG=bspatch
POBJS= xmd5.o \
      bspatch.o

all: $(DPRG) $(PPRG)


$(DPRG): $(DOBJS)
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

$(PPRG): $(POBJS)
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

clean: 
	$(RM) *.o $(DPRG) $(PPRG) 

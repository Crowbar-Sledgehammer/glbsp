#
# glBSP Plugin Makefile for Win32
#
# (Note: requires GNU make)
# 

# SHELL=bash.exe

BIN=libglbsp.a

# INCS=-I../DEV-CPP/include  -Iinclude

CC=gcc.exe
CFLAGS=-O3 -Wall -DGLBSP_PLUGIN -DWIN32 -DINLINE_G=inline

AR=ar r
RANLIB=ranlib

# ----- OBJECTS ------------------------------------------------------

OBJS= \
    analyze.o \
    blockmap.o \
    glbsp.o \
    level.o \
    node.o \
    reject.o \
    seg.o \
    system.o \
    util.o \
    wad.o

# ----- TARGETS ------------------------------------------------------

all: $(BIN)

clean:
	rm -f $(OBJS) $(BIN) 
	
$(BIN): $(OBJS)
	$(AR) $(BIN) $(OBJS)
	$(RANLIB) $(BIN)
 
.PHONY: all clean


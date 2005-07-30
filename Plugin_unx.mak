#
# glBSP Plugin Makefile for UNIX/Linux
#

BIN=libglbsp.a

CC=gcc
CFLAGS=-O2 -Wall -DGLBSP_PLUGIN -DUNIX -DINLINE_G=inline

AR=ar rc
RANLIB=ranlib

# ----- OBJECTS ------------------------------------------------------

OBJS=analyze.o  \
     blockmap.o \
     glbsp.o    \
     level.o    \
     node.o     \
     reject.o   \
     seg.o      \
     system.o   \
     util.o     \
     wad.o


# ----- TARGETS ------------------------------------------------------

all:    $(BIN)

clean:
	rm -f $(BIN) *.o

$(BIN): $(OBJS)
	$(AR) $(BIN) $(OBJS)
	$(RANLIB) $(BIN)

.PHONY: all clean


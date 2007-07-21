#
# glBSP Plugin Makefile for UNIX/Linux
#

BIN=libglbsp.a

CC=gcc
CFLAGS=-O2 -Wall -DGLBSP_PLUGIN -DUNIX -DINLINE_G=inline

AR=ar rc
RANLIB=ranlib

# ----- OBJECTS ------------------------------------------------------

OBJS= \
     src/analyze.o  \
     src/blockmap.o \
     src/glbsp.o    \
     src/level.o    \
     src/node.o     \
     src/reject.o   \
     src/seg.o      \
     src/system.o   \
     src/util.o     \
     src/wad.o


# ----- TARGETS ------------------------------------------------------

all:    $(BIN)

clean:
	rm -f $(BIN) *.o

$(BIN): $(OBJS)
	$(AR) $(BIN) $(OBJS)
	$(RANLIB) $(BIN)

.PHONY: all clean


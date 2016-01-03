################################################################################
SDIR =src
IDIR =inc
CC=g++
CFLAGS=-I$(IDIR) -g

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = Client.h Constants.h Help.h Server.h AddressList.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Client.o Server.o Help.o sarangde_proj1.o AddressList.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -v -o $@ $< $(CFLAGS)
all: ntwrk_asignmnt1

ntwrk_asignmnt1: $(OBJ)
	g++ -v -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 


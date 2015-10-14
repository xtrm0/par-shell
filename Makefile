IDIR=include
ODIR=objects
CDIR=source
TDIR=$(CDIR)/tests
BDIR=bin

OBJECTS=commandlinereader.o processManager.o
OBJECTSPATH = $(patsubst %,$(ODIR)/%,$(OBJECTS))
LNLIBS=-lpthread

CC = gcc
LN = gcc
FLAGS=-O3 -Ofast -c -Wall -I$(IDIR)
LNFLAGS=-O3 -Ofast

.PHONY: all clean debug run drun

all: $(BDIR)/par-shell

#Compila e corre o projeto
run: $(BDIR)/par-shell
		$(BDIR)/par-shell

#Compila o projeto em modo debug e corre
drun: debug
		$(BDIR)/par-shell

release: clean
release: $(BDIR)/par-shell

debug: clean
debug: FLAGS = -c -D_DEBUG -g -Wall -I$(IDIR)
debug: LNFLAGS = -D_DEBUG -g -Wall
debug: $(BDIR)/par-shell

clean:
		-rm -v $(ODIR)/*.o $(BDIR)/par-shell
#rules to link par-shell
$(BDIR)/par-shell: $(OBJECTSPATH) $(ODIR)/main.o
		$(LN) $(LNFLAGS) $(OBJECTSPATH) $(ODIR)/main.o $(LNLIBS) -o $(BDIR)/par-shell

#rules to compile objects on code dir
$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) $(FLAGS) $< -o $@

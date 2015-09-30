FLAGS=-O3 -Ofast -c -Wall -Iinclude
LNFLAGS=-O3 -Ofast
LNLIBS=
CC = gcc
LN = gcc
IDIR=include
ODIR=objects
CDIR=source
TDIR=$(CDIR)/tests
BDIR=bin
OBJECTS=commandlinereader.o
OBJECTSPATH = $(patsubst %,$(ODIR)/%,$(OBJECTS))

.PHONY: all clean debug precommit

all: par-shell

#Compila e corre o projeto
run: par-shell
		$(BDIR)/par-shell

#Compila o projeto em modo debug e corre
drun: debug
		$(BDIR)/par-shell

release: clean
release: par-shell

debug: clean
debug: FLAGS = -c -D_DEBUG -g -Wall
debug: LNFLAGS += -D_DEBUG -g -Wall
debug: par-shell

clean:
		rm -v $(OBJECTSPATH) $(ODIR)/main.o $(BDIR)/par-shell
#rules to link par-shell
par-shell: $(OBJECTSPATH) $(ODIR)/main.o
		$(LN) $(LNFLAGS) $(OBJECTSPATH) $(ODIR)/main.o $(LNLIBS) -o $(BDIR)/par-shell

#rules to compile objects on code dir
$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) $(FLAGS) $< -o $@
#rules to compile objects on tests dir
$(ODIR)/%.o: $(TDIR)/%.c
	$(CC) $(FLAGS) $< -o $@

#before making commits, make clean shall be run!
precommit: clean

##############
# UNIT TESTS #
##############
#NONE YET

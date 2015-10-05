
#compilador:
CC = gcc -Wall -O3 -Ofast

#regra para compilar ./par-shell
par-shell: commandlinereader.o main.o
	$(CC) -o par-shell main.o commandlinereader.o

#regra para compilar ./commandlinereader.o
commandlinereader.o: commandlinereader.c commandlinereader.h
	$(CC) -g -c commandlinereader.c

#regra para compilar ./main.o
main.o: main.c commandlinereader.h defines.h
	$(CC) -g -c main.c

#pseudo regras: (nao definem ficheiros para compilar, mas sim modos de funcionamento do make)
.PHONY: all clean

all: par-shell

clean:
	rm ./*.o ./par-shell

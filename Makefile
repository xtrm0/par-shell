
#compilador:
CC = gcc -Wall -O3 -Ofast

all: par-shell par-shell-terminal fibonacci

#regra para compilar ./par-shell
par-shell: commandlinereader.o processList.o terminalList.o main.o
	$(CC) -o par-shell main.o commandlinereader.o processList.o terminalList.o -lpthread -lm

par-shell-terminal: terminal.c
	$(CC) -o par-shell-terminal terminal.c

#regra para compilar ./commandlinereader.o
commandlinereader.o: commandlinereader.c commandlinereader.h
	$(CC) -g -c commandlinereader.c

#regra para compilar ./main.o
main.o: main.c commandlinereader.h defines.h processList.h
	$(CC) -g -c main.c

#regra para compilar ./processList.o
processList.o: processList.c processList.h
	$(CC) -g -c processList.c

#regra para compilar ./terminalList.o
terminalList.o: terminalList.c terminalList.h
	$(CC) -g -c terminalList.c


#pseudo regras: (nao definem ficheiros para compilar, mas sim modos de funcionamento do make)
.PHONY: all clean

clean:
	rm -f ./*.o ./par-shell ./par-shell-terminal ./fibonacci log.txt par-shell-in par-shell-out-*

fibonacci: fibonacci.c
	gcc fibonacci.c -o fibonacci

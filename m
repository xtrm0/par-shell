all: par-shell fibonacci div0

par-shell: par-shell.o commandlinereader.o
	gcc -o par-shell par-shell.o commandlinereader.o

par-shell.o: par-shell.c commandlinereader.h
	gcc -g -Wall -c par-shell.c

commandlinereader.o: commandlinereader.c 
	gcc -g -Wall -c commandlinereader.c

fibonacci: fibonacci.c
	gcc -o fibonacci -Wall fibonacci.c

div0: div.c
	gcc -o div0 -g -Wall div.c

run:
	clear
	./par-shell < in2.txt

clean:
	rm -f *.o par-shell fibonacci div0 core


## makefile for smallsh project

smallsh: smallsh.o
	gcc smallsh.o -o smallsh

smallsh.o: smallsh.c
	gcc -c smallsh.c

clean:
	rm *.o smallsh
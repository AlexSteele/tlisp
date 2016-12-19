
CC = gcc
CCOPTS = -Wall

default: tlisp

tlisp: tlisp.o
	$(CC) $(CCOPTS) bin/tlisp.o -o bin/tlisp

tlisp.o: src/tlisp.c
	$(CC) $(CCOPTS) -c src/tlisp.c -o bin/tlisp.o

clean:
	rm -rf bin

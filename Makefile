
CC = gcc
CCOPTS = -Wall -Wpedantic

default: tlisp

bin:
	mkdir -p bin

tlisp: bin tlisp.o builtins.o core.o env.o read.o tlisp.o
	$(CC) $(CCOPTS) bin/builtins.o bin/core.o bin/env.o bin/read.o bin/tlisp.o -o bin/tlisp

builtins.o: bin src/builtins.c src/builtins.h
	$(CC) $(CCOPTS) -c src/builtins.c -o bin/builtins.o

core.o: bin src/core.c src/core.h
	$(CC) $(CCOPTS) -c src/core.c -o bin/core.o

env.o: bin src/env.c src/env.h
	$(CC) $(CCOPTS) -c src/env.c -o bin/env.o

read.o: bin src/read.c src/read.h
	$(CC) $(CCOPTS) -c src/read.c -o bin/read.o

tlisp.o: bin src/tlisp.c
	$(CC) $(CCOPTS) -c src/tlisp.c -o bin/tlisp.o

clean:
	rm -rf bin

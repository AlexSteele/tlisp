
CC = gcc
CCOPTS = -Wall -Wpedantic

default: tlisp

bin:
	mkdir -p bin

tlisp: bin tlisp.o builtins.o core.o dict.o env.o gc.o list.o process.o read.o struct.o tlisp.o vector.o
	$(CC) $(CCOPTS) bin/*.o -o bin/tlisp

builtins.o: bin src/builtins.c src/builtins.h
	$(CC) $(CCOPTS) -c src/builtins.c -o bin/builtins.o

core.o: bin src/core.c src/core.h
	$(CC) $(CCOPTS) -c src/core.c -o bin/core.o

dict.o: bin src/dict.c src/dict.h
	$(CC) $(CCOPTS) -c src/dict.c -o bin/dict.o

env.o: bin src/env.c src/env.h
	$(CC) $(CCOPTS) -c src/env.c -o bin/env.o

gc.o: bin src/gc.c src/gc.h
	$(CC) $(CCOPTS) -c src/gc.c -o bin/gc.o

list.o: bin src/list.c src/list.h
	$(CC) $(CCOPTS) -c src/list.c -o bin/list.o

process.o: bin src/process.c src/process.h
	$(CC) $(CCOPTS) -c src/process.c -o bin/process.o

read.o: bin src/read.c src/read.h
	$(CC) $(CCOPTS) -c src/read.c -o bin/read.o

struct.o: bin src/struct.c src/struct.h
	$(CC) $(CCOPTS) -c src/struct.c -o bin/struct.o

tlisp.o: bin src/tlisp.c
	$(CC) $(CCOPTS) -c src/tlisp.c -o bin/tlisp.o

vector.o: bin src/vector.c src/vector.h
	$(CC) $(CCOPTS) -c src/vector.c -o bin/vector.o

clean:
	rm -rf bin

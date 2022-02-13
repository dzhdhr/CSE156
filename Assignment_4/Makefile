CFLAGS=-Wall -Wextra -Werror -Wpedantic -g
CC=clang $(CFLAGS)

all : myclient myserver

list.o : src/list.c src/list.h
	$(CC) -c src/list.c -o bin/list.o

myclient.o : src/myclient.c src/list.h
	$(CC) -c src/myclient.c -o bin/myclient.o

myclient : myclient.o 	list.o
	$(CC) -o bin/myclient bin/myclient.o bin/list.o -lm


myserver.o : src/myserver.c src/list.h
	$(CC) -c src/myserver.c -o bin/myserver.o

myserver : myserver.o list.o
	$(CC) -o bin/myserver	bin/myserver.o bin/list.o -lm

clean :
	rm -f ./bin/*
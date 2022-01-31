CFLAGS=-Wall -Wextra -Werror -Wpedantic -g
CC=clang $(CFLAGS)

all : myclient myserver

myclient.o : src/myclient.c
	$(CC) -c src/myclient.c -o bin/myclient.o

myclient : myclient.o
	$(CC) -o bin/myclient	bin/myclient.o -lm


myserver.o : src/myserver.c
	$(CC) -c src/myserver.c -o bin/myserver.o

myserver : myserver.o
	$(CC) -o bin/myserver	bin/myserver.o -lm

clean :
	rm -f ./bin/*
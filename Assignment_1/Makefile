CFLAGS=-Wall -Wextra  -Werror  -pedantic
CC=clang $(CFLAGS)

myweb	:	myweb.o
	$(CC) -o bin/myweb	bin/myweb.o
myweb.o    :         src/myweb.c
	$(CC) -c src/myweb.c -o bin/myweb.o
clean	:
	rm -f ./bin/*
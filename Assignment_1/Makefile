CFLAGS=-Wall -Wextra  -Werror  -pedantic
CC=clang $(CFLAGS)

myweb	:	myweb.o
	$(CC) -o myweb	myweb.o
myweb.o    :         myweb.c
	$(CC) -c myweb.c
clean	:
	rm -f myweb	myweb.o
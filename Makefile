CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic -std=c99 -D_POSIX_C_SOURCE=200812L

default: bs

list.o: list.c
	$(CC) $(CFLAGS) -c list.c -o list.o

search.o: search.c
	$(CC) $(CFLAGS) -c search.c -o search.o

bs: bs.c search.o list.o
	$(CC) $(CFLAGS) search.o list.o bs.c -o bs

clean:
	@ rm -rf list.o search.o bs

# This is a comment line
CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS= -g -c -Wall -I../include

all: server client

server: server.o
	$(CC) server.o -o server

server.o: server.c
	$(CC) $(CFLAGS) server.c

client: client.o
	$(CC) client.o -o client

client.o: client.c
	$(CC) $(CFLAGS) client.c

clean: 
	rm -rf *.o

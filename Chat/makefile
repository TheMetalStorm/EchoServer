# This is a comment line
CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS= -g -c -Wall -I../include 
CLIBS= -lncurses
all: server client

server: server.o
	$(CC) server.o -o server $(CLIBS)

server.o: server.c
	$(CC) $(CFLAGS) server.c 

client: client.o
	$(CC) client.o -o client $(CLIBS)

client.o: client.c
	$(CC) $(CFLAGS) client.c

clean: 
	rm -rf *.o

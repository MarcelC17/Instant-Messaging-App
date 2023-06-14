CC=gcc
CFLAGS=-Wall -Wextra -std=c99 
DEBUG=-g -ggdb -O0 -march=native

all: server subscriber

verify: clean server subscriber

server: server.o
	$(CC) $(CFLAGS) $(DEBUG) server.o structs.o -lm -o server
	

server.o: 
	$(CC) $(CFLAGS) $(DEBUG) -c server.c -o server.o
	$(CC) $(CFLAGS) $(DEBUG) -c structs.c -lm -o structs.o 

subscriber: subscriber.o
	$(CC) $(CFLAGS) $(DEBUG) subscriber.o structs.o -lm -o subscriber
	$(CC) $(CFLAGS) $(DEBUG) -c subscriber.c -lm -o subscriber.o 
	
	
clean:
	rm subscriber server server.o structs.o subscriber.o
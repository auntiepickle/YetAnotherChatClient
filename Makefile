
CC = gcc

all: proj5 proj52

proj5:	client.o
	$(CC) -o proj5d client.o 
proj52: server.o
	$(CC) -o proj5 server.o 



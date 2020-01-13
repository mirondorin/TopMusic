all: server client

server: TCPserver.c
	gcc -pthread TCPserver.c -lsqlite3 -o TCPserver.o
client:
	gcc client.c -o client.o
clean:
	rm TCPserver TCPserver.o client client.o

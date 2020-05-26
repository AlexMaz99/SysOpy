CC=gcc -Wall

compile: client.c server.c
	$(CC) -o client client.c -lpthread -lm
	$(CC) -o server server.c -lpthread -lm

runServer: server
	./server 6666 socket

runClientNet: client
	./client "Jan" INET 127.0.0.1 6666

runClientLocal: client
	./client "Ala" LOCAL socket

clean:
	rm -f client server socket
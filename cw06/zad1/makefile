CC = gcc -Wall

compile:
	$(CC) -o client client.c
	$(CC) -o server server.c

clean:
	rm -f server client

test:
	make compile
	./server
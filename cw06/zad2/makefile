CC = gcc -Wall

compile:
	$(CC) -o client client.c -lrt
	$(CC) -o server server.c -lrt

clean:
	rm -f server client

test:
	make compile
	./server
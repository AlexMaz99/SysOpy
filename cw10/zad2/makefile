CC = gcc
FLAGS = -Wall

compile: client.c server.c
	$(CC) $(FLAGS) -o client client.c -lpthread -lm
	$(CC) $(FLAGS) -o server server.c -lpthread -lm

clean:
	rm -f client
	rm -f server
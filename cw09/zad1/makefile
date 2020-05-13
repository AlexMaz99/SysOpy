CC = gcc -Wall

compile:
	$(CC) -o main main.c -lpthread -lm

clean:
	rm -f main

test:
	make compile
	./main 2 8
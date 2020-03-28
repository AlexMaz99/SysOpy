CC = gcc -Wall

compile:
	$(CC) -o main main.c

test:
	make compile
	./main child
	./main segFault
	./main status

clean: main
	rm main
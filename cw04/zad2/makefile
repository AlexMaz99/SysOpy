CC = gcc
FLAGS = -std=c99 -Wall

compile:
	$(CC) $(FLAGS) main.c -o main
	$(CC) $(FLAGS) exec.c -o exec

clean:
	rm -rf main exec

forkTest:
	make clean
	make compile
	./main ignore fork
	./main pending fork
	./main mask fork
	./main handler fork

execTest:
	make clean
	make compile
	./main ignore exec
	./main pending exec
	./main mask exec
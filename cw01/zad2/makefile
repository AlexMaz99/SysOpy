CC = gcc
FLAGS = -std=c99 -Wall
LIB_NAME = lib_diff
LIB_PATH = ../zad1

static:
	$(CC) $(FLAGS) main.c -o main -I$(LIB_PATH)/$(LIB_NAME).a

test: 
	make static
	LD_LIBRARY_PATH=$(LIB_PATH) ./main $(shell cat args.txt)

clean:
	rm -rf main

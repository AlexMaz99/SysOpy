CC = gcc
FLAGS = -Wall
LIB_NAME = lib_diff
SOURCE_NAME = library


static:
	$(CC) $(FLAGS) -c $(SOURCE_NAME).c -o $(LIB_NAME).o
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

shared:
	$(CC) -fPIC $(FLAGS) -c library.c -o $(LIB_NAME).o
	$(CC) -fPIC $(FLAGS) -shared -o $(LIB_NAME).so $(LIB_NAME).o

clean:
	rm -f *.o

clean_all:
	rm main $(LIB_NAME).a $(LIB_NAME).so

2static:
	make static
	$(CC) -c main.c
	$(CC) main.o $(LIB_NAME).a -o main

2shared:
	make shared
	$(CC) main.c $(LIB_NAME).so -L ./ -o main -Wl,-rpath,$(shell pwd)

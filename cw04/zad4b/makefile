CC = gcc
FLAGS = -std=c99 -Wall

sender:
	$(CC) $(FLAGS) sender.c -o sender

catcher:
	$(CC) $(FLAGS) catcher.c -o catcher

compile:
	make sender
	make catcher

clean:
	rm -rf sender catcher
CC = gcc -Wall

all: test clean

compile:
	make main
	make receiver
	make packer
	make sender

main: main.c
	$(CC) -o main main.c common.c

receiver: receiver.c
	$(CC) -o receiver receiver.c common.c

packer: packer.c
	$(CC) -o packer packer.c common.c

sender: sender.c
	$(CC) -o sender sender.c common.c

clean:
	rm -f main receiver sender packer

test:
	make compile
	./main
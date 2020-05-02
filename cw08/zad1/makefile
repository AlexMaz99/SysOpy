CC = gcc -Wall

compile:
	$(CC) -o main main.c -lpthread -lm

clean:
	rm -f main Times.txt
	rm -rf results

test:
	make clean
	make compile
	mkdir results
	./main 1 sign mountain.ascii.pgm results/sign1.txt
	./main 1 block mountain.ascii.pgm results/block1.txt
	./main 1 interleaved mountain.ascii.pgm results/interleaved1.txt

	./main 2 sign mountain.ascii.pgm results/sign2.txt
	./main 2 block mountain.ascii.pgm results/block2.txt
	./main 2 interleaved mountain.ascii.pgm results/interleaved2.txt

	./main 4 sign mountain.ascii.pgm results/sign4.txt
	./main 4 block mountain.ascii.pgm results/block4.txt
	./main 4 interleaved mountain.ascii.pgm results/interleaved4.txt

	./main 8 sign mountain.ascii.pgm results/sign8.txt
	./main 8 block mountain.ascii.pgm results/block8.txt
	./main 8 interleaved mountain.ascii.pgm results/interleaved8.txt
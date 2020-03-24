CC = gcc -Wall

compileM:
	$(CC) macierz.c -lm -o macierz

compileG:
	$(CC) -o generator generator.c
	mkdir macierze

compileT:
	$(CC) -o test test.c

testAll:
	make clean
	make compileG
	./generator 15 4 18
	make compileM
	./macierz lista 10 100000 commonFile 1 100
	make compileT
	./test lista

clean:
	rm -f macierz generator lista test
	rm -rf macierze

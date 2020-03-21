CC = gcc -Wall

compile:
	$(CC) -o zad1 main.c

clean:
	rm -f zad1

test:
	mkdir dir1
	mkdir dir1/dir2
	mkdir dir1/dir2/dir3
	touch dir1/file1
	touch dir1/dir2/file2
	touch dir1/dir2/dir3/file3
	touch dir1/file12
	ln -s dir1 link1
	ln -s dir1/dir2 link2
	make compile
	./zad1 ~/OperatingSystems/cw03/zad1

clean_all:
	make clean
	rm -f file1
	rm -f file2
	rm -f file3
	rm -f link1
	rm -f link2
	rm -r dir1
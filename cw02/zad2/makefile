CC = gcc -Wall

compile:
	$(CC) -o myFind main.c
	
clean:
	rm -f myFind

test:
	mkdir dir1
	mkdir dir1/dir2
	mkdir dir1/dir2/dir3
	touch dir1/file1
	touch dir1/dir2/file2
	touch dir1/dir2/dir3/file3
	make compile
	./myFind ../ -maxdepth 2 -mtime 0

clean_all:
	make clean
	rm -f file1
	rm -f file2
	rm -f file3
	rm -r dir1
CC = gcc -Wall -std=c11 -ggdb

compile:
	$(CC) -o program main.c
	
clean:
	rm -f program wyniki.txt

test:
	make clean
	make compile

	#generate and copy
	for length in 1 4 512 1024 4096 8192 ; do \
		for numberOfRecords in 1000 5000 10000; do \
			./program generate dane $$numberOfRecords $$length ;\
			./program copy dane dane.$$numberOfRecords.$$length.lib $$numberOfRecords $$length lib ;\
			./program copy dane dane.$$numberOfRecords.$$length.sys $$numberOfRecords $$length sys ;\
		done; \
	done

	#sort
	for length in 1 4 512 1024 4096 8192 ; do \
		for numberOfRecords in 1000 5000 10000; do \
			./program sort dane.$$numberOfRecords.$$length.lib $$numberOfRecords $$length lib ;\
			./program sort dane.$$numberOfRecords.$$length.sys $$numberOfRecords $$length sys ;\
		done ;\
	done
	rm -rf dane*

CC = gcc
FLAGS = -std=c99 -Wall
LIB_NAME = lib_diff
LIB_PATH = ../zad1
TEST_PATH = ../zad2
TEST_SOURCE_NAME = main.c
REPORT_FILE = raport3a.txt

static:
	$(CC) $(FLAGS) $(TEST_PATH)/$(TEST_SOURCE_NAME) -o main -I$(LIB_PATH) $(LIB_PATH)/$(LIB_NAME).a 

shared:
	$(CC) $(FLAGS) $(TEST_PATH)/$(TEST_SOURCE_NAME) -o main -I$(LIB_PATH) -L$(LIB_PATH) -l_diff 

dynamic_main: main_dyn.c
	make shared
	$(CC) $(FLAGS) main_dyn.c -ldl -o main_dyn -I .

clean_all:
	make clean
	rm -f main
	rm -f main_dyn
	rm -rf raport3a.txt

test:
	LD_LIBRARY_PATH=$(LIB_PATH) ./main $(shell cat args.txt)

test_dyn:
	LD_LIBRARY_PATH=$(LIB_PATH) ./main_dyn $(shell cat args.txt)

testStatic:
	printf "static:\n" > raport.txt
	make static
	make test
	mv raport.txt $(REPORT_FILE)

testShared:
	printf "shared:\n" >> raport.txt
	make shared
	make test
	mv raport.txt $(REPORT_FILE)

testDynamic:
	printf "dynamic:\n" >> raport.txt
	make dynamic_main
	make test_dyn
	mv raport.txt $(REPORT_FILE)

test_all:
	printf "static:\n" > raport.txt
	make static
	make test
	printf "shared:\n" >> raport.txt
	make shared
	make test
	printf "dynamic:\n" >> raport.txt
	make dynamic_main
	make test_dyn
	mv raport.txt $(REPORT_FILE)

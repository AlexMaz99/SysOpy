CC = gcc
FLAGS = -std=c99 -Wall
LIB_NAME = lib_diff
LIB_PATH = ../zad1
TEST_PATH = ../zad2
TEST_SOURCE_NAME = main.c
REPORT_FILE = raport3b.txt
O = O1

static:
	$(CC) $(FLAGS) $(TEST_PATH)/$(TEST_SOURCE_NAME) -o main -I$(LIB_PATH) $(LIB_PATH)/$(LIB_NAME).a 

staticO:
	$(CC) $(FLAGS) $(TEST_PATH)/$(TEST_SOURCE_NAME) -o main -$O -I$(LIB_PATH) $(LIB_PATH)/$(LIB_NAME).a 

shared:
	$(CC) $(FLAGS) $(TEST_PATH)/$(TEST_SOURCE_NAME) -o main -I$(LIB_PATH) -L$(LIB_PATH) -l_diff

sharedO:
	$(CC) $(FLAGS) $(TEST_PATH)/$(TEST_SOURCE_NAME) -o main -$O -I$(LIB_PATH) -L$(LIB_PATH) -l_diff

dynamic_main: main_dyn.c
	make shared
	$(CC) $(FLAGS) main_dyn.c -ldl -o main_dyn -I .

dynamic_mainO:
	make CC="gcc -$O" dynamic_main

clean_all:
	make clean
	rm -rf main raport2.txt
	rm -f main_dyn

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

test_allO:
	printf "static O1:\n" > raport.txt
	make staticO O=O1
	make test
	printf "shared: O1\n" >> raport.txt
	make sharedO O=O1
	make test
	printf "dynamic: O1\n" >> raport.txt
	make dynamic_mainO O=O1
	make test_dyn
	printf "static O2:\n" >> raport.txt
	make staticO O=O2
	make test
	printf "shared: O2\n" >> raport.txt
	make sharedO O=O2
	make test
	printf "dynamic: O2\n" >> raport.txt
	make dynamic_mainO O=O2
	make test_dyn
	printf "static: Ofast\n" >> raport.txt
	make staticO O=Ofast
	make test
	printf "static: Os\n" >> raport.txt
	make staticO O=Os
	make test
	printf "static: O0\n" >> raport.txt
	make staticO O=O0
	make test
	printf "shared: O0\n" >> raport.txt
	make sharedO O=O0
	make test
	printf "dynamic: O0\n" >> raport.txt
	make dynamic_mainO O=O0
	make test_dyn
	mv raport.txt $(REPORT_FILE)

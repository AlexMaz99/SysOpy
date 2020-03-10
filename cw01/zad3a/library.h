#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H


struct Block{
    int numberOfOperations;
    char** operations;
};

struct MainArray{
    int numberOfBlocks; //number of pairs of files
    struct Block ** blocks;
};

struct MainArray* createArray(int numberOfBlocks);

struct Block *createBlock(int numberOfOperations);

void deleteBlock(struct MainArray* mainArray, int index);

void deleteOperation(struct MainArray* mainArray, int block_index, int operation_index);

void deleteArray(struct MainArray* mainArray);

void compareTwoFiles(char* file1, char*file2);

int countOperationsInBlock(char *file1, char *file2);

struct Block* createBlockAndOperations(char *tmp, int numberOfOperations);

void definePairSequence(char* files, char**newFiles);

void comparePairs(char *filesInString, struct MainArray *mainArray);

int countNumberOfFiles(char* filesInString);

int getNumberOfOperations(struct MainArray* mainArray, int indexOfBlock);

char* getOperation(struct MainArray* mainArray, int indexOfBlock, int indexOfOperation);

#endif //LAB1_LIBRARY_H

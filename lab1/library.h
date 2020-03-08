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

#endif //LAB1_LIBRARY_H
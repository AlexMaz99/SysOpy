#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include <ctype.h>

int main(){
    struct MainArray* mainArray = createArray(3);
    int size = 6;

    comparePairs("a.txt:b.txt c.txt:d.txt e.txt:f.txt", size, mainArray);

    printf("%d\n\n", mainArray->blocks[0]->numberOfOperations);
    printf("%s\n\n", mainArray->blocks[0]->operations[0]);
    printf("%s\n\n", mainArray->blocks[0]->operations[1]);
    printf("%s\n\n", mainArray->blocks[0]->operations[2]);
    printf("%d\n\n", mainArray->blocks[1]->numberOfOperations);
    printf("%s\n\n", mainArray->blocks[1]->operations[0]);
    printf("%s\n\n", mainArray->blocks[1]->operations[1]);
    printf("%s\n\n", mainArray->blocks[1]->operations[2]);
    
    deleteBlock(mainArray, 0);
    if (mainArray -> blocks[0] == NULL) printf("block 0 deleted\n");
    deleteBlock(mainArray, 1);
    if (mainArray -> blocks[1] == NULL) printf("block 1 deleted\n");
    deleteBlock(mainArray, 2);
    if (mainArray -> blocks[2] == NULL) printf("block 2 deleted\n");

    return 0;
}

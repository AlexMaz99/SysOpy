#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include <ctype.h>

struct MainArray* createArray(int numberOfBlocks){
    if (numberOfBlocks <= 0) return NULL;
    struct MainArray *mainArray= (struct MainArray*)calloc(1,sizeof(struct MainArray));
    mainArray -> numberOfBlocks = 0;
    mainArray -> blocks = (char**)calloc(numberOfBlocks, sizeof(char*));

    return mainArray;
}

struct Block *createBlock(int numberOfOperations){
  if(numberOfOperations <= 0) return NULL;
  struct Block * block=(struct Block*)calloc(1,sizeof(struct Block));
  block -> operations=(char**)calloc(numberOfOperations,sizeof(char*));
  block -> numberOfOperations = numberOfOperations;

  return block;
}

void deleteBlock(struct MainArray* mainArray, int index){
    if(index < 0 || mainArray == NULL || mainArray -> blocks[index] == NULL) return;
    for(int i = 0; i < mainArray -> blocks[index] -> numberOfOperations; i++){
        free(mainArray -> blocks[index] -> operations[i]);
        mainArray -> blocks[index] -> operations[i] = NULL;
    }
    free(mainArray -> blocks[index] -> operations);
    mainArray -> blocks[index] -> operations = NULL;
    free(mainArray -> blocks[index]);
    mainArray -> blocks[index] = NULL;

}

void deleteOperation(struct MainArray* mainArray, int block_index, int operation_index){
    free(mainArray -> blocks[block_index] -> operations[operation_index]);
    mainArray -> blocks[block_index] -> operations[operation_index] = NULL;
    int numberOfOperations = mainArray -> blocks[block_index] -> numberOfOperations;
    if (numberOfOperations - 1 <= 0) deleteBlock(mainArray, block_index);
    else mainArray -> blocks[block_index] -> numberOfOperations = numberOfOperations - 1;
}

void deleteArray(struct MainArray* mainArray){
    for (int i = 0; i < mainArray -> numberOfBlocks; i++){
        deleteBlock(mainArray, i);
    }
    free(mainArray -> blocks);
    free(mainArray);
}

void compareTwoFiles(char* file1, char*file2){
    system("touch tmp.txt");
    char tmp[12 + strlen(file1) + strlen(file2)];
    strcpy(tmp, "diff ");
    strcat(tmp, file1);
    strcat(tmp, " ");
    strcat(tmp, file2);
    strcat(tmp, " >> tmp.txt");
    system(tmp);
}

int countOperationsInBlock(char *file1, char *file2){
  FILE *fp = popen("diff a.txt b.txt | grep -c ^[0-9]","r");
  int counter;
  fscanf(fp,"%d", &counter);
  return counter;
}


struct Block* createBlockAndOperations(char *tmp, int numberOfOperations){
  FILE *f = fopen(tmp, "r");
  if(f == NULL)  exit(0);
  char *line = NULL;
  size_t len = 0;

  struct Block * block = createBlock(numberOfOperations);
  char operation[256];
  strcpy(operation,"");
  int i = 0;
  while(getline(&line, &len, f) != -1){
    if(line[0] >= '0' && line[0] <= '9'){
      if(strcmp(operation, "") != 0){
        block -> operations[i]=(char*)calloc(strlen(operation), sizeof(char));
        strcpy(block -> operations[i], operation);
        printf("%s\n", operation);
        i++;
        strcpy(operation ,"");
      }
    }
    strcat(operation, line);
  }

  if(strcmp(operation, "") != 0){ //last editing operation
    block -> operations[i]=(char*)calloc(strlen(operation), sizeof(char));
    strcpy(block -> operations[i], operation);
  }

  fclose(f);
  if (line) free(line);
  return block;
}

void processFiles(char *files[], int size, struct MainArray *mainArray){
  if (size%2 != 0) exit(0);
  int index = 0;
  for (int i = 0; i< size-1; i+=2){
    int numberOfOperations = countOperationsInBlock(files[i], files[i+1]);
    compareTwoFiles(files[i], files[i+1]);
    struct Block* block = createBlockAndOperations("tmp.txt", numberOfOperations);
    system("rm tmp.txt");

    // for (int i = 0; i < mainArray->numberOfBlocks; i++) {
    //     if (mainArray -> blocks[i] != NULL) {
    //         index = i;
    //         break;
    //     }
    // }
    // if (index == -1) {
    //     return;
    // }
    mainArray -> blocks[index] = block;
    index ++;
    mainArray -> numberOfBlocks++;
  }
}


int main(){
    struct MainArray* mainArray = createArray(2);
    char*a = "a.txt";
    char*b = "b.txt";
    char*c = "c.txt";
    char*d = "d.txt";
    char* files[4] = {a, b, c, d};
    process_files(files, 4, mainArray);
}
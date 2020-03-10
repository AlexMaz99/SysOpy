#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include <ctype.h>


//create main array
struct MainArray* createArray(int numberOfBlocks){
    if (numberOfBlocks <= 0) return NULL;
    struct MainArray *mainArray= (struct MainArray*)calloc(1,sizeof(struct MainArray));
    mainArray -> numberOfBlocks = numberOfBlocks;
    mainArray -> blocks = (struct Block**)calloc(numberOfBlocks, sizeof(struct Block*));
    return mainArray;
}

//create Block
struct Block *createBlock(int numberOfOperations){
  if(numberOfOperations < 0) return NULL;
  struct Block * block=(struct Block*)calloc(1,sizeof(struct Block));
  if (numberOfOperations == 0) block -> operations = NULL;
  else block -> operations=(char**)calloc(numberOfOperations,sizeof(char*));
  block -> numberOfOperations = numberOfOperations;
  return block;
}

//delete block at index
void deleteBlock(struct MainArray* mainArray, int index){
    if(mainArray -> numberOfBlocks <= index || index < 0 || mainArray == NULL || mainArray -> blocks == NULL || mainArray -> blocks[index] == NULL) return;
    for(int i = 0; i < mainArray -> blocks[index] -> numberOfOperations; i++){
        if (mainArray -> blocks[index] -> operations[i] != NULL){
          free(mainArray -> blocks[index] -> operations[i]);
          mainArray -> blocks[index] -> operations[i] = NULL;
        }
    }
    if (mainArray -> blocks[index] != NULL){
      free(mainArray -> blocks[index]);
      mainArray -> blocks[index] = NULL;
    }

}

//delete operation at index from block at index
void deleteOperation(struct MainArray* mainArray, int block_index, int operation_index){
    if (mainArray -> blocks[block_index] -> operations[operation_index] == NULL) return;
    free(mainArray -> blocks[block_index] -> operations[operation_index]);
    mainArray -> blocks[block_index] -> operations[operation_index] = NULL;
}

//delete main array
void deleteArray(struct MainArray* mainArray){
    if (mainArray == NULL) return;
    for (int i = 0; i < mainArray -> numberOfBlocks; i++){
        deleteBlock(mainArray, i);
    }
    free(mainArray);
}

//compare two files with diff command
void compareTwoFiles(char* file1, char*file2){
    system("touch tmp.txt");
    char tmp[50 + strlen(file1) + strlen(file2)];
    strcpy(tmp, "diff ");
    strcat(tmp, file1);
    strcat(tmp, " ");
    strcat(tmp, file2);
    strcat(tmp, " >> tmp.txt");
    system(tmp);
}

//count operations in block for two files
int countOperationsInBlock(char *file1, char *file2){
  char command[20 + strlen(file1) + strlen(file2)];
  strcpy(command, "diff ");
  strcat(command, file1);
  strcat(command, " ");
  strcat(command, file2);
  strcat (command, " | grep -c  ^[0-9]");

  FILE *fp = popen(command,"r");
  int counter;
  fscanf(fp,"%d", &counter);
  return counter;
}

//create block and operations for file tmp.txt
struct Block* createBlockAndOperations(char *tmp, int numberOfOperations){
  FILE *f = fopen(tmp, "r");
  if(f == NULL)  exit(0);
  char *line = NULL;
  size_t len = 0;

  struct Block * block = createBlock(numberOfOperations);
  if(numberOfOperations == 0) {
    block -> operations = NULL;
    fclose(f);
    return block;
  }
  char operation[100000];
  strcpy(operation,"");
  int i = 0;
  while(getline(&line, &len, f) != -1){
    if(line[0] >= '0' && line[0] <= '9'){
      if(strcmp(operation, "") != 0){
        block -> operations[i]=(char*)calloc(10000, sizeof(char));
        strcpy(block -> operations[i], operation);
        i++;
        strcpy(operation ,"");
      }
    }
    strcat(operation, line);
  }

  if(strcmp(operation, "") != 0){ //last editing operation
    block -> operations[i]=(char*)calloc(10000, sizeof(char));
    strcpy(block -> operations[i], operation);
  }

  fclose(f);
  if (line) free(line);
  return block;
}

//conversion string with files to array with files
void definePairSequence(char* files, char**newFiles) // file1A.txt:file1B.txt file2A.txt:file2B.txt
{ 

    if(files == NULL || strlen(files) == 0) exit (0); 
    char *filesCpy = calloc(strlen(files), sizeof(char));
    strcpy(filesCpy, files);
    char *ptr = strtok(filesCpy, ": ");
    char *file1, *file2;
    int index = 0;

    while (ptr != NULL)
    {
        file1 = ptr;
        ptr = strtok(NULL, ": ");
        newFiles[index] = file1;
        index ++;

        if(ptr == NULL) exit (-1);
        file2 = ptr;
        ptr = strtok(NULL, ": ");
        newFiles[index] = file2;
        index++;

    }
}

// count number of files in string
int countNumberOfFiles(char* filesInString){
  int numberOfFiles = 0;
  char*string = strdup(filesInString);
  char*token = strtok(string, " ");
  while(token!=NULL){
    token = strtok(NULL, " ");
    numberOfFiles ++;
  }
  return 2 * numberOfFiles;
}

//compare every pair of files and create structures
void comparePairs(char *filesInString, struct MainArray *mainArray){
  int size = countNumberOfFiles(filesInString);
  if (size%2 != 0) exit(0);
  char** files = (char**)calloc(size, sizeof(char*));
  definePairSequence(filesInString, files);

  for (int i = 0; i< size-1; i+=2){
    compareTwoFiles(files[i], files[i+1]);
    int numberOfOperations = countOperationsInBlock(files[i], files[i+1]);
    struct Block* block = createBlockAndOperations("tmp.txt", numberOfOperations);
  
    system("rm tmp.txt");

    for(int i =0 ; i<mainArray->numberOfBlocks;i++){
      if(mainArray->blocks[i]==NULL){
        mainArray -> blocks[i] = block;
      break;
      }
    }
  }
}

//number of operations for block at index
int getNumberOfOperations(struct MainArray* mainArray, int indexOfBlock){
  if (mainArray == NULL || mainArray -> blocks[indexOfBlock] == NULL) return 0;
  return mainArray -> blocks[indexOfBlock] -> numberOfOperations;
}

//operations at index from block at index
char* getOperation(struct MainArray* mainArray, int indexOfBlock, int indexOfOperation){
  return mainArray -> blocks[indexOfBlock] -> operations[indexOfOperation];
}

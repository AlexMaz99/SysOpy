#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include <ctype.h>


//tworzenie tablicy głównej
struct MainArray* createArray(int numberOfBlocks){
    if (numberOfBlocks <= 0) return NULL;
    struct MainArray *mainArray= (struct MainArray*)calloc(1,sizeof(struct MainArray));
    mainArray -> numberOfBlocks = 0;
    mainArray -> blocks = (struct Block**)calloc(numberOfBlocks, sizeof(struct Block*));
    return mainArray;
}

//tworzenie bloku
struct Block *createBlock(int numberOfOperations){
  if(numberOfOperations < 0) return NULL;
  struct Block * block=(struct Block*)calloc(1,sizeof(struct Block));
  if (numberOfOperations == 0) block -> operations = NULL;
  else block -> operations=(char**)calloc(numberOfOperations,sizeof(char*));
  block -> numberOfOperations = numberOfOperations;
  return block;
}

//usuwanie bloku o danym indeksie
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

//usuwanie operacji o danym indeksie z bloku o danym indeksie
void deleteOperation(struct MainArray* mainArray, int block_index, int operation_index){
    free(mainArray -> blocks[block_index] -> operations[operation_index]);
    mainArray -> blocks[block_index] -> operations[operation_index] = NULL;
    int numberOfOperations = mainArray -> blocks[block_index] -> numberOfOperations;
    if (numberOfOperations - 1 <= 0) deleteBlock(mainArray, block_index);
    else mainArray -> blocks[block_index] -> numberOfOperations = numberOfOperations - 1;
}

//usuwanie tablicy głównej
void deleteArray(struct MainArray* mainArray){
    for (int i = 0; i < mainArray -> numberOfBlocks; i++){
        deleteBlock(mainArray, i);
    }
    free(mainArray -> blocks);
    free(mainArray);
}

//porównanie dwóch plików poleceniem diff i zapisanie wyniku do pliku tymczasowego
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

//obliczenie ilości operacji dla danych dwóch plików
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

//stworzenie bloku i operacji dla zawartości pliku tmp.txt
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
        block -> operations[i]=(char*)calloc(strlen(operation), sizeof(char));
        strcpy(block -> operations[i], operation);
        //printf("%s\n\n", operation);
        i++;
        strcpy(operation ,"");
      }
    }
    strcat(operation, line);
  }

  if(strcmp(operation, "") != 0){ //last editing operation
    block -> operations[i]=(char*)calloc(strlen(operation), sizeof(char));
    strcpy(block -> operations[i], operation);
    //printf("%s\n\n", operation);
  }

  fclose(f);
  if (line) free(line);
  return block;
}

//zamiana stringa z plikami na tablicę stringów
void definePairSequence(char* files, char**newFiles)
{ // file1A.txt:file1B.txt file2A.txt:file2B.txt

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

//porównanie wszystkich par plików i utworzenie odpowiednich struktur
void comparePairs(char *filesInString, int size, struct MainArray *mainArray){
  if (size%2 != 0) exit(0);
  char** files = (char**)calloc(size, sizeof(char*));
  definePairSequence(filesInString, files);

  int index = 0;
  for (int i = 0; i< size-1; i+=2){
    compareTwoFiles(files[i], files[i+1]);
    int numberOfOperations = countOperationsInBlock(files[i], files[i+1]);
    struct Block* block = createBlockAndOperations("tmp.txt", numberOfOperations);
  
    system("rm tmp.txt");

    mainArray -> blocks[index] = block;
    index ++;
    mainArray -> numberOfBlocks++;
  }
}

//liczba operacji dla bloku o danym indeksie
int getNumberOfOperations(struct MainArray* mainArray, int indexOfBlock){
  if (mainArray == NULL || mainArray -> blocks[indexOfBlock] == NULL) return 0;
  return mainArray -> blocks[indexOfBlock] -> numberOfOperations;
}

//operacja o danym indeksie z danego bloku
char* getOperation(struct MainArray* mainArray, int indexOfBlock, int indexOfOperation){
  return mainArray -> blocks[indexOfBlock] -> operations[indexOfOperation];
}

// int main(){
//     struct MainArray* mainArray = createArray(3);
//     int size = 6;

//     comparePairs("a.txt:b.txt c.txt:d.txt e.txt:f.txt", size, mainArray);

//     printf("%d\n\n", mainArray->blocks[0]->numberOfOperations);
//     printf("%s\n\n", mainArray->blocks[0]->operations[0]);
//     printf("%s\n\n", mainArray->blocks[0]->operations[1]);
//     printf("%s\n\n", mainArray->blocks[0]->operations[2]);
//     printf("%d\n\n", mainArray->blocks[1]->numberOfOperations);
//     printf("%s\n\n", mainArray->blocks[1]->operations[0]);
//     printf("%s\n\n", mainArray->blocks[1]->operations[1]);
//     printf("%s\n\n", mainArray->blocks[1]->operations[2]);
    
//     deleteBlock(mainArray, 0);
//     if (mainArray -> blocks[0] == NULL) printf("block 0 deleted\n");
//     deleteBlock(mainArray, 1);
//     if (mainArray -> blocks[1] == NULL) printf("block 1 deleted\n");
//     deleteBlock(mainArray, 2);
//     if (mainArray -> blocks[2] == NULL) printf("block 2 deleted\n");

//     return 0;
// }

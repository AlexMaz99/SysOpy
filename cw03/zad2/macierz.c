#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <ftw.h>
#include <math.h>

typedef struct{
    char* path;
    int numberOfRows;
    int numberOfColumns;
    int widthOfColumn; // number of signs in one column
} Matrix;

Matrix **AA;
Matrix **BB;
Matrix **CC;
int numberOfPairs;
const int MAX_ABS_VAL = 100;

int min(int a, int b){
    return a <= b ? a : b;
}

// count number of rows in matrix from file
int countNumberOfRows(char* path){
    FILE *file = fopen(path, "r");
    if (file == NULL){
        printf("Cannot open file %s\n", path);
        exit(EXIT_FAILURE);
    }
    int counter = 0;
    fseek(file, 0, 0);
    char *line = NULL;
    size_t len = 0;

    while(getline(&line, &len, file) != -1 && line[0] != ' ' && line[0] != '\n'){
       counter++;
    }
    fclose(file);
    return counter;
}

// count number of columns in matrix from file
int countNumberOfColumns(char* path){
    FILE *file = fopen(path, "r");
    if (file == NULL){
        printf("Cannot open file %s\n", path);
        exit(EXIT_FAILURE);
    }
    int counter = 1;
    fseek(file, 0, 0);
    for(char i = getc(file); i != '\n'; i = getc(file)){
        if (i == ' '){
            counter++;
        }
    }
    fclose(file);
    return counter;
}

// read matrix from file and create structure
Matrix *readMatrix(char *path){
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix -> path = path;
    matrix -> numberOfRows = countNumberOfRows(path);
    matrix -> numberOfColumns = countNumberOfColumns(path);
    return matrix;
}

// fill result matrix with empty spaces
void fillMatrixFile(FILE *file, Matrix *C){
    for (int row = 0; row < C -> numberOfRows; row ++){
        for (int col = 0; col < C -> numberOfColumns * C -> widthOfColumn; col ++){
            fwrite(" ", 1, 1, file);
        }
        fwrite("\n", 1, 1, file);
    }
}

// create result of multiplying Matrix
Matrix* createResultMatrix(char *path, Matrix *A, Matrix *B){
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix -> path = path;
    matrix -> numberOfRows = A -> numberOfRows;
    matrix -> numberOfColumns = B -> numberOfColumns;
    matrix -> widthOfColumn = (int) ceil(log10(A -> numberOfColumns * MAX_ABS_VAL * MAX_ABS_VAL)) + 3;
    FILE *file = fopen(path, "w+");
    fillMatrixFile(file, matrix);
    fclose(file);
    return matrix;
}

// remove matrices
void freeMatrix(){
    for (int i = 0; i < numberOfPairs; i ++){
        free(AA[i]);
        free(BB[i]);
        free(CC[i]);
    }
    free(AA);
    free(BB);
    free(CC);
}

// read files from list and create Matrix structures
void readList(char *lista){
    FILE *file = fopen(lista, "r");
    if (file == NULL){
        printf("Cannot opern list\n");
        exit(EXIT_FAILURE); 
    }
    char*line = calloc(1000, sizeof(char));
    int pairs = 0;
    while(fgets(line, 1000, file) != NULL) numberOfPairs ++;

    AA = (Matrix**) calloc(pairs, sizeof(Matrix*));
    BB = (Matrix**) calloc(pairs, sizeof(Matrix*));
    CC = (Matrix**) calloc(pairs, sizeof(Matrix*));

    numberOfPairs = pairs;
    fseek(file, 0, 0);
    int counter = 0;

    while(fgets(line, 1000, file) != NULL){
        char* A = strtok(line, " ");
        char* B = strtok(NULL, " ");
        char* C = strtok(NULL, "\n");
        AA[counter] = readMatrix(A);
        BB[counter] = readMatrix(B);
        CC[counter] = createResultMatrix(C, AA[counter], BB[counter]);
    }

    free(line);
    fclose(file);
}

// TO DO
int multiply(){

}

// TO DO
void makePaste(){

}

int main(int argc, char** argv){
    if (argc != 5){
        printf("Wrong number of arguments. Expected: [path] [number of process] [time limit] [save mode: commonFile / distinctFiles]");
        exit(EXIT_FAILURE);
    }
    char *lista = argv[1];
    int numberOfProcess = atoi(argv[2]);
    int timeLimit = atoi(argv[3]);
    int isDistinct = 0;

    if (!strcmp("distinctFiles", argv[4])){
        isDistinct = 1;
    }

    if (numberOfProcess <= 0){
        printf("Number of process must be positive\n");
        exit(EXIT_FAILURE);
    }
    
    readList(lista);

    if (isDistinct == 1){
        int *processesForMatrix = calloc(numberOfPairs, sizeof(int));
        for (int i = 0; i < numberOfPairs; i ++){
            processesForMatrix[i] = min(numberOfProcess, BB[i] -> numberOfColumns);
        }
    }

    pid_t *childPids = calloc(numberOfProcess, sizeof(pid_t));

    for (int i = 0; i < numberOfProcess; i ++){
        pid_t childPid = fork();
        if (childPid == 0){
            exit(multiply()); //TODO
        }
        else if (childPid > 0){
            childPids[i] = childPid;
        }
        else{
            printf("Fork error\n");
            return -1;
        }
    }

    for (int i = 0; i < numberOfProcess; i++){
        int status;
        waitpid(childPids[i], &status, 0);
        printf("The process with PID = %d have done %d multiplying operations\n", childPids[i], WEXITSTATUS(status));
    }

    if (isDistinct == 1){
        makePaste(); //TODO
    }

    freeMatrix();

    return 0;
}
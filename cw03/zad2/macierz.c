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
const int MAX_ROW_LEN = 100000;

clock_t start_t, end_t;
struct tms start_cpu, end_cpu;

void timerStart(){
    start_t = times(&start_cpu);
}

void timerStop(){
    end_t = times(&end_cpu);
}

double timeDiff(clock_t t1, clock_t t2){
    return ((double) (t2 - t1) / sysconf(_SC_CLK_TCK));
}

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
    matrix -> path = calloc(FILENAME_MAX, sizeof(char));
    strcpy(matrix -> path, path);
    matrix -> numberOfRows = countNumberOfRows(path);
    matrix -> numberOfColumns = countNumberOfColumns(path);
    return matrix;
}

// fill result matrix with empty spaces
void fillMatrixFile(FILE *file, Matrix *C){
    for (int row = 0; row < C -> numberOfRows; row ++){
        for (int col = 0; col < C -> numberOfColumns * C -> widthOfColumn - 1; col ++){
            fwrite(" ", 1, 1, file);
        }
        fwrite("\n", 1, 1, file);
    }
}

// create result of multiplying Matrix
Matrix* createResultMatrix(char *path, Matrix *A, Matrix *B){
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix -> path = calloc(FILENAME_MAX, sizeof(char));
    strcpy(matrix -> path, path);
    matrix -> numberOfRows = A -> numberOfRows;
    matrix -> numberOfColumns = B -> numberOfColumns;
    matrix -> widthOfColumn = (int) log10(MAX_ABS_VAL * MAX_ABS_VAL) + 3;
    FILE *file = fopen(matrix -> path, "w+");
    fillMatrixFile(file, matrix);
    fclose(file);
    return matrix;
}

// remove one matrix
void freeMatrix(Matrix *m){
    free(m -> path);
    free(m);
}
// remove matrices
void freeList(){
    for (int i = 0; i < numberOfPairs; i ++){
        freeMatrix(AA[i]);
        freeMatrix(BB[i]);
        freeMatrix(CC[i]);
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
    char*line = calloc(MAX_ROW_LEN, sizeof(char));
    int pairs = 0;
    while(fgets(line, MAX_ROW_LEN, file) != NULL) pairs ++;

    AA = (Matrix**) calloc(pairs, sizeof(Matrix*));
    BB = (Matrix**) calloc(pairs, sizeof(Matrix*));
    CC = (Matrix**) calloc(pairs, sizeof(Matrix*));

    numberOfPairs = pairs;
    fseek(file, 0, 0);
    int counter = 0;

    while(fgets(line, MAX_ROW_LEN, file) != NULL){
        char* A = strtok(line, " ");
        char* B = strtok(NULL, " ");
        char* C = strtok(NULL, "\n");
        AA[counter] = readMatrix(A);
        BB[counter] = readMatrix(B);
        CC[counter] = createResultMatrix(C, AA[counter], BB[counter]);
        counter++;
    }

    free(line);
    fclose(file);
}

// return number from matrix at given position
int getValueAtPosition(Matrix *matrix, int row, int column, FILE *file){
    if (row >= matrix -> numberOfRows || column >= matrix -> numberOfColumns){
        printf("The matrix: %s has not given position (%d, %d)\n", matrix -> path, row, column);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, 0);
    int counter = 0;
    char* buffer = calloc(MAX_ROW_LEN, sizeof(char));
    while(fgets(buffer, MAX_ROW_LEN, file) != NULL && counter < matrix -> numberOfRows && counter < row) counter ++;
    counter = 0;
    char *token = strtok(buffer, " ");
    while (token != NULL && counter < matrix -> numberOfColumns && counter < column){
        token = strtok(NULL, " ");
        counter ++;
    }
    int result;
    if (token == NULL){
        printf("Cannot find position (%d, %d) in file %s", row, column, matrix -> path);
        exit(EXIT_FAILURE);
    }
    result = atoi(token);
    free(buffer);
    return result;
}

// return index in result file on which value should be written
int getIndex(Matrix *matrix, int row, int column){
    return (matrix -> widthOfColumn * matrix -> numberOfColumns * row + column * matrix -> widthOfColumn);
}

// write value at given position in result file
void writeValueAtPosition(Matrix *matrix, int row, int column, FILE *file, int value){
    fseek(file, 0, 0);
    int index = getIndex(matrix, row, column);
    char *number = calloc(matrix -> widthOfColumn, sizeof(char));
    sprintf(number, "%d", value);
    int digs = floor(log10(abs(value))) + 1;
    if (value < 0) digs ++;
    for (int i = digs; i < matrix -> widthOfColumn -1; i++){
        number[i] = ' ';
    }
    fseek(file, index, 0);
    fwrite(number, sizeof(char), matrix -> widthOfColumn - 1, file);
    free(number);

}

// multiply matrix A by one column from matrix B and save result in file
void multiplyColumn(Matrix *A, Matrix *B, Matrix*C, FILE *Afile, FILE *Bfile, FILE *Cfile, int column){
    fseek(Afile, 0, 0);
    fseek(Bfile, 0, 0);
    fseek(Cfile, 0, 0);

    for (int row = 0; row < A -> numberOfRows; row ++){
        int result = 0;
        for (int col = 0; col < A -> numberOfColumns; col ++){
            result += getValueAtPosition(A, row, col, Afile) * getValueAtPosition(B, col, column, Bfile);
        }
        writeValueAtPosition(C, row, column, Cfile, result);
    }
}


int multiply(int process, int numberOfProcess, int timeLimit, int isDistinct){
    timerStart();
    int multiplyingCounter = 0;

    for (int i = 0; i < numberOfPairs; i ++){
        int numberOfColumnsPerProcess = 1;
        int startColumn;

        if (process >= BB[i] -> numberOfColumns) continue;
        if (numberOfProcess > BB[i] -> numberOfColumns){
            numberOfColumnsPerProcess = 1;
            startColumn = process;
        }
        else {
            numberOfColumnsPerProcess = BB[i] -> numberOfColumns / numberOfProcess;
            startColumn = numberOfColumnsPerProcess * process;
            if (process == numberOfProcess - 1){
                numberOfColumnsPerProcess += BB[i] -> numberOfColumns - numberOfColumnsPerProcess * (process + 1);
            }
        }
        if (isDistinct == 1){
            createNewFiles();
        }
        else{
            FILE *A = fopen(AA[i] -> path, "r");
            FILE *B = fopen(BB[i] -> path, "r");
            FILE *C = fopen(CC[i] -> path, "r+");
            if (A == NULL || B == NULL || C == NULL){
                printf("Cannot open file\n");
                exit(EXIT_FAILURE);
            }
            flock(fileno(C), LOCK_EX);
            for (int col = startColumn; col < startColumn + numberOfColumnsPerProcess; col ++){
                multiplyColumn(AA[i], BB[i], CC[i], A, B, C, col);
            }
            flock(fileno(C), LOCK_UN);
            fclose(A);
            fclose(B);
            fclose(C);
        }
        multiplyingCounter ++;
        timerStop();
        if ((int) timeDiff(start_t, end_t) >= timeLimit){
            exit(multiplyingCounter);
        }

    }
    exit(multiplyingCounter);
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
            multiply(i, numberOfProcess, timeLimit, isDistinct);
        }
        else if (childPid > 0){
            childPids[i] = childPid;
        }
        else{
            exit(EXIT_FAILURE);
        }
    }

    if (isDistinct == 1){
        makePaste(); //TODO
    }

    for (int i = 0; i < numberOfProcess; i++){
        int status;
        waitpid(childPids[i], &status, 0);
        printf("The process with PID = %d have done %d multiplying operations\n", childPids[i], WEXITSTATUS(status));
    }


    freeList();

    return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
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
Matrix* createResultMatrix(char *path, int numberOfRows, int numberOfColumns){
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix -> path = calloc(FILENAME_MAX, sizeof(char));
    strcpy(matrix -> path, path);
    matrix -> numberOfRows = numberOfRows;
    matrix -> numberOfColumns = numberOfColumns;
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
    char*line = calloc(100, sizeof(char));
    int pairs = 0;
    while(fgets(line, 100, file) != NULL) pairs ++;

    AA = (Matrix**) calloc(pairs, sizeof(Matrix*));
    BB = (Matrix**) calloc(pairs, sizeof(Matrix*));
    CC = (Matrix**) calloc(pairs, sizeof(Matrix*));

    numberOfPairs = pairs;
    fseek(file, 0, 0);
    int counter = 0;

    while(fgets(line, 100, file) != NULL){
        char* A = strtok(line, " ");
        char* B = strtok(NULL, " ");
        char* C = strtok(NULL, "\n");
        AA[counter] = readMatrix(A);
        BB[counter] = readMatrix(B);
        CC[counter] = createResultMatrix(C, AA[counter] -> numberOfRows, BB[counter] -> numberOfColumns);
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
    char *number = (char*)calloc(matrix -> widthOfColumn, sizeof(char));
    sprintf(number, "%d", value);
    int j = matrix -> widthOfColumn - 1;
    while(number[j] == 0) {
        number[j]=' ';
        j--;
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

// return name of additional file for given number of process
char *getNameOfFile(int indexOfMatrix, int process){
    char* path = calloc(FILENAME_MAX, sizeof(char));
    char number[3];
    sprintf(number, "%d", process);
    strcpy(path, CC[indexOfMatrix] -> path);
    strcat(path, "_");
    strcat(path, number);
    return path;
}

// crate additional files
void createNewFiles(int indexOfMatrix, int startColumn, int numberOfColumnsPerProcess, int process){
    FILE * A = fopen(AA[indexOfMatrix] -> path, "r");
    FILE * B = fopen(BB[indexOfMatrix] -> path, "r");
    if (A == NULL || B == NULL){
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    char*path = getNameOfFile(indexOfMatrix, process);
    Matrix *newMatrix = createResultMatrix(path, CC[indexOfMatrix] -> numberOfRows, numberOfColumnsPerProcess);
    FILE * file = fopen(path, "r+");

    for (int i = 0; i < numberOfColumnsPerProcess; i ++){
        for (int j = 0; j < AA[indexOfMatrix] -> numberOfRows; j ++){
            int result = 0;
            for (int c = 0; c < AA[indexOfMatrix] -> numberOfColumns; c++){
                result += getValueAtPosition(AA[indexOfMatrix], j, c, A) * getValueAtPosition(BB[indexOfMatrix], c, startColumn + i, B);
            }
            writeValueAtPosition(newMatrix, j, i, file, result);
        }
    }
    freeMatrix(newMatrix);
    fclose(file);
    fclose(A);
    fclose(B);
    free(path);
}

// mulitply all matrices
void multiply(int process, int numberOfProcess, int timeLimit, int isDistinct, int *operationCounter){
    timerStart();
    (*operationCounter) = 0;

    for (int i = 0; i < numberOfPairs; i ++){
        int numberOfColumnsPerProcess = 1;
        int startColumn;

        if (process >= BB[i] -> numberOfColumns) continue;
        
        startColumn = process * ceil((double) BB[i] -> numberOfColumns / numberOfProcess);
        numberOfColumnsPerProcess = min((process + 1) * (int) ceil((double) BB[i] -> numberOfColumns / numberOfProcess), BB[i] -> numberOfColumns) - startColumn;
        
        if (isDistinct == 1){
            createNewFiles(i, startColumn, numberOfColumnsPerProcess, process);
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
        (*operationCounter) ++;
        timerStop();
        if ((int) timeDiff(start_t, end_t) >= timeLimit){
            break;
        }
    }
    return;
}

// create empty files which will be needed to paste
void createEmptyFiles(int indexOfMatrix, int process){
    for (int i = 0; i < process; i ++){
        char* file = getNameOfFile(indexOfMatrix, process);
        FILE * f = fopen(file, "w+");
        fclose(f);
        free(file);
    }
}

// paste files to one file
void makePaste(int *processesForMatrix, int numberOfProcess){
    for (int i = 0; i < numberOfProcess; i++) {
        wait(0);
    }

    for(int i = 0; i < numberOfPairs; i++){
        char** arg = calloc(processesForMatrix[i] + 2, sizeof(char*));
        arg[0] = (char*) calloc(6, sizeof(char));
        strcpy(arg[0], "paste");

        for (int j = 0; j < processesForMatrix[i]; j++){
            arg[j + 1] = calloc(FILENAME_MAX, sizeof(char));
            strcpy(arg[j + 1], getNameOfFile(i, j));
        }
        arg[processesForMatrix[i] + 1]=NULL;

        pid_t childPid = fork();
        if (childPid == 0){
            int fd = open(CC[i] -> path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(fd,1); // system call creates a copy of a file descriptor
            close(fd);
            execvp("paste", arg);
            strcpy(arg[0], "rm");
        }
        else wait(0);

        // if this is comment additional files aren't remove
        pid_t childPid2 = fork();
        if (childPid2 == 0){
            strcpy(arg[0], "rm");
            execvp("rm", arg);
        }
        else wait(0);
        //

        for (int j = 0; j < processesForMatrix[i] + 1; j++) {
            free(arg[j]);
        }
    }
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
    int *processesForMatrix = NULL;

    if (isDistinct == 1){
        processesForMatrix = calloc(numberOfPairs, sizeof(int));
        for (int i = 0; i < numberOfPairs; i ++){
            processesForMatrix[i] = min(numberOfProcess, BB[i] -> numberOfColumns);
        }
        for (int i = 0; i < numberOfPairs; i ++){
            createEmptyFiles(i, processesForMatrix[i]);
        }
    }

    pid_t *childPids = calloc(numberOfProcess, sizeof(pid_t));

    for (int i = 0; i < numberOfProcess; i ++){
        pid_t childPid = fork();
        if (childPid == 0){
            int operationCounter;
            multiply(i, numberOfProcess, timeLimit, isDistinct, &operationCounter);
            exit(operationCounter);
        }
        else if (childPid > 0){
            childPids[i] = childPid;
        }
        else{
            exit(EXIT_FAILURE);
        }
    }


    for (int i = 0; i < numberOfProcess; i++){
        int status;
        waitpid(childPids[i], &status, 0);
        printf("The process with PID = %d has done %d multiplying operations\n", childPids[i], WEXITSTATUS(status));
    }
    if (isDistinct == 1){
        makePaste(processesForMatrix, numberOfProcess);
    }

    freeList();

    return 0;
}
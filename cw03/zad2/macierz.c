#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <ftw.h>

typedef struct{
    FILE *file;
    unsigned int numberOfRows;
    unsigned int numberOfColumns;
} Matrix;

Matrix *create_matrix(char* path, unsigned int numberOfRows, unsigned int numberOfColumns){
    FILE *file = fopen(path, "w+");
    if (file == NULL) return NULL;
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix -> file = file;
    matrix -> numberOfRows = numberOfRows;
    matrix -> numberOfColumns = numberOfColumns;

    // TO DO: matrix -> file
    return matrix;
};

int countNumberOfRows(FILE *file){
    int counter = 0;
    fseek(file, 0, 0);
    char *line = NULL;
    size_t len = 0;

    while(getline(&line, &len, file) != -1 && line[0] != ' ' && line[0] != '\n'){
       counter++;
    }

    return counter;
}

int countNumberOfColumns(FILE *file){
    int counter = 1;
    fseek(file, 0, 0);
    for(char i = getc(file); i != '\n'; i = getc(file)){
        if (i == ' '){
            counter++;
        }
    }
    return counter;
}


void free_matrix(Matrix *matrix){
    fclose(matrix -> file);
    free(matrix);
}


int main(){
    return 0;
}
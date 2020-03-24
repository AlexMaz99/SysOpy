#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int MIN = -100;
int MAX = 100;

int randomNumber(int minValue, int maxValue){
    return rand() % (maxValue - minValue + 1) + minValue;
}

void generate(int numberOfPairs, int minValue, int maxValue){
    srand(time(NULL));
    FILE *lista = fopen("lista", "w+");
    if (lista == NULL) return;
    char* file = (char*) calloc(100, sizeof(char));

    for (int i = 0; i < numberOfPairs; i++){
        char number[3];
        sprintf(number, "%d", i);

        strcpy(file, "macierze/");
        strcat(file, "A");
        strcat(file, number);
        FILE *A = fopen(file, "w+");
        fprintf(lista, "%s ", file);

        strcpy(file, "macierze/");
        strcat(file, "B");
        strcat(file, number);
        FILE *B = fopen(file, "w+");
        fprintf(lista, "%s ", file);

        strcpy(file, "macierze/");
        strcat(file, "C");
        strcat(file, number);
        fprintf(lista, "%s\n", file);

        int rowsA = randomNumber(minValue, maxValue);
        int colsA = randomNumber(minValue, maxValue);
        int colsB = randomNumber(minValue, maxValue);
        
        for (int row = 0; row < rowsA; row ++){
            for (int col = 0; col < colsA; col ++){
                fprintf(A, "%d", randomNumber(MIN, MAX));
                if (col != colsA - 1){
                    fprintf(A, " ");
                }
            }
            fprintf(A, "\n");
        }

        for (int row = 0; row < colsA; row ++){
            for (int col = 0; col < colsB; col ++){
                fprintf(B, "%d", randomNumber(MIN, MAX));
                if (col != colsB - 1){
                    fprintf(B, " ");
                }
            }
            fprintf(B, "\n");
        }

        fclose(A);
        fclose(B);
    }
}

int main(int argc, char **argv){
    if (argc != 4){
        printf("Wrong number of arguments.\nExpected: ./generator [number of pairs] [min value] [max value]\n");
        exit(EXIT_FAILURE);
    }
    int numberOfPairs = atoi(argv[1]);
    int minValue = atoi(argv[2]);
    int maxValue = atoi(argv[3]);

    if (numberOfPairs <= 0 || minValue <= 0 || maxValue <= 0){
        printf("Argument must be positive\n");
        exit(EXIT_FAILURE);
    }

    if (minValue > maxValue){
        int tmp = maxValue;
        maxValue = minValue;
        minValue = tmp;
    }

    generate(numberOfPairs, minValue, maxValue);
    return 0;
}
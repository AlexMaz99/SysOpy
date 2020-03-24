#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Matrix{
    int **val;
    int col;
    int row;
} Matrix;

Matrix * readMatrix(char *path){
    FILE *file = fopen(path, "r");
    char *row = (char*)calloc(1000, sizeof(char));
    Matrix *m = (Matrix*)malloc(sizeof(Matrix));
    m -> val = (int**) calloc(1000, sizeof(int *));
    int i = 0, j;
    
    while(fgets(row, 1000, file) != NULL){
        m -> val[i] = (int*)calloc(1000, sizeof(int));
        j = 0;
        char*number = strtok(row, " \n");
        while(number!=NULL){
            m -> val[i][j ++] = atoi(number);
            number = strtok(NULL, " \n");
        }
        i++;
    }
    m -> row = i;
    m -> col = j;
    free(row);
    fclose(file);
    return m;
}

Matrix* readMatrixC(char*path) {
	Matrix* M = malloc(sizeof(Matrix));
	FILE *file = fopen(path, "r");
    if (file == NULL){
        printf("Cannot open file %s\n", path);
        exit(EXIT_FAILURE);
    }
	char input[1024];
	char* val;
	char limit[] = " \t\n";
	int rows = 1;
	int cols = 1;

	if(fgets(input, sizeof input, file)){
		strtok(input, limit);
		while (strtok(NULL, limit) != NULL) cols++;
	}
	while (fgets(input, sizeof input, file)) rows++;
	rewind(file);

	M -> row = rows;
	M -> col = cols;
	M -> val = calloc(rows, sizeof(int*));
	
    for(int r = 0; r<rows; r++) M->val[r] = calloc(cols, sizeof(int));
	int r = 0;

	while (fgets(input, sizeof input, file)) {
		int c = 0;
		val = strtok(input, limit);
		M -> val[r][c] = atoi(val);
		while (val != NULL && c < cols) {
			M ->val[r][c] = atoi(val);
			val = strtok(NULL, limit);
			c++;
		}
		r++;
	}
	fclose(file);
	return M;
}

void printMatrix(Matrix *m){
    for (int r = 0; r < m -> row; r ++){
        for (int c = 0; c < m -> col; c ++){
            printf("%d ", m->val[r][c]);
        }
        printf("\n");
    }
    printf("\n");
}

Matrix *multiplyMatrices(Matrix *A, Matrix *B){
    Matrix *C = (Matrix *) malloc(sizeof(Matrix));
    C -> col = B -> col;
    C -> row = A -> row;
    C -> val = (int **)calloc(C -> row, sizeof(int *));
    for (int i = 0; i < C -> row; i ++){
        C -> val[i] = (int *)calloc(C -> col, sizeof(int));
    }
    for (int r = 0; r < C -> row; r ++){
        for (int c = 0; c < C -> col; c ++){
            C -> val[r][c] = 0;
            for (int k = 0; k < A -> col; k ++){
                C -> val[r][c] += A -> val[r][k] * B -> val[k][c];
            }
        }
    }
    return C;
}

bool checkMatrices(Matrix *C1, Matrix * C2){
    if (C1 -> col != C2 -> col || C1 -> row != C2 -> row) return false;

    for (int r = 0; r < C1 -> row; r ++){
        for (int c = 0; c < C1 -> col; c ++){
            if (C1 -> val[r][c] != C2 -> val[r][c]) return false;
        }
    }
    return true;
}

void removeMatrix(Matrix *m){
    for (int r = 0; r < m -> row; r++) free(m -> val[r]);
    free(m -> val);
    free(m);
}

int main(int argc, char**argv){
    if (argc < 2){
        printf("Wrong number of arguments. Expected: [path to list]\n");
        exit(EXIT_FAILURE);
    }

    FILE *lista = fopen(argv[1], "r");
    if (lista == NULL){
        printf("Cannot open file: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    char file[1000];
    int i = 0;
    Matrix *A, *B, *C;

    while(fscanf(lista, "%s", file) != EOF){
        if (i % 3 == 0){
            A = readMatrixC(file);
        }
        else if (i % 3 == 1){
            B = readMatrixC(file);
        }
        else {
            C = readMatrixC(file);
            int correct = checkMatrices(multiplyMatrices(A, B), C);
            printf("Test nr %d: %s\n", i / 3, correct ? "PASS" : "FAIL");
            free(A);
            free(B);
            free(C);
        }
        i++;
    }
    return 0;

}
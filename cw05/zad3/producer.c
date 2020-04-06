#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

const int MAX_MSG_LEN = 100;

int main(int argc, char **argv){
    if (argc != 4){
        printf("Wrong number of arguments. Expected: [pipe's path] [file's path] [number of characters]\n");
        exit(EXIT_FAILURE);
    }

    char* pipe_path = argv[1];
    char* file_path = argv[2];
    int N = atoi(argv[3]);

    srand(time(NULL));

    FILE *pipe = fopen(pipe_path, "w");

    if (pipe == NULL){
        printf("Cannot open file %s\n", pipe_path);
        exit(EXIT_FAILURE);
    }
    FILE *file = fopen(file_path, "r");

    if (file == NULL){
        printf("Cannot open file %s\n", file_path);
        fclose(pipe);
        exit(EXIT_FAILURE);
    }

    char buffer[N];

    while(fread(buffer, 1, N, file) > 0){
        sleep(rand() % 2 + 1);
        char msg [N + 20];
        sprintf(msg, "#%d#%s\n", getpid(), buffer);
        fwrite(msg, 1, strlen(msg), pipe);
    }

    fclose(pipe);
    fclose(file);
    return 0;
}
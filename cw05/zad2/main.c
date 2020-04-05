#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

const int MAX_LINE_LEN = 10000;

void error(char *msg){
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    if (argc != 2){
        error("Wrong number of arguments. Expected: [path]");
    }

    FILE *file = fopen(argv[1], "r");
    char* line = malloc(MAX_LINE_LEN);
    FILE *sorted_file = popen("sort", "w");
    while(fgets(line, MAX_LINE_LEN, file) != NULL){
        fputs(line, sorted_file);
    }
    pclose(sorted_file);
    free(line);
    return 0;
}
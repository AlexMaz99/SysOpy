#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>

void generate_files(){
    char file_name[100];
    char number[2];

    for (int i = 0; i < 5; i++){
        strcpy(file_name, "files/file");
        sprintf(number, "%d", i);
        strcat(file_name, number);
        FILE *f = fopen(file_name, "w+");

        for (int j = 0; j < 10; j++){
            char c = 'A' + i;
            fwrite(&c, 1, 1, f);
        }
        fclose(f);
    }
}

int main(int argc, char **argv){

    pid_t pids[6];

    if (mkfifo("pipe", 0666) < 0){
        printf("Cannot create named pipe.\n");
        exit(EXIT_FAILURE);
    }

    generate_files();

    char* consumer [] = {"./consumer", "pipe", "./files/results", "10", NULL};
    char* producer0 [] = {"./producer", "pipe", "./files/file0", "5", NULL};
    char* producer1 [] = {"./producer", "pipe", "./files/file1", "5", NULL};
    char* producer2 [] = {"./producer", "pipe", "./files/file2", "5", NULL};
    char* producer3 [] = {"./producer", "pipe", "./files/file3", "5", NULL};
    char* producer4 [] = {"./producer", "pipe", "./files/file4", "5", NULL};


    pids[0]=fork();
    if (pids[0] == 0){
        execvp(producer0[0], producer0);
    }
    
    pids[1] = fork();
    if (pids[1] == 0){
        execvp(producer1[0], producer1);
    }

    pids[2] = fork();
    if (pids[2] == 0){
        execvp(producer2[0], producer2);
    }

    pids[3] = fork();
    if (pids[3] == 0){
        execvp(producer3[0], producer3);
    }

    pids[4] = fork();
    if (pids[4] == 0){
        execvp(producer4[0], producer4);
    }

    pids[5] = fork();
    if (pids[5] == 0){
        execvp(consumer[0], consumer);
    }

    for (int i = 0; i < 6; i++){
        waitpid(pids[i], NULL, 0);
    }

    return 0;
}
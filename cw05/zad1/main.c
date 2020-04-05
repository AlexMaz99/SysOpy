#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

const int MAX_LINE_LEN = 10000;
const int MAX_COMMANDS = 20;
const int MAX_ARGS = 10;

void error(char *msg){
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

// separate words in given command and save them to array
void separate_words(char **row, char*command){
    char* cmd_tmp = command;
    char*arg = strtok_r(command, " ", &cmd_tmp);
    int i = 0;
    while(arg != NULL && arg[0] != EOF){
        row[i] = arg;
        i ++;
        arg = strtok_r(NULL, " ", &cmd_tmp);
    }
}


int main(int argc, char **argv){
    if (argc != 2){
        error("Wrong number of arguments. Expected: [path]");
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        error("Cannot open file");
    }

    char *line = malloc(MAX_LINE_LEN);

    while(fgets(line, MAX_LINE_LEN,  file) != NULL){

        if (line[strlen(line) - 1] == '\n'){
            line[strlen(line) - 1] = '\0';
        }
        printf("%s\n", line);

        char* commands [MAX_COMMANDS][MAX_ARGS];

        for (int i = 0; i < MAX_COMMANDS; i ++){
            for (int j = 0; j < MAX_ARGS; j++){
                commands[i][j] = NULL;
            }
        }

        char *cmd_tmp = line;
        char *cmd = strtok_r(line, "|", &cmd_tmp);
        int command_count = 0;

        while(cmd != NULL){
            separate_words(commands[command_count], cmd);
            command_count ++;
            cmd = strtok_r(NULL, "|", &cmd_tmp);
        }

        int pipes[MAX_COMMANDS][2];

        for (int i = 0; i < command_count - 1; i ++){
            if (pipe(pipes[i]) < 0){
                error("Cannot make pipes.");
            }
        }

        for (int i = 0; i < command_count; i++){
            pid_t pid = fork();

            if (pid < 0){
                error("Cannot fork.");
            }
            else if (pid == 0){
                if (i > 0){
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                if (i < command_count - 1){
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                for (int j = 0; j < command_count - 1; j ++){
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                execvp(commands[i][0], commands[i]);
                exit(0);
            }
        }

        for (int j = 0; j < command_count - 1; j ++){
            close(pipes[j][0]);
            close(pipes[j][1]);
        }

        for (int j = 0; j < command_count; j ++){
            wait(0);
        }
    }

    free(line);
    fclose(file);
    
    return 0;
}
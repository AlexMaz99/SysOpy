#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>

void find_dir(){

    DIR* dir = opendir(".");
    if (dir == NULL){
        printf("Cannot open directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *file;

    while ((file = readdir(dir)) != NULL){
        
        struct stat buffer;

        // check information about a file
        if (lstat(file -> d_name, &buffer) < 0){
            printf("Cannot lstat file %s: ", file -> d_name);
            exit(EXIT_FAILURE);
        }
        
        // check if file is a directory
        if (S_ISDIR(buffer.st_mode)){

            if (strcmp(file -> d_name, ".") == 0 || strcmp(file -> d_name, "..") == 0){
                continue;
            }

            if (chdir(file -> d_name) != 0){
                printf("Cannot change path to %s\n", file -> d_name);
                exit(EXIT_FAILURE);
            }

            find_dir();

            pid_t pid_fork = fork();

            if (pid_fork < 0){
                printf("Cannot fork\n");
                exit(EXIT_FAILURE);
            }

            else if(pid_fork == 0){
                char cwd[4096];

                // get the pathname of the current working directory
                if (getcwd(cwd, 4096) == NULL){
                    printf("Cannot get current working directory\n");
                    exit(EXIT_FAILURE);
                }
                
                // get process ID (PID) of the calling process
                printf("DIRECTORY: %s, PID: %d\n", cwd, getpid());

                // replace the current process image with a new process image
                int exec_status = execlp("ls", "ls", "-l", NULL);
                if (exec_status != 0){
                    printf("Exec failed");
                    exit(EXIT_FAILURE);
                }
                exit(exec_status);
            }

            else{
                wait(0);
            }

            // change path
            if  (chdir("..") != 0){
                printf("Cannot change path to %s", file -> d_name);
                exit(EXIT_FAILURE);
            }
        }
        
    }
    closedir(dir);
}

int main(int argc, char** argv){
    if (argc != 2){
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    if (chdir(argv[1]) != 0){
        printf("Cannot chmod to %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    find_dir();

    return 0;
}
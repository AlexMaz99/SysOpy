#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
    if (argc != 2){
        printf("Wrong number of arguments. Expected: ./prog [mask / pending]\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp("pending", argv[1]) != 0){
        raise(SIGUSR1);
    }

    if (!strcmp("pending", argv[1]) || !strcmp("mask", argv[1])){
        sigset_t mask;
        sigpending(&mask);
        if(sigismember(&mask, SIGUSR1)){
            printf("Signal pending in child.\n\n");
        }
        else{
            printf("Signal not pending in child.\n\n");
        }
    }

    return 0;
}
#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>

typedef enum Mode{
    IGNORE,
    HANDLER,
    MASK,
    PENDING
} Mode;

void signalHandle(int sig){
    printf("Signal received\n");
}

int main(int argc, char **argv){
    if (argc != 3){
        printf("Wrong number of arguments. Expected: ./main [ignore / handler / mask / pending] [fork / exec]\n");
        return -1;
    }

    Mode mode;

    if (!strcmp("ignore", argv[1])){
        mode = IGNORE;
        printf("IGNORE\n");
        signal(SIGUSR1, SIG_IGN);
    }
    else if (!strcmp("handler", argv[1])){
        mode = HANDLER;
        printf("HANDLER\n");
        signal(SIGUSR1, signalHandle);
    }
    else if (!strcmp("mask", argv[1]) || !strcmp("pending", argv[1])){
        if (!strcmp("mask", argv[1])) {
            mode = MASK;
            printf("MASK\n");
        }
        else {
            mode = PENDING;
            printf("PENDING\n");
        }

        sigset_t mask; // set of signals blocked during operate current process
        sigemptyset (&mask);
        sigaddset(&mask, SIGUSR1);

        // set mask for current process
        if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0){
            printf("Cannot block signals\n");
            exit(EXIT_FAILURE);
        }

    } 
    else{
        printf("Wrong argument. There isn't option like: %s\n", argv[1]);
        return -1;
    }

    sigset_t mask;

    raise(SIGUSR1);

    if (mode == MASK || mode == PENDING){
        sigpending(&mask); // returns the set of signals that are pending for delivery to the calling thread
        
        if (sigismember(&mask, SIGUSR1)){ // check if SIGUSR1 is in given set
            printf("Signal pending in parent.\n");
        }
    }

    if (!strcmp(argv[2], "fork")){
        pid_t childPid = fork();

        if (childPid == 0){
            if (mode != PENDING) raise(SIGUSR1); // send a signal to the calling process = kill(getpid(), SIGUSR1)
            
            if (mode == MASK || mode == PENDING){
                sigpending(&mask);
                
                if (sigismember(&mask, SIGUSR1)){
                    printf("Signal pending in child.\n\n");
                }
                else{
                    printf("Signal not pending in child.\n\n");
                }
            }
        }
    }
    else if(!strcmp(argv[2], "exec") && mode != HANDLER){
        execl("./exec",  "./exec", argv[1], NULL);
    }
    
    wait(0);

    return 0;
}
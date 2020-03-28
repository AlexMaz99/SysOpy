#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>

void childHandle(int sig, siginfo_t *info, void *uncontex){
    printf("\nChild has finished execution\n");
    printf("Signal number: %d\n", info -> si_signo);
    printf("Child exit value: %d\n", info -> si_status);
    printf("Sending PID: %d\n\n", info -> si_pid);
}

void segFaultHandle(int sig, siginfo_t *info, void *uncontex){
    printf("\nSegmentation fault occured\n");
    printf("Fault address: %p\n\n", info -> si_addr);
    exit(0);
}

void statusHandle(int sig, siginfo_t *info, void *uncontex){
    printf("\n\nSignal number: %d\n", info -> si_signo);
    printf("Sending PID: %d\n", info ->si_pid);
    if (info -> si_code == SI_KERNEL){
        printf("Send by KERNEL\n\n");
    } 
    else if (info -> si_code == SI_USER){
        printf("Send by USER\n\n");
    }
}

int main(int argc, char **argv){
    if (argc != 2){
        printf("Wrong number of arguments. Expected: ./main [child / segFault / status]");
        exit(EXIT_FAILURE);
    }

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    if (!strcmp("child", argv[1])){
        act.sa_sigaction = childHandle;
        sigaction(SIGCHLD, &act, NULL);
        pid_t childPid = fork();

        if (childPid == 0){
            exit(123);
        }
        wait(NULL);
    }
    else if (!strcmp("segFault", argv[1])){
        act.sa_sigaction = segFaultHandle;
        sigaction(SIGSEGV, &act, NULL);

        char *a = NULL;
        a[10] = 'a';
    }
    else if (!strcmp("status", argv[1])){
        act.sa_sigaction = statusHandle;
        sigaction(SIGINT, &act, NULL);
        pause();
    }

    return 0;
}
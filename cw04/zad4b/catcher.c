#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>

int COUNT_SIGNAL;
int END_SIGNAL;
char* MODE;

int receivedSignals = 0;

union sigval val;

void signalsHandle(int sig, siginfo_t *info, void *uncontext){
    if (sig == COUNT_SIGNAL){
        receivedSignals ++;

        if (!strcmp("kill", MODE) || !strcmp("sigrt", MODE)){
            kill(info -> si_pid, COUNT_SIGNAL);
        }
        else{
            sigqueue(info -> si_pid, COUNT_SIGNAL, val);
        }
    }
    else if (sig == END_SIGNAL){
        if (!strcmp("kill", MODE) || !strcmp("sigrt", MODE)){
            kill(info -> si_pid, END_SIGNAL);
        }
        else{
            sigqueue(info -> si_pid, END_SIGNAL, val);
        }
        printf("Catcher received: %d signals\n", receivedSignals);
        exit(0);
    }
}

int main(int argc, char **argv){
    if (argc != 2){
        printf("Wrong number of arguments. Expected: [mode: kill / queue / sigrt]\n");
        exit(EXIT_FAILURE);
    }

    MODE = argv[1];

    if (!strcmp("kill", MODE)){
        COUNT_SIGNAL = SIGUSR1;
        END_SIGNAL = SIGUSR2;
    }
    else if(!strcmp("queue", MODE)){
        COUNT_SIGNAL = SIGUSR1;
        END_SIGNAL = SIGUSR2;
    }
    else if (!strcmp("sigrt", MODE)){
        COUNT_SIGNAL = SIGRTMIN + 1;
        END_SIGNAL = SIGRTMIN + 2;
    }
    else{
        printf("Wrong mode. Expected: [kill / queue / sigrt]\n");
        exit(EXIT_FAILURE);
    }

    val.sival_int = 0;

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, COUNT_SIGNAL);
    sigdelset(&mask, END_SIGNAL);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0){
        printf("Cannot block signals\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = signalsHandle;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, COUNT_SIGNAL);
    sigaddset(&act.sa_mask, END_SIGNAL);

    sigaction(COUNT_SIGNAL, &act, NULL);
    sigaction(END_SIGNAL, &act, NULL);

    printf("Created catcher with PID: %d\n", getpid());

    while (1){
        usleep(100);
    }
    return 0;
}
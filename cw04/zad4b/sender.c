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

int catcherPID;
int signalsToSend;
int receivedSignals;

union sigval val;
int signalsSent = 0;

void sendNextSignal(){
    signalsSent ++;
    if (!strcmp("kill", MODE) || !strcmp("sigrt", MODE)){
        kill(catcherPID, COUNT_SIGNAL);
    }
    else{
        sigqueue(catcherPID, COUNT_SIGNAL, val);
    }
}

void sendEndSignal(){
     if (!strcmp("kill", MODE) || !strcmp("sigrt", MODE)){
         kill(catcherPID, END_SIGNAL);
     }
     else{
         sigqueue(catcherPID, END_SIGNAL, val);
     }
}

void signalsHandle(int sig, siginfo_t *info, void *uncontext){
    if (sig == COUNT_SIGNAL){
        receivedSignals ++;
        if (signalsSent < signalsToSend){
            sendNextSignal();
        }
        else{
            sendEndSignal();
        }
    }
    else if (sig == END_SIGNAL){
        printf("Sender received: %d signals, expected: %d\n", receivedSignals, signalsToSend);
        exit(0);
    }
}

int main(int argc, char **argv){
    // argv[1] = catcher's PID
    // argv[2] = number of signals to send
    // argv[3] = mode of sending signal

    if (argc != 4){
        printf("Wrong number of arguments. Expected: [catcher's PID] [number of signals to send] [mode: kill / queue / sigrt]\n");
        exit(EXIT_FAILURE);
    }
    catcherPID = atoi(argv[1]);
    signalsToSend = atoi(argv[2]);
    MODE = argv[3];

    if (!strcmp("kill", MODE)){
        COUNT_SIGNAL = SIGUSR1;
        END_SIGNAL = SIGUSR2;
    }
    else if (!strcmp("queue", MODE)){
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

    printf("Created sender with PID: %d\n", getpid());

    sendNextSignal();

    while (1){
        usleep(100);
    }
    return 0;

}
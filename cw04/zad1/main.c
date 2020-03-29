#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int stop = 0;

// SIGINT - CTRL+C
void signalSIGINT(int signum){
    printf("\nOdebrano sygnal SIGINT\n");
    exit(0);
}

// SIGTSTP - CTRL+Z
void signalSTP(){
    if (stop == 0){
        printf("\nOczekuje na CTRL + Z - kontynuacja albo CTR + C - zakonczenie programu\n");
    }
    else{
        printf("\n");
    }
    stop = 1 - stop;
}

int main(){
    // set the disposition of the signal SIGINT to function signalSIGINT()
    signal(SIGINT, signalSIGINT);

    // change the action taken by a process on receipt of signal SIGSTP
    struct sigaction act;
    act.sa_handler = signalSTP;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask); // initialize the signal set given by set to empty
    sigaction(SIGTSTP, &act, NULL);

    while(1){
        if (stop == 1){
            pause(); // wait for signal
        }
        system("ls");
        sleep(2); // sleep process
    }

    return 0;
}
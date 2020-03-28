#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int stop = 0;

void signalSIGINT(int signum){
    printf("\nOdebrano sygnal SIGINT\n");
    exit(0);
}

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
    signal(SIGINT, signalSIGINT);

    struct sigaction act;
    act.sa_handler = signalSTP;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTSTP, &act, NULL);

    while(1){
        if (stop == 1){
            pause();
        }
        system("ls");
        sleep(2);
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "../zad1/library.h"

clock_t startTime, endTime;
struct tms st_cpu, en_cpu;
FILE *resultFile;

void startTimer(){
    startTime = times(&st_cpu);
}
void endTimer(){
    endTime = times(&en_cpu);
}

void writeResultToFile(FILE *file, char*name){
    endTimer();
    double realTime = (double) (endTime - startTime) / sysconf(_SC_CLK_TCK);

}

int main(int argc, char**argv) {
    
}

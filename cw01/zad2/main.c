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

void writeResultToFile(){
    endTimer();
    double realTime = (double) (endTime - startTime) / sysconf(_SC_CLK_TCK);
    double userTime = (double) (en_cpu.tms_utime - st_cpu.tms_utime) / sysconf(_SC_CLK_TCK);
    double systemTime = (double)(en_cpu.tms_stime - st_cpu.tms_stime) / sysconf(_SC_CLK_TCK);
    printf("%f", realTime);
    printf("%f", userTime);
    printf("%f", systemTime);

}



int main(int argc, char**argv) {
    if (argc < 3){
        printf("Number of argument must be at least 3");
        return -1;
    }
    if(strcmp(argv[1], "create_table") !=0 ){
        printf("First argument has to be create_table");
        return -1;
    }
    
    //resultFile = fopen("raport2.txt", "a");

    int numberOfBlocks = atoi(argv[2]);
    struct MainArray* mainArray = createArray(numberOfBlocks);
    int i = 3;
    while (i < argc){
        startTimer();

        if (!strcmp(argv[i], "compare_pairs")){
            comparePairs(argv[i+1], numberOfBlocks * 2, mainArray);
            i += 2;
        }
        else if (!strcmp(argv[i], "remove_operation")){
            int indexOfBlock = atoi(argv[i + 1]);
            int indexOfOperation = atoi(argv[i + 2]);
            deleteOperation(mainArray, indexOfBlock, indexOfOperation);
            i += 3;
        }
        else if (!strcmp(argv[i], "remove_block")){
            int indexOfBlock = atoi(argv[i + 1]);
            deleteBlock(mainArray, indexOfBlock);
            i += 2;
        }
        writeResultToFile();
    }
    deleteArray(mainArray);
    return 0;
}

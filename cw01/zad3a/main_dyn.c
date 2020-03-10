#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <dlfcn.h>

clock_t startTime, endTime;
struct tms startCpu, endCpu;

FILE *resultFile;
void* handle;

void startTimer(){
    startTime = times(&startCpu);
}
void endTimer(){
    endTime = times(&endCpu);
}

void writeResultToFile(FILE* file, char* name){
    endTimer();
    double realTime = (double) (endTime - startTime) / sysconf(_SC_CLK_TCK);
    double userTime = (double) (endCpu.tms_utime - startCpu.tms_utime) / sysconf(_SC_CLK_TCK);
    double systemTime = (double)(endCpu.tms_stime - startCpu.tms_stime) / sysconf(_SC_CLK_TCK);
    fprintf(file, "Operation: %s\n", name);
    fprintf(file, "Real time: %f\n", realTime);
    fprintf(file, "User time: %f\n", userTime);
    fprintf(file, "System time: %f\n\n", systemTime);

    printf("Operation: %s\n", name);
    printf("Real time: %f\n", realTime);
    printf("User time: %f\n", userTime);
    printf("System time: %f\n\n", systemTime);
}



int main(int argc, char**argv) {
    handle = dlopen("../zad1/lib_diff.so", RTLD_LAZY);
    if (!handle){
        printf("Cannot find lib_diff.so");
        exit(EXIT_FAILURE);
    }

    struct MainArray* (*createArray)();
    void (*deleteBlock)();
    void (*deleteOperation)();
    void (*deleteArray)();
    // void (*compareTwoFiles)();
    // int (*countOperationsInBlock)();
    // struct Block* (*createBlockAndOperations)();
    // void (*definePairSequence)();
    void (*comparePairs)();
    // int (*countNumberOfFiles)();
    // int (*getNumberOfOperations)();
    // char* (*getOperation)();
    
    createArray = dlsym(handle, "createArray");
    deleteBlock = dlsym(handle, "deleteBlock");
    deleteOperation = dlsym(handle, "deleteOperation");
    deleteArray = dlsym(handle, "deleteArray");
    // compareTwoFiles = dlsym(handle, "compareTwoFiles");
    // countOperationsInBlock = dlsym(handle, "countOperationsInBlock");
    // createBlockAndOperations = dlsym(handle, "createBlockAndOperations");
    // definePairSequence = dlsym(handle, "definePairSequence");
    comparePairs = dlsym(handle, "comparePairs");
    // countNumberOfFiles = dlsym(handle, "countNumberOfFiles");
    // getNumberOfOperations = dlsym(handle, "getNumberOfOperations");
    // getOperation = dlsym(handle, "getOperation");

    if (argc < 3){
        printf("Number of argument must be at least 3");
        return -1;
    }
    if(strcmp(argv[1], "create_table") !=0 ){
        printf("First argument has to be create_table");
        return -1;
    }
    
    resultFile = fopen("raport.txt", "a");

    int numberOfBlocks = atoi(argv[2]);
    struct MainArray* mainArray = createArray(numberOfBlocks);
    int i = 3;
    char* operation;
    while (i < argc){
        startTimer();
        
        if (!strcmp(argv[i], "compare_pairs")){
            comparePairs(argv[i+1], mainArray);
            i += 2;
            operation = "compare_pairs";
        }
        else if (!strcmp(argv[i], "remove_operation")){
            int indexOfBlock = atoi(argv[i + 1]);
            int indexOfOperation = atoi(argv[i + 2]);
            deleteOperation(mainArray, indexOfBlock, indexOfOperation);
            i += 3;
            operation = "remove_operation";
        }
        else if (!strcmp(argv[i], "remove_block")){
            int indexOfBlock = atoi(argv[i + 1]);
            deleteBlock(mainArray, indexOfBlock);
            i += 2;
            operation = "remove_block";
        }
        else{
            operation = "wrong operation";
            i ++;
        }
        writeResultToFile(resultFile, operation);
    }
    deleteArray(mainArray);
    dlclose(handle);
    return 0;
}

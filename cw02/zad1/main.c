#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

clock_t startTime, endTime;
struct tms startCpu, endCpu;
FILE *resultFile;

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

char *generate_string(int size)
{
  if (size < 1) return NULL;
  char *base = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  size_t length = strlen(base);
  char *result = malloc(size * sizeof(char));

  for (int i = 0; i < size; i++)
    result[i] = base[rand() % length];

  return result;
}

void generate(char *path, int numberOfRecords, int length) {
    FILE *file = fopen(path, "w+"); // write and read, if file exists - overwrite, if not - create 
    char *result = malloc(length * sizeof(char) + 1);

    for (int i = 0; i < numberOfRecords; i++) {
        result = generate_string(length);
        result[length] = '\n';
        fwrite(result, sizeof(char), (size_t) length + 1, file);
    }
    fclose(file);
    free(result);
};

void copy_lib(char* sourceFile, char* destinationFile, int numberOfRecords, int length){
    FILE *source = fopen(sourceFile, "r");
    if (source == NULL){
        fprintf(stderr, "Cannot open source file in function copy_lib\n");
        exit(-1);
    }
    FILE *destination = fopen(destinationFile, "w+");
    if (destination == NULL){
        fprintf(stderr, "Cannot open destination file in function copy_lib\n");
        exit(-1);
    }
    char *tmp = malloc(length * sizeof(char));

    for (int i = 0; i < numberOfRecords; i++){
        if (fread(tmp, sizeof(char), (size_t) (length + 1), source) != length + 1){
            fprintf(stderr, "Cannot read from source file in function copy_lib\n");
            exit(-1);
        }
        if(fwrite(tmp, sizeof(char), (size_t)(length + 1), destination) != length + 1){
            fprintf(stderr, "Cannot read from destination file in function copy_lib\n");
            exit(-1);
        }
    }
    fclose(source);
    fclose(destination);
    free(tmp);
}

void copy_sys(char* sourceFile, char* destinationFile, int numberOfRecords, int length){
    int source = open(sourceFile, O_RDONLY);
    if (source < 0){
        fprintf(stderr, "Cannot open source file in function copy_sys\n");
        exit(-1);
    }
    int destination = open(destinationFile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (destination < 0){
        fprintf(stderr, "Cannot open destination file in function copy_sys\n");
        exit(-1);
    }
    char *tmp = malloc(length * sizeof(char));

    for (int i = 0; i < numberOfRecords; i++){
        if (read(source, tmp, length + 1) < 0){
            fprintf(stderr, "Cannot read from source file in function copy_sys\n");
            exit(-1);
        }
        if (write(destination, tmp, length + 1) < 0){
            fprintf(stderr, "Cannot read from destination file in function copy_sys\n");
            exit(-1);
        }
    }
    close(source);
    close(destination);
    free(tmp);
}

void swap_lines_lib(FILE * file, int i, int j, int length){
    char* line1 = calloc(length + 1, sizeof(char));
    char* line2 = calloc(length + 1, sizeof(char));

    fseek(file, (length + 1) * i, 0);
    fread(line1, sizeof(char), length + 1, file);

    fseek(file, (length + 1) * j, 0);
    fread(line2, sizeof(char), length + 1, file);

    fseek(file, (length + 1) * j, 0);
    fwrite(line1, sizeof(char), length + 1, file);

    fseek(file, (length + 1) * i, 0);
    fwrite(line2, sizeof(char), length + 1, file);

    free(line1);
    free(line2);
}

void swap_lines_sys(int file, int i, int j, int length){
    char* line1 = calloc(length + 1, sizeof(char));
    char* line2 = calloc(length + 1, sizeof(char));

    lseek(file, (length + 1) * i, 0);
    read(file, line1, length + 1);

    lseek(file, (length + 1) * j, 0);
    read(file, line2, length + 1);

    lseek(file, (length + 1) * j, 0);
    write(file, line1, length + 1);

    lseek(file, (length + 1) * i, 0);
    write(file, line2, length + 1);

    free(line1);
    free(line2);
}

int partition_lib(FILE *file, int length, int low, int high){
    char* pivot = calloc(length + 1, sizeof(char));
    fseek(file, (length + 1) * high, 0);
    fread(pivot, sizeof(char), length + 1, file);
    
    int i = low - 1;
    char * tmp = calloc(length + 1, sizeof(char));
    for (int j = low; j < high; j ++){
        fseek(file, (length + 1) * j, 0);
        fread(tmp, sizeof(char), length + 1, file);
        if (strcmp(tmp, pivot) < 0){
            i++;
            swap_lines_lib(file, i, j, length);
        }
    }
    swap_lines_lib(file, i + 1, high, length);
    free(tmp);
    free(pivot);
    return (i + 1);
}

int partition_sys(int file, int length, int low, int high){
    char* pivot = calloc(length + 1, sizeof(char));
    lseek(file, (length + 1) * high, 0);
    read(file, pivot, length + 1);
    
    int i = low - 1;
    char * tmp = calloc(length + 1, sizeof(char));
    for (int j = low; j < high; j ++){
        lseek(file, (length + 1) * j, 0);
        read(file, tmp, length + 1);
        if (strcmp(tmp, pivot) < 0){
            i++;
            swap_lines_sys(file, i, j, length);
        }
    }
    swap_lines_sys(file, i + 1, high, length);
    free(tmp);
    free(pivot);
    return (i + 1);
}

void quick_sort_lib(FILE *file, int length,  int low, int high){
    if (low < high){
        int pivot = partition_lib(file, length, low, high);
        quick_sort_lib(file, length, low, pivot - 1);
        quick_sort_lib(file, length, pivot + 1, high);
    }
}

void quick_sort_sys(int file, int length,  int low, int high){
    if (low < high){
        int pivot = partition_sys(file, length, low, high);
        quick_sort_sys(file, length, low, pivot - 1);
        quick_sort_sys(file, length, pivot + 1, high);
    }
}

void sort(char* file, int numberOfRecords, int length, int lib){
    if (lib == 1){
        FILE *source = fopen(file, "r+");
        if (source == NULL){
            fprintf(stderr, "Cannot open file in function sort\n");
            exit(-1);
        }
        quick_sort_lib(source, length, 0, numberOfRecords - 1);
        fclose(source);
    }
    else {
        int source = open(file, O_RDWR);
        if (source < 0){
            fprintf(stderr, "Cannot open file in function sort\n");
            exit(-1);
        }
        quick_sort_sys(source, length, 0, numberOfRecords - 1);
        close(source);
    }

}

void exec_generate(int argc, char** argv, int i){
    if (i + 3 > argc){
        fprintf(stderr, "Wrong arguments in generate command");
        exit(-1);
    }

    char*file = argv[i + 1];
    int numberOfRecords = atoi(argv[i + 2]);
    int length = atoi(argv[i + 3]);
    generate(file, numberOfRecords, length);
}

void exec_sort(int argc, char** argv, int i){
    if (i + 4 >= argc){
        fprintf(stderr, "Wrong arguments in sort command");
        exit(-1);
    }

    char* file = argv[i + 1];
    int numberOfRecords = atoi(argv[i + 2]);
    int length = atoi(argv[i + 3]);
    char* lib_sys = argv[i + 4];
    char operation[64];
    snprintf(operation, sizeof operation,"sort %s, records: %d, bytes: %d", lib_sys, numberOfRecords, length);
    
    if (!strcmp(lib_sys, "lib")){
        startTimer();
        sort(file, numberOfRecords, length, 1);
        writeResultToFile(resultFile, operation);
    }
    else if (!strcmp(lib_sys, "sys")){
        startTimer();
        sort(file, numberOfRecords, length, 0);
        writeResultToFile(resultFile, operation);
    }
    else{
        fprintf(stderr, "Wrong argument in sort command");
        exit(-1);
    }

}

void exec_copy(int argc, char** argv, int i){
    if (i + 5 >= argc){
        fprintf(stderr, "Wrong arguments in copy command");
        exit(-1);
    }
    char* source = argv[i + 1];
    char* destination = argv[i + 2];
    int numberOfRecords = atoi(argv[i + 3]);
    int length = atoi(argv[i + 4]);
    char* lib_sys = argv[i + 5];

    char operation[64];
    snprintf(operation, sizeof operation,"copy %s, records: %d, bytes: %d", lib_sys, numberOfRecords, length);

    if (!strcmp(lib_sys, "lib")){
        startTimer();
        copy_lib(source, destination, numberOfRecords, length);
        writeResultToFile(resultFile, operation);
    }
    else if (!strcmp(lib_sys, "sys")){
        startTimer();
        copy_sys(source, destination, numberOfRecords, length);
        writeResultToFile(resultFile, operation);
    }
}

int main(int argc, char** argv){

    if (argc < 5){
        printf("Number of argument must be at least 5");
        return -1;
    }

    char* file = "wyniki.txt";
    resultFile = fopen(file, "a");
    if (resultFile == NULL){
        exit(-1);
    }
    int i = 1;
    while (i < argc){
        if (!strcmp(argv[i], "generate")){
            exec_generate(argc, argv, i);
            i += 4;
        }
        else if (!strcmp(argv[i], "sort")){
            exec_sort(argc, argv, i);
            i += 5;
        }
        else if (!strcmp(argv[i], "copy")){
            exec_copy(argc, argv, i);
            i += 6;
        }
        else{
            printf("Wrong command");
            i++;
        }
    }
    fclose(resultFile);
    return 0;
}

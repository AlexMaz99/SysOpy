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

void copy(char* sourceFile, char* destinationFile, int numberOfRecords, int length, int lib){
    if (lib == 1){
        copy_lib(sourceFile, destinationFile, numberOfRecords, length);
    }
    else {
        copy_sys(sourceFile, destinationFile, numberOfRecords, length);
    }
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

void quick_sort_lib(FILE *file, int length,  int low, int high){
    if (low < high){
        int pivot = partition_lib(file, length, low, high);
        quick_sort_lib(file, length, low, pivot - 1);
        quick_sort_lib(file, length, pivot + 1, high);
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
        //quick_sort_sys(source, length, 0, numberOfRecords - 1);
        close(source);
    }

}

int main(int argc, char** argv){
    generate("wyniki.txt", 100, 8);
    sort("wyniki.txt", 100, 8, 1);

    return 0;
}

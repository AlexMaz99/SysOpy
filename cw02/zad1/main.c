#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include<sys/types.h>
#include <time.h>

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

int main(int argc, char** argv){
    generate("wyniki.txt", 100, 512);

    return 0;
}

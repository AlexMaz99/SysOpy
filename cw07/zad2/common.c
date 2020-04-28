#include "common.h"

int rand_int(){
    return (rand() % (MAX_VAL - MIN_VAL + 1) + MIN_VAL);
}

int rand_time(){
    return ((rand() % (MAX_SLEEP - MIN_SLEEP + 1) + MIN_SLEEP) * 1000);
}

void error_exit(char* message, char* place){
    printf("ERROR: %s, in %s\n", message, place);
    exit(EXIT_FAILURE);
}
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

int get_semaphore_id(){
    key_t key = ftok(getenv("HOME"), 0);
    int id = semget(key, 0, 0);
    if (id < 0) error_exit("Cannot get semaphore id", "function get_semaphore_id");
    return id;
}

int get_shared_memory_id(){
    key_t key = ftok(getenv("HOME"), 1);
    int id = shmget(key, 0, 0);
    if (id < 0) error_exit("Cannot access shared memory", "function get_shared_memory");
    return id;
}
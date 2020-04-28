#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define MAX_ORDERS 5
#define MIN_SLEEP 100
#define MAX_SLEEP 1000
#define MIN_VAL 1
#define MAX_VAL 100

#define RECEIVERS 3
#define SENDERS 3
#define PACKERS 3

#define ALL_WORKERS (RECEIVERS + SENDERS + PACKERS)

#define SHARED_MEMORY "/SHARED_MEMORY"


#define SEMAPHORES_NUMBER 6

// is array modifying right now by someone
#define FREE_ARRAY 0
// first free index in array
#define FREE_INDEX 1
// first index order to prepare (pack)
#define PACK_INDEX 2
// number of orders to prepare (pack)
#define PACK_NUMBER 3
// first index order to send
#define SEND_INDEX 4
// number of orders to send
#define SEND_NUMBER 5

extern const char* SEMAPHORES_NAMES[6];

typedef struct orders{
    int values[MAX_ORDERS];
}orders;

int rand_int();
int rand_time();
void error_exit(char *message, char* place);

#endif //COMMON_H
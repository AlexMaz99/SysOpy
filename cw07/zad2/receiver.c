#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>

#include "common.h"

sem_t * semaphores[SEMAPHORES_NUMBER];
int shared_memory_descriptor;
const char* SEMAPHORES_NAMES[6] = {"/FREE_ARRAY", "/FREE_INDEX", "/PACK_INDEX", "/PACK_NUMBER", "/SEND_INDEX", "/SEND_NUMBER"};

void sigint_handle(int signum){
    for (int i = 0; i < SEMAPHORES_NUMBER; i ++){
        if (sem_close(semaphores[i]) < 0) {
            error_exit("Cannot close semaphore", "receiver");
        }
    }
    exit(EXIT_SUCCESS);
}

int get_value(int index){
    int val;
    sem_getvalue(semaphores[index], &val);
    return val;
}

void add_order(){
    sem_wait(semaphores[FREE_ARRAY]);
    sem_post(semaphores[FREE_INDEX]);

    orders* orders = mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_descriptor, 0);
    if (orders == (void *) -1) error_exit("Cannot map shared memory", "function add_order (receiver)");
    

    int index = (get_value(1) - 1) % MAX_ORDERS;
    orders -> values[index] = rand_int();

    int orders_to_send = get_value(SEND_NUMBER);
    int orders_to_prepare = get_value(PACK_NUMBER) + 1;

    printf("[%d %ld] RECEIVER: Received number: %d. Number of orders to prepare: %d. Number of orders to send: %d.\n", 
            getpid(), time(NULL), orders -> values[index], orders_to_prepare, orders_to_send);

    if (munmap(orders, sizeof(orders)) < 0) error_exit("Cannot unmap shared memory", "function add_order (receiver)");

    sem_post(semaphores[FREE_ARRAY]);
    sem_post(semaphores[PACK_NUMBER]);
}

int main(){
    srand(time(NULL));

    signal(SIGINT, sigint_handle);

    // init semaphores
    for (int i = 0; i < SEMAPHORES_NUMBER; i ++){
        semaphores[i] = sem_open(SEMAPHORES_NAMES[i], O_RDWR);
        if (semaphores[i] < 0) error_exit("Cannot access semaphore", "receiver");
    }
    
    // init shared memory
    shared_memory_descriptor = shm_open(SHARED_MEMORY, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shared_memory_descriptor < 0) error_exit("Cannot access shared memory", "receiver");

    while(1){
        usleep(rand_time());
        if (get_value(PACK_NUMBER) + get_value(SEND_NUMBER) < MAX_ORDERS){
            add_order();
        }
    }
    return 0;
}
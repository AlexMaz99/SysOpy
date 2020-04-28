#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "common.h"

void add_order(int semaphore_id, int shared_memory_id){
    sembuf *sops = (sembuf *) calloc(3, sizeof(sembuf));

    sops[0].sem_num = FREE_ARRAY;
    sops[0].sem_op = 0;

    sops[1].sem_num = FREE_ARRAY;
    sops[1].sem_op = 1;

    sops[2].sem_num = FREE_INDEX;
    sops[2].sem_op = 1;

    for (int i = 0; i < 3; i ++){
        sops[i].sem_flg = 0;
    }

    if (semop(semaphore_id, sops, 3) < 0) error_exit("Cannot excecute operations on semaphores", "function add_order (receiver)");

    
    orders* orders = shmat(shared_memory_id, NULL, 0);
    if (orders == (void *) -1) error_exit("Cannot access shared memory", "function add_order (receiver)");

    int index = semctl(semaphore_id, FREE_INDEX, GETVAL, NULL);
    if (index == -1) error_exit("Cannot get index", "add_order (receiver)");
    index = (index - 1) % MAX_ORDERS;

    orders -> values[index] = rand_int();

    int orders_to_send = semctl(semaphore_id, SEND_NUMBER, GETVAL, NULL);
    if (orders_to_send == -1) error_exit("Cannot take number of orders to send", "function add_order (receiver)");
    
    int orders_to_prepare = semctl(semaphore_id, PACK_NUMBER, GETVAL, NULL) + 1;
    if (orders_to_prepare - 1 == -1) error_exit("Cannot take number of orders to prepare", "function add_order (receiver)");

    printf("[%d %ld] RECEIVER: Received number: %d. Number of orders to prepare: %d. Number of orders to send: %d.\n", 
            getpid(), time(NULL), orders -> values[index], orders_to_prepare, orders_to_send);

    if (shmdt(orders) == -1) error_exit("Cannot detach the shared memory segment", "function add_order (receiver)");

    
    sembuf *finalize = calloc(2, sizeof(sembuf));

    finalize[0].sem_num = FREE_ARRAY;
    finalize[0].sem_op = -1;

    finalize[1].sem_num = PACK_NUMBER;
    finalize[1].sem_op = 1;

    for (int i = 0; i < 2; i ++){
        finalize[i].sem_flg = 0;
    }

    if (semop(semaphore_id, finalize, 2) < 0) error_exit("Cannot excecute operations on semaphores", "function add_order (receiver)");

}

int main(){
    srand(time(NULL));

    int semaphore_id = get_semaphore_id();
    int shared_memory_id = get_shared_memory_id();

    while(1){
        usleep(rand_time());
        if (semctl(semaphore_id, PACK_NUMBER, GETVAL, NULL) + semctl(semaphore_id, SEND_NUMBER, GETVAL, NULL) < MAX_ORDERS){
            add_order(semaphore_id, shared_memory_id);
        }
    }
    return 0;
}
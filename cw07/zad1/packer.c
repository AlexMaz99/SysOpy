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

void pack_order(int semaphore_id, int shared_memory_id){
    sembuf *sops = (sembuf *) calloc(4, sizeof(sembuf));

    sops[0].sem_num = FREE_ARRAY;
    sops[0].sem_op = 0;

    sops[1].sem_num = FREE_ARRAY;
    sops[1].sem_op = 1;

    sops[2].sem_num = PACK_INDEX;
    sops[2].sem_op = 1;

    sops[3].sem_num = PACK_NUMBER;
    sops[3].sem_op = -1;

    for (int i = 0; i < 4; i ++){
        sops[i].sem_flg = 0;
    }

    if (semop(semaphore_id, sops, 4) < 0) error_exit("Cannot excecute operations on semaphores", "function pack_order (packer)");

    
    orders* orders = shmat(shared_memory_id, NULL, 0);
    if (orders == (void *) -1) error_exit("Cannot access shared memory", "function pack_order (packer)");

    int index = semctl(semaphore_id, PACK_INDEX, GETVAL, NULL);
    if (index == -1) error_exit("Cannot get index", "pack_order (packer)");
    index = (index - 1) % MAX_ORDERS;

    orders -> values[index] *= 2;

    int orders_to_send = semctl(semaphore_id, SEND_NUMBER, GETVAL, NULL) + 1;
    if (orders_to_send - 1 == -1) error_exit("Cannot take number of orders to send", "function pack_order (packer)");
    
    int orders_to_prepare = semctl(semaphore_id, PACK_NUMBER, GETVAL, NULL);
    if (orders_to_prepare == -1) error_exit("Cannot take number of orders to prepare", "function pack_order (packer)");

    printf("[%d %ld] PACKER: Prepared order at size: %d. Number of orders to prepare: %d. Number of orders to send: %d.\n", 
            getpid(), time(NULL), orders -> values[index], orders_to_prepare, orders_to_send);

    if (shmdt(orders) == -1) error_exit("Cannot detach the shared memory segment", "function pack_order (packer)");

    
    sembuf *finalize = calloc(2, sizeof(sembuf));

    finalize[0].sem_num = FREE_ARRAY;
    finalize[0].sem_op = -1;

    finalize[1].sem_num = SEND_NUMBER;
    finalize[1].sem_op = 1;

    for (int i = 0; i < 2; i ++){
        finalize[i].sem_flg = 0;
    }
    
    if (semop(semaphore_id, finalize, 2) < 0) error_exit("Cannot excecute operations on semaphores", "function pack_order (packer)");

}

int main(){
    srand(time(NULL));

    int semaphore_id = get_semaphore_id();
    int shared_memory_id = get_shared_memory_id();

    while(1){
        usleep(rand_time());
        if (semctl(semaphore_id, PACK_NUMBER, GETVAL, NULL) > 0){
            pack_order(semaphore_id, shared_memory_id);
        }
    }
    return 0;
}
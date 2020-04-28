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

void send_order(int semaphore_id, int shared_memory_id){
    sembuf *sops = (sembuf *) calloc(4, sizeof(sembuf));

    sops[0].sem_num = FREE_ARRAY;
    sops[0].sem_op = 0;

    sops[1].sem_num = FREE_ARRAY;
    sops[1].sem_op = 1;

    sops[2].sem_num = SEND_INDEX;
    sops[2].sem_op = 1;

    sops[3].sem_num = SEND_NUMBER;
    sops[3].sem_op = -1;

    for (int i = 0; i < 4; i ++){
        sops[i].sem_flg = 0;
    }

    if (semop(semaphore_id, sops, 4) < 0) error_exit("Cannot excecute operations on semaphores", "function send_order (sender)");

    
    orders* orders = shmat(shared_memory_id, NULL, 0);
    if (orders == (void *) -1) error_exit("Cannot access shared memory", "function send_order (sender)");

    int index = semctl(semaphore_id, SEND_INDEX, GETVAL, NULL);
    if (index == -1) error_exit("Cannot get index", "send_order (sender)");
    index = (index - 1) % MAX_ORDERS;

    orders -> values[index] *= 3;

    int orders_to_send = semctl(semaphore_id, SEND_NUMBER, GETVAL, NULL);
    if (orders_to_send == -1) error_exit("Cannot take number of orders to send", "function send_order (sender)");
    
    int orders_to_prepare = semctl(semaphore_id, PACK_NUMBER, GETVAL, NULL);
    if (orders_to_prepare == -1) error_exit("Cannot take number of orders to prepare", "function send_order (sender)");

    printf("[%d %ld] SENDER: Send order at size: %d. Number of orders to prepare: %d. Number of orders to send: %d.\n", 
            getpid(), time(NULL), orders -> values[index], orders_to_prepare, orders_to_send);

    orders -> values[index] = 0;

    if (shmdt(orders) == -1) error_exit("Cannot detach the shared memory segment", "function send_order (sender)");

    
    sembuf *finalize = calloc(1, sizeof(sembuf));

    finalize[0].sem_num = FREE_ARRAY;
    finalize[0].sem_op = -1;
    finalize[0].sem_flg = 0;

    if (semop(semaphore_id, finalize, 1) < 0) error_exit("Cannot excecute operations on semaphores", "function send_order (sender)");

}

int main(){
    srand(time(NULL));

    int semaphore_id = get_semaphore_id();
    int shared_memory_id = get_shared_memory_id();

    while(1){
        usleep(rand_time());
        if (semctl(semaphore_id, 5, GETVAL, NULL) > 0){
            send_order(semaphore_id, shared_memory_id);
        }
    }
    return 0;
}
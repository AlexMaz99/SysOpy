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

int semaphore_id;
int shared_memory_id;
pid_t child_pids[ALL_WORKERS];

void clear_before_exit(){
    semctl(semaphore_id, 0, IPC_RMID, NULL);
    shmctl(shared_memory_id, IPC_RMID, NULL);
}

void sigint_handle(int signum){
    printf("\nExit\n");
    for (int i = 0; i < ALL_WORKERS; i ++){
        kill(child_pids[i], SIGINT);
    }
    clear_before_exit();
    exit(EXIT_SUCCESS);
}

void create_semaphore(){
    key_t semaphore_key = ftok(getenv("HOME"), 0);
    semaphore_id = semget(semaphore_key, SEMAPHORES, IPC_CREAT | 0666);
    if (semaphore_id < 0) error_exit("Cannot create semaphore", "function create_semaphore (main)");

    union semun arg;
    arg.val = 0;

    semctl(semaphore_id, FREE_ARRAY, SETVAL, arg);
    semctl(semaphore_id, FREE_INDEX, SETVAL, arg);
    semctl(semaphore_id, PACK_INDEX, SETVAL, arg);
    semctl(semaphore_id, PACK_NUMBER, SETVAL, arg);
    semctl(semaphore_id, SEND_INDEX, SETVAL, arg);
    semctl(semaphore_id, SEND_NUMBER, SETVAL, arg);
}

void run_receivers(){
    for (int i = 0; i < RECEIVERS; i ++){
        pid_t child_pid = fork();
        if (child_pid == 0){
            execlp("./receiver", "receiver", NULL);
        }
        child_pids[i] = child_pid;
    }
}

void run_packers(){
    for (int i = 0; i < PACKERS; i ++){
        pid_t child_pid = fork();
        if (child_pid == 0){
            execlp("./packer", "packer", NULL);
        }
        child_pids[i + RECEIVERS] = child_pid;
    }
}

void run_senders(){
    for (int i = 0; i < SENDERS; i ++){
        pid_t child_pid = fork();
        if (child_pid == 0){
            execlp("./sender", "sender", NULL);
        }
        child_pids[i + RECEIVERS + PACKERS] = child_pid;
    }

    for (int i = 0; i < ALL_WORKERS; i++){
        wait(NULL);
    }
}

void create_shared_memory(){
    key_t key = ftok(getenv("HOME"), 1);
    shared_memory_id = shmget(key, sizeof(orders), IPC_CREAT | 0666);
    if (shared_memory_id < 0) error_exit("Cannot create shared memory", "function create_shared_memory (main)");
}

int main(){

    signal(SIGINT, sigint_handle);
    
    create_semaphore();
    create_shared_memory();

    run_receivers();
    run_packers();
    run_senders();

    clear_before_exit();
    return 0;
}
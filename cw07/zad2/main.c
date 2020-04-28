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

pid_t child_pids[ALL_WORKERS];
const char* SEMAPHORES_NAMES[6] = {"/FREE_ARRAY", "/FREE_INDEX", "/PACK_INDEX", "/PACK_NUMBER", "/SEND_INDEX", "/SEND_NUMBER"};

void clear_before_exit(){
    for (int i = 0; i < SEMAPHORES_NUMBER; i ++){
        if (sem_unlink(SEMAPHORES_NAMES[i]) < 0) error_exit("Cannot delete semaphore", "function clear_before_exit (main)");
    }
    if (shm_unlink(SHARED_MEMORY)) error_exit("Cannot delete shared memory", "function clear_before_exit (main)");
}

void sigint_handle(int signum){
    printf("\nExit\n");
    for (int i = 0; i < ALL_WORKERS; i ++){
        kill(child_pids[i], SIGINT);
    }
    clear_before_exit();
    exit(0);
}

void create_semaphore(){
    sem_t *sem = sem_open(SEMAPHORES_NAMES[FREE_ARRAY], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 1);
    if (sem == SEM_FAILED) error_exit("Cannot create semaphore FREE_ARRAY", "function create_semaphore (main)");
    sem_close(sem);

    sem = sem_open(SEMAPHORES_NAMES[FREE_INDEX], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (sem == SEM_FAILED) error_exit("Cannot create semaphore FREE_INDEX", "function create_semaphore (main)");
    sem_close(sem);

    sem = sem_open(SEMAPHORES_NAMES[PACK_INDEX], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (sem == SEM_FAILED) error_exit("Cannot create semaphore PACK_INDEX", "function create_semaphore (main)");
    sem_close(sem);

    sem = sem_open(SEMAPHORES_NAMES[PACK_NUMBER], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (sem == SEM_FAILED) error_exit("Cannot create semaphore PACK_NUMBER", "function create_semaphore (main)");
    sem_close(sem);

    sem = sem_open(SEMAPHORES_NAMES[SEND_INDEX], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (sem == SEM_FAILED) error_exit("Cannot create semaphore SEND_INDEX", "function create_semaphore (main)");
    sem_close(sem);

    sem = sem_open(SEMAPHORES_NAMES[SEND_NUMBER], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (sem == SEM_FAILED) error_exit("Cannot create semaphore SEND_NUMBER", "function create_semaphore (main)");
    sem_close(sem);
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
    int shared_memory = shm_open(SHARED_MEMORY, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shared_memory < 0) error_exit("Cannot create shared memory", "function create_shared_memory (main)");
    if (ftruncate(shared_memory, sizeof(orders)) < 0) error_exit("Cannot set memory size", "function create_shared_memory (main)");
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/times.h>

#define BARBER_TIME 5
#define WAITING_TIME 5
#define CLIENT_CREATION_TIME 3

int number_of_clients;
int number_of_chairs;
int clients_in_waiting_room = 0;
int shaved_clients = 0;
int is_barber_sleeping = 0;

pthread_t client_on_barber_chair;
pthread_t* chairs;
int next_free_chair = 0;
int next_client_chair = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void error_exit(char* message) {
    printf("ERROR: %s\n", message);
    exit(EXIT_FAILURE);
}

int is_everyone_shaved(){
    return shaved_clients == number_of_clients;
}

void barber_to_sleep(){
    printf("Golibroda: ide spac\n");
    is_barber_sleeping = 1;
    pthread_cond_wait(&cond, &mutex);
    is_barber_sleeping = 0;
}

void client_to_barber_chair(){
    clients_in_waiting_room--;
    client_on_barber_chair = chairs[next_client_chair];
    next_client_chair = (next_client_chair + 1) % number_of_chairs;
}

void shave_client(){
    printf("Golibroda: czeka %d klientow, gole klienta %ld\n", clients_in_waiting_room, client_on_barber_chair);

    // unblock all threads currently blocked on the specified condition variable cond
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    sleep(rand() % BARBER_TIME + 1);

    pthread_mutex_lock(&mutex);

    // send a cancellation request to the thread client_on_barber_chair
    pthread_cancel(client_on_barber_chair);
    
    shaved_clients++;
    client_on_barber_chair = 0;
}

void* barber(void* arg) {
    while(!is_everyone_shaved()) {
        pthread_mutex_lock(&mutex);

        if(!clients_in_waiting_room) {
            barber_to_sleep();
        } 
        else {
            client_to_barber_chair();
        }

        shave_client();
        
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit((void *) 0);
}

int is_free_place_in_waiting_room(){
    return clients_in_waiting_room < number_of_chairs;
}

void wake_up_barber(pthread_t client_id){
    printf("Klient: budze golibrode; %ld\n", client_id);
    client_on_barber_chair = client_id;
    // unblock barber thread
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

void client_to_waiting_room(pthread_t client_id){
    chairs[next_free_chair] = client_id;
    next_free_chair = (next_free_chair + 1) % number_of_chairs;
    clients_in_waiting_room++;
    printf("Klient: poczekalnia, wolne miejsca %d; %ld\n", number_of_chairs - clients_in_waiting_room, client_id);
    pthread_mutex_unlock(&mutex);
}

void wait_for_place_in_waiting_room(pthread_t client_id){
    printf("Klient: zajete; %ld\n", client_id);
    pthread_mutex_unlock(&mutex);
    sleep(rand() % WAITING_TIME + 1);
}

void* client(void* arg) {
    // get id of calling thread
    pthread_t client_id = pthread_self();

    while(1) {
        pthread_mutex_lock(&mutex);

        if(is_barber_sleeping) {
            wake_up_barber(client_id);
            break;
        } 
        else if(is_free_place_in_waiting_room()) {
            client_to_waiting_room(client_id);
            break;
        }
        wait_for_place_in_waiting_room(client_id);
        
    }

    pthread_exit((void *) 0);
}


int main(int argc, char** argv) {
    if(argc != 3) error_exit("Wrong number of arguments. Expected: ./main [number of chairs] [number of clients].");
    srand(time(NULL));

    number_of_chairs = atoi(argv[1]);
    number_of_clients = atoi(argv[2]);

    if (number_of_chairs < 1) error_exit("Number of chairs must be positive.");
    if (number_of_clients < 1) error_exit("Number of clients must be positive");

    // init mutex
    if(pthread_mutex_init(&mutex, NULL) != 0) error_exit("Cannot init mutex.");

    chairs = (pthread_t*) calloc(number_of_chairs, sizeof(pthread_t));

    // create barber thread
    pthread_t barber_thread;
    pthread_create(&barber_thread, NULL, barber, NULL);

    // create clients threads
    pthread_t* client_threads = (pthread_t*) calloc(number_of_clients, sizeof(pthread_t));
    for(int i = 0; i < number_of_clients; i++) {
        sleep(rand() % CLIENT_CREATION_TIME + 1);
        pthread_create(&client_threads[i], NULL, client, NULL);
    }

    // join threads
    for(int i = 0; i < number_of_clients; i++) {
        if(pthread_join(client_threads[i], NULL) > 0) {
            error_exit("Cannot join client's threads.");
        }
    }
    if (pthread_join(barber_thread, NULL) > 0){
        error_exit("Cannot join barber's thread.");
    }

    // destroy mutex and cond
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
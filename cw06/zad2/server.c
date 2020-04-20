#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "common.h"

char* clients_queues[MAX_CLIENTS];
int available_clients[MAX_CLIENTS];

mqd_t server_queue_id;

void quit(int signum){
    char *reply_msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));

    for (int i = 0; i < MAX_CLIENTS; i ++){
        if (clients_queues[i] != NULL){
            mqd_t client_queue_id = mq_open(clients_queues[i], O_RDWR, 0666, NULL);
            if (client_queue_id < 0){
                printf("ERROR: Cannot access client's queue in function quit\n");
                exit (EXIT_FAILURE);
            }
            if (mq_send(client_queue_id, reply_msg, MAX_MESSAGE_LEN, STOP) < 0){
                printf("ERROR: Cannot send message in function quit\n");
                exit(EXIT_FAILURE);
            }
            if (mq_receive(server_queue_id, reply_msg, MAX_MESSAGE_LEN, NULL) < 0){
                printf("ERROR: Cannot receive message in function quit\n");
                exit(EXIT_FAILURE);
            }
            if (mq_close(client_queue_id) < 0){
                printf("ERROR: Cannot close queue in function quit\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (mq_close(server_queue_id) < 0){
        printf("ERROR: Cannot close queue in function quit\n");
        exit(EXIT_FAILURE);
    }
    if (mq_unlink(SERVER_QUEUE_NAME) < 0){
        printf("ERROR: Cannot delete server's queue in function quit\n");
        exit(EXIT_FAILURE);
    }
    exit(0);
}


int next_client_id(){
    int i = 0;
    while (i < MAX_CLIENTS && clients_queues[i] != NULL) i ++;
    if (i < MAX_CLIENTS) return (i + 1);
    else return -1;
}

void init_handler(char *msg){
    printf("INIT\n");

    int new_id = next_client_id();
    if (new_id < 0) return;

    char *reply_msg = (char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));

    mqd_t client_queue_id = mq_open(msg, O_RDWR, 0666, NULL);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client's queue in function init_handler\n");
        exit(EXIT_FAILURE);
    }

    available_clients[new_id - 1] = 1;
    clients_queues[new_id - 1] = (char*)calloc(NAME_LEN, sizeof(char));
    strcpy(clients_queues[new_id - 1], msg);

    if (mq_send(client_queue_id, reply_msg, MAX_MESSAGE_LEN, new_id) < 0 ){
        printf("ERROR: Cannot send message in function init_handler\n");
        exit(EXIT_FAILURE);
    }
    if (mq_close(client_queue_id) < 0 ){
        printf("ERROR: Cannot close client's queue in function init_handler\n");
        exit(EXIT_FAILURE);
    }
}

void list_handler(char *msg){
    printf("LIST\n");

    char *reply_msg = (char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));
    int client_id = (int) msg[0];

    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queues[i] != NULL){
            sprintf(reply_msg + strlen(reply_msg), "ID: %d, client is available: %d\n", i + 1, available_clients[i] == 1);
        }
    }

    mqd_t client_queue_id = mq_open(clients_queues[client_id - 1], O_RDWR, 0666, NULL);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client's queue in function list_handler\n");
        exit(EXIT_FAILURE);
    }

    if (mq_send(client_queue_id, reply_msg, MAX_MESSAGE_LEN, LIST) < 0){
        printf("ERROR: Cannot send message in function list_handler\n");
        exit(EXIT_FAILURE);
    }
    if (mq_close(client_queue_id) < 0 ){
        printf("ERROR: Cannot close client's queue in function list_handler\n");
        exit(EXIT_FAILURE);
    }
}

void connect_handler(char *msg){
    printf("CONNECT\n");

    char *reply_msg = (char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));
    int client_id = (int) msg[0];
    int second_client_id = (int) msg[1];

    mqd_t client_queue_id = mq_open(clients_queues[client_id - 1], O_RDWR, 0666, NULL);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client's queue in function connect_handler\n");
        exit (EXIT_FAILURE);
    }

    reply_msg[0] = second_client_id;
    strcat(reply_msg, clients_queues[second_client_id - 1]);
    if (mq_send(client_queue_id, reply_msg, MAX_MESSAGE_LEN, CONNECT) < 0){
        printf("ERROR: Cannot send message in function connect_handler\n");
        exit (EXIT_FAILURE);
    }

    memset(reply_msg, 0, strlen(reply_msg));
    client_queue_id = mq_open(clients_queues[second_client_id - 1], O_RDWR, 0666, NULL);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client's queue in function connect_handler\n");
        exit (EXIT_FAILURE);
    }
    reply_msg[0] = client_id;
    strcat(reply_msg, clients_queues[client_id - 1]);
    if (mq_send(client_queue_id, reply_msg, MAX_MESSAGE_LEN, CONNECT) < 0){
        printf("ERROR: Cannot send message in function connect_handler\n");
        exit (EXIT_FAILURE);
    }
    if (mq_close(client_queue_id) < 0 ){
        printf("ERROR: Cannot close client's queue in function connect_handler\n");
        exit(EXIT_FAILURE);
    }

    available_clients[second_client_id - 1] = 0;
    available_clients[client_id - 1] = 0;
}

void disconnect_handler(char *msg){
    printf("DISCONNECT\n");

    char *reply_msg = (char*)calloc(MAX_MESSAGE_LEN ,sizeof(char));
    int client_id = (int) msg[0];
    int second_client_id = (int) msg[1];

    mqd_t client_queue_id = mq_open(clients_queues[second_client_id - 1], O_RDWR, 0666, NULL);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client's queue in function disconnect_handler\n");
        exit (EXIT_FAILURE);
    }
    if (mq_send(client_queue_id, reply_msg, MAX_MESSAGE_LEN, DISCONNECT) < 0){
        printf("ERROR: Cannot send message in function disconnect_handler\n");
        exit (EXIT_FAILURE);
    }
    if (mq_close(client_queue_id) < 0 ){
        printf("ERROR: Cannot close client's queue in function disconnect_handler\n");
        exit(EXIT_FAILURE);
    }

    available_clients[second_client_id - 1] = 1;
    available_clients[client_id - 1] = 1;
}

void stop_handler(char *msg){
    printf("Stop\n");
    available_clients[(int) msg[0] - 1] = 0;
    clients_queues[(int) msg[0] - 1] = NULL;
}

void process_message(char *msg, int type){
    switch(type){
        case INIT:
            init_handler(msg);
            break;
        case LIST:
            list_handler(msg);
            break;
        case CONNECT:
            connect_handler(msg);
            break;
        case DISCONNECT:
            disconnect_handler(msg);
            break;
        case STOP:
            stop_handler(msg);
            break;
        default:
            printf("ERROR: There is no type of message like %d\n", type);
    }
}

int main(){
    for (int i = 0; i < MAX_CLIENTS; i++){
        clients_queues[i] = NULL;
    }

    server_queue_id = mq_open(SERVER_QUEUE_NAME, O_RDWR | O_CREAT, 0666, NULL);
    if (server_queue_id < 0){
        printf("ERROR: Cannot create queue in main\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, quit);

    char*msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    unsigned int type;

    while(1){
        if (mq_receive(server_queue_id, msg, MAX_MESSAGE_LEN, &type) < 0){
            printf("ERROR: Cannot receive message\n");
            exit(EXIT_FAILURE);
        }
        process_message(msg, type);
    }
    return 0;
}
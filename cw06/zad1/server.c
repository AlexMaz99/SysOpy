#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "common.h"

key_t clients_queues[MAX_CLIENTS];
int available_clients[MAX_CLIENTS];

int server_queue;

void quit(int signum){
    message *reply_msg = (message*)malloc(sizeof(message));

    for (int i = 0; i < MAX_CLIENTS; i ++){
        key_t queue_key = clients_queues[i];
        if (queue_key != -1){
            reply_msg -> message_type = STOP;
            int client_queue_id = msgget(queue_key, 0);
            if (client_queue_id < 0){
                printf("ERROR: Cannot access client's queue in function quit\n");
                exit (EXIT_FAILURE);
            }
            if (msgsnd(client_queue_id, reply_msg, MESSAGE_SIZE, 0) < 0){
                printf("ERROR: Cannot send message in function quit\n");
                exit(EXIT_FAILURE);
            }
            if (msgrcv(server_queue, reply_msg, MESSAGE_SIZE, STOP, 0) < 0){
                printf("ERROR: Cannot receive message in function quit\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    msgctl(server_queue, IPC_RMID, NULL);
    exit(0);
}

int next_client_id(){
    int i = 0;
    while (i < MAX_CLIENTS && clients_queues[i] != -1) i ++;
    if (i < MAX_CLIENTS) return (i + 1);
    else return -1;
}

void init_handler(message *msg){
    printf("INIT\n");

    int new_id = next_client_id();
    if (new_id < 0) return;

    message *reply_msg = (message*)malloc(sizeof(message));
    reply_msg -> message_type = new_id;
    int client_queue_id = msgget(msg -> queue_key, 0);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client queue in function init_handler\n");
        exit(EXIT_FAILURE);
    }

    available_clients[new_id - 1] = 1;
    clients_queues[new_id - 1] = msg -> queue_key;

    if (msgsnd(client_queue_id, reply_msg, MESSAGE_SIZE, 0) < 0 ){
        printf("ERROR: Cannot send message in function init_handler\n");
        exit(EXIT_FAILURE);
    }
}

void list_handler(message *msg){
    printf("LIST\n");

    message *reply_msg = (message*)malloc(sizeof(message));
    strcpy(reply_msg -> text, "");

    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients_queues[i] != -1){
            sprintf(reply_msg -> text + strlen(reply_msg -> text), "ID: %d, client is available: %d\n", i + 1, available_clients[i] == 1);
        }
    }

    int client_queue_id = msgget(clients_queues[msg -> client_id - 1], 0);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client's queue in function list_handler\n");
        exit(EXIT_FAILURE);
    }

    reply_msg -> message_type = msg -> client_id;
    if (msgsnd(client_queue_id, reply_msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function list_handler\n");
        exit(EXIT_FAILURE);
    }
}

void connect_handler(message *msg){
    printf("CONNECT\n");

    message *reply_msg = (message*)malloc(sizeof(message));

    reply_msg -> message_type = CONNECT;
    reply_msg -> queue_key = clients_queues[msg -> connected_client_id - 1];
    int client_queue_id = msgget(clients_queues[msg -> client_id - 1], 0);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client's queue in function connect_handler\n");
        exit (EXIT_FAILURE);
    }
    if (msgsnd(client_queue_id, reply_msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function connect_handler\n");
        exit (EXIT_FAILURE);
    }

    reply_msg -> message_type = CONNECT;
    reply_msg -> queue_key = clients_queues[msg -> client_id - 1];
    reply_msg -> client_id = msg -> client_id;
    client_queue_id = msgget(clients_queues[msg -> connected_client_id - 1], 0);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client queue in function connect_handler\n");
        exit (EXIT_FAILURE);
    }
    if (msgsnd(client_queue_id, reply_msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function connect_handler\n");
        exit (EXIT_FAILURE);
    }

    available_clients[msg -> connected_client_id - 1] = 0;
    available_clients[msg -> client_id - 1] = 0;
}

void disconnect_handler(message *msg){
    printf("DISCONNECT\n");

    message *reply_msg = (message*)malloc(sizeof(message));
    reply_msg -> message_type = DISCONNECT;

    int client_queue_id = msgget(clients_queues[msg -> connected_client_id - 1], 0);
    if (client_queue_id < 0){
        printf("ERROR: Cannot access client queue in function disconnect_handler\n");
        exit(EXIT_FAILURE);
    }
    if (msgsnd(client_queue_id, reply_msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function disconnect_handler\n");
        exit(EXIT_FAILURE);
    }

    available_clients[msg -> connected_client_id - 1] = 1;
    available_clients[msg -> client_id - 1] = 1;
}

void stop_handler(message *msg){
    printf("Stop\n");
    available_clients[msg -> client_id - 1] = 0;
    clients_queues[msg -> client_id - 1] = -1;
}

void process_message(message *msg){
    switch(msg -> message_type){
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
            printf("ERROR: There is no type of message like %ld\n", msg -> message_type);
    }
}

int main(){
    for (int i = 0; i < MAX_CLIENTS; i++){
        clients_queues[i] = -1;
    }

    key_t queue_key = ftok(getenv("HOME"), SERVER_KEY_ID);
    printf("Server queue key: %d\n", queue_key);

    server_queue = msgget(queue_key, IPC_CREAT | 0666);
    printf("Server queue ID: %d\n", server_queue);

    signal(SIGINT, quit);

    message *msg = (message*)malloc(sizeof(message));

    while(1){
        if (msgrcv(server_queue, msg, MESSAGE_SIZE, -6, 0) < 0){
            printf("ERROR: Cannot received message\n");
            exit(EXIT_FAILURE);
        }
        process_message(msg);
    }
    return 0;
}
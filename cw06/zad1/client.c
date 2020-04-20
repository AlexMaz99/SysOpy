#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>
#include "common.h"

int client_id;
int queue_id;
int server_queue_id;
key_t queue_key;


void stop(){
    message *msg = (message*)malloc(sizeof(message));
    msg -> client_id = client_id;
    msg -> message_type = STOP;

    if (msgsnd(server_queue_id, msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function stop\n");
        exit(EXIT_FAILURE);
    }
    if (msgctl(queue_id, IPC_RMID, NULL) < 0){
        printf("ERROR: Cannot delete queue in function stop\n");
        exit(EXIT_FAILURE);
    }
    msgctl(queue_id, IPC_RMID, NULL);
    exit(0);
}


void chat(int second_client_id, int second_queue_id){
    message *msg = (message*)malloc(sizeof(message));
    char* cmd = NULL;
    size_t len = 0;
    ssize_t read = 0;

    while (1){
        printf("Enter message or disconnect: ");
        read = getline(&cmd, &len, stdin);
        cmd[read - 1] = '\0';

        if (msgrcv(queue_id, msg, MESSAGE_SIZE, STOP, IPC_NOWAIT) >= 0){
            printf("Received STOP from server. Good bye!\n");
            stop();
        }

        if (msgrcv(queue_id, msg, MESSAGE_SIZE, DISCONNECT, IPC_NOWAIT) >= 0){
            printf("End of chat. Disconnecting ...\n");
            break;
        }

        while(msgrcv(queue_id, msg, MESSAGE_SIZE, 0, IPC_NOWAIT) >= 0){
            printf("[Client %d]: %s\n", second_client_id, msg -> text);
        }

        if (strcmp(cmd, "DISCONNECT") == 0){
            msg -> client_id = client_id;
            msg -> connected_client_id = second_client_id;
            msg -> message_type = DISCONNECT;
            if (msgsnd(server_queue_id, msg, MESSAGE_SIZE, 0) < 0){
                printf("ERROR: Cannot send message in function chat\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
        else if(strcmp(cmd, "") != 0){
            msg -> message_type = CONNECT;
            strcpy(msg -> text, cmd);
            if (msgsnd(second_queue_id, msg, MESSAGE_SIZE, 0) < 0){
                printf("ERROR: Cannot send message in function chat\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void connect(int second_client_id){
    message *msg = (message*)malloc(sizeof(message));
    msg -> message_type = CONNECT;
    msg -> client_id = client_id;
    msg -> connected_client_id = second_client_id;

    if (msgsnd(server_queue_id, msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function connect\n");
        exit(EXIT_FAILURE);
    }

    message *reply_msg = (message*)malloc(sizeof(message));
    if (msgrcv(queue_id, reply_msg, MESSAGE_SIZE, 0, 0) < 0){
        printf("ERROR: Cannot receive message in function connect\n");
        exit(EXIT_FAILURE);
    }
    key_t second_queue_key = reply_msg -> queue_key;
    int second_queue_id = msgget(second_queue_key, 0);
    if (second_queue_id < 0){
        printf("ERROR: Cannot access client's queue key in function connect\n");
        exit(EXIT_FAILURE);
    }

    chat(second_client_id, second_queue_id);
}

void list(){
    message *msg = (message*)malloc(sizeof(message));
    msg -> client_id = client_id;
    msg -> message_type = LIST;
    if (msgsnd(server_queue_id, msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function list\n");
        exit(EXIT_FAILURE);
    }

    message *reply_msg = (message*)malloc(sizeof(message));
    if (msgrcv(queue_id, reply_msg, MESSAGE_SIZE, 0, 0) < 0){
        printf("ERROR: Cannot receive message in function list\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", reply_msg -> text);
}

void check_message_from_server(){
    message *msg = (message*)malloc(sizeof(message));

    if (msgrcv(queue_id, msg, MESSAGE_SIZE, 0, IPC_NOWAIT) < 0) return;

    if (msg -> message_type == CONNECT){
        printf("Connecting to client: %d\n", msg -> client_id);
        int second_queue_id = msgget(msg -> queue_key, 0);
        if (second_queue_id < 1){
            printf("ERROR: Cannot access second client's queue in function check_message\n");
            exit(EXIT_FAILURE);
        }
        chat(msg -> client_id, second_queue_id);
    }
    else if (msg -> message_type == STOP){
        printf("Received STOP signal from server. Good bye!\n");
        stop();
    }
}


void init(){
    srand(time(NULL));

    queue_key = ftok(getenv("HOME"), rand() % 255 + 1);
    printf("Queue key: %d\n", queue_key);

    queue_id = msgget(queue_key, IPC_CREAT | 0666);
    if (queue_id < 0){
        printf("ERROR: Cannot create queue\n");
        exit (EXIT_FAILURE);
    }
    printf("Queue ID: %d\n", queue_id);

    key_t server_key = ftok(getenv("HOME"), SERVER_KEY_ID);
    server_queue_id = msgget(server_key, 0);
    if (server_queue_id < 0){
        printf("ERROR: Cannot access server queue\n");
        exit(EXIT_FAILURE);
    }
    printf("Server queue ID: %d\n", server_queue_id);

    message *msg = (message*)malloc(sizeof(message));
    msg -> queue_key = queue_key;
    msg -> message_type = INIT;
    if (msgsnd(server_queue_id, msg, MESSAGE_SIZE, 0) < 0){
        printf("ERROR: Cannot send message in function init\n");
        exit(EXIT_FAILURE);
    }

    message *reply_msg = (message*)malloc(sizeof(message));
    if (msgrcv(queue_id, reply_msg, MESSAGE_SIZE, 0, 0) < 0){
        printf("ERROR: Cannot receive message in function init\n");
        exit(EXIT_FAILURE);
    }
    client_id = reply_msg -> message_type;
    printf("Client ID: %d\n", client_id);
}

void quit(int signum){
    stop();
}

int main(){
    init();
    signal(SIGINT, quit);

    char* cmd = NULL;
    size_t len = 0;
    ssize_t read = 0;
    

    while (1){
        printf("Enter your command: ");
        read = getline(&cmd, &len, stdin);
        cmd[read - 1] = '\0';

        check_message_from_server();

        if (strcmp(cmd, "") == 0){
            continue;
        }

        char* buffer = strtok(cmd, " ");
        if (!strcmp(buffer, "STOP")){
            printf("STOP\n");
            stop();
        }
        else if (!strcmp(buffer, "LIST")){
            printf("LIST\n");
            list();
        }
        else if (!strcmp(buffer, "CONNECT")){
            printf("CONNECT\n");
            buffer = strtok(NULL, " ");
            int id = atoi(buffer);
            connect(id);
        }
        else{
            printf("There is no command like %s\n", cmd);
        }
    }
    return 0;
}
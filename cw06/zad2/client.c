#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <mqueue.h>
#include "common.h"

char *queue_name;
int client_id;
mqd_t queue_id;
mqd_t server_queue_id;

void stop(){
    char *msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    msg[0] = client_id;

    if (mq_send(server_queue_id, msg, MAX_MESSAGE_LEN, STOP) < 0){
        printf("ERROR: Cannot send message in function stop\n");
        exit(EXIT_FAILURE);
    }
    if (mq_close(server_queue_id) < 0){
        printf("ERROR: Cannot delete queue in function stop\n");
        exit(EXIT_FAILURE);
    }
    exit(0);
}


void chat(int second_client_id, mqd_t second_queue){
    char *msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    char* cmd = NULL;
    size_t len = 0;
    ssize_t read = 0;

    while (1){
        printf("Enter message or disconnect: ");
        read = getline(&cmd, &len, stdin);
        cmd[read - 1] = '\0';

        struct timespec* timespec = (struct timespec*)malloc(sizeof(struct timespec));
        unsigned int type;
        int is_disconnect = 0;

        while(mq_timedreceive(queue_id, msg, MAX_MESSAGE_LEN, &type, timespec) >= 0){
            if (type == STOP){
                printf("Received STOP from server. Good bye!\n");
                stop();
            }
            else if(type == DISCONNECT){
                printf("End of chat. Disconnecting ...\n");
                is_disconnect = 1;
                break;
            }
            else {
                printf("[Client %d]: %s\n", second_client_id, msg);
            }
        }
        if (is_disconnect == 1) break;

        if (strcmp(cmd, "DISCONNECT") == 0){
            msg[0] = client_id;
            msg[1] = second_client_id;
            if (mq_send(server_queue_id, msg, MAX_MESSAGE_LEN, DISCONNECT) < 0){
                printf("ERROR: Cannot send message in function chat\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
        else if(strcmp(cmd, "") != 0){
            strcpy(msg, cmd);
            if (mq_send(second_queue, msg, MAX_MESSAGE_LEN, CONNECT) < 0){
                printf("ERROR: Cannot send message in function chat\n");
                exit(EXIT_FAILURE);
            }
        }
    }

}

void connect(int second_client_id){
    char *msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    msg[0] = client_id;
    msg[1] = second_client_id;

    if (mq_send(server_queue_id, msg, MAX_MESSAGE_LEN, CONNECT) < 0){
        printf("ERROR: Cannot send message in function connect\n");
        exit(EXIT_FAILURE);
    }
    if (mq_receive(queue_id, msg, MAX_MESSAGE_LEN, NULL) < 0){
        printf("ERROR: Cannot receive message in function connect\n");
        exit(EXIT_FAILURE);
    }

    char *second_queue_name = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    strncpy(second_queue_name, msg + 1, strlen(msg) - 1);
    printf("Other name: %s\n", second_queue_name);

    mqd_t second_queue = mq_open(second_queue_name, O_RDWR, 0666, NULL);
    if (second_queue < 0){
        printf("ERROR: Cannot access client's queue key in function connect\n");
        exit(EXIT_FAILURE);
    }

    chat(second_client_id, second_queue);
}

void list(){
    char *msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    msg[0] = client_id;
    if (mq_send(server_queue_id, msg, MAX_MESSAGE_LEN, LIST) < 0){
        printf("ERROR: Cannot send message in function list\n");
        exit(EXIT_FAILURE);
    }

    if (mq_receive(queue_id, msg, MAX_MESSAGE_LEN, NULL) < 0){
        printf("ERROR: Cannot receive message in function list\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", msg);
}

void check_message_from_server(){
    char *msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    struct timespec* timespec = (struct timespec*)malloc(sizeof(struct timespec));
    unsigned int type;
    if (mq_timedreceive(queue_id, msg, MAX_MESSAGE_LEN, &type, timespec) < 0) return;

    if (type == CONNECT){
        printf("Connecting to client\n");
        char* second_queue_name = (char*)calloc(NAME_LEN, sizeof(char));
        strncpy(second_queue_name, msg + 1, strlen(msg) - 1);
        printf("Other name: %s\n", second_queue_name);
        mqd_t second_queue_id = mq_open(second_queue_name, O_RDWR, 0666, NULL);
        if (second_queue_id < 0){
            printf("ERROR: Cannot access second client's queue in function check_message\n");
            exit(EXIT_FAILURE);
        }
        chat((int) msg[0], second_queue_id);
    }
    else if (type == STOP){
        printf("Received STOP signal from server. Good bye!\n");
        stop();
    }
}

void init(){
    server_queue_id = mq_open(SERVER_QUEUE_NAME, O_RDWR, 0666, NULL);
    if (server_queue_id < 0){
        printf("ERROR: Cannot access server queue\n");
        exit(EXIT_FAILURE);
    }

    char* msg = (char*)calloc(MAX_MESSAGE_LEN, sizeof(char));
    strcpy(msg, queue_name);
    if (mq_send(server_queue_id, msg, MAX_MESSAGE_LEN, INIT) < 0){
        printf("ERROR: Cannot receive message in function init\n");
        exit(EXIT_FAILURE);
    }

    unsigned int c_id;
    if (mq_receive(queue_id, msg, MAX_MESSAGE_LEN, &c_id) < 0){
        printf("ERROR: Cannot receive message in function init\n");
        exit(EXIT_FAILURE);
    }
    printf("Received client ID: %d\n", c_id);
    client_id = c_id;
}

void quit(int signum){
    stop();
}

int main(int argc, char**argv){
    queue_name = argv[1];
    queue_id = mq_open(queue_name, O_RDWR | O_CREAT, 0666, NULL);
    if (queue_id < 0){
        printf("ERROR: Cannot create queue\n");
        exit(EXIT_FAILURE);
    }
    init();
    signal(SIGINT, quit);
    

    while (1){
        printf("Enter your command: ");
        char* cmd = NULL;
        size_t len = 0;
        ssize_t read = 0;
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
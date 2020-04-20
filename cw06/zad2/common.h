#ifndef COMMON_H
#define COMMON_H

#define MAX_CLIENTS 5
#define MAX_MESSAGE_LEN 8192
#define NAME_LEN 20

typedef enum message_type{
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
}message_type;

const char* SERVER_QUEUE_NAME = "/MYSERVER";

#endif //COMMON_H
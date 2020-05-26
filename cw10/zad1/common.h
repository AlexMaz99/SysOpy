#ifndef SYSOPY_COMMON_H
#define SYSOPY_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#define MAX_CLIENTS 13
#define MSG_SIZE 20
#define NAME_LEN 8

#define PING_INTERVAL 10
#define PING_WAIT 5

enum connection_type{
	LOCAL,
	INET
}; typedef enum connection_type connection_type;

enum message_type{
	CONNECT,
	CONNECT_FAILED,
	PING,
	WAIT,
	GAME_FOUND,
	MOVE,
	GAME_FINISHED,
	DISCONNECT,
	EMPTY
}; typedef enum message_type message_type;

struct game{
	char board[9];
	char turn;
	char winner; // -/O/X/D
}; typedef struct game game;

struct message{
	message_type message_type;
	game game;
	char name[NAME_LEN];
}; typedef struct message message;

struct client{
	int fd;
	char* name;	
	game* game;
	int active;
	int opponent_idx;
	char symbol; // O/X
}; typedef struct client client;

void error_exit(char* message){
	printf("ERROR: %s\n", message);
	exit(EXIT_FAILURE);
}

void new_board(game* game){
	for(int i = 0; i < 9; i++) game->board[i] = '-';
	game->turn = 'O';
	game->winner = '-';
}

void send_message(int fd, message_type type, game* game, char* name){
	char* message = calloc(MSG_SIZE, sizeof(char));
	if(type == CONNECT) sprintf(message, "%d %s", (int) type, name);
	else if(game == NULL) sprintf(message, "%d", (int) type);
	else sprintf(message, "%d %s %c %c", (int) type, game->board, game->turn, game->winner);

	if(write(fd, message, MSG_SIZE) < 0) error_exit("Cannot send message.");
	free(message);
}

message receive_message(int fd, int nonblocking){

	message msg;
	int count; // number of bytes received
	char* buffer = calloc(MSG_SIZE, sizeof(char));

	if (nonblocking && (count = recv(fd, buffer, MSG_SIZE, MSG_DONTWAIT)) < 0){ // revceive message from socket
		msg.message_type = EMPTY;
		free(buffer);
		return msg;
	}
	else if(!nonblocking && (count = read(fd, buffer, MSG_SIZE)) < 0) error_exit("Cannot receive message.");
	if(count == 0){
		msg.message_type = DISCONNECT;
		free(buffer);
		return msg;
	}

	char* token;
	char* rest = buffer;
	strcpy(msg.name, "");
	new_board(&msg.game);
	token = strtok_r(rest, " ", &rest);

	if (nonblocking) {
		char* p;
		msg.message_type = (message_type) strtol(token, &p, 10);
	}
	else msg.message_type = (message_type) atoi(token);

	if (msg.message_type == CONNECT){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
	}
	else if (!nonblocking && (msg.message_type == PING || msg.message_type == DISCONNECT || msg.message_type == WAIT)){
		free(buffer);
		return msg;
	}
	else if (msg.message_type == MOVE || msg.message_type == GAME_FINISHED || msg.message_type == GAME_FOUND){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.game.board, token);
		token = strtok_r(rest, " ", &rest);
		msg.game.turn = token[0];
		token = strtok_r(rest, " ", &rest);
		msg.game.winner = token[0];
	}

	free(buffer);
	return msg;
}

#endif //SYSOPY_COMMON_H
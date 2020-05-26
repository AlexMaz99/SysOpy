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
#define MSG_SIZE 30
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
	char winner;	
}; typedef struct game game;

struct message{
	message_type message_type;
	game game;				//null for non game messages
	char name[NAME_LEN];
}; typedef struct message message;

struct client{
	int fd;	
	struct sockaddr* addr;
	char* name;	
	int active;
	int opponent_idx;
	game* game;
	char symbol;
}; typedef struct client client;

void new_board(game* game){
	for(int i = 0; i < 9; i++) game->board[i] = '-';
	game->turn = 'O';
	game->winner = '-';
}


void error_exit(char* message){
	printf("ERROR: %s\n", message);
	exit(EXIT_FAILURE);
}

void send_message(int fd, message_type type, game* game, char* nick){
	char* message = calloc(MSG_SIZE, sizeof(char));
	if(game == NULL) sprintf(message, "%d %s", (int) type, nick);
	else sprintf(message, "%d %s %c %c %s", (int) type, game->board, game->turn, game->winner, nick);
	if(write(fd, message, MSG_SIZE) < 0) error_exit("Could not send message.");
	free(message);
}

void send_message_to(int fd, struct sockaddr* addr, message_type type, game* game, char* nick){
	char* message = calloc(MSG_SIZE, sizeof(char));
	if(game == NULL)sprintf(message, "%d %s", (int) type, nick);
	else sprintf(message, "%d %s %c %c %s", (int) type, game->board, game->turn, game->winner, nick);
	if(sendto(fd, message, MSG_SIZE, 0, addr, sizeof(struct sockaddr)) < 0) error_exit("Could not send message.");
	free(message);
}

message receive_message(int fd){

	message msg;
	int count;
	char* msg_buf = calloc(MSG_SIZE, sizeof(char));

	if((count = read(fd, msg_buf, MSG_SIZE)) < 0) error_exit("Could not receive message.");
	if(count == 0){
		msg.message_type = DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char* token;
	char* rest = msg_buf;
	strcpy(msg.name, "");
	new_board(&msg.game);
	token = strtok_r(rest, " ", &rest);
	msg.message_type = (message_type) atoi(token);

	if (msg.message_type == CONNECT){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
	}
	else if (msg.message_type == DISCONNECT || msg.message_type == PING || msg.message_type == WAIT){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
		free(msg_buf);
		return msg;
	}
	else if (msg.message_type == MOVE || msg.message_type == GAME_FINISHED || msg.message_type == GAME_FOUND){
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.game.board, token);
		token = strtok_r(rest, " ", &rest);
		msg.game.turn = token[0];
		token = strtok_r(rest, " ", &rest);
		msg.game.winner = token[0];
		token = strtok_r(rest, " ", &rest);
		strcpy(msg.name, token);
	}

	free(msg_buf);

	return msg;

}

message receive_message_from(int fd, struct sockaddr* addr, socklen_t len){

	message msg;
	int count;
	char* msg_buf = calloc(MSG_SIZE, sizeof(char));

	if((count = recvfrom(fd, msg_buf, MSG_SIZE, 0, addr, &len)) < 0) // receive message from socket
		error_exit("Could not receive message.");

	if(count == 0){
		msg.message_type=DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char* token;
	char* rest = msg_buf;
	strcpy(msg.name, "");
	new_board(&msg.game);
	token = strtok_r(rest, " ", &rest);
	msg.message_type = (message_type) atoi(token);

	switch(msg.message_type){
		case CONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		case PING: case WAIT: case DISCONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			free(msg_buf);
			return msg;
		case MOVE: case GAME_FOUND: case GAME_FINISHED:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.game.board, token);
			token = strtok_r(rest, " ", &rest);
			msg.game.turn = token[0];
			token = strtok_r(rest, " ", &rest);
			msg.game.winner = token[0];
			break;
		default:
			break;
	}

	free(msg_buf);

	return msg;

}

message receive_message_nonblock(int fd){

	message msg;
	char* msg_buf = calloc(MSG_SIZE, sizeof(char));
	int count;

	if((count = recv(fd, msg_buf, MSG_SIZE, MSG_DONTWAIT)) < 0){
		msg.message_type = EMPTY;
		free(msg_buf);
		return msg;
	}
	
	if(count == 0){
		msg.message_type = DISCONNECT;
		free(msg_buf);
		return msg;
	}

	char *token;
	char *rest = msg_buf;

	char* p;

	strcpy(msg.name, "");
	new_board(&msg.game);
	token = strtok_r(rest, " ", &rest);

	msg.message_type = (message_type) strtol(token, &p, 10);

	switch (msg.message_type) {
		case CONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			break;
		case PING: case WAIT: case DISCONNECT:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.name, token);
			free(msg_buf);
			return msg;
		case MOVE: case GAME_FOUND: case GAME_FINISHED:
			token = strtok_r(rest, " ", &rest);
			strcpy(msg.game.board, token);
			token = strtok_r(rest, " ", &rest);
			msg.game.turn = token[0];
			token = strtok_r(rest, " ", &rest);
			msg.game.winner = token[0];
			break;
		default:
			break;
	}

	free(msg_buf);

	return msg;

}

#endif //SYSOPY_COMMON_H
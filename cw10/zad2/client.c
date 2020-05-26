#include "common.h"

connection_type connection;
int port_number;
char* server;
char* name;
int server_fd;
char sign;
int move;

void connect_to_local_server(){
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, server);

	server_fd = socket(AF_UNIX, SOCK_DGRAM, 0); // SOCK_DRGAM - supports datagrams (conntectionless, unreliable messages of a fixed max length)
	if(server_fd < 0) error_exit("Socket to server failed.");

	struct sockaddr_un c_addr;
	c_addr.sun_family = AF_UNIX;
	strcpy(c_addr.sun_path, name);

	if(bind(server_fd, (struct sockaddr*) &c_addr, sizeof(c_addr)) < 0) error_exit("Cannot bind");
	if(connect(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) error_exit("Cannot connect to server");
}

void connect_to_inet_server(){
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);
	addr.sin_addr.s_addr = inet_addr(server);

	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(server_fd < 0) error_exit("Canot create socket");

	struct sockaddr_in c_addr;
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = 0;
	c_addr.sin_addr.s_addr = inet_addr(server);

	if(bind(server_fd, (struct sockaddr*) &c_addr, sizeof(c_addr)) < 0) error_exit("Cannot bind");
	if(connect(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) error_exit("Cannot connect to server");
}

void disconnect_from_server(){
	printf("Disconnecting from server...\n");
	send_message(server_fd, DISCONNECT, NULL, name);
	if(connection == LOCAL) unlink(name);
}

void sigint_handler_client(int signo){
	exit(EXIT_SUCCESS);
}

void exit_fun(){
	disconnect_from_server();
}

void print_gameboard(game* game){
	printf("BOARD\n");
	printf("Your sign: %c\n\n", sign);

	for(int i = 0; i < 9; i++){
		if (game -> board[i] == '-') printf("%d", i);
		else printf("%c",game->board[i]);

		if(i % 3 == 2) printf("\n");
		else printf (" ");
	}

	printf("\n");
}

void enemy_move(void* arg){
	message* msg = (message*) arg;
	printf("Enter your move: ");

	int move_char = getchar();
	move = move_char - '0';

	while(move < 0 || move > 8 || msg->game.board[move] != '-'){
		move_char = getchar();
		move = move_char - '0';
	}
	pthread_exit(0);
}

void make_move(message *msg){
	move = -1;
	pthread_t move_thread;
	pthread_create(&move_thread, NULL, (void*) enemy_move, msg);

	while(1){
		if(move < 0 || move > 8 || msg->game.board[move] != '-'){
			message rec_msg = receive_message_nonblock(server_fd);

			if (rec_msg.message_type == DISCONNECT){
				sigint_handler_client(SIGINT);
				exit(EXIT_SUCCESS);
			}

			else if (rec_msg.message_type == PING){
				send_message(server_fd, PING, NULL, name);
			}
		}

		else{
			break;
		}
	}

	pthread_join(move_thread, NULL);
	msg->game.board[move] = sign;
	print_gameboard(&msg->game);
	send_message(server_fd, MOVE, &msg->game, name);
}

void client_exec(){
	while(1){

		message msg = receive_message(server_fd);
		
		if (msg.message_type == WAIT) printf("Waiting for an enemy\n");
		else if (msg.message_type == PING) send_message(server_fd, PING, NULL, name);
		else if (msg.message_type == DISCONNECT) exit(EXIT_SUCCESS);
		else if (msg.message_type == GAME_FOUND){
			sign = msg.game.winner;
			printf("Game started. Your symbol: %c\n", sign);
			print_gameboard(&msg.game);
			if(sign == 'O') make_move(&msg);
			else printf("Waiting for enemy move\n");
		}
		else if (msg.message_type == MOVE){
			print_gameboard(&msg.game);
			make_move(&msg);
		}
		else if (msg.message_type == GAME_FINISHED){
			print_gameboard(&msg.game);
			if(msg.game.winner == sign) printf("You won! Congratulations!\n");
			else if(msg.game.winner == 'D') printf("It's draw\n");
			else printf("You lost! Maybe next time will be better :)\n");
			exit(EXIT_SUCCESS);
		}
		else break;
	}
}

int main(int argc, char** argv){

	if(argc < 4) error_exit("Wrong number of arguments. Expected: [client name] [LOCAL / INET] [address of server]");

	name = argv[1];
	atexit(exit_fun);
	signal(SIGINT, sigint_handler_client);

	if(strcmp(argv[2], "LOCAL") == 0) connection = LOCAL;
	else if(strcmp(argv[2], "INET") == 0) connection = INET;
	else  error_exit("Wrong type of connections. Expected: LOCAL / INET");

	if(connection == LOCAL){
		server = argv[3];
		connect_to_local_server();
	}
	else{
		if(argc < 5) error_exit("Wrong arguments. Expected: [client name] [INET] [adress IP] [port number]");
		server = argv[3];
		port_number = atoi(argv[4]);
		printf("Server IP: %s port: %d\n",server, port_number);
		connect_to_inet_server();
	}

	printf("Welcome in game %s!\n", name);
	send_message(server_fd, CONNECT, NULL, name);
	message msg = receive_message(server_fd);

	if(msg.message_type == CONNECT){
		printf("Client is connected to server\n");
		client_exec();
	}
	if(msg.message_type == CONNECT_FAILED){
		printf("Cannot connect %s\n",msg.name);
		if(shutdown(server_fd, SHUT_RDWR) < 0)
			error_exit("Cannot shutdown");
		if(close(server_fd) < 0)
			error_exit("Cannot close server descriptor");
		exit(EXIT_FAILURE);
	}
	
	disconnect_from_server();
}
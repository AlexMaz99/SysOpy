#include "common.h"

char* client_name;
int server_fd;
char sign;
int move;
pthread_mutex_t move_mutex;

void disconnect_from_server(){
	printf("Disconnecting from server...\n");
	send_message(server_fd, DISCONNECT, NULL, NULL);
	if(shutdown(server_fd, SHUT_RDWR) < 0) error_exit("Cannot shutdown.");
	if(close(server_fd) < 0) error_exit("Cannot close server descriptor.");
	exit(EXIT_SUCCESS);
}

void sigint_handler_client(int signo){
	disconnect_from_server();
}

void connect_to_local_server(char* server){
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, server);
	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	
	if(server_fd < 0) error_exit("Socket to server failed.");
	if(connect(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) error_exit("Connect to server failed.");
}

void connect_to_inet_server(int port_number, char* server){
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);
	addr.sin_addr.s_addr = inet_addr(server);
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(server_fd < 0) error_exit("Socket to server failed.");
	if(connect(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) error_exit("Connect to server failed.");
}


void print_board(game* game){
	printf("BOARD\n");
	printf("Your sign: %c\n\n", sign);
	for(int i = 0; i < 9; i++){
		if (game -> board[i] == '-') printf("%d", i);
		else printf("%c", game->board[i]);

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

void disconnect(){
	printf("Received DISCONNECT from server. Good bye!\n");
	sigint_handler_client(SIGINT);
	exit(EXIT_SUCCESS);
}

void my_move(message *msg){
	move = -1;
	pthread_t move_thread;
	pthread_create(&move_thread, NULL, (void*) enemy_move, msg);

	while(1) {
		if (move < 0 || move > 8 || msg->game.board[move] != '-') {
			message receive_msg = receive_message(server_fd, 1);

			if (receive_msg.message_type == PING) send_message(server_fd, PING, NULL, NULL);
			else if (receive_msg.message_type == DISCONNECT) disconnect();
		}
		else break;
	}

	pthread_join(move_thread, NULL);
	msg->game.board[move] = sign;
	print_board(&msg->game);
	send_message(server_fd, MOVE, &msg->game, NULL);
}


void finish_game(game game){
	print_board(&game);
	if(game.winner == sign) printf("YOU WON!\n");
	else if(game.winner == 'D') printf("IT'S A DRAW!\n");
	else printf("YOU LOST. REALLY?\n");
	disconnect_from_server();
	exit(EXIT_SUCCESS);
}

void start_game(message msg){
	sign = msg.game.winner;
	printf("Game started. Your sign: %c\n", sign);
	print_board(&msg.game);
	if(sign == 'O') my_move(&msg);
	else printf("Waiting for enemy move\n");
}

void make_move(message msg){
	printf("MOVE!\n");
	print_board(&msg.game);
	my_move(&msg);
}

void client_exec(){
	while(1){
		message msg = receive_message(server_fd, 0);
		if (msg.message_type == WAIT) printf("Waiting for an opponent.\n");
		else if (msg.message_type == PING) send_message(server_fd, PING, NULL, NULL);
		else if (msg.message_type == DISCONNECT) disconnect();
		else if (msg.message_type == GAME_FOUND) start_game(msg);
		else if (msg.message_type == MOVE) make_move(msg);
		else if (msg.message_type == GAME_FINISHED) finish_game(msg.game);
		else break;
	}
}

void connect_with_server(){
	send_message(server_fd, CONNECT, NULL, client_name);

	message msg = receive_message(server_fd, 0);

	if(msg.message_type == CONNECT){
		printf("You connected to server successfully\n");
		client_exec();
	}
	if(msg.message_type == CONNECT_FAILED){
		printf("You didn't connect to server. %s\n",msg.name);
		if(shutdown(server_fd, SHUT_RDWR) < 0) error_exit("Cannot shutdown.");
		if(close(server_fd) < 0) error_exit("Cannot close server descriptor.");
		exit(EXIT_FAILURE);
	}

	printf("Something went wrong\n");
}


int main(int argc, char** argv){

	if(argc < 4) error_exit("Wrong number of arguments. Expected: [client name] [LOCAL / INET] [address of server]");
	srand(time(NULL));
	signal(SIGINT, sigint_handler_client);

	client_name = argv[1];

	connection_type connection;
	if(strcmp(argv[2], "LOCAL") == 0) connection = LOCAL;
	else if(strcmp(argv[2], "INET") == 0) connection = INET;
	else  error_exit("Wrong type of connections. Expected: LOCAL / INET");


	char* server = argv[3];
	if(connection == LOCAL){
		connect_to_local_server(server);
	}
	else{
		if(argc < 5) error_exit("Wrong arguments. Expected: [client name] [INET] [adress IP] [port number]");
		printf("Server IP: %s port: %d\n",server, atoi(argv[4]));
		connect_to_inet_server(atoi(argv[4]), server);
	}

	printf("Welcome in game %s!\n", client_name);
	connect_with_server();
	disconnect_from_server();
}
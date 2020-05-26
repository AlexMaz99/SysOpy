#include "common.h"

int port_number;
char* socket_path;

struct sockaddr local_sockaddr;
int local_sock;

struct sockaddr_in inet_sockaddr;
int inet_sock;

client clients[MAX_CLIENTS];
int waiting_idx;
int first_free;

pthread_mutex_t clients_mutex;
pthread_t net_thread;
pthread_t ping_thread;

void close_server(){
	if(pthread_cancel(net_thread) == -1) error_exit("Could not cancel net tread");
	if(pthread_cancel(ping_thread) == -1) error_exit("Could not cancel ping thread");
	close(local_sock);
	unlink(socket_path);
	close(inet_sock);
}

void start(){
	// local
	local_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(local_sock == -1) error_exit("Cannot initalize local socket");
	local_sockaddr.sa_family = AF_UNIX;
	strcpy(local_sockaddr.sa_data, socket_path);
	if(bind(local_sock, &local_sockaddr, sizeof(local_sockaddr)) == -1) error_exit("Cannot bind");
	printf("Local socket fd: %d\n", local_sock);

	// inet
	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(inet_sock == -1) error_exit("Cannot initalize local socket");
	inet_sockaddr.sin_family = AF_INET;
	inet_sockaddr.sin_port = htons(port_number);
	inet_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(bind(inet_sock, (struct sockaddr*) &inet_sockaddr, sizeof(inet_sockaddr)) == -1) error_exit("Cannot bind");
}

void empty_client(int i){
	if(clients[i].name != NULL) free(clients[i].name);
	if(clients[i].addr != NULL) free(clients[i].addr);
	clients[i].name = NULL;
	clients[i].addr = NULL;
	clients[i].fd = -1;
	clients[i].game = NULL;
	clients[i].active = 0;
	clients[i].symbol = '-';
	clients[i].opponent_idx = -1;
	if(waiting_idx == i) waiting_idx = -1;
}

int client_exist(int i){
	return i>= 0 && i < MAX_CLIENTS && clients[i].fd != -1;
}

int get_free_idx(){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(!client_exist(i)) return i;
	}
	return -1;
}

int get_client_index(char* name){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(client_exist(i) && strcmp(name, clients[i].name) == 0) return i;
	}
	return -1;
}

int is_name_available(char* name){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(client_exist(i) && strcmp(name, clients[i].name) == 0) return 0;
	}
	return 1;
}

void check_game_status(game* game){
	static int wins[8][3] = { {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6} };

	for(int i = 0; i < 8; i++){
		if (game->board[wins[i][0]] == game->board[wins[i][1]] && game->board[wins[i][1]] == game->board[wins[i][2]])
		{
			game->winner = game->board[wins[i][0]];
			return;
		}
	}

	int is_move = 0;
	for(int i = 0; i < 9; i++){
		if(game->board[i] == '-'){
			is_move = 1;
			break;
		}
	}
	if(is_move == 0) game->winner = 'D';
	if(game->turn == 'X') game->turn = 'O';
	else if(game->turn == 'O') game->turn = 'X';
}

void start_game(int id1, int id2){

	clients[id1].opponent_idx = id2;
	clients[id2].opponent_idx = id1;

	if (rand() % 2 == 1){
		clients[id1].symbol = 'O';
		clients[id2].symbol = 'X';
	}
	else{
		clients[id1].symbol = 'X';
		clients[id2].symbol = 'O';
	}

	game* game = malloc(sizeof(game));
	new_board(game);

	clients[id1].game = clients[id2].game = game;
	game->winner = clients[id1].symbol;
	send_message_to(clients[id1].fd, clients[id1].addr, GAME_FOUND, game, clients[id1].name);
	game->winner = clients[id2].symbol;
	send_message_to(clients[id2].fd, clients[id2].addr, GAME_FOUND, game, clients[id2].name);
	game->winner = '-';
}

void connect_client(int fd, struct sockaddr* addr, char* new_nick){
	printf("Connecting client to server\n");

	char* nick = calloc(NAME_LEN, sizeof(char));
	strcpy(nick, new_nick);

	if(is_name_available(nick) == 0){
		send_message_to(fd, addr, CONNECT_FAILED, NULL, "This name is taken");
		free(nick);
		return;
	}
	if(first_free == -1){
		send_message_to(fd, addr, CONNECT_FAILED, NULL, "Server is full");
		free(nick);
		return;
	}

	send_message_to(fd, addr, CONNECT, NULL, "Connected");

	clients[first_free].name = nick;
	clients[first_free].active = 1;
	clients[first_free].fd = fd;
	clients[first_free].addr = addr;


	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients[i].name != NULL) printf("%d: %s\n",i, clients[i].name);
	}

	if(waiting_idx != -1){
		start_game(first_free, waiting_idx);
		waiting_idx = -1;
	}
	else{
		waiting_idx = first_free;
		send_message_to(fd, clients[first_free].addr, WAIT, NULL, nick);
	}

	first_free=get_free_idx();

	printf("Client connected successfully\n");
}

void net_exec(void* arg){

	struct pollfd poll_fds[2];

	poll_fds[0].fd = local_sock;
	poll_fds[1].fd = inet_sock;
	poll_fds[0].events = POLLIN;
	poll_fds[1].events = POLLIN;

	while(1){

		pthread_mutex_lock(&clients_mutex);
		for (int i = 0; i < 2; i++) {
			poll_fds[i].events = POLLIN;
			poll_fds[i].revents = 0;
		}
		pthread_mutex_unlock(&clients_mutex);
		if(poll(poll_fds, 2, -1) == -1) error_exit("Cannot poll");
		pthread_mutex_lock(&clients_mutex);

		for(int i = 0; i < 2; i++){
			if(poll_fds[i].revents && POLLIN){

				struct sockaddr* addr = malloc(sizeof(struct sockaddr));
				socklen_t len = sizeof(&addr);
				printf("Descriptor: %d\n", poll_fds[i].fd);

				message msg = receive_message_from(poll_fds[i].fd, addr, len);
				printf("Message received in server\n");
				int idx;

				if (msg.message_type == CONNECT){
					connect_client(poll_fds[i].fd, addr, msg.name);
				}

				else if (msg.message_type == MOVE){

					printf("Received move from client\n");
					idx = get_client_index(msg.name);
					check_game_status(&msg.game);

					if(msg.game.winner == '-') {
						send_message_to(clients[clients[idx].opponent_idx].fd, clients[clients[idx].opponent_idx].addr, MOVE, &msg.game, clients[clients[idx].opponent_idx].name);
					}

					else{
						send_message_to(clients[idx].fd, clients[idx].addr, GAME_FINISHED, &msg.game, clients[idx].name);
						send_message_to(clients[clients[idx].opponent_idx].fd, clients[clients[idx].opponent_idx].addr, GAME_FINISHED, &msg.game, clients[clients[idx].opponent_idx].name);
						free(clients[idx].game);
					}
					free(addr);
				}

				else if (msg.message_type == DISCONNECT){

					idx = get_client_index(msg.name);
					printf("Received disconnect from client\n");

					if(client_exist(clients[idx].opponent_idx)){
						send_message_to(clients[clients[idx].opponent_idx].fd, clients[clients[idx].opponent_idx].addr, DISCONNECT, NULL, clients[clients[idx].opponent_idx].name);
						empty_client(clients[idx].opponent_idx);
					}

					empty_client(idx);
					free(addr);
				}

				else if (msg.message_type == PING){
					idx = get_client_index(msg.name);
					clients[idx].active = 1;
					free(addr);
				}

				else free(addr);
				
			}
		}
		pthread_mutex_unlock(&clients_mutex);
	}
}

void ping_exec(void* arg){
	while(1){

		sleep(PING_INTERVAL);
		printf("Started PING ...\n");
		pthread_mutex_lock(&clients_mutex);

		for(int i = 0; i < MAX_CLIENTS; i++){
			if(client_exist(i)){
				clients[i].active = 0;
				send_message_to(clients[i].fd, clients[i].addr, PING, NULL, clients[i].name);
			}
		}

		pthread_mutex_unlock(&clients_mutex);
		sleep(PING_WAIT);
		pthread_mutex_lock(&clients_mutex);

		for(int i = 0; i < MAX_CLIENTS; i++){

			if(client_exist(i) && clients[i].active == 0) {
				printf("Client %d didn't respond. Disconnecting %d...\n", i, i);
				send_message_to(clients[i].fd, clients[i].addr, DISCONNECT, NULL, clients[i].name);
				
				if(client_exist(clients[i].opponent_idx)){
					send_message_to(clients[clients[i].opponent_idx].fd, clients[clients[i].opponent_idx].addr, DISCONNECT, NULL, clients[clients[i].opponent_idx].name);
					empty_client(clients[i].opponent_idx);
				}
				empty_client(i);
			}
		}

		pthread_mutex_unlock(&clients_mutex);
	}
}


void exit_fun(){
	close_server();
}

void sigint_handler_server(int signo){
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){

	if(argc < 3) error_exit("Wrong number of arguments. Expected: [number of port] [path to socket]");
	srand(time(NULL));

	port_number = atoi(argv[1]);
	socket_path = argv[2];
	signal(SIGINT, sigint_handler_server);
	atexit(exit_fun);

	for(int i = 0; i < MAX_CLIENTS; i++) empty_client(i);

	start();
	waiting_idx = -1;
	first_free = 0;

	if(pthread_create(&net_thread, NULL, (void*) net_exec, NULL) == -1) error_exit("Cannot create net thread.");
	if(pthread_create(&ping_thread, NULL, (void*) ping_exec, NULL) == -1) error_exit("Cannot create ping thread");


	if(pthread_join(net_thread, NULL) < 0) error_exit("Cannot join net thread.");
	if(pthread_join(ping_thread, NULL) < 0) error_exit("Cannot join ping thread.");

	close_server();
	exit(EXIT_SUCCESS);
}
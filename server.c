#define PORT 27993
#define MAX_BUFFER 256
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>


#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>

static volatile int signal_flag = 1;
int sockfd;
char hostname[50], NUID[20], flag[50];
void intHandler(int dummy) {
    signal_flag = 0;
    close(sockfd);
    exit(1);
}

int validate_solution_message(char buffer[]){
	char *token = strtok(buffer, " ");

	if (strcmp(token, "cs5700fall2017") != 0){
		return 0;
	}
	token = strtok(NULL, "\n");
	if (strcmp(token, "") == 0){
		return 0;
	}	
	int answer = atoi(token); 
	return answer;
}
char *validate_hello_message(char buffer[]){
	char *token = strtok(buffer, " ");
	if (strcmp(token, "cs5700fall2017") != 0){
		return NULL;
	}
	token = strtok(NULL, " ");
	if (strcmp(token, "HELLO") != 0){
		return NULL;
	}
	token = strtok(NULL, "\n");
	if (strcmp(token, &NUID[0]) != 0){
		return "Unknown_Husky_ID";
	}	
	return token;
}


int main(int argc, char* argv[])
{   
	int port = PORT;
	
	if (argc != 4 && argc != 6) {
    	printf("please follow the format : %s localhost [NEU ID] [FLAG]\n", argv[0]);
    	exit(1);
  	}
  	if (argc == 6) {
		port = atoi(argv[2]);
		memcpy(hostname, argv[3], strlen(argv[3]));
		hostname[strlen(argv[3])] = '\0';
		
		memcpy(NUID, argv[4], strlen(argv[4]));
		NUID[strlen(argv[4])] = '\0';
		
		memcpy(flag, argv[5], strlen(argv[5]));
		flag[strlen(argv[5])] = '\0';
 	}
 	else{
		port = PORT;
		memcpy(hostname, argv[1], strlen(argv[1]));
		hostname[strlen(argv[1])] = '\0';
		
		memcpy(NUID, argv[2], strlen(argv[2]));
		NUID[strlen(argv[2])] = '\0';
		
		memcpy(flag, argv[3], strlen(argv[3]));
		flag[strlen(argv[3])] = '\0';
 	}
 	if(strcmp(hostname, "localhost") !=0){
 		printf("\nHostname should be localhost\n");
 		exit(1);
 	}
	int clientfd;;
    signal(SIGINT, intHandler);
	struct sockaddr_in self;
	char buffer[MAX_BUFFER];
	srand(time(NULL));  
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		exit(errno);
	}

	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
	self.sin_addr.s_addr = INADDR_ANY;

    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("socket--bind");
		exit(errno);
	}

	if ( listen(sockfd, 20) != 0 )
	{
		perror("socket--listen");
		exit(errno);
	}
	char expression[100],NUID[20];

	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	while (1)
	{	

		struct sockaddr_in client_addr;
		int addrlen=sizeof(client_addr);

		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		int data_size = recv(clientfd, buffer, MAX_BUFFER, 0);
		char *response = validate_hello_message(buffer);
		if(response == NULL){
			printf("Invalid hello message. Closing client connection\n");
			close(clientfd);
			continue;

		}
		else if(strcmp(response,"Unknown_Husky_ID") == 0){
			strcpy(flag, "Unknown_Husky_ID");
		}

		int no_operations = rand() % 100;
		for(int i = 0;i< no_operations;i++){
			int num1 = rand() % 1000;
			int num2 = rand() % 1000;
			int operation = rand() % 4;
			char operator = ' ';
			if (operation == 0) {
			  operator = '+';
			} else if (operation == 1) {
			  operator = '-';
			} else if (operation == 2) {
			  operator = '/';
			} else if (operation == 3) {
			  operator = '*';
			}
			sprintf(expression, "cs5700fall2017 STATUS %d %c %d\n", num1, operator, num2);
			send(clientfd, expression, strlen(expression), 0);
			int solution_size = recv(clientfd, buffer, MAX_BUFFER, 0);
			buffer[solution_size] = '\0';
			printf("response_solution from client %s", buffer);
			int answer = validate_solution_message(buffer);
			int expected_answer = 0;
			if (operation == 0) {
			  expected_answer = num1 + num2;
			} else if (operation == 1) {
			  expected_answer = num1 - num2;
			} else if (operation == 2) {
			  expected_answer = num1 / num2;
			} else if (operation == 3) {
			  expected_answer = num1 * num2;
			}

			if(answer != expected_answer){
				printf("wrong solution. Closing client connection\n");
				close(clientfd);
				continue;
			}
		}
		sprintf(expression, "cs5700fall2017 %s BYE\n", flag);
		printf("sending expression %s", expression);
		send(clientfd, expression, strlen(expression), 0);
		close(clientfd);
	}
	close(sockfd);
	return 0;
}
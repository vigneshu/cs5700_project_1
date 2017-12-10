#define PORT 27993
#define MAX_BUFFER 256
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>

static volatile int signal_flag = 1;
int sockfd, file_len;
char hostname[50], NUID_file_name[20], flag_file_name[50];
char nuids_from_file[150][50], flags_from_file[150][20];
void intHandler(int dummy) {
    signal_flag = 0;
    close(sockfd);
    exit(1);
}

void load_file(){
	FILE *fp;
	printf("NUID_file_name %s",NUID_file_name);
	fp = fopen(NUID_file_name, "r");
	if (fp == NULL){
		printf("failed to open nuids_from_file file\n");
        exit(EXIT_FAILURE);
	}
    char * line = NULL;
    size_t len = 0;
    ssize_t read = 0;
	int line_no = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		// printf("Retrieved line of length %zu :\n", read);
		// line[len - 1] = '\0';
		// memcpy(nuids_from_file[line_no], line, len - 1);
		// nuids_from_file[line_no][len - 5] = '\0';		
		// printf("%s\n\n", nuids_from_file[line_no]);
		strcpy(nuids_from_file[line_no], line);
		nuids_from_file[line_no][read - 1] = '\0';
		line_no++;
	} 
	fclose(fp);

	fp = fopen(flag_file_name, "r");
	if (fp == NULL){
		printf("failed to open flgs file\n");
        exit(EXIT_FAILURE);
	}
	line_no = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		// printf("Retrieved line of length %zu :\n", read);
		// line[len - 1] = '\0';

		strcpy(flags_from_file[line_no], line);
		// memcpy(flags_from_file[line_no], line, len - 1);
		flags_from_file[line_no][read - 1] = '\0';		
		// printf("%s", flags_from_file[line_no]);		
		line_no++;
	} 
	file_len = line_no - 1;
	fclose(fp);	

	printf("nuids_from_file \n");
	int i = 0;	
	for (i=0; i<line_no; i++) {
	    printf ("%s" , nuids_from_file[i]);
	}
	printf("flags_from_file \n");
	for (i=0; i<line_no; i++) {
	    printf ("%s" , flags_from_file[i]);
	}
    if (line)
    	free(line);
}
//validates whether the solultion from client is right
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

char* search_nuid(char *token){
	int i = 0, index = 0, flag = 0;
	// printf("token %s\n",token);
	for(i = 0; i < file_len;i++){
		printf("nuids_from_file[i] %s",nuids_from_file[i]);
		if(strcmp(nuids_from_file[i], token) == 0){
			// printf("Inside\n");
			 flag = 1;
			 index = i;
			 break;
		}
		// else if(strcmp(nuids_from_file[i], "001627557\n") == 0){
		// 	// printf("elif Inside\n");
		// 	 flag = 1;
		// 	 break;			
		// }
	} 
	if (flag == 1){
		return flags_from_file[index];
	}
	else{
		return "Unknown_Husky_ID";
	}

}
// validates the first message received from client to see if it is a hwllo message
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
	char* secret_key = search_nuid(token);
	return secret_key;
	// if (strcmp(token, &NUID_file_name[0]) != 0){
	// 	return "Unknown_Husky_ID";
	// }	
	// return token;
}

void * process_client(int clientfd){
	char expression[100];
	printf("client processing %d\n",clientfd);

	char buffer[MAX_BUFFER];
	int data_size = recv(clientfd, buffer, MAX_BUFFER, 0);

	char *secret_key = validate_hello_message(buffer);
	if(secret_key == NULL){
		printf("Invalid hello message. Closing client connection\n");
		close(clientfd);
		return;

	}
	else if(strcmp(secret_key,"Unknown_Husky_ID") == 0){
		strcpy(flag_file_name, "Unknown_Husky_ID");
	}

	int no_operations = rand() % 100;
	int i = 0;
	for(i = 0;i< no_operations;i++){
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
		// printf("response_solution from client %s", buffer);
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
			return;
		}
	}
	sprintf(expression, "cs5700fall2017 %s BYE\n", secret_key);
	printf("sending expression %s", expression);
	send(clientfd, expression, strlen(expression), 0);
	close(clientfd);
}
int main(int argc, char* argv[])
{   
	int port = PORT;
	
	if (argc != 4 && argc != 6) {
    	printf("please follow the format : %s localhost [NEU ID] [flag_file_name]\n", argv[0]);
    	exit(1);
  	}
  	if (argc == 6) {
		port = atoi(argv[2]);
		memcpy(hostname, argv[3], strlen(argv[3]));
		hostname[strlen(argv[3])] = '\0';
		
		memcpy(NUID_file_name, argv[4], strlen(argv[4]));
		NUID_file_name[strlen(argv[4])] = '\0';
		
		memcpy(flag_file_name, argv[5], strlen(argv[5]));
		flag_file_name[strlen(argv[5])] = '\0';
 	}
 	else{
		port = PORT;
		memcpy(hostname, argv[1], strlen(argv[1]));
		hostname[strlen(argv[1])] = '\0';
		
		memcpy(NUID_file_name, argv[2], strlen(argv[2]));
		NUID_file_name[strlen(argv[2])] = '\0';
		
		memcpy(flag_file_name, argv[3], strlen(argv[3]));
		flag_file_name[strlen(argv[3])] = '\0';
 	}
 	if(strcmp(hostname, "localhost") !=0){
 		printf("\nHostname should be localhost\n");
 		exit(1);
 	}
 	load_file();
	int clientfd;;
    signal(SIGINT, intHandler);
	struct sockaddr_in self;
	
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
	char NUID_file_name[20];

	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	while (1)
	{	

		struct sockaddr_in client_addr;
		int addrlen=sizeof(client_addr);

		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		pthread_t handle_client_thread;
		pthread_create(&handle_client_thread, NULL, process_client, clientfd);

		
	}
	close(sockfd);
	return 0;
}
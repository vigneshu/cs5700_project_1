#define PORT 27993
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_BUFFER 256


//connects to the serve, given hostname and port
int connect_server(char *host_name, int port) {
	struct sockaddr_in addr;
	int fd;
	fd = socket(AF_INET, SOCK_STREAM, 0);//TCP/IP network and TCP protocol.
	if(fd == -1)
	{
	  printf("Error opening socket\n");
	  return -1;
	}
	struct hostent *host;
	if ((host = gethostbyname(host_name)) == NULL) {  // get the host info from host name
        printf("gethostbynamesss");
        return -1;
    }
    struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
    memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;   
	addr.sin_port =  htons(port);       
	addr.sin_addr.s_addr = 
        *(long*)addr_list[0]; 
    if (!connect(fd, (const struct sockaddr* )&addr, sizeof(addr)) == -1) {// connect to server
		perror("could not connect to host");
        return -1;
    }
    return fd;
}


//parses the message from the server, adds them up and sendss back the solution
int parse_msg(char *buf, int fd) {
	char solution[100];
	char *token, *third_word;
	token = strtok(buf, " ");
	token = strtok(NULL, " "); // second word
	third_word = strtok(NULL, " "); // third word
	if (strcmp(token, "STATUS") == 0) {
		char *num1, *operator, *num2;
		num1 = third_word;
		operator = strtok(NULL, " "); 
		num2 = strtok(NULL, " "); 
		int num1_int = atoi(num1);
		int num2_int = atoi(num2);
		int ans;
		if (operator[0] == '+') {
		  ans = num1_int + num2_int;
		} else if (operator[0] == '-') {
		  ans = num1_int - num2_int;
		} else if (operator[0] == '*') {
		  ans = num1_int * num2_int; 
		} else if (operator[0] == '/') {
		  ans = num1_int / num2_int;
		}
		sprintf(solution, "cs5700fall2017 %d\n", ans);
		
		if (send(fd, solution, strlen(solution), 0) == -1) {
		  printf("failed to send solution to server\n");
		  exit(1);
		}
		return 0;
	} 
	else if (strcmp(third_word , "BYE\n") == 0) {
		printf("Token from server : %s\n", token);
		return 1;
	}
	else{
		printf("Unknown message from server : %s\n", buf);
		exit(1);
		return 0;
	}
}

// Sends hello message to sever, starting the communication
void start_communication(int fd, char NUID[]) {
	char buffer[MAX_BUFFER];
	char hello_msg[50] = "cs5700fall2017 HELLO ";
	strcat(hello_msg, NUID);
	strcat(hello_msg, "\n");
	
	int read_size;
	if ((read_size = send(fd, hello_msg, strlen(hello_msg), 0)) == -1) {
		printf("could not send to server\n");
		exit(1);
	}
	int completed = 0;
	while (!completed) {
		read_size = recv(fd, buffer, MAX_BUFFER, 0);
		if (( read_size== -1)) {
		  printf("problem receiving from server\n");
		  exit(1);
		}

		else if(read_size == 0){
			printf("connection closed\n");
			exit(1);
		}
		buffer[read_size] = '\0';
		completed = parse_msg(buffer, fd);

		memset(buffer, 0, MAX_BUFFER);
	}
	close(fd);
}


int main(int argc, char* argv[]) {
	int port = PORT;
	char hostname[50];
	char NUID[20];
	 if (argc != 3 && argc != 5) {
    	printf("please follow the format : %s [hostname] [NEU ID]\n", argv[0]);
    	exit(1);
  	}
 	if (argc == 5) {
		port = atoi(argv[2]);
		memcpy(hostname, argv[3], strlen(argv[3]));
		hostname[strlen(argv[3])] = '\0';
		
		memcpy(NUID, argv[4], strlen(argv[4]));
		NUID[strlen(argv[4])] = '\0';
 	}
 	else{
 		memcpy(hostname, argv[1], strlen(argv[1]));
 		memcpy(NUID, argv[2], strlen(argv[2]));
 		NUID[strlen(argv[2])] = '\0';
 		hostname[strlen(argv[1])] = '\0';
 	}
 	printf("hostname: %s port: %d NUID: %s\n", hostname, port, NUID);
 	int fd = connect_server(hostname, port);
 	start_communication(fd, NUID);
 	return 0;
}
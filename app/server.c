#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


int main() {
	// Disable output buffering
	setbuf(stdout, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");
    int server_fd, client_addr_len;

	// Uncomment this block to pass the first stage
	struct sockaddr_in client_addr;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
    int connected_fd;
    while(1) {
    
        if ((connected_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
            perror("accept error.");
            continue;
        }
        else {
            printf("Client connected\n");
            char *status_line = "HTTP/1.1 200 OK \r\n\r\n";
            if (send(connected_fd, status_line, strlen(status_line), 0) == -1){
                perror("send error.");
                close(connected_fd);
                exit(1);
            }
            char incoming_msg[1024];
            int buff_size = 1024;
            if (recv(connected_fd, incoming_msg, buff_size, 0) < 1) {
                perror("receive error.");
                exit(1);
            }
            char *path_start, *path_end;
            if ((path_start = strchr(incoming_msg, '/')) == NULL) {
                perror("Input error.");
                exit(1);
            }
            if ((path_end = strchr(path_start, ' ')) == NULL) {
                perror("Input error2.");
                exit(1);
            }
            char output[1024] = {'\0'};
            strncpy(output, path_start, (path_end-path_start+1)/sizeof(char));
            printf("%s\n", output);
            char *response;
            if (strcmp(output, "\\") == 0) {
                response = "HTTP/1.1 200 OK \r\n\r\n";
                if (send(connected_fd, response, strlen(response), 0) == -1) {
                    perror("send error 2.");
                    close(connected_fd);
                    exit(1);
                }
            }
            else {
                response = "HTTP/1.1 404 Not Found \r\n\r\n";
                 if (send(connected_fd, response, strlen(response), 0) == -1) {
                    perror("send error 2.");
                    close(connected_fd);
                    exit(1);
                }
            }
            

            
            

            close(connected_fd);
            exit(0);
        }
        


    }
	
	
	close(server_fd);

	return 0;
}

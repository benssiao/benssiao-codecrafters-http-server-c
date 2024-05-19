#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
int check_if_valid_path(const char*);
void send200(int);
void send404(int);
char **extract_path(const char *incoming);
void free_pathlist(char**);
char *extract_user_agent(const char *incoming);
void free_user_agent(char *user_agent);
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
        // begin listening loop.
        if ((connected_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
            perror("accept error.");
            continue;
        }
        else {
            printf("Client connected\n");
            char incoming_msg[1024];
            int buff_size = 1024;
            if (recv(connected_fd, incoming_msg, buff_size, 0) < 1) {
                perror("receive error.");
                exit(1);
            }
            printf("incoming_msg: %s\n", incoming_msg);
            char **path_list = extract_path(incoming_msg);

            
            for (char **iter = path_list; strcmp(*iter, "") != 0; iter++) {
                printf("%s\n", *iter);
            }
            
            printf("Comparison result: %d\n", strcmp(path_list[0], ""));
            if (strcmp(path_list[0], "") == 0) {
                free_pathlist(path_list);
                send200(connected_fd);
            } 
            else if (strcmp(path_list[0], "echo") == 0) {
                // printf("%s\n", output);
                char response[1100];
                snprintf(response, 1100, \
                        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s"\
                        , strlen(path_list[1]), path_list[1]);
                // printf("%s\n", response);
                if (send(connected_fd, response, strlen(response), 0) == -1) {
                    perror("send error 3.");
                    close(connected_fd);
                    free_pathlist(path_list);
                    exit(1);
                    }
                free_pathlist(path_list);
                }
            else if (strcmp(path_list[0], "user-agent")) {
                char *user_agent = extract_user_agent(incoming_msg);
                char response[1100];
                snprintf(response, 1100, \
                        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s"\
                        , strlen(user_agent), user_agent);

                if (send(connected_fd, response, strlen(response), 0) == -1) {
                    perror("send error 3.");
                    close(connected_fd);
                    free_pathlist(path_list);
                    free_user_agent(user_agent);
                    exit(1);
                    }


            }
            else {
                // printf("This is wrong!!!");
                free_pathlist(path_list);
                send404(connected_fd);
            }
            }
            
            
            close(connected_fd);
            exit(0);
            }
    }

void send404(int socket) {
    char *response; 
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
    if (send(socket, response, strlen(response), 0) == -1) {
        perror("senderror 4.");
        close(socket);
        exit(1);
    }
}
void send200(int socket) {
    char *response; 
    response = "HTTP/1.1 200 OK\r\n\r\n";
    if (send(socket, response, strlen(response), 0 ) == -1) {
        perror("senderror 5.");
        close(socket);
        exit(1);
    }
}
char **extract_path(const char* incoming) {
    char *path_start, *path_end;
    char **output;
    if ((path_start = strchr(incoming, '/')) == NULL) {
        perror("Input error.");
        exit(1);
    }
    if ((path_end = strchr(path_start, ' ')) == NULL) {
        perror("Input error2.");
        exit(1);
    }
    path_end--;
    
    // printf("%d\n", path_end-path_start+1);
    char path_str[(path_end-path_start+1)];
    memcpy(path_str, path_start, sizeof(char)*(path_end-path_start));
    path_str[path_end-path_start] = '\0';
    // printf("%s\n", path_str);
    output = (char**) malloc(sizeof(char*)*(10)); // Need 1 more for the terminating null char.
    int i = 0;
    for (char *iter = strtok(path_str, "/"); iter != NULL; iter = strtok(NULL, "/")) {
        char *curr_str = (char*)malloc(strlen(iter)*sizeof(char)+1);
        memcpy(curr_str, iter, sizeof(char)*strlen(iter));
        curr_str[strlen(iter)] = '\0';
        output[i] = curr_str;
        i++;
    }
    char *end = (char*)malloc(sizeof(char));
    *end = '\0';
    output[i] = end;
    return output;
}
char *extract_user_agent(const char *incoming) {
    char *path_start, *path_end;
    char *output;
    if ((path_start = strchr(incoming, '\n')) == NULL) {
        perror("Input error.");
        exit(1);
    }
    path_start++;
    if ((path_start = strchr(path_start, '\n')) == NULL) {
        perror("Input error.");
        exit(1);
    }
    if ((path_start = strchr(path_start, ' ')) == NULL) {
        perror("Input error.");
        exit(1);
    }
    if ((path_end = strchr(path_start, '\r')) == NULL) {
        perror("Input error2.");
        exit(1);
    }
    output = (char*) malloc((path_end-path_start)*sizeof(char));
    memcpy(output, path_start+1, path_end-path_start-1);
    output[path_end-path_start] = '\0';
    return output;
}

void free_user_agent(char *user_agent) {
    free(user_agent);
}
void free_pathlist(char **path_list){
    for (char **iter = path_list; strcmp(*iter, "") != 0; iter++) {
        free(*iter);
    }
    free(path_list);
}
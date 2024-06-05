#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sendCodes.h"
#include "HTTP_str_processing.h"
#include <stddef.h>


void free_user_agent(char *user_agent);
int check_file_exists(const char *fname);

int main(int argc, char *argv[]) {
    char directory[100] = "";
    if (argc > 2) {
        if (strcmp(argv[1], "--directory") == 0) {
            memcpy(directory, argv[2], sizeof(char)*strlen(argv[2]));
        }
    }
	// Disable output buffering
	setbuf(stdout, NULL);
    struct sockaddr_in client_addr;
    int server_fd, client_addr_len;
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

    while(1) {  // begin listening loop.
        // printf("Listening loop\n");
        connected_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        // printf("Connected woohoo\n");
        if (connected_fd == -1) {
            // perror("accept error.");
            continue;
        }
        else {
            if (!fork()) {
                close(server_fd);
                char incoming_msg[1024];
                int buff_size = 1024;

                if (recv(connected_fd, incoming_msg, buff_size, 0) < 1) {
                    perror("receive error.");
                }
                char **path_list = extract_path(incoming_msg);
                char *command = get_command(incoming_msg);

                if (strcmp(command, "GET") == 0) {
                    free(command);
                    if (strcmp(path_list[0], "") == 0) { // GET /
                        send200(connected_fd);
                    } 
                    else if (strcmp(path_list[0], "echo") == 0) { // GET /echo
                        if (send200WithContentHeader(connected_fd, path_list[1], strlen(path_list[1]), "text/plain") == -1) {
                            perror("send error 3.");  
                        }
                    }
                    else if (strcmp(path_list[0], "user-agent") == 0) { // GET /user-agent
                        char *user_agent = extract_user_agent(incoming_msg);
                        // printf("user_agent: %s, user_agent length: %d\n", user_agent, strlen(user_agent));
                        if (send200WithContentHeader(connected_fd, user_agent, strlen(user_agent), "text/plain") == -1) {
                            perror("send error: user-agent.");
                        } 
                    }
                    else if (strcmp(path_list[0], "files") == 0) { // GET /files/<filename>
                        if (strcmp(path_list[1], "") != 0 && strcmp(path_list[2], "") == 0) {
                            char *filename = path_list[1];
                            if (strcmp(directory, "") != 0) {
                                strcat(directory, filename);
                                if (check_file_exists(directory)) {
                                    FILE *fptr;
                                    fptr = fopen(directory, "r");
                                    if (fptr) {
                                        fseek (fptr, 0, SEEK_END);
                                        int length = ftell (fptr);
                                        fseek (fptr, 0, SEEK_SET);
                                        char *buffer = (char *) malloc (length+1);
                                        if (buffer)
                                        {
                                            fread(buffer, sizeof(char), length, fptr);
                                        }
                                        buffer[length] = '\0';
                                        // printf("buffer: %s\n", buffer);
                                        if (send200WithContentHeader(connected_fd, buffer, strlen(buffer), "application/octet-stream") == -1) {
                                            perror("send error: file.\n");
                                        }
                                        fclose (fptr);
                                    }
                                }
                                else{
                                    send404(connected_fd);
                                }
                            }
                            else {
                                perror("Get file: input error. No directory");
                            }
                        }
                        else{
                            perror("GET file: input error.");
                        }
                    }
                    else { // ERRORNOUS INPUT.
                        send404(connected_fd);
                    }
                    free_pathlist(path_list);
                    close(connected_fd);
                }
                else if (strcmp(command, "POST") == 0) {
                    free(command);
                    if (strcmp(path_list[1], "") != 0 && strcmp(path_list[2], "") == 0) {
                        char *filename = path_list[1];
                        if (strcmp(directory, "") != 0) {
                            strcat(directory, filename);
                            char *response_body = get_response_body(incoming_msg);
                            FILE *newfile;
                            if ((newfile = fopen(directory, "w")) == NULL) {
                                perror("Couldn't open.");
                            }
                            //printf("directory: %s\n", directory);
                            //printf("response_body: %s\n", response_body);
                            fputs(response_body, newfile);
                            //printf("put it in the file");
                            fclose(newfile);
                            free(response_body);
                            char *response; 
                            response = "HTTP/1.1 201 Created\r\n\r\n";
                            //printf("response: %s\n", response);
                            if (send(connected_fd, response, strlen(response), 0) == -1) {
                                perror("senderror 5.");
                            }
                            
                        }
                    }
                }
            }
        }
        close(connected_fd);
    }
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


int check_file_exists(const char *fname) {
    FILE *file;
    if ((file = fopen(fname, "r"))) {
        fclose(file);
        return 1;
    }
    return 0;
}

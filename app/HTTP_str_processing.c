#include "HTTP_str_processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include "sendCodes.h"
int get_content_length(char *incoming_msg) {
    int output;
    char *content_length_start;
    if ((content_length_start = strstr(incoming_msg, "Content-Length")) == NULL) {
        return -1;
    }
    content_length_start = strchr(content_length_start, ' ');
    char *content_length_end = strchr(content_length_start, '\r');
    int digit_len = content_length_end-content_length_start;
    char int_as_str[digit_len+1];
    memcpy(int_as_str, content_length_start, sizeof(char)*digit_len);
    int_as_str[digit_len] = '\0';
    return atoi(int_as_str);
}
char *get_response_body(char *incoming_msg) {
    /* Start by seeking the empty line "\r\n". Get the content-length and return, */
    char *body_start, *body_end;
    if ((body_start = strchr(incoming_msg, '\n')) == NULL) {
        perror("Input error.");
        exit(1);
    }
    body_start++;
    while (*body_start != '\r') {
        if ((body_start = strchr(body_start, '\n')) == NULL) {
            perror("Input error.");
            exit(1);
        }
        body_start++;
    }
    if ((body_start = strchr(body_start, '\n')) == NULL) {
            perror("Input error.");
            exit(1);
        }
    body_start++;
    int content_length = get_content_length(incoming_msg);
    char *output = (char *) malloc(sizeof(char)*(content_length+1));
    memcpy(output, body_start, content_length);
    output[content_length] = '\0';
    return output;
}
char* get_command(char *incoming_msg) {
    char *command = malloc(sizeof(char)*10);
    int command_length = 0;
    for (int iter = 0; incoming_msg[iter] != ' '; iter++) {
        command_length++;
    } 
    memcpy(command, incoming_msg, command_length);
    command[command_length] = '\0';
    return command;
}



char **extract_path(const char* incoming) { // If incoming == /hello/world then outputs path_list = [hello, world, ""]
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
    // printf("%d\n", path_end-path_start+1);
    char path_str[(path_end-path_start+1)];
    memcpy(path_str, path_start, sizeof(char)*(path_end-path_start));
    path_str[path_end-path_start] = '\0';
    // printf("path_str: %s\n", path_str);
    output = (char**) malloc(sizeof(char*)*(10)); // Need 1 more for the terminating null char. Also I'm assuming there wont be more than 10 depth to any folder.
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
    char *colon;
    char *output;

    if ((path_start = strstr(incoming, "User-Agent")) == NULL) {
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
    //printf("User-Agent I Got : %s\n", output);
    return output;
}

#include "sendCodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include "sendCodes.h"
void send404(int socket) {
    char *response; 
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
    if (send(socket, response, strlen(response), 0) == -1) {
        perror("senderror 4.");
    }
}


void send200(int socket) {
    char *response; 
    response = "HTTP/1.1 200 OK\r\n\r\n";
    if (send(socket, response, strlen(response), 0 ) == -1) {
        perror("senderror 5.");
    }
}


int send200WithContentHeader(int socket, char* msg, size_t msg_len, char* Content_Type) {
    char response[100 + msg_len];
    snprintf(response, 100 + msg_len, \
            "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\n\r\n%s"\
            , Content_Type, msg_len, msg);
    if (send(socket, response, strlen(response), 0) == -1) {
        return -1;
        }
    return 0;
}
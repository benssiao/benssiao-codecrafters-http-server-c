#include <stddef.h>
#ifndef SENDCODES_H
#define SENDCODES_H
int send200WithContentHeader(int socket, char* msg, size_t msg_len, char* Content_Type);
void send200(int);
void send404(int);
#endif
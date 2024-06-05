#ifndef HTTP_STR_PROCESS_H
#define HTTP_STR_PROCESS_H
char **extract_path(const char *incoming);
void free_pathlist(char**);
char *extract_user_agent(const char *incoming);
int get_content_length(char* incoming_msg);
char *get_response_body(char *incoming_msg);
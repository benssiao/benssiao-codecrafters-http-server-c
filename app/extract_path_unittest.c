#include <stdio.h>
#include <stdlib.h>

#include <string.h>

char **extract_path(const char* incoming);
int main(void){

    char **path_list = extract_path("/ ");
    int i = 0;
    for (char **iter = path_list; strcmp(*iter, "") != 0; iter++){
        printf("%s\n", *iter);
        i++;
    }
    printf("%d\n", strcmp(path_list[0], ""));
    free_pathlist(path_list);
    
    exit(0);
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
    
    // printf("%d\n", path_end-path_start+1);
    char path_str[(path_end-path_start+1)];
    memcpy(path_str, path_start, sizeof(char)*(path_end-path_start));
    path_str[path_end-path_start] = '\0';
    // printf("%s\n", path_str);
    output = (char**) malloc(sizeof(char*)*(10)); // Need 1 more for the terminating null char.
    int i = 0;
    for (char *iter = strtok(path_str, "/"); iter != NULL; iter = strtok(NULL, "/")) {
        char *curr_str = (char*)malloc(strlen(iter)*sizeof(char)+1);
        if (memcpy(curr_str, iter, sizeof(char)*strlen(iter)) == -1) {
            perror("memcpy error");
            exit(1);
        }
        curr_str[strlen(iter)] = '\0';
        output[i] = curr_str;
        i++;
    }
    char *end = (char*)malloc(sizeof(char));
    *end = '\0';
    output[i] = end;
    return output;
}


void free_pathlist(char **path_list){
    for (char **iter = path_list; strcmp(*iter, "") != 0; iter++) {
        free(*iter);
    }
    free(path_list);
}
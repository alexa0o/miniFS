#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"

char* read_file(char* file_name, size_t* size) {
    FILE *f = fopen(file_name, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size) {
        *size = fsize;
    }

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    return string;
}

int main(int argc, char* argv[]) {
    int fd = 0;
    struct sockaddr_in dest;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &dest.sin_addr.s_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    dest.sin_port = htons(atoi(argv[2]));

    if (connect(fd, (struct sockaddr*)&dest, sizeof(dest)) < 0) {
        perror("connect");
        return 1;
    }

    for(;;) {
        char command[30];
        printf("MiniFS(remote):> ");
        scanf("%s", command);
        if (strcmp(command, "quit") == 0) {
            char c = QUIT;
            send(fd, &c, sizeof(c), 0);
            break;
        }
        if (strcmp(command, "ls") == 0) {
            char c = LS;
            char path[100];
            scanf("%s", path);
            send(fd, &c, sizeof(c), 0);
            send(fd, path, 100 * sizeof(char), 0);

            size_t recv_size = 0;
            recv(fd, &recv_size, sizeof(recv_size), 0);
            char* list = malloc(recv_size);
            recv(fd, list, recv_size, 0);
            printf("%s\n", list);
            free(list);
        }
        if (strcmp(command, "mkdir") == 0) {
            char c = MKDIR;
            char path[100];
            char name[12];
            scanf("%s%s", path, name);

            send(fd, &c, sizeof(c), 0);
            send(fd, path, 100 * sizeof(char), 0);
            send(fd, name, 12 * sizeof(char), 0);
        }
        if (strcmp(command, "rmdir") == 0) {
            char c = RMDIR;
            char path[100];
            scanf("%s", path);

            send(fd, &c, sizeof(c), 0);
            send(fd, path, 100 * sizeof(char), 0);
        }
        if (strcmp(command, "rm") == 0) {
            char c = RM;
            char path[100];
            scanf("%s", path);

            send(fd, &c, sizeof(c), 0);
            send(fd, path, 100 * sizeof(char), 0);
        }
        if (strcmp(command, "put") == 0) {
            char c = PUT;
            size_t size;
            char path[100];
            char lpath[100];
            char name[12];
            scanf("%s%s%s", lpath, path, name);
            char* file = read_file(lpath, &size);

            send(fd, &c, sizeof(c), 0);
            send(fd, path, 100 * sizeof(char), 0);
            send(fd, name, 12 * sizeof(char), 0);
            send(fd, &size, sizeof(size), 0);
            send(fd, file, size, 0);
            free(file);
        }
        if (strcmp(command, "cat") == 0 || strcmp(command, "get") == 0) {
            char c = GET;
            char path[100];
            scanf("%s", path);

            send(fd, &c, sizeof(c), 0);
            send(fd, path, 100 * sizeof(char), 0);

            size_t size;
            recv(fd, &size, sizeof(size), 0);
            char* file = malloc(size);
            recv(fd, file, size, 0);
            if (command[0] == 'c') {
                printf("%s\n", file);
            } else {
                char lpath[100];
                scanf("%s", lpath);
                FILE* out = fopen(lpath, "w");
                fputs(file, out);
                fclose(out);
            }
            free(file);
        }
    }

    close(fd);
    return 0;
}
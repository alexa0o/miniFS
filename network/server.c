#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"
#include "utils.h"
#include "fs.h"

int main(int argc, char* argv[]) {
    if (fork() == 0) {
        setsid();
        FILE *dev = fopen(argv[1], "r+b");

        if (dev == NULL) {
            perror("fopen(dev)");
            return 1;
        }

        if (open_fs(dev)) {
            create_fs(dev);
            open_fs(dev);
        }

        struct sockaddr_in serv, dest;
        socklen_t socksize = sizeof(struct sockaddr_in);
        int fd;

        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = htonl(INADDR_ANY);
        serv.sin_port = htons(atoi(argv[2]));

        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            perror("socket");
            return 1;
        }
        if (bind(fd, (struct sockaddr *) &serv, sizeof(struct sockaddr)) == -1) {
            perror("bind");
            return 1;
        }

        if (listen(fd, 5) == -1) {
            perror("listen");
            return 1;
        }
        int destfd = accept(fd, (struct sockaddr *) &dest, &socksize);
        if (destfd < 0) {
            perror("accept");
            return 1;
        }
        while (destfd) {
            for (;;) {
                char c;
                recv(destfd, &c, sizeof(c), 0);
                if (c == QUIT) {
                    break;
                }
                if (c == LS) {
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    size_t ressize;
                    char *list = ls(path, &ressize);
                    send(destfd, &ressize, sizeof(ressize), 0);
                    send(destfd, list, ressize * sizeof(char), 0);
                    free(list);
                }
                if (c == MKDIR) {
                    char path[100];
                    char name[12];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    recv(destfd, name, 12 * sizeof(char), 0);
                    mkdir(path, name);
                }
                if (c == RMDIR) {
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    mrmdir(path);
                }
                if (c == RM) {
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);
                    rm(path);
                }
                if (c == PUT) {
                    char path[100];
                    char name[12];
                    size_t size;

                    recv(destfd, path, 100 * sizeof(char), 0);
                    recv(destfd, name, 12 * sizeof(char), 0);
                    recv(destfd, &size, sizeof(size), 0);
                    char *file = malloc(size);
                    recv(destfd, file, size, 0);
                    create(path, name, file, size);
                    free(file);
                }
                if (c == GET) {
                    char path[100];
                    recv(destfd, path, 100 * sizeof(char), 0);

                    char *file = cat_remote(path);
                    size_t size = strlen(file);
                    send(destfd, &size, sizeof(size), 0);
                    send(destfd, file, size, 0);
                    free(file);
                }
            }
            close(destfd);
            destfd = accept(fd, (struct sockaddr *) &dest, &socksize);
        }
    } else {
        printf("Daemon successfully started...\n");
    }
    return 0;
}


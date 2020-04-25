#include <stdio.h>
#include <string.h>
#include "fs.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    printf("Welcome to MiniFS!\n");
    char command[30];
    FILE* dev = fopen(argv[1], "r+b");

    if (dev == NULL) {
        perror("fopen(dev)");
        return 1;
    }

    if(open_fs(dev)) {
        create_fs(dev);
        open_fs(dev);
    }

    for(;;) {
        printf("MiniFS:> ");
        scanf("%s", command);
        if (strcmp(command, "quit") == 0) {
            break;
        }
        if (strcmp(command, "create") == 0) {
            char path[100];
            char name[12];
            char file[100];
            scanf("%s%s%s", path, name, file);
            create(path, name, file, 100);
        }
        if (strcmp(command, "cat") == 0) {
            char path[100];
            scanf("%s", path);
            cat(path);
        }
        if (strcmp(command, "mkdir") == 0) {
            char path[100];
            char name[12];
            scanf("%s%s", path, name);
            mkdir(path, name);
        }
        if (strcmp(command, "ls") == 0) {
            char path[100];
            scanf("%s", path);
            ls(path);
        }
        if (strcmp(command, "rm") == 0) {
            char path[100];
            scanf("%s", path);
            rm(path);
        }
        if (strcmp(command, "rmdir") == 0) {
            char path[100];
            scanf("%s", path);
            rmdir(path);
        }
        if (strcmp(command, "put") == 0) {
            char file[100];
            char path[100];
            scanf("%s%s", file, path);
            put(file, path);
        }
        if (strcmp(command, "get") == 0) {
            char file[100];
            char path[100];
            scanf("%s%s", path, file);
            get(file, path);
        }
    }
    fclose(dev);
    return 0;
}

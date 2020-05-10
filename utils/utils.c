#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../fs/fs.h"

char* ls(char* path, size_t* ressize) {
    size_t indx = find_file(path);
    size_t size = get_inode(indx)->size_of_file;
    dir_record* files = read_file(indx);

    char* buf = malloc(sizeof(char));
    buf[0] = 0;
    size_t bufsize = 1;

    for (int i = 0; i < size / sizeof(dir_record); ++i) {
        bufsize += strlen(files[i].name) + 1;
        buf = realloc(buf, bufsize);
        strcat(buf, files[i].name);
        strcat(buf, "\n");
    }
    free(files);
    if (ressize) {
        *ressize = bufsize;
    }
    return buf;
}

void mkdir(char* path, char* name) {
    size_t parent_indx = find_file(path);
    create_file(NULL, 0, name, parent_indx, DIR);
}

void create(char* path, char* name, void* file, size_t size) {
    size_t parent_indx = find_file(path);
    create_file(file, size, name, parent_indx, TEXT);
}

void cat_impl(char* path, FILE* out, char** buf) {
    size_t indx = find_file(path);
    char* text = read_file(indx);
    text[get_inode(indx)->size_of_file] = 0;
    if (out) {
        fputs(text, out);
        printf("\n");
        free(text);
    } else {
        *buf = text;
    }
}

void cat(char* path) {
    cat_impl(path, stdout, NULL);
}

int get_last_delim(const char* path) {
    int last_delim = 0;
    for (int i = 0; path[i]; ++i) {
        if (path[i] == '/') {
            last_delim = i;
        }
    }
    return last_delim;
}

void remove_impl(char* path, int type) {
    int delim = get_last_delim(path);
    char parent[100];
    strncpy(parent, path, delim);
    size_t parent_indx = find_file(parent);
    size_t indx = find_file(path);
    if (get_inode(indx)->type_of_file == type) {
        return;
    }
    remove_file(indx, parent_indx);
}

void rm(char* path) {
    remove_impl(path, DIR);
}

void mrmdir(char* path) {
    remove_impl(path, TEXT);
}

void put(char* file_name, char* new_path) {
    FILE *f = fopen(file_name, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;

    int delim = get_last_delim(new_path);
    char parent[100];
    char* name = new_path + delim + 1;
    strncpy(parent, new_path, delim);

    size_t pindx = find_file(parent);

    create_file(string, fsize, name, pindx, TEXT);
    free(string);
}

void get(char* out_file, char* path) {
    FILE* out = fopen(out_file, "w");
    cat_impl(path, out, NULL);
    fclose(out);
}

char* cat_remote(char* path) {
    char* buf;
    cat_impl(path, NULL, &buf);
    return buf;
}
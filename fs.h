#ifndef MINIFS_FS_H
#define MINIFS_FS_H
#include <stdio.h>

#define TEXT 1
#define DIR 2

#define NUMBER_OF_PTRS 5
#define MAGIC_NUMBER 1337
#define NUMBER_OF_INODES 16
#define BLOCK_SIZE 1024
#define NUMBER_OF_BLOCKS 64

typedef struct {
    int type_of_file;
    size_t size_of_file;
    long pointers[NUMBER_OF_PTRS + 1];
} inode;

typedef struct {
    int index;
    char name[12];
} dir_record;

int open_fs(FILE*);
void create_fs(FILE*);

void create_file(void*, size_t, char*, size_t, int);
void remove_file(size_t, size_t);
size_t find_file(char*);
void* read_file(size_t);
inode* get_inode(size_t);

#endif //MINIFS_FS_H

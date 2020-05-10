#ifndef MINIFS_UTILS_H
#define MINIFS_UTILS_H

char* ls(char*, size_t*);
void mkdir(char*, char*);
void create(char*, char*, void*, size_t);
void cat(char*);
void rm(char*);
void mrmdir(char*);
void put(char*, char*);
void get(char*, char*);
char* cat_remote(char*);

#endif //MINIFS_UTILS_H

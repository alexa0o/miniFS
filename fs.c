#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "fs.h"

typedef struct {
    size_t magic_number;
    size_t number_of_blocks;
    long size_of_block;
    size_t size_of_inode_table;
    long head_of_blocks;
    long long bitmap_inodes;
} super_block;

super_block sb;
inode inode_table[NUMBER_OF_INODES];
FILE* dev;

void fs_read(long pos, void* ptr, size_t size, size_t nmemb) {
    fseek(dev, pos, SEEK_SET);
    fread(ptr, size, nmemb, dev);
}

void fs_write(long pos, void* ptr, size_t size, size_t nmemb) {
    fseek(dev, pos, SEEK_SET);
    fwrite(ptr, size, nmemb, dev);
}

void save_dev() {
    fs_write(0, &sb, sizeof(super_block), 1);
    fs_write(BLOCK_SIZE, inode_table, sizeof(inode), NUMBER_OF_INODES);
}

long get_block() {
    long block = sb.head_of_blocks;
    fseek(dev, block, SEEK_SET);
    fread(&sb.head_of_blocks, sizeof(block), 1, dev);
    return block;
}

int open_fs(FILE* ndev) {
    dev = ndev;
    fs_read(0, &sb, sizeof(sb), 1);
    if (sb.magic_number != MAGIC_NUMBER) {
        perror("Invalid superblock!\n");
        return 1;
    }
    fs_read(sb.size_of_block, inode_table, sizeof(inode), NUMBER_OF_INODES);
    return 0;
}

void make_block_list(long begin) {
    fseek(dev, begin, SEEK_SET);
    for (long i = 0; i < sb.number_of_blocks; ++i) {
        long next_pos = begin + (i + 1) * sb.size_of_block;
        fwrite(&next_pos, sizeof(next_pos), 1, dev);
        fseek(dev, sb.size_of_block - sizeof(next_pos), SEEK_CUR);
    }
}

long* write_ptrs(size_t size, void* file) {
    size_t count_ptrs = (size + sb.size_of_block) / sb.size_of_block;
    long* ptrs = malloc((count_ptrs + 1) * sizeof(size_t));
    for (size_t i = 0; i < count_ptrs; ++i) {
        long next_block = get_block();
        size_t wsize = size < sb.size_of_block ? size : sb.size_of_block;
        fs_write(next_block, file, wsize, 1);
        size -= wsize;
        file += wsize;
        ptrs[i] = next_block;
    }
    ptrs[count_ptrs] = 0;
    return ptrs;
}

void read_ptrs(long* ptrs, size_t size, void* file) {
    size_t count_ptrs = (size + sb.size_of_block) / sb.size_of_block;
    for (size_t i = 0; i < count_ptrs; ++i) {
        size_t rsize = size < sb.size_of_block ? size : sb.size_of_block;
        fs_read(ptrs[i], file, rsize, 1);
        file += rsize;
        size -= rsize;
    }
}

void write_file(size_t indx, void* file) {
    inode* node = &inode_table[indx];
    size_t size = node->size_of_file;
    long* ptrs = write_ptrs(size, file);
    size_t count_ptrs = (size + sb.size_of_block) / sb.size_of_block;
    for (size_t i = 0; i < NUMBER_OF_PTRS; ++i) {
        node->pointers[i] = ptrs[i];
    }
    if (count_ptrs < NUMBER_OF_PTRS) {
        node->pointers[NUMBER_OF_PTRS] = 0;
    } else {
        long new_block = get_block();
        node->pointers[NUMBER_OF_PTRS] = new_block;
        fs_write(new_block, ptrs + NUMBER_OF_PTRS, sizeof(long), count_ptrs - NUMBER_OF_PTRS + 1);
    }
    free(ptrs);
}

void* read_file(size_t indx) {
    inode* node = &inode_table[indx];
    void* file = malloc(node->size_of_file + 1);
    if (node->pointers[NUMBER_OF_PTRS] == 0) {
        read_ptrs(node->pointers, node->size_of_file, file);
    } else {
        long* ptrs = malloc((sb.size_of_block + NUMBER_OF_PTRS) * sizeof(long));
        for (int i = 0; i < NUMBER_OF_PTRS; ++i) {
            ptrs[i] = node->pointers[i];
        }
        fs_read(node->pointers[NUMBER_OF_PTRS], ptrs + NUMBER_OF_PTRS, sb.size_of_block, 1);
        read_ptrs(ptrs, node->size_of_file, file);
        free(ptrs);
    }
    return file;
}

void create_root() {
    dir_record root[2];
    size_t indx = set_index(&sb.bitmap_inodes);
    inode* node = &inode_table[indx];
    node->type_of_file = DIR;
    root[0].index = indx;
    root[1].index = indx;
    strcpy(root[0].name, ".");
    strcpy(root[1].name, "..");
    node->size_of_file = 2 * sizeof(dir_record);
    write_file(indx, root);
}

void create_file(void* file, size_t size, char* name, size_t parent_indx, int type) {
    size_t indx = set_index(&sb.bitmap_inodes);
    inode* node = &inode_table[indx];
    node->type_of_file = type;
    if (type == DIR) {
        dir_record root[2];
        root[0].index = indx;
        root[1].index = parent_indx;
        strcpy(root[0].name, ".");
        strcpy(root[1].name, "..");
        node->size_of_file = 2 * sizeof(dir_record);
        write_file(indx, root);
    } else {
        node->size_of_file = size;
        write_file(indx, file);
    }

    // add to parent directory
    dir_record new_record;
    new_record.index = indx;
    strcpy(new_record.name, name);
    dir_record* parent = read_file(parent_indx);
    inode* parent_node = &inode_table[parent_indx];
    size_t new_size = parent_node->size_of_file + sizeof(dir_record);
    parent = realloc(parent, new_size);
    parent[parent_node->size_of_file / sizeof(dir_record)] = new_record;
    size_t count_blocks = (parent_node->size_of_file + sb.size_of_block) / sb.size_of_block;
    size_t new_count_blocks = (new_size + sb.size_of_block) / sb.size_of_block;
    parent_node->size_of_file = new_size;

    dir_record* tmp = parent;
    int i;
    for (i = 0; i < NUMBER_OF_PTRS && parent_node->pointers[i]; ++i) {
        size_t wsize = new_size > sb.size_of_block ? sb.size_of_block : new_size;
        new_size -= wsize;
        fs_write(parent_node->pointers[i], tmp, wsize, 1);
        tmp += wsize;
    }
    if (new_count_blocks > count_blocks) {
        long new_block = get_block();
        fs_write(new_block, tmp, new_size, 1);
        parent_node->pointers[i] = new_block;
    }
    free(parent);
    save_dev();
}

void remove_file(size_t indx, size_t parent_indx) {
    inode* node = &inode_table[indx];
    int i;
    for (i = 0; i < (NUMBER_OF_PTRS - 1) && node->pointers[i + 1]; ++i) {
        fs_write(node->pointers[i], &node->pointers[i + 1], sizeof(long), 1);
    }
    if (node->pointers[NUMBER_OF_PTRS] != 0) {
        long* ptrs = malloc(sb.size_of_block / sizeof(long) * sizeof(long));
        fs_read(node->pointers[NUMBER_OF_PTRS], ptrs, sb.size_of_block, 1);
        int j;
        for (j = 0; ptrs[j + 1]; ++j) {
            fs_write(ptrs[j], &ptrs[j + 1], sizeof(long), 1);
        }
        fs_write(node->pointers[NUMBER_OF_PTRS - 1], &ptrs[0], sizeof(long), 1);
        fs_write(ptrs[j], &sb.head_of_blocks, sizeof(long), 1);
        free(ptrs);
    } else {
        fs_write(node->pointers[i], &sb.head_of_blocks, sizeof(long), 1);
    }
    sb.head_of_blocks = node->pointers[0];
    free_index(&sb.bitmap_inodes, indx);

    // delete from parent catalog
    inode* parent = &inode_table[parent_indx];
    dir_record* files = read_file(parent_indx);
    dir_record* new_files = malloc(sizeof(dir_record) * (parent->size_of_file - sizeof(dir_record)));
    int j = 0;
    for (i = 0; i < parent->size_of_file / sizeof(dir_record); ++i) {
        if (files[i].index == indx) {
            continue;
        }
        new_files[j++] = files[i];
    }
    free(files);
    dir_record* tmp = new_files;
    size_t new_size = parent->size_of_file - sizeof(dir_record);
    parent->size_of_file = new_size;
    for (i = 0; parent->pointers[i]; ++i) {
        size_t wsize = new_size > sb.size_of_block ? sb.size_of_block : new_size;
        new_size -= wsize;
        fs_write(parent->pointers[i], tmp, wsize, 1);
        tmp += wsize;
    }
    save_dev();
}

size_t find_file(char* path) {
    size_t indx = 0;
    strtok(path, "/");
    for (char* next = strtok(NULL, "/"); next; next = strtok(NULL, "/")) {
        dir_record* files = read_file(indx);
        int flag = 0;
        for (int i = 0; i < inode_table[indx].size_of_file / sizeof(dir_record); ++i) {
            if (strcmp(files[i].name, next) == 0) {
                indx = files[i].index;
                flag = 1;
                break;
            }
        }
        if (flag) {
            continue;
        }
        return 0;
    }
    return indx;
}

void create_fs(FILE* ndev) {
    dev = ndev;
    sb.number_of_blocks = NUMBER_OF_BLOCKS;
    sb.magic_number = MAGIC_NUMBER;
    sb.size_of_block = BLOCK_SIZE;
    sb.bitmap_inodes = 0;
    sb.size_of_inode_table = NUMBER_OF_INODES * sizeof(inode);
    sb.head_of_blocks = 2 * BLOCK_SIZE;

    make_block_list(2 * BLOCK_SIZE);
    create_root();

    save_dev();
}

inode* get_inode(size_t indx) {
    return &inode_table[indx];
}
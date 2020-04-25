#ifndef MINIFS_BITMAP_H
#define MINIFS_BITMAP_H

size_t set_index(long long* bitmap) {
    size_t index = 0;
    long long i;
    for (i = 1; *bitmap & i; i <<= 1) {
        ++index;
    }
    *bitmap += i;
    return index;
}

void free_index(long long* bitmap, size_t index) {
    *bitmap -= 1ll << index;
}

#endif //MINIFS_BITMAP_H

#include <stddef.h>

extern void *heap_start;
extern void *heap_end;

void *malloc(size_t size) {
    char* r = heap_start;
    size = (size + 0xFU) & (~0xFU);
    if ((unsigned)((char*)heap_end - r) >= size) {
        heap_start = r + size;
        return r;
    }
    return 0;
}

void free(void *ptr) {
}

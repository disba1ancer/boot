#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

static void memswp(void * restrict a, void * restrict b, size_t size) {
    byte *dst = a;
    byte *src = b;
    for (size_t i = 0; i < size; ++i, ++dst, ++src) {
        byte t = *dst;
        *dst = *src;
        *src = t;
    }
}

void qsort(void* ptr_, size_t count, size_t size, int (*comp)(const void*, const void*))
{
    if (count <= 1) {
        return;
    }
    byte *ptr = ptr_;
    size_t cLeft = 0;
    size_t cRight = count;
    size_t center = cRight / 2;
    while(cLeft < cRight) {
        while (comp(ptr + cLeft * size, ptr + center * size) < 0) {
            cLeft += 1;
        }
        if (cLeft == cRight) {
            break;
        }
        while (comp(ptr + (cRight - 1) * size, ptr + center * size) >= 0) {
            cRight -= 1;
        }
        if (cLeft == cRight) {
            break;
        }
        memswp(ptr + cLeft * size, ptr + (cRight - 1) * size, size);
        if (cLeft == center) {
            center = cRight - 1;
        }
    };
    memswp(ptr + cLeft * size, ptr + center * size, size);
    qsort(ptr, cLeft, size, comp);
    cLeft += 1;
    qsort(ptr + cLeft * size, count - cLeft, size, comp);
}

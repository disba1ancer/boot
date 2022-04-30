#include <stddef.h>

//void *memcpy(void * restrict a_dest, const void * restrict a_src, size_t a_size)
//{
//    unsigned char* restrict dest = a_dest;
//    const unsigned char* restrict src = a_src;
//    while (a_size) {
//        *(dest++) = *(src++);
//        --a_size;
//    }
//    return a_dest;
//}

//int strcmp(const char * a_dest, const char * a_src)
//{
//    for (; (*a_dest) && (*a_dest == *a_src); ++a_dest, ++a_src) {}
//    return (unsigned char)(*a_dest) - (unsigned char)(*a_src);
//}

//int strlen(const char * str)
//{
//    const char* p_str = str;
//    for (; (*p_str); ++p_str) {}
//    return p_str - str;
//}

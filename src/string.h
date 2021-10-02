#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include "btstdbeg.h"

void* memset(void *dst, int val, size_t size);
void* memcpy(void * BOOT_STD_RESTRICT dst, const void * BOOT_STD_RESTRICT src, size_t size);
int strcmp(const char *dst, const char *src);
size_t strlen(char *dst);

#include "btstdend.h"

#endif // STRING_H

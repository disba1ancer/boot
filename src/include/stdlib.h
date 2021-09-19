#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#include "btstdbeg.h"

BOOT_STD_NORETURN void abort(void);
void *malloc(size_t size);
void *realloc(void* ptr, size_t size);
void free(void *ptr);

#include "btstdend.h"

#endif // STDLIB_H

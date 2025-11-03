#include <new>
#include <stdlib.h>

void *operator new(size_t size)
{
    auto ptr = malloc(size);
    if (ptr == nullptr) {
        abort();
    }
    return ptr;
}

void operator delete(void *ptr) noexcept
{
    return free(ptr);
}

void *operator new[](size_t size)
{
    return operator new(size);
}

void operator delete[](void *ptr) noexcept
{
    return operator delete(ptr);
}

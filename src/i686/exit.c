#include "bt_util.h"
#include <stdint.h>
#include <stdlib.h>

typedef void cxa_atexit_func(void*);

BOOT_STRUCT(cxa_atexit_entry) {
    cxa_atexit_func* func;
    void *arg;
    void *dso;
};

BOOT_STRUCT(cxa_atexit_array) {
    size_t allocated;
    size_t used;
    cxa_atexit_entry entries[];
};

extern void *__dso_handle;

static cxa_atexit_array *handlers;

void _fini(void);

int __cxa_atexit(cxa_atexit_func *func, void *arg, void *dso)
{
    if (handlers == NULL) {
        handlers = malloc(sizeof(cxa_atexit_array) + sizeof(cxa_atexit_entry[2]));
        if (handlers == NULL) {
            return 1;
        }
        handlers->allocated = 2;
        handlers->used = 0;
    }
    if (handlers->used == handlers->allocated) {
        size_t newAllocated = handlers->allocated + handlers->allocated / 2;
        cxa_atexit_array *newHandlers;
        newHandlers = realloc(handlers, sizeof(cxa_atexit_array) + sizeof(cxa_atexit_entry[newAllocated]));
        if (newHandlers == NULL) {
            return 1;
        }
        handlers = newHandlers;
    }
    handlers->entries[handlers->used++] = (cxa_atexit_entry) {
        .func = func,
        .arg = arg,
        .dso = dso,
    };
    return 0;
}

static void call_entry(cxa_atexit_entry *entry)
{
    if (entry->arg != NULL) {
        (entry->func)(entry->arg);
    } else {
        ((void(*)(void))entry->func)();
    }
}

void __cxa_finalize(void *dso)
{
    if (handlers == NULL || handlers->used == 0) {
        return;
    }
    size_t current = handlers->used;
    if (dso == NULL) {
        while (current--) {
            cxa_atexit_entry *entry = handlers->entries + current;
            call_entry(entry);
        }
    } else {
        while (current--) {
            cxa_atexit_entry *entry = handlers->entries + current;
            if (entry->dso == dso) {
                call_entry(entry);
                entry->func = NULL;
            }
        }
        for (size_t i = 0; i < handlers->used; ++i) {
            cxa_atexit_entry *entry = handlers->entries + i;
            if (entry->func == NULL) {
                continue;
            }
            if (current != i) {
                handlers->entries[current] = *entry;
            }
            ++current;
        }
        handlers->used = current;
    }
}

void exit(int exit_code)
{
    _fini();
    __cxa_finalize(0);
    _Exit(exit_code);
}

void atexit(void(*func)(void))
{
    __cxa_atexit((cxa_atexit_func*)func, NULL, __dso_handle);
}

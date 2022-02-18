#ifndef BT_UTIL_H
#define BT_UTIL_H

#include <stddef.h>
#include <stdlib.h>

#define BOOT_STRUCT(name) \
    typedef struct name name; \
    struct name

#define BOOT_TYPEDEF_STRUCT(name) \
    typedef struct name name; \
    typedef struct name

BOOT_STRUCT(DoublyLinkedListElement) {
    DoublyLinkedListElement *next;
    DoublyLinkedListElement *prev;
};

BOOT_STRUCT(DoublyLinkedList) {
    DoublyLinkedListElement *begin;
};

//void DoublyLinkedList_Add(DoublyLinkedList *list, DoublyLinkedListElement *elem);
//void DoublyLinkedList_Remove(DoublyLinkedList *list, DoublyLinkedListElement *elem);

#ifdef __cplusplus
extern "C" {
#endif

inline void DoublyLinkedList_Add(DoublyLinkedList *list, DoublyLinkedListElement *elem)
{
    elem->prev = NULL;
    elem->next = list->begin;
    list->begin = elem;
    if (elem->next != NULL) {
        elem->next->prev = elem;
    }
}

inline void DoublyLinkedList_Remove(DoublyLinkedList *list, DoublyLinkedListElement *elem)
{
    if (elem->prev != NULL) {
        elem->prev->next = elem->next;
    } else {
        list->begin = elem->next;
    }
    if (elem->next != NULL) {
        elem->next->prev = elem->prev;
    }
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BT_UTIL_H

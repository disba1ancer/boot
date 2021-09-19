#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "membios.h"

#define BOOT_ALLOC_BLOCK_LOG_SIZE ((size_t)4U)
#define BOOT_ALLOC_BLOCK_SIZE (((size_t)1U) << BOOT_ALLOC_BLOCK_LOG_SIZE)
#define BOOT_ALLOC_BLOCK_BIT_SIZE (CHAR_BIT * BOOT_ALLOC_BLOCK_SIZE)

static void *AllocFromRange(void **start, void *end, size_t size);
static void *boot_LinearAlloc(size_t size);
static int bsrszt(size_t val);

struct DoublyLinkedListElement {
    struct DoublyLinkedListElement *next;
    struct DoublyLinkedListElement *prev;
};

struct DoublyLinkedList {
    struct DoublyLinkedListElement *begin;
};

static void DoublyLinkedList_Add(struct DoublyLinkedList *list, struct DoublyLinkedListElement *elem);
static void DoublyLinkedList_Remove(struct DoublyLinkedList *list, struct DoublyLinkedListElement *elem);

void boot_InitBuddyAlloc();

struct boot_BuddyFreeBlockHeader {
    struct DoublyLinkedListElement elem;
};

struct boot_BuddyBlockStore {
    struct DoublyLinkedList freeList;
    size_t pairBitmapStart;
};

/* TODO: Add locks for working in multithread and with interrupts */
struct boot_BuddyRegion {
    void *start;
    void *end;
    unsigned *bitmap;
    size_t maxOrder;

    struct boot_BuddyBlockStore buddyArray[];
};

static struct boot_BuddyRegion *boot_BuddyRegion_Construct(void *regionStart, void *regionEnd);
static void boot_BuddyRegion_Construct_ph2(struct boot_BuddyRegion *region, void* ptr);
static int boot_BuddyRegion_TogglePairBit(struct boot_BuddyRegion *region, void *block, size_t order);
static void *boot_BuddyRegion_AllocBlock(struct boot_BuddyRegion *region, size_t order);
static void boot_BuddyRegion_FreeBlock(struct boot_BuddyRegion *region, void *block, size_t order);

extern void *heap_start;
extern void *heap_end;

static struct boot_BuddyRegion *allocRegion;

void *AllocFromRange(void **start, void *end, size_t size)
{
    char* r = *start;
    size = (size + BOOT_ALLOC_BLOCK_SIZE - 1) & (~(BOOT_ALLOC_BLOCK_SIZE - 1));
    if ((size_t)((char*)end - r) >= size) {
        *start = r + size;
        return r;
    }
    return 0;
}

void *boot_LinearAlloc(size_t size)
{
    return AllocFromRange(&heap_start, heap_end, size);
}

int bsrszt(size_t val)
{
    if (val == 0) {
        return (sizeof(val) * CHAR_BIT);
    }
    int result = 0;
    while (val > 1) {
        ++result;
        val >>= 1;
    }
    return result;
}

static uintptr_t AlignPtrUp(uintptr_t val, unsigned logAlign)
{
    unsigned mask = ((1U << logAlign) - 1U);
    return (val + mask) & ~mask;
}

static uintptr_t AlignPtrDown(uintptr_t val, unsigned logAlign)
{
    unsigned mask = ((1U << logAlign) - 1U);
    return val & ~mask;
}

int toggleBit(unsigned *val, unsigned bitNum)
{
    unsigned mask = 1U << bitNum;
    *val = *val ^ mask;
    return !!(*val & mask);
}

void DoublyLinkedList_Add(struct DoublyLinkedList *list, struct DoublyLinkedListElement *elem)
{
    elem->prev = NULL;
    elem->next = list->begin;
    list->begin = elem;
    if (elem->next != NULL) {
        elem->next->prev = elem;
    }
}

void DoublyLinkedList_Remove(struct DoublyLinkedList *list, struct DoublyLinkedListElement *elem)
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

void boot_InitBuddyAlloc()
{
    struct boot_BuddyRegion *initialRegion = boot_BuddyRegion_Construct(heap_start, heap_end);
    allocRegion = initialRegion;
//  TODO: Add additional regions from int 0x15 AX=0xE820
}

struct boot_BuddyRegion *boot_BuddyRegion_Construct(void *regionStart, void *regionEnd)
{
    void *current = regionStart;
    size_t region_size = (size_t)((char*)regionEnd - (char*)current);
    if (region_size == 0) {
        abort();
    }
    size_t regionBlockCount = region_size / BOOT_ALLOC_BLOCK_SIZE;

    size_t maxOrder = (size_t)bsrszt(regionBlockCount * 2 - 1);
    struct boot_BuddyRegion *buddyRegion;
    size_t buddyRegionSize = sizeof(struct boot_BuddyRegion) + maxOrder * sizeof(struct boot_BuddyBlockStore);
    buddyRegion = AllocFromRange(&current, regionEnd, buddyRegionSize);
    memset(buddyRegion, 0, buddyRegionSize);

    buddyRegion->start = regionStart;
    buddyRegion->end = regionEnd;
    buddyRegion->maxOrder = maxOrder;

    size_t regionBitmapSize = AlignPtrUp(regionBlockCount, CHAR_BIT) / CHAR_BIT;
    buddyRegion->bitmap = AllocFromRange(&current, regionEnd, regionBitmapSize);
    memset(buddyRegion->bitmap, 0, regionBitmapSize);

    buddyRegion->buddyArray[0].pairBitmapStart = (size_t)-1;
    size_t bitmapStart = 0;
    for (int i = 0; i < (int)maxOrder; ++i) {
        buddyRegion->buddyArray[i].pairBitmapStart = bitmapStart;
        size_t half_bmp_size = regionBlockCount / 2;
        bitmapStart += half_bmp_size + (regionBlockCount & 1);
        regionBlockCount = half_bmp_size;
    }
    boot_BuddyRegion_Construct_ph2(buddyRegion, current);
    return buddyRegion;
}

/* TODO: Check working of this algorithm */
void boot_BuddyRegion_Construct_ph2(struct boot_BuddyRegion *region, void* current)
{
    size_t size = (size_t)((char*)region->end - (char*)current);
    size_t memBlockOffset = (size_t)((char*)current - (char*)region->start);
    size_t order = BOOT_ALLOC_BLOCK_SIZE;
    size_t i;
    for (i = 0; i < region->maxOrder && size >= order; ++i, order <<= 1) {
        if (memBlockOffset & order) {
            boot_BuddyRegion_FreeBlock(region, (char*)region->start + memBlockOffset, i);
            memBlockOffset += order;
            size -= order;
        }
    }
    for (; i <= region->maxOrder; i--, order >>= 1) {
        if (size >= order) {
            boot_BuddyRegion_FreeBlock(region, (char*)region->start + memBlockOffset, i);
            memBlockOffset += order;
            size -= order;
        }
    }
    return;
}

int boot_BuddyRegion_TogglePairBit(struct boot_BuddyRegion *region, void *block, size_t order)
{
    if (order < region->maxOrder - 1){
        size_t pairNum = ((size_t)((char*)block - (char*)region->start) / BOOT_ALLOC_BLOCK_SIZE) >> (order + 1);
        size_t bitNum = region->buddyArray[order].pairBitmapStart + pairNum;
        size_t bitmapBlock = bitNum / (CHAR_BIT * sizeof(unsigned));
        size_t bitNumInBlock = bitNum % (CHAR_BIT * sizeof(unsigned));
        return toggleBit(region->bitmap + bitmapBlock, bitNumInBlock);
    }
    return !!(1);
}

void *boot_BuddyRegion_AllocBlock(struct boot_BuddyRegion *region, size_t order)
{
    if (order >= region->maxOrder || order >= sizeof(unsigned) * CHAR_BIT) {
        return NULL;
    }
    struct DoublyLinkedList *list = &(region->buddyArray[order].freeList);
    struct DoublyLinkedListElement *elem;
    elem = list->begin;
    if (elem != NULL) {
        DoublyLinkedList_Remove(list, elem);
        boot_BuddyRegion_TogglePairBit(region, elem, order);
        return elem;
    } else {
        void *largeBlock = boot_BuddyRegion_AllocBlock(region, order + 1);
        if (largeBlock != NULL) {
            DoublyLinkedList_Add(list, (void*)((char*)largeBlock + (BOOT_ALLOC_BLOCK_SIZE << order)));
            boot_BuddyRegion_TogglePairBit(region, largeBlock, order);
        }
        return largeBlock;
    }
    return NULL;
}

void boot_BuddyRegion_FreeBlock(struct boot_BuddyRegion *region, void *block, size_t order)
{
    size_t blockOffset = (size_t)((char*)block - (char*)region->start);
    if (!boot_BuddyRegion_TogglePairBit(region, block, order)) {
        struct boot_BuddyFreeBlockHeader *block;
        size_t mask = BOOT_ALLOC_BLOCK_SIZE << order;
        block = (void*)((char*)region->start + (blockOffset ^ mask));
        DoublyLinkedList_Remove(&(region->buddyArray[order].freeList), &(block->elem));
        void *upOrderBlock = (char*)region->start + (blockOffset & (~mask));
        boot_BuddyRegion_FreeBlock(region, upOrderBlock, order + 1);

        return;
    }
    DoublyLinkedList_Add(&(region->buddyArray[order].freeList), block);
}

void *malloc(size_t size)
{
    size = AlignPtrUp(size, BOOT_ALLOC_BLOCK_LOG_SIZE);
    size = size / BOOT_ALLOC_BLOCK_SIZE;
    size_t order = (size_t)bsrszt(size * 2 - 1);
    char* block = boot_BuddyRegion_AllocBlock(allocRegion, order);
    if (block == NULL) {
        return NULL;
    }
    *((size_t*)block) = order;
    return block + BOOT_ALLOC_BLOCK_SIZE;
}

static void *tryRealloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return NULL;
    }
    size = AlignPtrUp(size, BOOT_ALLOC_BLOCK_LOG_SIZE);
    size = size / BOOT_ALLOC_BLOCK_SIZE;
    size_t reqOrder = (size_t)bsrszt(size * 2 - 1);
    char *block = (char*)ptr - BOOT_ALLOC_BLOCK_SIZE;
    size_t order = *((size_t*)block);
    return reqOrder <= order ? ptr : NULL;
}

void *realloc(void* ptr, size_t size)
{
    void *newPtr = tryRealloc(ptr, size);
    if (newPtr == NULL) {
        newPtr = malloc(size);
        if (ptr != NULL) {
            char *block = (char*)ptr - BOOT_ALLOC_BLOCK_SIZE;
            size_t order = *((size_t*)block);
            memcpy(newPtr, ptr, BOOT_ALLOC_BLOCK_SIZE << order);
        }
    }
    return newPtr;
}

void free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    void* block = (char*)ptr - BOOT_ALLOC_BLOCK_SIZE;
    size_t order = *((size_t*)block);
    boot_BuddyRegion_FreeBlock(allocRegion, block, order);
}

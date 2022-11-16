#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>
#include "membios.h"
#include "boot/util.h"
#include "alloc.h"
#include "boot/virtual_alloc.h"

#define BOOT_ALLOC_BLOCK_SIZE ((size_t)16U)

_Static_assert(BOOT_ALLOC_BLOCK_SIZE >= alignof(max_align_t), "Fundamental alignment stricter than BOOT_ALLOC_BLOCK_SIZE");

typedef unsigned char byte;

static void *AllocFromRange(void **start, void *end, size_t size);
static void *AllocFromRangeAligned(void **start, void *end, size_t size, size_t align);
static void *AlignRangeStart(void *start, void *end);
static void *boot_LinearAlloc(size_t size);
static uintptr_t AlignUp(uintptr_t val, size_t align);
static void *AlignPtrUp(void *val, size_t align);
static uintptr_t AlignDown(uintptr_t val, size_t align);
static void *AlignPtrDown(void *val, size_t align);
static int toggleBit(unsigned char *val, size_t bitNum);

BOOT_STRUCT(boot_BuddyFreeBlockHeader) {
    boot_DoublyLinkedListElement elem;
};

BOOT_STRUCT(boot_BuddyBlockStore) {
    boot_DoublyLinkedList freeList;
    size_t pairBitmapStart;
};

/* TODO: Add locks for working in multithread and with interrupts */
BOOT_STRUCT(boot_BuddyRegion) {
    void *start;
    void *end;
    unsigned char *bitmap;
    size_t maxOrder;

    struct boot_BuddyBlockStore buddyArray[];
};

static boot_BuddyRegion *boot_BuddyRegion_Construct(void *regionStart, void *regionEnd);
static void boot_BuddyRegion_Construct_phase2(boot_BuddyRegion *region, void* ptr);
static int boot_BuddyRegion_TogglePairBit(boot_BuddyRegion *region, void *block, size_t order);
static void *boot_BuddyRegion_AllocBlock(boot_BuddyRegion *region, size_t order);
static void boot_BuddyRegion_FreeBlock(boot_BuddyRegion *region, void *block, size_t order);

void *heap_start = __bss_end;
byte *heap_end;

static boot_BuddyRegion *allocRegion;

void *AllocFromRange(void **start, void *end, size_t size)
{
    return AllocFromRangeAligned(start, end, size, BOOT_ALLOC_BLOCK_SIZE);
}

//void *AllocFromRangePacked(void **start, void *end, size_t size)
//{
//    byte* r = (byte*)(*start);
//    if ((size_t)((byte*)end - r) >= size) {
//        *start = (void*)(r + size);
//        return (void*)r;
//    }
//    return 0;
//}

void *AllocFromRangeAligned(void **start, void *end, size_t size, size_t align)
{
    uintptr_t r = AlignUp((uintptr_t)(*start), align);
    if (r < (uintptr_t)end && (uintptr_t)end - r > size) {
        *start = (void*)(r + size);
        return (void*)r;
    }
    return NULL;
}

 #define ALIGN_HELPER(name) sizeof(name), alignof(name)
 #define ALIGN_HELPER2(name, count) sizeof(name) * (count), alignof(name)

void *AlignRangeStart(void *start, void *end)
{
    uintptr_t alignedStart = AlignUp((uintptr_t)start, BOOT_ALLOC_BLOCK_SIZE);
    return (alignedStart < (uintptr_t)end) ? (void*)alignedStart : NULL;
}

void *boot_LinearAlloc(size_t size)
{
    return AllocFromRange(&heap_start, heap_end, size);
}

uintptr_t AlignUp(uintptr_t val, size_t align)
{
    uintptr_t mask = align - 1;
    return (val + mask) & ~mask;
}

void *AlignPtrUp(void *val, size_t align)
{
    return (void*)AlignUp((uintptr_t)val, align);
}

uintptr_t AlignDown(uintptr_t val, size_t align)
{
    uintptr_t mask = align - 1;
    return val & ~mask;
}

void *AlignPtrDown(void *val, size_t logAlign)
{
    return (void*)AlignDown((uintptr_t)val, logAlign);
}

int toggleBit(unsigned char *val, size_t bitNum)
{
    unsigned char mask = (unsigned char)(1U << bitNum);
    *val = *val ^ mask;
    return !!(*val & mask);
}

static int calc_flag_weight(const boot_MemoryMapEntry *entry)
{
    return (entry->type == i686_bios_mem_AvailableMemory) &&
        ((entry->flags & 0xF) == 1);
}

static int reg_comp(const void *a, const void *b) {
    const boot_MemoryMapEntry *left = a;
    const boot_MemoryMapEntry *right = b;
    int leftk = calc_flag_weight(left);
    int rightk = calc_flag_weight(right);
    if (leftk == rightk) {
        if (left->begin < right->begin) {
            return -1;
        } else if (left->begin == right->begin) {
            return 0;
        }
        return 1;
    }
    return leftk - rightk;
}

static boot_MemoryMap *FakeMemmapRegions(void **mem_start, void *end);
static boot_MemoryMap *PrepareMemmapRegions(void **mem_start, void *end);
static size_t MergeNormalRegions(boot_MemoryMap *memmap);
static void ExcludeAbnormalRegions(boot_MemoryMap *memmap, size_t count, void **mem_start, void *end);

static boot_MemoryMapEntry fakeMap[] = {
//    {0x9FC00, 0x400, 2, 1},
//    {0x500, 0x7000, 1, 1},
//    {0x7000, 0xF9000, 1, 1},
//    {0x8000, 0x1000, 1, 1},
//    {0x100000, 0x300000, 1, 1},
    {0x300, 0xFD00, 1, 1},
    {0x200000, 0x100000, 1, 1},
    {0x20000, 0xE0000, 2, 1},
    {0x240000, 0x80000, 2, 1},
    {0x400000, 0x100000, 2, 1},
    {0x8000, 0x18000, 2, 1},
    {0x0, 0x351, 2, 1},
    {0x18000, 0x2000, 1, 1},

};

void boot_InitBuddyAlloc()
{
    void *mem_start = heap_start;
    boot_MemoryMap *memmap = PrepareMemmapRegions(&mem_start, heap_end);
//    memmap->count = MergeNormalRegions(memmap);
    ExcludeAbnormalRegions(memmap, MergeNormalRegions(memmap), &mem_start, heap_end);
    mem_start = AlignRangeStart(mem_start, heap_end);
    boot_BuddyRegion *initialRegion = boot_BuddyRegion_Construct(mem_start, heap_end);
    allocRegion = initialRegion;
    boot_InitVirtualAlloc();
}

static boot_MemoryMapEntry* ReadEntries(uint64_t ref)
{
    return (void*)(uintptr_t)ref;
}

boot_MemoryMap *FakeMemmapRegions(void **mem_start, void *end)
{
    boot_MemoryMap *memmap;
//    memmap = AllocFromRangePacked(mem_start, end, sizeof(i686_mem_entries) + sizeof(fakeMap));
    memmap = AllocFromRangeAligned(mem_start, end, ALIGN_HELPER(boot_MemoryMap));
    if (memmap == NULL) {
        abort();
    }
    memmap->count = sizeof(fakeMap) / sizeof(boot_MemoryMapEntry);
    boot_MemoryMapEntry *entries = AllocFromRangeAligned(mem_start, end,
        ALIGN_HELPER2(boot_MemoryMap, (size_t)memmap->count));
    if (entries == NULL) {
        abort();
    }
    memmap->entries = (uintptr_t)entries;
    memcpy(entries, fakeMap, sizeof(fakeMap));
    qsort(entries, (size_t)memmap->count,
        sizeof(boot_MemoryMapEntry), reg_comp);
    return memmap;
}

boot_MemoryMap *PrepareMemmapRegions(void **mem_start, void *end)
{
    boot_MemoryMap *memmap;
    memmap = AllocFromRangeAligned(mem_start, end, ALIGN_HELPER(boot_MemoryMap));
    if (memmap == NULL) {
        abort();
    }
    memmap->count = 0;
    boot_MemoryMapEntry *entries = AllocFromRangeAligned(mem_start, end,
        ALIGN_HELPER2(boot_MemoryMap, (size_t)memmap->count));
    if (entries == NULL) {
        abort();
    }
    memmap->entries = (uintptr_t)entries;
    unsigned c = 0;
    do {
        if (AllocFromRangeAligned(mem_start, end,
            ALIGN_HELPER(boot_MemoryMapEntry)) == NULL)
        {
            abort();
        }
        if (!i686_bios_mem_GetMap(&c, entries + memmap->count)) {
            break;
        }
        memmap->count += 1;
    } while (c);
    if (memmap->count == 0) {
        abort();
    }
    qsort(entries, (size_t)memmap->count,
        sizeof(boot_MemoryMapEntry), reg_comp);
    return memmap;
}

size_t MergeNormalRegions(boot_MemoryMap *memmap) {
    size_t i, lastMerged = 0;
    boot_MemoryMapEntry *entries = ReadEntries(memmap->entries);
    for (; lastMerged < memmap->count; ++lastMerged) {
        if (entries[lastMerged].type == i686_bios_mem_AvailableMemory &&
            (entries[lastMerged].flags & 0xF) == 1)
        {
            break;
        }
    }
    if (lastMerged == memmap->count) {
        return lastMerged;
    }
    i = lastMerged + 1;
    for (; i < memmap->count; ++i) {
        boot_MemoryMapEntry
            *last = entries + lastMerged,
            *current = last + 1
        ;
        if (current != entries + i) {
            memcpy(current, entries + i,
                sizeof(boot_MemoryMapEntry));
        }
        if (last->begin + last->size >= current->begin) {
            uint64_t a = current->begin - last->begin + current->size;
            uint64_t b = last->size;
            last->size = boot_MaxU64(a, b);
//            memset(current, 0, sizeof(i686_bios_mem_MapEntry));
        } else {
            ++lastMerged;
        }
    }
    return lastMerged + 1;
}

enum ExcludeResult {
    Left = -2,
    LeftIntersect = -1,
    CenterIntersect = 0,
    RightIntersect = 1,
    Right = 2,
    FullIntersect = -3
};

static int ExcludeRegion(boot_MemoryMapEntry *a, const boot_MemoryMapEntry *b,
    boot_MemoryMapEntry *leftover)
{
    uint64_t aEnd = a->begin + a->size;
    uint64_t bEnd = b->begin + b->size;
    if (bEnd <= a->begin) {
        return Left;
    }
    if (b->begin >= aEnd) {
        return Right;
    }
    if (bEnd < aEnd) {
        *leftover = *a;
        a->begin = bEnd;
        a->size = aEnd - bEnd;
        if (leftover->begin < b->begin) {
            leftover->size = b->begin - leftover->begin;
            return CenterIntersect;
        }
        return LeftIntersect;
    }
    if (a->begin < b->begin) {
        a->size = b->begin - a->begin;
        return RightIntersect;
    }
    return FullIntersect;
}

void ExcludeAbnormalRegions(boot_MemoryMap *memmap, size_t count,
    void **mem_start, void *end)
{
    boot_MemoryMapEntry *entries = ReadEntries(memmap->entries);
    size_t i = 0;
    size_t allocatedCount = count;
    for (; i < count; ++i) {
        if (entries[i].type == i686_bios_mem_AvailableMemory &&
            (entries[i].flags & 0xF) == 1)
        {
            break;
        }
    }
    size_t reservedCount = i;
    size_t reservedCurrent = 0;
    size_t lastRemoved = i;
    boot_MemoryMapEntry tempEntry;
    while (reservedCurrent < reservedCount && i < count) {
        int result = ExcludeRegion(entries + i,
            entries + reservedCurrent, &tempEntry);
        switch (result) {
            case Left:
            case LeftIntersect:
                ++reservedCurrent;
                break;
            case CenterIntersect:
                if (lastRemoved < i) {
                    entries[lastRemoved++] = tempEntry;
                } else if (allocatedCount < memmap->count) {
                    entries[allocatedCount++] = tempEntry;
                } else {
                    if (AllocFromRangeAligned(mem_start, end,
                        ALIGN_HELPER(boot_MemoryMapEntry)) == NULL)
                    {
                        abort();
                    }
                    entries[allocatedCount++] = tempEntry;
                    memmap->count = allocatedCount;
                }
                break;
            case RightIntersect:
            case Right:
                if (lastRemoved != i) {
                    entries[lastRemoved] = entries[i];
                }
                lastRemoved++;
                /* fallthrough */
            case FullIntersect:
                i++;
                break;
        }
    }
    memmap->count = allocatedCount;
    if (i == lastRemoved) {
        return;
    }
    while (i < allocatedCount) {
        entries[lastRemoved++] = entries[i++];
    }
    qsort(entries + reservedCount, (size_t)memmap->count - reservedCount, sizeof(boot_MemoryMapEntry), reg_comp);
}

boot_BuddyRegion *boot_BuddyRegion_Construct(void *regionStart, void *regionEnd)
{
    void *current = regionStart;
    size_t region_size = (size_t)((byte*)regionEnd - (byte*)current);
    if (region_size == 0) {
        abort();
    }
    size_t regionBlockCount = region_size / BOOT_ALLOC_BLOCK_SIZE;

    size_t maxOrder = (size_t)boot_Log2U64(regionBlockCount * 2 - 1);
    boot_BuddyRegion *buddyRegion;
    size_t buddyRegionSize = sizeof(boot_BuddyRegion) + sizeof(boot_BuddyBlockStore[maxOrder]);
    buddyRegion = AllocFromRange(&current, regionEnd, buddyRegionSize);
    if (buddyRegion == NULL) {
        abort();
    }
    memset(buddyRegion, 0, buddyRegionSize);

    buddyRegion->start = regionStart;
    buddyRegion->end = regionEnd;
    buddyRegion->maxOrder = maxOrder;

    size_t regionBitmapSize = AlignUp(regionBlockCount, CHAR_BIT) / CHAR_BIT;
    buddyRegion->bitmap = AllocFromRange(&current, regionEnd, regionBitmapSize);
    if (buddyRegion->bitmap == NULL) {
        abort();
    }
    memset(buddyRegion->bitmap, 0, regionBitmapSize);

    size_t bitmapStart = 0;
    for (size_t i = 0; i < maxOrder; ++i) {
        buddyRegion->buddyArray[i].pairBitmapStart = bitmapStart;
        size_t half_bmp_size = regionBlockCount / 2;
        bitmapStart += half_bmp_size + (regionBlockCount & 1);
        regionBlockCount = half_bmp_size;
    }
    boot_BuddyRegion_Construct_phase2(buddyRegion, current);
    return buddyRegion;
}

void boot_BuddyRegion_Construct_phase2(boot_BuddyRegion *region, void* current)
{
    size_t size = (size_t)((byte*)region->end - (byte*)current);
    size_t memBlockOffset = (size_t)((byte*)current - (byte*)region->start);
    size_t order = BOOT_ALLOC_BLOCK_SIZE;
    size_t i;
    for (i = 0; i < region->maxOrder && size >= order; ++i, order <<= 1) {
        if (memBlockOffset & order) {
            boot_BuddyRegion_FreeBlock(region, (byte*)region->start + memBlockOffset, i);
            memBlockOffset += order;
            size -= order;
        }
    }
    for (; i <= region->maxOrder; i--, order >>= 1) {
        if (size >= order) {
            boot_BuddyRegion_FreeBlock(region, (byte*)region->start + memBlockOffset, i);
            memBlockOffset += order;
            size -= order;
        }
    }
    return;
}

int boot_BuddyRegion_TogglePairBit(boot_BuddyRegion *region, void *block, size_t order)
{
    if (order < region->maxOrder - 1) {
        uintptr_t pairNum = ((size_t)((byte*)block - (byte*)region->start) / BOOT_ALLOC_BLOCK_SIZE) >> (order + 1);
        size_t bitNum = region->buddyArray[order].pairBitmapStart + pairNum;
        size_t bitmapBlock = bitNum / CHAR_BIT;
        size_t bitNumInBlock = bitNum % CHAR_BIT;
        return toggleBit(region->bitmap + bitmapBlock, bitNumInBlock);
    }
    return !!(1);
}

void *boot_BuddyRegion_AllocBlock(boot_BuddyRegion *region, size_t order)
{
    if (order >= region->maxOrder) {
        return NULL;
    }
    boot_DoublyLinkedList *list = &(region->buddyArray[order].freeList);
    boot_DoublyLinkedListElement *elem;
    elem = list->begin;
    if (elem != NULL) {
        boot_DoublyLinkedList_Remove(list, elem);
        boot_BuddyRegion_TogglePairBit(region, elem, order);
        return elem;
    } else {
        void *largeBlock = boot_BuddyRegion_AllocBlock(region, order + 1);
        if (largeBlock != NULL) {
            boot_DoublyLinkedList_Add(list, (void*)((byte*)largeBlock + (BOOT_ALLOC_BLOCK_SIZE << order)));
            boot_BuddyRegion_TogglePairBit(region, largeBlock, order);
        }
        return largeBlock;
    }
    return NULL;
}

void boot_BuddyRegion_FreeBlock(boot_BuddyRegion *region, void *block, size_t order)
{
    size_t blockOffset = (size_t)((byte*)block - (byte*)region->start);
    if (!boot_BuddyRegion_TogglePairBit(region, block, order)) {
        boot_BuddyFreeBlockHeader *block;
        size_t mask = BOOT_ALLOC_BLOCK_SIZE << order;
        block = (void*)((byte*)region->start + (blockOffset ^ mask));
        boot_DoublyLinkedList_Remove(&(region->buddyArray[order].freeList), &(block->elem));
        void *upOrderBlock = (void*)((byte*)region->start + (blockOffset & (~mask)));
        boot_BuddyRegion_FreeBlock(region, upOrderBlock, order + 1);

        return;
    }
    boot_DoublyLinkedList_Add(&(region->buddyArray[order].freeList), block);
}

void *malloc(size_t size)
{
    size = AlignUp(size, BOOT_ALLOC_BLOCK_SIZE);
    size = size / BOOT_ALLOC_BLOCK_SIZE + 1;
    size_t order = (size_t)boot_Log2U64(size * 2 - 1);
    void* block = boot_BuddyRegion_AllocBlock(allocRegion, order);
    if (block == NULL) {
        return NULL;
    }
    *((size_t*)block) = order;
    return (byte*)block + BOOT_ALLOC_BLOCK_SIZE;
}

static void *mextend(void* ptr, size_t size)
{
    if (ptr == NULL) {
        return NULL;
    }
    size = AlignUp(size, BOOT_ALLOC_BLOCK_SIZE);
    size = size / BOOT_ALLOC_BLOCK_SIZE + 1;
    size_t reqOrder = (size_t)boot_Log2U64(size * 2 - 1);
    void *block = (byte*)ptr - BOOT_ALLOC_BLOCK_SIZE;
    size_t order = *((size_t*)block);
    return reqOrder <= order ? ptr : NULL;
}

void *realloc(void* ptr, size_t size)
{
    void *newPtr = mextend(ptr, size);
    if (newPtr == NULL) {
        newPtr = malloc(size);
        if (ptr != NULL) {
            void *block = (byte*)ptr - BOOT_ALLOC_BLOCK_SIZE;
            size_t order = *((size_t*)block);
            memcpy(newPtr, ptr, (BOOT_ALLOC_BLOCK_SIZE << order) - BOOT_ALLOC_BLOCK_SIZE);
            free(ptr);
        }
    }
    return newPtr;
}

void free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    void* block = (byte*)ptr - BOOT_ALLOC_BLOCK_SIZE;
    size_t order = *((size_t*)block);
    boot_BuddyRegion_FreeBlock(allocRegion, block, order);
}

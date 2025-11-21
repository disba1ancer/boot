#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>
#include "membios.h"
#include "boot/util.h"
#include "alloc.h"

extern unsigned char __bss_end[];

#define BOOT_ALLOC_BLOCK_SIZE ((size_t)16U)

_Static_assert(BOOT_ALLOC_BLOCK_SIZE >= alignof(max_align_t), "Fundamental alignment stricter than BOOT_ALLOC_BLOCK_SIZE");

typedef unsigned char byte;

BOOT_STRUCT(boot_Heap_BlockHeader) {
    boot_DoublyLinkedListElement elem;
    ptrdiff_t prev;
    ptrdiff_t next;
};

BOOT_STRUCT(boot_Heap) {
    boot_DoublyLinkedListElement freeList;
    boot_Heap_BlockHeader* last;
};

enum boot_HeapConsts {
    boot_HeapFreeBlock,
    boot_HeapAllocatedBlock,
    boot_HeapBlockSize = BOOT_ALLOC_BLOCK_SIZE,
    boot_HeapBlockMask = boot_HeapBlockSize - 1,
};

static uintptr_t AlignUp(uintptr_t val, size_t align);
static uintptr_t AlignDown(uintptr_t val, size_t align);

static uint64_t AlignUp64(uint64_t val, size_t align);
static uint64_t AlignDown64(uint64_t val, size_t align);

static boot_MemoryMapEntry* ReadEntries(uint64_t ref);
static boot_MemoryMap *FakeMemmapRegions(void);
static uint32_t MapRegionType(uint32_t in);
static void InsertRegion(boot_MemoryMap* memmap, uint64_t begin, uint64_t end, uint32_t type);
static boot_MemoryMap *PrepareMemmapRegions(void);
static int IsZero(const boot_MemoryMapEntry* a);
static int reg_comp(const void *a, const void *b);
static int reg_comp2(const void *a, const void *b);
static int reg_comp3(const void *a, const void *b);
static int IsOwnedBy(uint64_t p, boot_MemoryMapEntry* a);
static int IsOverlapped(boot_MemoryMapEntry* a, boot_MemoryMapEntry* b);
static int IsAdjacent(boot_MemoryMapEntry* a, boot_MemoryMapEntry* b);
static void MergeMemmap(boot_MemoryMap *memmap);
static boot_MemoryMapEntry ExcludeRegion(boot_MemoryMapEntry *a, const boot_MemoryMapEntry *b);
static void MemmapExcludeByOrder(boot_MemoryMap *memmap);

static int boot_Heap_GetBlockType(boot_Heap_BlockHeader* blk);
static void boot_Heap_SetBlockType(boot_Heap_BlockHeader* blk, int type);
static ptrdiff_t boot_Heap_GetBlockSize(boot_Heap_BlockHeader* blk);
static void boot_Heap_SetBlockSize(boot_Heap_BlockHeader* blk, ptrdiff_t size);
static boot_Heap_BlockHeader* boot_Heap_GetBlockNext(boot_Heap_BlockHeader* blk);
static boot_Heap_BlockHeader* boot_Heap_GetBlockPrev(boot_Heap_BlockHeader* blk);
static void boot_Heap_Construct(void);
static boot_Heap_BlockHeader* boot_Heap_FindAtLeast(ptrdiff_t size);
static void boot_Heap_AllocFrom(boot_Heap_BlockHeader* blk, ptrdiff_t size);
static void boot_Heap_ConcatWithNext(boot_Heap_BlockHeader* blk);
static boot_Heap_BlockHeader* boot_Heap_AllocBlock(ptrdiff_t size);
static void boot_Heap_FreeBlock(boot_Heap_BlockHeader* blk);
static int boot_Heap_ResizeBlock(boot_Heap_BlockHeader* blk, ptrdiff_t newSize);
static void* boot_Heap_Alloc(ptrdiff_t size);
static boot_Heap_BlockHeader* boot_Heap_ToBlock(void* ptr);
static void boot_Heap_Free(void* ptr);
static int boot_Heap_Resize(void* ptr, ptrdiff_t newSize);
static void* boot_Heap_Realloc(void* ptr, ptrdiff_t newSize);

static boot_Heap heap;

void *heap_start = __bss_end;
byte *heap_end;

uintptr_t AlignUp(uintptr_t val, size_t align)
{
    uintptr_t mask = align - 1;
    return AlignDown(val + mask, align);
}

uintptr_t AlignDown(uintptr_t val, size_t align)
{
    uintptr_t mask = align - 1;
    return val & ~mask;
}

uint64_t AlignUp64(uint64_t val, size_t align)
{
    uint64_t mask = align - 1;
    return AlignDown(val + mask, align);
}

uint64_t AlignDown64(uint64_t val, size_t align)
{
    uint64_t mask = align - 1;
    return val & ~mask;
}

static boot_MemoryMapEntry fakeMap[] = {
//    {0x9FC00, 0x400, 2, 1},
//    {0x500, 0x7000, 1, 1},
//    {0x7000, 0xF9000, 1, 1},
//    {0x8000, 0x1000, 1, 1},
//    {0x100000, 0x300000, 1, 1},
    {0x300, 0x10000, 1},
    {0x200000, 0x300000, 1},
    {0x20000, 0x100000, 2},
    {0x240000, 0x2C0000, 2},
    {0x400000, 0x500000, 2},
    {0x8000, 0x20000, 2},
    {0x0, 0x351, 2},
    {0x18000, 0x1A000, 1},

};

void boot_InitAlloc(void)
{
    boot_Heap_Construct();
    boot_MemoryMap *memmap = PrepareMemmapRegions();
    MergeMemmap(memmap);
    MemmapExcludeByOrder(memmap);
    boot_Heap_Resize(ReadEntries(memmap->entries), sizeof(boot_MemoryMapEntry) * (memmap->count + 1));
    boot_InitVirtualAlloc(memmap);
}

boot_MemoryMapEntry* ReadEntries(uint64_t ref)
{
    return (void*)(uintptr_t)ref;
}

boot_MemoryMap *FakeMemmapRegions(void)
{
    boot_MemoryMap *memmap;
    memmap = boot_Heap_Alloc(sizeof(boot_MemoryMap));
    if (memmap == NULL) {
        abort();
    }
    memmap->count = sizeof(fakeMap) / sizeof(boot_MemoryMapEntry);
    boot_MemoryMapEntry *entries = boot_Heap_Alloc(sizeof(boot_MemoryMapEntry) * memmap->count);
    if (entries == NULL) {
        abort();
    }
    memmap->entries = (uintptr_t)entries;
    memcpy(entries, fakeMap, sizeof(fakeMap));
    return memmap;
}

uint32_t MapRegionType(uint32_t in)
{
    switch (in) {
    case i686_bios_mem_AvailableMemory:
        return boot_MemoryMapEntryType_AvailableMemory;
    case i686_bios_mem_ACPIReclaimMemory:
        return boot_MemoryMapEntryType_SystemReclaimable;
    }
    return boot_MemoryMapEntryType_ReservedMemory;
}

void InsertRegion(boot_MemoryMap* memmap, uint64_t begin, uint64_t end, uint32_t type)
{
    boot_MemoryMapEntry *entries = ReadEntries(memmap->entries);
    if (!boot_Heap_Resize(entries, sizeof(boot_MemoryMapEntry) * (++memmap->count)))
    {
        abort();
    }
    entries[memmap->count - 1].begin = begin;
    entries[memmap->count - 1].end = end;
    entries[memmap->count - 1].type = type;
}

boot_MemoryMap *PrepareMemmapRegions(void)
{
    boot_MemoryMap *memmap;
    memmap = boot_Heap_Alloc(sizeof(boot_MemoryMap));
    if (memmap == NULL) {
        abort();
    }
    memmap->count = 0;
    boot_MemoryMapEntry *entries = boot_Heap_Alloc(sizeof(boot_MemoryMapEntry));
    i686_bios_mem_MemoryMapEntry entry;
    if (entries == NULL) {
        abort();
    }
    memmap->entries = (uintptr_t)entries;
    unsigned c = 0;
    while (i686_bios_mem_GetMap(&c, &entry)) {
        uint64_t begin = AlignUp64(entry.begin, boot_Alloc_PageSize);
        uint64_t end = AlignDown64(entry.begin + entry.size, boot_Alloc_PageSize);
        if (end <= begin) {
            goto cont;
        }
        InsertRegion(memmap, begin, end, MapRegionType(entry.type));
    cont:
        if (c == 0) break;
    }
    if (memmap->count == 0) {
        abort();
    }
    InsertRegion(memmap, 0, (uint64_t)heap_end, boot_MemoryMapEntryType_BootReclaimable);
    return memmap;
}

int IsZero(const boot_MemoryMapEntry* a)
{
    return a->begin == a->end;
}

int reg_comp(const void *a, const void *b)
{
    const boot_MemoryMapEntry *left = a;
    const boot_MemoryMapEntry *right = b;
    return (right->begin < left->begin) - (left->begin < right->begin);
}

int reg_comp2(const void *a, const void *b)
{
    const boot_MemoryMapEntry *left = a;
    const boot_MemoryMapEntry *right = b;
    int r = (left->type < right->type) - (left->type > right->type);
    if (r != 0) {
        return r;
    }
    return reg_comp(a, b);
}

int reg_comp3(const void *a, const void *b)
{
    const boot_MemoryMapEntry *left = a;
    const boot_MemoryMapEntry *right = b;
    int r = IsZero(left) - IsZero(right);
    if (r != 0) {
        return r;
    }
    return reg_comp2(a, b);
}

int IsOwnedBy(uint64_t p, boot_MemoryMapEntry* a)
{
    return a->begin <= p && p < a->end;
}

/* a->begin <= b->begin */
int IsOverlapped(boot_MemoryMapEntry* a, boot_MemoryMapEntry* b)
{
    return IsOwnedBy(b->begin, a);
}

/* a->begin <= b->begin */
int IsAdjacent(boot_MemoryMapEntry* a, boot_MemoryMapEntry* b)
{
    return IsOwnedBy(b->begin, a) || a->end == b->begin;
}

void MergeMemmap(boot_MemoryMap *memmap)
{
    boot_MemoryMapEntry *entries = ReadEntries(memmap->entries);
    qsort(entries, (size_t)memmap->count,
        sizeof(boot_MemoryMapEntry), reg_comp2);
    boot_MemoryMapEntry *a = entries;
    boot_MemoryMapEntry *end = entries + memmap->count;
    unsigned erased = 0;
    while (a != end) {
        boot_MemoryMapEntry *b = a + 1;
        while (b != end && a->type == b->type && IsAdjacent(a, b)) {
            a->begin = boot_MinU64(a->begin, b->begin);
            a->end = boot_MaxU64(a->end, b->end);
            b->begin = b->end = 0;
            b->type = 0;
            ++erased;
            ++b;
        }
        a = b;
    }
    qsort(entries, (size_t)memmap->count,
        sizeof(boot_MemoryMapEntry), reg_comp3);
    memmap->count -= erased;
}

boot_MemoryMapEntry ExcludeRegion(boot_MemoryMapEntry *a, const boot_MemoryMapEntry *b)
{
    boot_MemoryMapEntry left = { .type = a->type };
    if (a->end <= b->begin) {
        left = *a;
        a->begin = a->end;
        return left;
    }
    if (b->end <= a->begin) {
        return left;
    }
    left.begin = a->begin;
    left.end = boot_Max(a->begin, b->begin);
    a->begin = boot_Min(a->end, b->end);
    return left;
}

void MemmapExcludeByOrder(boot_MemoryMap *memmap)
{
    boot_MemoryMapEntry* entries = ReadEntries(memmap->entries);
    if (!boot_Heap_Resize(entries, sizeof(boot_MemoryMapEntry) * memmap->count * 2)) {
        abort();
    }
    boot_MemoryMapEntry* result = boot_Heap_Alloc(sizeof(boot_MemoryMapEntry) * memmap->count * 2);
    if (result == NULL) {
        abort();
    }
    ptrdiff_t resultCount = 0;
    boot_MemoryMapEntry* newResult = boot_Heap_Alloc(sizeof(boot_MemoryMapEntry) * memmap->count * 2);
    if (newResult == NULL) {
        abort();
    }
    for (ptrdiff_t i = 0; i < (ptrdiff_t)memmap->count; ++i) {
        ptrdiff_t newResultCount = 0;
        for (ptrdiff_t j = 0; j < resultCount; ++j) {
            boot_MemoryMapEntry left = ExcludeRegion(entries + i, result + j);
            if (left.begin != left.end) {
                newResult[newResultCount++] = left;
            }
            newResult[newResultCount++] = result[j];
        }
        if (!IsZero(entries + i)) {
            newResult[newResultCount++] = entries[i];
        }
        // qsort(newResult, newResultCount, sizeof(boot_MemoryMapEntry), reg_comp);
        boot_MemoryMapEntry* tmpPtr = result;
        result = newResult;
        newResult = tmpPtr;
        resultCount = newResultCount;
    }
    memcpy(entries, result, resultCount * sizeof(boot_MemoryMapEntry));
    memmap->count = resultCount;
    boot_Heap_Free(newResult);
    boot_Heap_Free(result);
}

int boot_Heap_GetBlockType(boot_Heap_BlockHeader* blk)
{
    return blk->next & boot_HeapBlockMask;
}

void boot_Heap_SetBlockType(boot_Heap_BlockHeader* blk, int type)
{
    type = type & boot_HeapBlockMask;
    blk->next ^= type ^ (blk->next & boot_HeapBlockMask);
}

ptrdiff_t boot_Heap_GetBlockSize(boot_Heap_BlockHeader* blk)
{
    return blk->next ^ (blk->next & boot_HeapBlockMask);
}

void boot_Heap_SetBlockSize(boot_Heap_BlockHeader* blk, ptrdiff_t size)
{
    blk->next = size ^ (blk->next & boot_HeapBlockMask);
}

boot_Heap_BlockHeader* boot_Heap_GetBlockNext(boot_Heap_BlockHeader* blk)
{
    return (void*)((byte*)blk + boot_Heap_GetBlockSize(blk));
}

boot_Heap_BlockHeader* boot_Heap_GetBlockPrev(boot_Heap_BlockHeader* blk)
{
    return (void*)((byte*)blk - blk->prev);
}

void boot_Heap_Construct(void)
{
    boot_Heap_BlockHeader* heapBlk = memset(heap_start, 0, sizeof(boot_Heap_BlockHeader));
    boot_Heap_SetBlockSize(heapBlk, heap_end - (byte*)heap_start);
    heap.last = heapBlk;
    boot_DoublyLinkedList_Init(&heap.freeList);
    boot_DoublyLinkedList_Add((void*)&heap, (void*)heapBlk);
}

boot_Heap_BlockHeader* boot_Heap_FindAtLeast(ptrdiff_t size)
{
    boot_DoublyLinkedListElement* snt = &heap.freeList;
    boot_DoublyLinkedListElement* current = snt->next;
    while (current != snt && boot_Heap_GetBlockSize((void*)current) < size) {
        current = current->next;
    }
    return (void*)current;
}

void boot_Heap_AllocFrom(boot_Heap_BlockHeader* blk, ptrdiff_t size)
{
    boot_DoublyLinkedList_Remove((void*)blk);
    if (boot_Heap_GetBlockSize(blk) == size) {
        boot_Heap_SetBlockType(blk, boot_HeapAllocatedBlock);
        return;
    }
    ptrdiff_t newSize = boot_Heap_GetBlockSize(blk) - size;
    boot_Heap_SetBlockSize(blk, size);
    boot_Heap_BlockHeader* newBlk = memset(boot_Heap_GetBlockNext(blk), 0, sizeof(boot_Heap_BlockHeader));
    boot_Heap_SetBlockSize(newBlk, newSize);
    newBlk->prev = size;
    boot_DoublyLinkedList_Add((void*)&heap, (void*)newBlk);
    boot_Heap_SetBlockType(blk, boot_HeapAllocatedBlock);
    if (heap.last == blk) {
        heap.last = newBlk;
        return;
    }
    boot_Heap_GetBlockNext(newBlk)->prev = newSize;
}

void boot_Heap_ConcatWithNext(boot_Heap_BlockHeader* blk)
{
    if (heap.last == blk) {
        return;
    }
    if (boot_Heap_GetBlockType(blk) == boot_HeapAllocatedBlock) {
        return;
    }
    boot_Heap_BlockHeader* nextBlk = boot_Heap_GetBlockNext(blk);
    if (boot_Heap_GetBlockType(nextBlk) == boot_HeapAllocatedBlock) {
        return;
    }
    boot_DoublyLinkedList_Remove((void*)nextBlk);
    ptrdiff_t newSize = boot_Heap_GetBlockSize(blk) + boot_Heap_GetBlockSize(nextBlk);
    boot_Heap_SetBlockSize(blk, newSize);
    if (heap.last == nextBlk) {
        heap.last = blk;
        return;
    }
    boot_Heap_GetBlockNext(blk)->prev = newSize;
}

boot_Heap_BlockHeader* boot_Heap_AllocBlock(ptrdiff_t size)
{
    boot_Heap_BlockHeader* blk = boot_Heap_FindAtLeast(size);
    if (blk == (void*)&heap) {
        return NULL;
    }
    boot_Heap_AllocFrom(blk, size);
    return blk;
}

void boot_Heap_FreeBlock(boot_Heap_BlockHeader* blk)
{
    boot_Heap_SetBlockType(blk, boot_HeapFreeBlock);
    boot_Heap_ConcatWithNext(blk);
    boot_DoublyLinkedList_Add((void*)&heap, (void*)blk);
    blk = boot_Heap_GetBlockPrev(blk);
    boot_Heap_ConcatWithNext(blk);
}

int boot_Heap_ResizeBlock(boot_Heap_BlockHeader* blk, ptrdiff_t newSize)
{
    if (boot_Heap_GetBlockSize(blk) == newSize) return 1;
    boot_Heap_SetBlockType(blk, boot_HeapFreeBlock);
    boot_Heap_ConcatWithNext(blk);
    if (boot_Heap_GetBlockSize(blk) < newSize) return 0;
    boot_Heap_AllocFrom(blk, newSize);
    return 1;
}

void* boot_Heap_Alloc(ptrdiff_t size)
{
    if (size < 0) return NULL;
    size = AlignUp(size + boot_HeapBlockSize, boot_HeapBlockSize);
    return (void*)((byte*)boot_Heap_AllocBlock(size) + boot_HeapBlockSize);
}

boot_Heap_BlockHeader* boot_Heap_ToBlock(void* ptr)
{
    return (void*)((byte*)ptr - boot_HeapBlockSize);
}

void boot_Heap_Free(void* ptr)
{
    boot_Heap_FreeBlock(boot_Heap_ToBlock(ptr));
}

int boot_Heap_Resize(void* ptr, ptrdiff_t newSize)
{
    if (newSize < 0) return 0;
    newSize = AlignUp(newSize + boot_HeapBlockSize, boot_HeapBlockSize);
    return boot_Heap_ResizeBlock(boot_Heap_ToBlock(ptr), newSize);
}

void* boot_Heap_Realloc(void* ptr, ptrdiff_t newSize)
{
    if (newSize < 0) return 0;
    if (boot_Heap_Resize(ptr, newSize)) {
        return ptr;
    }
    void* newPtr = boot_Heap_Alloc(newSize);
    newPtr = memcpy(newPtr, ptr, boot_Heap_GetBlockSize(boot_Heap_ToBlock(ptr)) - boot_HeapBlockSize);
    boot_Heap_Free(ptr);
    return newPtr;
}

void *malloc(size_t size)
{
    return boot_Heap_Alloc(size);
}

void *realloc(void* ptr, size_t size)
{
    return boot_Heap_Realloc(ptr, size);
}

void free(void *ptr)
{
    boot_Heap_Free(ptr);
}

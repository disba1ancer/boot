#include "alloc.h"
#include "boot/virtual_alloc.h"
#include "processor.h"

static ptrdiff_t currentMemEntry;
x86_64_PageEntry (*boot_x86_64_pml4)[512];
boot_MemoryMap* boot_memmap;

static ptrdiff_t FindNextAvailableEntry(const boot_MemoryMap* memmap, ptrdiff_t from)
{
    const boot_MemoryMapEntry *entries = (void*)(uintptr_t)(memmap->entries);
    for (ptrdiff_t i = from + 1; i < (ptrdiff_t)memmap->count; ++i) {
        if (entries[i].type == boot_MemoryMapEntryType_AvailableMemory)
        {
            return i;
        }
    }
    return memmap->count;
}

static void InitPagedAlloc(void)
{
    boot_MemoryMapEntry *entries = (void*)(uintptr_t)(boot_memmap->entries);
    currentMemEntry = FindNextAvailableEntry(boot_memmap, -1);
    if (currentMemEntry == (ptrdiff_t)boot_memmap->count) {
        abort();
    }
    uint64_t regionStart = entries[currentMemEntry].begin;
    if (regionStart > UINTPTR_MAX) {
        abort();
    }
    entries[boot_memmap->count].begin = regionStart;
    entries[boot_memmap->count].end = regionStart;
    entries[boot_memmap->count].type = boot_MemoryMapEntryType_Kernel;
}

uint64_t boot_GetAllocCurrentAddr(void)
{
    boot_MemoryMapEntry *entries = (void*)(uintptr_t)(boot_memmap->entries);
    uintptr_t page = entries[boot_memmap->count].end;
    if (page == 0) {
        return -1;
    }
    return page;
}

void* boot_AllocPage(void)
{
    boot_MemoryMapEntry *entries = (void*)(uintptr_t)(boot_memmap->entries);
    uintptr_t page = entries[boot_memmap->count].end;
    if (page == 0) {
        return NULL;
    }
    entries[boot_memmap->count].end += boot_Alloc_PageSize;
    uint64_t regionStart = (entries[currentMemEntry].begin += boot_Alloc_PageSize);
    uint64_t regionEnd = entries[currentMemEntry].end;

    if (regionStart > UINTPTR_MAX) {
        entries[boot_memmap->count] = (boot_MemoryMapEntry){0};
        return (void*)page;
    }
    if (regionStart != regionEnd) {
        return (void*)page;
    }
    entries[currentMemEntry] = entries[boot_memmap->count];
    currentMemEntry = FindNextAvailableEntry(boot_memmap, currentMemEntry);
    regionStart = entries[currentMemEntry].begin;
    if (currentMemEntry == (ptrdiff_t)boot_memmap->count || regionStart > UINTPTR_MAX) {
        entries[boot_memmap->count] = (boot_MemoryMapEntry){0};
        return (void*)page;
    }
    entries[boot_memmap->count].begin = regionStart;
    entries[boot_memmap->count].end = regionStart;

    return (void*)page;
}

void boot_InitVirtualAlloc(boot_MemoryMap* memmap) {
    boot_memmap = memmap;
    InitPagedAlloc();
}

static int AllocIfNotPresent(x86_64_PageEntry* entry)
{
    uint32_t pageFlags = i686_PageEntryFlag_Present
        | i686_PageEntryFlag_Write | i686_PageEntryFlag_User;
    if (entry->data & i686_PageEntryFlag_Present) {
        return 1;
    }
    void* dirPtr = boot_AllocPage();
    if (dirPtr == NULL) {
        return 0;
    }
    memset(dirPtr, 0, 4096);
    *entry = (x86_64_PageEntry)x86_64_MakePageEntry((uintptr_t)dirPtr, pageFlags);
    return 1;
}

static uint32_t TranslateFlags(int flags)
{
    uint32_t result = i686_PageEntryFlag_Present;
    result |= i686_PageEntryFlag_NX * !(flags & boot_MemoryFlags_Execute);
    result |= i686_PageEntryFlag_Write * !!(flags & boot_MemoryFlags_Write);
    result |= i686_PageEntryFlag_User * !(flags & boot_MemoryFlags_Kernel);
    result |= i686_PageEntryFlag_PWT
        * !!(flags & boot_MemoryFlags_DeviceWrite);
    result |= i686_PageEntryFlag_PCD * !!(flags & boot_MemoryFlags_Device);
    return result;
}

static int AllocPageEntry(x86_64_PageEntry* entry, int flags)
{
    uint32_t pageFlags = TranslateFlags(flags);
    void* dirPtr = boot_AllocPage();
    if (dirPtr == NULL) {
        return 0;
    }
    *entry = (x86_64_PageEntry)x86_64_MakePageEntry((uintptr_t)dirPtr, pageFlags);
    return 1;
}

static x86_64_PageEntry* PageDirectory(x86_64_PageEntry* entry, size_t index)
{
    x86_64_PageEntry* pageEntry = (void*)(uintptr_t)
        x86_64_PageEntry_GetAddr(entry);
    pageEntry += (index & 0x1FF);
    if (!AllocIfNotPresent(pageEntry)) {
        return NULL;
    }
    if (pageEntry->data & i686_PageEntryFlag_PAT) {
        return NULL;
    }
    return pageEntry;
}

static x86_64_PageEntry* PageEntry(x86_64_PageEntry* entry, size_t index,
    int flags)
{
    x86_64_PageEntry* pageEntry = (void*)(uintptr_t)
        x86_64_PageEntry_GetAddr(entry);
    pageEntry += (index & 0x1FF);
    if (pageEntry->data & i686_PageEntryFlag_Present) {
        return NULL;
    }
    if (!AllocPageEntry(pageEntry, flags)) {
        return NULL;
    }
    return pageEntry;
}

static x86_64_PageEntry* PageEntry2(x86_64_PageEntry* entry, size_t index)
{
    x86_64_PageEntry* pageEntry = (void*)(uintptr_t)
        x86_64_PageEntry_GetAddr(entry);
    pageEntry += (index & 0x1FF);
    return pageEntry;
}

static x86_64_PageEntry Pml4(void)
{
    if (boot_x86_64_pml4 == NULL) {
        boot_x86_64_pml4 = boot_AllocPage();
        memset(boot_x86_64_pml4, 0, sizeof(*boot_x86_64_pml4));
    }
    return (x86_64_PageEntry)x86_64_MakePageEntry((uintptr_t)boot_x86_64_pml4, 0);
}

static x86_64_PageEntry* FindAllocPageDirectory(uint64_t virtPageAddr)
{
    x86_64_PageEntry pml4ptr = Pml4();
    x86_64_PageEntry* pageEntry;
    pageEntry = PageDirectory(&pml4ptr, (size_t)(virtPageAddr >> 39));
    if (pageEntry == NULL) { return NULL; }
    pageEntry = PageDirectory(pageEntry, (size_t)(virtPageAddr >> 30));
    if (pageEntry == NULL) { return NULL; }
    return PageDirectory(pageEntry, (size_t)(virtPageAddr >> 21));
}

static x86_64_PageEntry* AllocEntryByVirtualAddress(uint64_t virtPageAddr,
    int flags)
{
    x86_64_PageEntry* pageEntry = FindAllocPageDirectory(virtPageAddr);
    if (pageEntry == NULL) { return NULL; }
    pageEntry = PageEntry(pageEntry, (size_t)(virtPageAddr >> 12), flags);
    return pageEntry;
}

void* boot_VirtualAlloc(uint64_t virtPageAddr, int flags)
{
    x86_64_PageEntry* pageEntry = AllocEntryByVirtualAddress(virtPageAddr, flags);
    if (pageEntry == NULL) {
        return NULL;
    }
    return (void*)(uintptr_t)x86_64_PageEntry_GetAddr(pageEntry);
}

void* boot_VirtualToPtr(uint64_t virtAddr)
{
    x86_64_PageEntry entry;
    entry = (*boot_x86_64_pml4)[(virtAddr >> 39) & 0x1FF];
    if (!(x86_64_PageEntry_GetFlags(&entry) & i686_PageEntryFlag_Present)) {
        return NULL;
    }
    x86_64_PageEntry (*pdpt)[512] = (void*)x86_64_PageEntry_GetAddr(&entry);
    entry = (*pdpt)[(virtAddr >> 30) & 0x1FF];
    if (!(x86_64_PageEntry_GetFlags(&entry) & i686_PageEntryFlag_Present)) {
        return NULL;
    }
    if (x86_64_PageEntry_GetFlags(&entry) & i686_PageEntryFlag_PAT) {
        return (void*)(x86_64_PageEntry_GetAddr(&entry) | (virtAddr & 0x3FFFFFFF));
    }
    x86_64_PageEntry (*pdt)[512] = (void*)x86_64_PageEntry_GetAddr(&entry);
    entry = (*pdt)[(virtAddr >> 21) & 0x1FF];
    if (!(x86_64_PageEntry_GetFlags(&entry) & i686_PageEntryFlag_Present)) {
        return NULL;
    }
    if (x86_64_PageEntry_GetFlags(&entry) & i686_PageEntryFlag_PAT) {
        return (void*)(x86_64_PageEntry_GetAddr(&entry) | (virtAddr & 0x1FFFFF));
    }
    x86_64_PageEntry (*pt)[512] = (void*)x86_64_PageEntry_GetAddr(&entry);
    entry = (*pt)[(virtAddr >> 12) & 0x1FF];
    if (!(x86_64_PageEntry_GetFlags(&entry) & i686_PageEntryFlag_Present)) {
        return NULL;
    }
    return (void*)(x86_64_PageEntry_GetAddr(&entry) | (virtAddr & 0xFFF));
}

static int boot_VirtualMap1G(uint64_t virtPageAddr, uint64_t phyPageAddr, int flags)
{
    x86_64_PageEntry pml4ptr = Pml4();
    x86_64_PageEntry* pageEntry;
    pageEntry = PageDirectory(&pml4ptr, (size_t)(virtPageAddr >> 39));
    if (pageEntry == NULL) { return 0; }
    pageEntry = PageEntry2(pageEntry, virtPageAddr >> 21);
    if (x86_64_PageEntry_GetFlags(pageEntry) & i686_PageEntryFlag_Present) {
        return 0;
    }
    *pageEntry = (x86_64_PageEntry)x86_64_MakePageEntry(phyPageAddr, TranslateFlags(flags) | i686_PageEntryFlag_PAT);
    return 1;
}

static int Is1GBPagesPagesSupported(void)
{
    i686_CPUIDLeaf r = i686_cpuid(0x80000000, 0);
    if (r.eax < 1) {
        return 0;
    }
    r = i686_cpuid(0x80000001, 0);
    return !!((r.edx >> 26) & 1);
}

static int boot_VirtualMap2M(uint64_t virtPageAddr, uint64_t phyPageAddr, int flags)
{
    x86_64_PageEntry pml4ptr = Pml4();
    x86_64_PageEntry* pageEntry;
    pageEntry = PageDirectory(&pml4ptr, (size_t)(virtPageAddr >> 39));
    if (pageEntry == NULL) { return 0; }
    pageEntry = PageDirectory(pageEntry, (size_t)(virtPageAddr >> 30));
    if (pageEntry == NULL) { return 0; }
    pageEntry = PageEntry2(pageEntry, virtPageAddr >> 21);
    if (x86_64_PageEntry_GetFlags(pageEntry) & i686_PageEntryFlag_Present) {
        return 0;
    }
    *pageEntry = (x86_64_PageEntry)x86_64_MakePageEntry(phyPageAddr, TranslateFlags(flags) | i686_PageEntryFlag_PAT);
    return 1;
}

static const boot_MemoryMapEntry* FindNextMemEntry(const boot_MemoryMapEntry* from)
{
    const boot_MemoryMapEntry* entries = (void*)(boot_memmap->entries);
    const boot_MemoryMapEntry* end = entries + (ptrdiff_t)boot_memmap->count + 1;
    if (from == NULL) {
        from = entries;
    } else {
        from += 1;
    }
    for (const boot_MemoryMapEntry* i = from; i != end; ++i) {
        switch (i->type) {
        case boot_MemoryMapEntryType_AvailableMemory:
        case boot_MemoryMapEntryType_BootReclaimable:
        case boot_MemoryMapEntryType_SystemReclaimable:
        case boot_MemoryMapEntryType_Kernel:
        case boot_MemoryMapEntryType_Inherited:
            return i;
        }
    }
    return NULL;
}

static void MakeIdentityMap(void)
{
    uint32_t pageFlags = boot_MemoryFlags_Execute
        | boot_MemoryFlags_Write;
    uint64_t maxAddr = 0;
    const boot_MemoryMapEntry* ent = NULL;
    while ((ent = FindNextMemEntry(ent))) {
        if (ent->end > maxAddr) {
            maxAddr = ent->end;
        }
    }
    if (!Is1GBPagesPagesSupported()) {
        for (uint64_t i = 0; i < maxAddr; i += 0x200000) {
            boot_VirtualMap2M(i, i, pageFlags);
        }
        return;
    }
    for (uint64_t i = 0; i < maxAddr; i += 0x40000000) {
        boot_VirtualMap1G(i, i, pageFlags);
    }
}

int boot_VirtualMap(uint64_t virtPageAddr, uint64_t phyPageAddr, int flags)
{
    x86_64_PageEntry* pageEntry = FindAllocPageDirectory(virtPageAddr);
    if (pageEntry == NULL) { return 0; }
    pageEntry = PageEntry2(pageEntry, virtPageAddr >> 12);
    if (x86_64_PageEntry_GetFlags(pageEntry) & i686_PageEntryFlag_Present) {
        return 0;
    }
    *pageEntry = (x86_64_PageEntry)x86_64_MakePageEntry(phyPageAddr, TranslateFlags(flags));
    return 1;
}

void boot_VirtualEnterASM(uint64_t entryPoint, const boot_LdrData* data);

static void CommitMemory(uint32_t newType)
{
    boot_MemoryMapEntry *entries = (void*)(uintptr_t)(boot_memmap->entries);
    boot_MemoryMapEntry *back = entries + boot_memmap->count;
    if (back->begin == back->end) {
        back->type = newType;
        return;
    }
    boot_MemoryMapEntry temp = *back;
    boot_MemoryMapEntry* current = entries + currentMemEntry;
    memmove(current + 1, current, sizeof(temp) * (boot_memmap->count - currentMemEntry));
    *current = temp;
    ++boot_memmap->count;
    if (newType == boot_MemoryMapEntryType_ReservedMemory) {
        return;
    }
    ++currentMemEntry;
    back[1].begin = temp.end;
    back[1].end = temp.end;
    back[1].type = newType;
}

void boot_VirtualEnter(uint64_t entryPoint)
{
    boot_LdrData *data = malloc(sizeof(boot_LdrData) * 2);
    data[0] = (boot_LdrData){ .type = boot_LdrDataType_EntriesCount, .value = 2 };
    data[1] = (boot_LdrData){ .type = boot_LdrDataType_MemoryMap,
        .value = (uintptr_t)(void*)boot_memmap };

    MakeIdentityMap();
    CommitMemory(boot_MemoryMapEntryType_ReservedMemory);

    boot_VirtualEnterASM(entryPoint, data);
    free(data);
}

void boot_CommitKernelMemory(void)
{
    CommitMemory(boot_MemoryMapEntryType_Inherited);
}

#include "alloc.h"
#include "boot/data.h"
#include "boot/virtual_alloc.h"
#include "processor.h"

static size_t currentMemEntry;
static uintptr_t freePagesStart;
x86_64_PageEntry (*boot_x86_64_pml4)[512];

static size_t findFirstAvailableEntry(const boot_MemoryMap* memmap)
{
    const boot_MemoryMapEntry *entries = (void*)(uintptr_t)(memmap->entries);
    for (size_t i = 0; i < memmap->count; ++i) {
        if (entries[i].type == boot_MemoryMapEntryType_AvailableMemory &&
            (entries[i].flags & 0xF) == 1)
        {
            return i;
        }
    }
    return memmap->count;
}

static void InitPagedAlloc()
{
    const boot_MemoryMap* memmap = (const boot_MemoryMap*)__bss_end;
    const boot_MemoryMapEntry *entries = (void*)(uintptr_t)(memmap->entries);
    currentMemEntry = findFirstAvailableEntry(memmap);
    freePagesStart = 0x100000;
    for (;currentMemEntry < memmap->count; ++currentMemEntry) {
        uint64_t regionStart = entries[currentMemEntry].begin;
        uint64_t regionEnd = regionStart + entries[currentMemEntry].size;
        regionStart = (regionStart + 0xFFF) & (~(uint64_t)0xFFF);
        regionEnd = regionEnd & (~(uint64_t)0xFFF);
        if (regionStart == regionEnd) {
            continue;
        }
        if (regionStart > UINTPTR_MAX) {
            abort();
        }
        if (regionStart >= freePagesStart) {
            freePagesStart = (uintptr_t)regionStart;
            break;
        } else if (regionEnd > freePagesStart) {
            break;
        }
    }
    if (currentMemEntry == memmap->count) {
        abort();
    }
}

void* boot_AllocPage()
{
    uintptr_t page = freePagesStart;
    if (page == 0) {
        return NULL;
    }
    freePagesStart += 0x1000;
    const boot_MemoryMap* memmap = (const boot_MemoryMap*)__bss_end;
    const boot_MemoryMapEntry* entries = (void*)(uintptr_t)(memmap->entries);
    uint64_t regionStart = entries[currentMemEntry].begin;
    uint64_t regionEnd = regionStart + entries[currentMemEntry].size;
    regionStart = (regionStart + 0xFFF) & (~(uint64_t)0xFFF);
    regionEnd = regionEnd & (~(uint64_t)0xFFF);

    if (freePagesStart == (uintptr_t)regionEnd) {
        ++currentMemEntry;
        for (; currentMemEntry != memmap->count; ++currentMemEntry) {
            regionStart = entries[currentMemEntry].begin;
            regionEnd = regionStart + entries[currentMemEntry].size;
            regionStart = (regionStart + 0xFFF) & (~(uint64_t)0xFFF);
            regionEnd = regionEnd & (~(uint64_t)0xFFF);
            if (regionStart == regionEnd) {
                continue;
            }
            if (regionStart > UINTPTR_MAX) {
                break;
            }
            freePagesStart = (uintptr_t)regionStart;
            return (void*)page;
        }
        freePagesStart = 0;
    }

    return (void*)page;
}

static void MapFirstMeg()
{
    uint32_t pageFlags = i686_PageEntryFlag_Present
        | i686_PageEntryFlag_Write | i686_PageEntryFlag_User;
    boot_x86_64_pml4 = boot_AllocPage();
    memset(boot_x86_64_pml4, 0, sizeof(*boot_x86_64_pml4));
    x86_64_PageEntry (*dirPtr)[512] = boot_AllocPage();
    memset(dirPtr, 0, sizeof(*dirPtr));
    (*boot_x86_64_pml4)[0] = (x86_64_PageEntry)
        x86_64_MakePageEntry((uintptr_t)dirPtr, pageFlags);
    x86_64_PageEntry (*dir)[512] = boot_AllocPage();
    memset(dir, 0, sizeof(*dir));
    (*dirPtr)[0] = (x86_64_PageEntry)x86_64_MakePageEntry((uintptr_t)dir,
        pageFlags);
    x86_64_PageEntry (*table)[512] = boot_AllocPage();
    memset(table, 0, sizeof(*table));
    (*dir)[0] = (x86_64_PageEntry)x86_64_MakePageEntry((uintptr_t)table,
        pageFlags);
    for (uint32_t i = 0; i < 0xA0; i += 1) {
        x86_64_PageEntry entry = x86_64_MakePageEntry((uintptr_t)(i * 0x1000),
            pageFlags);
        (*table)[i] = entry;
    }
    pageFlags |= i686_PageEntryFlag_OnlyReadCache;
    for (uint32_t i = 0xA0; i < 0x100; i += 1) {
        (*table)[i] = (x86_64_PageEntry)
            x86_64_MakePageEntry((uintptr_t)(i * 0x1000), pageFlags);
    }
}

void boot_InitVirtualAlloc() {
    InitPagedAlloc();
    MapFirstMeg();
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
    result |= i686_PageEntryFlag_Write * !!(flags & boot_MemoryFlags_Write);
    result |= i686_PageEntryFlag_User * !(flags & boot_MemoryFlags_Kernel);
    result |= i686_PageEntryFlag_OnlyReadCache
        * !!(flags & boot_MemoryFlags_DeviceWrite);
    result |= i686_PageEntryFlag_NoCache * !!(flags & boot_MemoryFlags_Device);
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

static x86_64_PageEntry* FindAllocPageDirectory(uint64_t virtPageAddr)
{
    x86_64_PageEntry pml4ptr =
        x86_64_MakePageEntry((uintptr_t)*boot_x86_64_pml4, 0);
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

static int CreateMappingWindow()
{
    const uint64_t mappingWindowPage = (uint64_t)-0x1000;
    x86_64_PageEntry* pageEntry = FindAllocPageDirectory(mappingWindowPage);
    if (pageEntry == NULL) { return 0; }
    x86_64_PageEntry (*pageTableAddr)[512] =
        (void*)(uintptr_t)x86_64_PageEntry_GetAddr(pageEntry);
    unsigned pageFlags = i686_PageEntryFlag_Present | i686_PageEntryFlag_Write;
    (*pageTableAddr)[511] = (x86_64_PageEntry)
        x86_64_MakePageEntry((uintptr_t)pageTableAddr, pageFlags);
    return 1;
}

void boot_VirtualEnterASM(uint64_t entryPoint, const boot_LdrData* data);

void boot_VirtualEnter(uint64_t entryPoint)
{
    if (CreateMappingWindow() == 0) abort();
    boot_LdrData *data = malloc(sizeof(boot_LdrData) * 2);
    boot_MemoryMap *memoryMap = (boot_MemoryMap*)__bss_end;
    memoryMap->allocatedBoundary = freePagesStart;
    data[0] = (boot_LdrData){ .type = boot_LdrDataType_EntriesCount, .value = 2 };
    data[1] = (boot_LdrData){ .type = boot_LdrDataType_MemoryMap,
        .value = (uintptr_t)(void*)memoryMap };
    boot_VirtualEnterASM(entryPoint, data);
    free(data);
}

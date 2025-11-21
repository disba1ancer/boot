#include "alloc.h"
#include "boot/data.h"
#include "boot/virtual_alloc.h"
#include "processor.h"

static ptrdiff_t currentMemEntry;
x86_64_PageEntry (*boot_x86_64_pml4)[512];
static boot_MemoryMap* memmap;

static ptrdiff_t FindNextAvailableEntry(const boot_MemoryMap* memmap, ptrdiff_t from)
{
    const boot_MemoryMapEntry *entries = (void*)(uintptr_t)(memmap->entries);
    for (ptrdiff_t i = from; i < (ptrdiff_t)memmap->count; ++i) {
        if (entries[i].type == boot_MemoryMapEntryType_AvailableMemory)
        {
            return i;
        }
    }
    return memmap->count;
}

static void InitPagedAlloc(void)
{
    boot_MemoryMapEntry *entries = (void*)(uintptr_t)(memmap->entries);
    currentMemEntry = FindNextAvailableEntry(memmap, 0);
    if (currentMemEntry == (ptrdiff_t)memmap->count) {
        abort();
    }
    uint64_t regionStart = entries[currentMemEntry].begin;
    if (regionStart > UINTPTR_MAX) {
        abort();
    }
    entries[memmap->count].begin = regionStart;
    entries[memmap->count].end = regionStart;
    entries[memmap->count].type = boot_MemoryMapEntryType_ReservedMemory;
}

void* boot_AllocPage(void)
{
    boot_MemoryMapEntry *entries = (void*)(uintptr_t)(memmap->entries);
    uintptr_t page = entries[memmap->count].end;
    if (page == 0) {
        return NULL;
    }
    entries[memmap->count].end += boot_Alloc_PageSize;
    uint64_t regionStart = (entries[currentMemEntry].begin += boot_Alloc_PageSize);
    uint64_t regionEnd = entries[currentMemEntry].end;

    if (regionStart > UINTPTR_MAX) {
        entries[memmap->count] = (boot_MemoryMapEntry){0};
        return (void*)page;
    }
    if (regionStart != regionEnd) {
        return (void*)page;
    }
    entries[currentMemEntry] = entries[memmap->count];
    ++currentMemEntry;
    currentMemEntry = FindNextAvailableEntry(memmap, currentMemEntry);
    regionStart = entries[currentMemEntry].begin;
    if (currentMemEntry == (ptrdiff_t)memmap->count || regionStart > UINTPTR_MAX) {
        entries[memmap->count] = (boot_MemoryMapEntry){0};
        return (void*)page;
    }
    entries[memmap->count].begin = regionStart;
    entries[memmap->count].end = regionStart;
    entries[memmap->count].type = boot_MemoryMapEntryType_ReservedMemory;

    return (void*)page;
}

static void MapFirstMeg(void)
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
        x86_64_PageEntry entry = x86_64_MakePageEntry((uintptr_t)(i * boot_Alloc_PageSize),
            pageFlags);
        (*table)[i] = entry;
    }
    pageFlags |= i686_PageEntryFlag_PWT | i686_PageEntryFlag_PCD;
    for (uint32_t i = 0xA0; i < 0x100; i += 1) {
        (*table)[i] = (x86_64_PageEntry)
            x86_64_MakePageEntry((uintptr_t)(i * boot_Alloc_PageSize), pageFlags);
    }
}

void boot_InitVirtualAlloc(boot_MemoryMap* memmap_) {
    memmap = memmap_;
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

static int CreateMappingWindow(void)
{
    unsigned pageFlags = i686_PageEntryFlag_Present | i686_PageEntryFlag_Write;
    (*boot_x86_64_pml4)[256] = (x86_64_PageEntry)
        x86_64_MakePageEntry((uintptr_t)boot_x86_64_pml4, pageFlags);
    return 1;
}

void boot_VirtualEnterASM(uint64_t entryPoint, const boot_LdrData* data);

void boot_VirtualEnter(uint64_t entryPoint)
{
    if (CreateMappingWindow() == 0) abort();
    boot_LdrData *data = malloc(sizeof(boot_LdrData) * 2);

    boot_MemoryMapEntry *entries = (void*)(uintptr_t)(memmap->entries);
    boot_MemoryMapEntry *back = entries + memmap->count;
    if (back->begin != back->end) {
        boot_MemoryMapEntry temp = *back;
        boot_MemoryMapEntry* current = entries + currentMemEntry;
        memmove(current + 1, current, sizeof(temp) * (memmap->count - currentMemEntry));
        *current = temp;
        ++memmap->count;
    }
    data[0] = (boot_LdrData){ .type = boot_LdrDataType_EntriesCount, .value = 2 };
    data[1] = (boot_LdrData){ .type = boot_LdrDataType_MemoryMap,
        .value = (uintptr_t)(void*)memmap };
    boot_VirtualEnterASM(entryPoint, data);
    free(data);
}

#include "platform/i686/processor.h"
#include "platform/i686/bios_video.h"
#include "platform/i686/membios.h"
#include "platform/i686/init.h"
#include "platform/i686/bios_disk.h"
#include "boot/gpt.h"
#include <ranges>
#include <span>
#include <stddef.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <memory>
#include "boot/Conout.hpp"
#include "platform/i686/bios_kbrd.h"
#include "platform/i686/PartitionDevice.h"
#include "ext2/Driver.h"
#include "ext2/File.h"
#include "ext2/Directory.h"
#include "boot/util.h"
#include "boot/virtual_alloc.h"
#include "elf/elf.h"

char str[] = "Loading /bin/kernel\n";

static void LoadAndStartKernel(boot_StartupInfo *si);

extern "C" void boot_main(boot_StartupInfo *si)
{
    using namespace i686::bios;
    auto &out = boot::Conout::instance;
    out.PutStr(str);
    LoadAndStartKernel(si);
    while (true) {
        auto key = kbrd::GetKey();
        switch (key.ascii) {
            case 4:
            case 3:
                goto whileEnd;
        }
        out.PutC(key.ascii);
    } whileEnd:;
}

static const boot_MemoryMapEntry* FindNextKernelEntry(const boot_MemoryMapEntry* from)
{
    auto entries = reinterpret_cast<const boot_MemoryMapEntry*>(boot_memmap->entries);
    auto end = entries + (ptrdiff_t)boot_memmap->count;
    if (from == nullptr) {
        from = entries;
    } else {
        from += 1;
    }
    for (auto i = from; i != end; ++i) {
        if (i->type == boot_MemoryMapEntryType_Kernel)
        {
            return i;
        }
    }
    return nullptr;
}

void LoadAndStartKernel(boot_StartupInfo *si)
{
    auto &out = boot::Conout::instance;
    i686::PartitionDevice part(si->diskNum, &si->part);
    boot::ext2::Driver ext2drv(&part);
    auto kernelFile = ext2drv.OpenByPath("/bin/kernel");
    size_t readSize;
    elf_ident ident;
    kernelFile.Read(&ident, sizeof(ident), &readSize, 0);
    constexpr byte elf_magic[] = { 0x7F, 'E', 'L', 'F' };
    if (
        memcmp(ident.magic, elf_magic, 4) != 0 ||
        ident.fileClass != elf_Class_C64 ||
        ident.data != elf_DataEnc_LSB
    ) {
        out.PutStr("Not an ELF or unsupported\n");
        std::terminate();
    }
    static constexpr auto e = boot::Endian_Little;
    elf64_Header header;
    kernelFile.Read(&header, sizeof(header), &readSize, sizeof(ident));
    header.type.r = boot::ELoad(header.type, e);
    if (
        (header.type.r != elf_Type_Executable &&
        header.type.r != elf_Type_Dynamic) ||
        boot::ELoad(header.machine,e) != elf_Machine_x86_64
    ) {
        out.PutStr("Not executable or unsupported platform\n");
        std::terminate();
    }
    static constexpr auto PageSize = 4096;
    header.progHeadEntryCount.r = boot::ELoad(header.progHeadEntryCount, e);
    header.progHeadEntrySize.r = boot::ELoad(header.progHeadEntrySize, e);
    auto programHeadersOff = boot::ELoad(header.programHeaderOff, e);
    std::unique_ptr<elf64_ProgramHeader[]> progHeaderArrayPtr(new elf64_ProgramHeader[header.progHeadEntryCount.r]{});
    std::span<elf64_ProgramHeader> progHeaderArray(progHeaderArrayPtr.get(), header.progHeadEntryCount.r);
    uint64_t minAddr = -1;
    uint64_t maxAddr = 0;
    uint64_t maxAlign = 0;
    for (auto&& programHeader : progHeaderArray) {
        kernelFile.Read(&programHeader, std::min<size_t>(sizeof(programHeader), header.progHeadEntrySize.r),
                        &readSize, programHeadersOff);
        programHeadersOff += header.progHeadEntrySize.r;
        programHeader.type.r = boot::ELoad(programHeader.type, e);
        if (programHeader.type.r != elf64_SegType_Load)
        {
            continue;
        }
        programHeader.align.r = boot::ELoad(programHeader.align, e);
        if (programHeader.align.r < PageSize) {
            out.PutStr("ELF segment alignment is less than page size\n");
            std::terminate();
        }
        programHeader.virtualAddress.r = boot::ELoad(programHeader.virtualAddress, e);
        programHeader.loadSize.r = boot::ELoad(programHeader.loadSize, e);
        uint64_t segmentFileStart = boot::ELoad(programHeader.offset, e);
        uint64_t segmentFileEnd = boot::ELoad(programHeader.size, e) + segmentFileStart;
        uint64_t segmentLoadEnd = programHeader.loadSize.r;
        minAddr = std::min(minAddr, programHeader.virtualAddress.r);
        maxAddr = std::max(maxAddr, programHeader.virtualAddress.r + segmentLoadEnd);
        maxAlign = std::max(maxAlign, boot::ELoad(programHeader.align, e));
        segmentLoadEnd += segmentFileStart;
        segmentFileStart ^= segmentFileStart & (PageSize - 1);
        for (; segmentFileStart < segmentFileEnd; segmentFileStart += PageSize) {
            auto page = static_cast<byte*>(boot_AllocPage());
            kernelFile.Read(page, PageSize, &readSize, segmentFileStart);
            auto leftoverSize = segmentFileEnd - segmentFileStart;
            if (leftoverSize < PageSize) {
                memset(page + leftoverSize, 0, PageSize - leftoverSize);
            }
        }
        for (; segmentFileStart < segmentLoadEnd; segmentFileStart += PageSize) {
            auto page = static_cast<byte*>(boot_AllocPage());
            memset(page + readSize, 0, PageSize);
        }
    }
    minAddr = minAddr & ~maxAlign;
    maxAddr = (maxAddr + maxAlign - 1) & ~maxAlign;
    uint64_t loadOffset = 0;
    if (header.type.r == elf_Type_Dynamic) {
        loadOffset = -(uint64_t)0x80000000 - minAddr;
    }
    boot_CommitKernelMemory();
    auto entry = FindNextKernelEntry(nullptr);
    auto curAddr = entry->begin;
    uint64_t dynamicStart = 0;
    uint64_t dynamicEnd = 0;
    for (auto&& programHeader : progHeaderArray) {
        if (programHeader.type.r ==
            elf64_SegType_Dynamic)
        {
            dynamicStart = boot::ELoad(programHeader.virtualAddress, e) + loadOffset;
            dynamicEnd = dynamicStart + boot::ELoad(programHeader.loadSize, e);
        }
        if (programHeader.type.r !=
            elf64_SegType_Load)
        {
            continue;
        }
        uint64_t segmentLoadStart = programHeader.virtualAddress.r + loadOffset;
        uint64_t segmentLoadEnd = programHeader.loadSize.r + segmentLoadStart;
        segmentLoadStart ^= segmentLoadStart & (PageSize - 1);
        for (; segmentLoadStart < segmentLoadEnd; segmentLoadStart += PageSize) {
            boot_VirtualMap(segmentLoadStart, curAddr, (boot::ELoad(programHeader.flags, e) & 7) | boot_MemoryFlags_Kernel);
            curAddr += PageSize;
            if (curAddr != entry->end) {
                continue;
            }
            entry = FindNextKernelEntry(entry);
            curAddr = entry->begin;
        }
    }
    if (header.type.r == elf_Type_Executable) {
        boot_VirtualEnter(boot::ELoad(header.entry, e));
    }
    uint64_t relAStart = 0;
    uint64_t relAEnd = 0;
    uint64_t relAEntSize = 0;
    for (;dynamicStart < dynamicEnd; dynamicStart += sizeof(elf64_Dynamic)) {
        elf64_Dynamic dynEntry;
        void* ptr = boot_VirtualToPtr(dynamicStart + offsetof(decltype(dynEntry), tag));
        dynEntry.tag = *static_cast<const boot_RE64S*>(ptr);
        ptr = boot_VirtualToPtr(dynamicStart + offsetof(decltype(dynEntry), val));
        dynEntry.val = *static_cast<const boot_RE64U*>(ptr);
        switch (boot::ELoad(dynEntry.tag, e)) {
        case elf64_DynamicTags_RelA:
            relAStart = boot::ELoad(dynEntry.val, e) + loadOffset;
        case elf64_DynamicTags_RelASize:
            relAEnd = boot::ELoad(dynEntry.val, e);
        case elf64_DynamicTags_RelAEntrySize:
            relAEntSize = boot::ELoad(dynEntry.val, e);
        }
    }
    relAEnd += relAStart;
    for (;relAStart < relAEnd; relAStart += relAEntSize) {
        auto relAOffPtr = static_cast<boot_RE64U*>(boot_VirtualToPtr(relAStart));
        auto relAInfoPtr = static_cast<boot_RE64U*>(boot_VirtualToPtr(relAStart + 8));
        auto relAAddendPtr = static_cast<boot_RE64U*>(boot_VirtualToPtr(relAStart + 16));
        if ((uint32_t)boot::ELoad(*relAInfoPtr, e) != 8) {
            continue;
        }
        relAOffPtr->r = boot::ELoad(*relAOffPtr, e);
        relAAddendPtr->r = boot::ELoad(*relAAddendPtr, e) + loadOffset;
        auto reloc = boot_VirtualToPtr(relAOffPtr->r + loadOffset);
        auto pageEndDistance = PageSize - ((size_t)relAOffPtr->r & (PageSize - 1));
        memcpy(reloc, relAAddendPtr, std::min(pageEndDistance, sizeof(uint64_t)));
        if (pageEndDistance < 8) {
            memcpy(boot_VirtualToPtr(relAOffPtr->r + loadOffset + pageEndDistance), relAAddendPtr, sizeof(uint64_t) - pageEndDistance);
        }
    }
    boot_VirtualEnter(boot::ELoad(header.entry, e) + loadOffset);
}

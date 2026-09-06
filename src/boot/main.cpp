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
    auto progHeaderArraySize = boot::ELoad(header.progHeadEntryCount, e);
    auto progHeaderArrayReadSize = boot::ELoad(header.progHeadEntrySize, e) * progHeaderArraySize;
    std::unique_ptr<elf64_ProgramHeader[]> progHeaderArrayPtr(new elf64_ProgramHeader[progHeaderArraySize]);
    std::span<elf64_ProgramHeader> progHeaderArray(progHeaderArrayPtr.get(), progHeaderArraySize);
    kernelFile.Read(progHeaderArrayPtr.get(), progHeaderArrayReadSize,
        &readSize, boot::ELoad(header.programHeaderOff, e));
    for (auto&& programHeader : progHeaderArray) {
        programHeader.type.r = boot::ELoad(programHeader.type, e);
        if (programHeader.type.r != elf64_SegType_Load)
        {
            continue;
        }
        if (boot::ELoad(programHeader.align, e) < PageSize) {
            out.PutStr("ELF segment alignment is less than page size\n");
            std::terminate();
        }
        uint64_t segmentFileStart = boot::ELoad(programHeader.offset, e);
        uint64_t segmentFileEnd = boot::ELoad(programHeader.size, e) + segmentFileStart;
        uint64_t segmentLoadEnd = boot::ELoad(programHeader.loadSize, e) + segmentFileStart;
        segmentFileStart ^= segmentFileStart & (PageSize - 1);
        for (; segmentFileStart < segmentFileEnd; segmentFileStart += PageSize) {
            auto page = static_cast<byte*>(boot_AllocPage());
            kernelFile.Read(page, PageSize, &readSize, segmentFileStart);
            memset(page + readSize, 0, PageSize - readSize);
        }
        for (; segmentFileStart < segmentLoadEnd; segmentFileStart += PageSize) {
            auto page = static_cast<byte*>(boot_AllocPage());
            memset(page + readSize, 0, PageSize);
        }
    }
    boot_CommitKernelMemory();
    auto entry = FindNextKernelEntry(nullptr);
    auto curAddr = entry->begin;
    for (auto&& programHeader : progHeaderArray) {
        if (boot::ELoad(programHeader.type, e) !=
            elf64_SegType_Load)
        {
            continue;
        }
        uint64_t segmentLoadStart = boot::ELoad(programHeader.virtualAddress, e);
        uint64_t segmentLoadEnd = boot::ELoad(programHeader.loadSize, e) + segmentLoadStart;
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
    boot_VirtualEnter(boot::ELoad(header.entry, e));
}
